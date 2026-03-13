#include "audioengine.h"
#include "track.h"
#include "vibrato.h"
#include "note.h"
#ifdef _WIN32
#include "audiodevicenotifier.h"
#endif
#include <iostream>
#include <cmath>
#include <random>

AudioEngine::AudioEngine()
    : QObject(nullptr)
    , gateOpen(false)
    , amplitude(0.0)
    , currentPitch(261.63)
    , noteDuration(1000.0)
    , noteStartSample(0)
    , currentSample(0)
    , segmentDurationMs(1000.0)  // 1 second segments by default
    , useRenderBuffer(false)
    , renderPlaybackPosition(0)
    , renderPlaybackSegmentIndex(0)
    , renderCacheDirty(true)
    , sampleRate(48000)
    , initialized(false)
    , useTrackPlayback(false)
    , trackPlaybackTimeMs(0.0)
    , trackPlaybackEndMs(0.0)
    , m_deviceSwitchInProgress(false)
#ifdef _WIN32
    , m_deviceNotifier(nullptr)
#endif
{
    // trackGraphs initialized as empty QMap
    // renderSegments initialized as empty vector
    // playbackTracks initialized as empty QList
}

AudioEngine::~AudioEngine()
{
    stopDeviceNotifier();
    shutdown();
    clearAllGraphs();
}

bool AudioEngine::initialize(unsigned int sampleRate, unsigned int bufferFrames)
{
    if (initialized) {
        std::cerr << "AudioEngine already initialized" << std::endl;
        return false;
    }

    this->sampleRate = sampleRate;
    generator.setSampleRate(static_cast<double>(sampleRate));

    // Try to open the default audio device
    if (audioDevice.getDeviceCount() < 1) {
        std::cerr << "No audio devices found!" << std::endl;
        return false;
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = audioDevice.getDefaultOutputDevice();
    parameters.nChannels = 2;  // Stereo (most devices expect this)
    parameters.firstChannel = 0;

    RtAudioFormat format = RTAUDIO_FLOAT32;

    try {
        audioDevice.openStream(&parameters, nullptr, format, sampleRate,
                              &bufferFrames, &audioCallback, this);
        audioDevice.startStream();
        initialized = true;
        std::cout << "AudioEngine initialized: " << sampleRate << " Hz, "
                  << bufferFrames << " frames" << std::endl;
        return true;
    }
    catch (RtAudioErrorType &error) {
        std::cerr << "RTAudio error: " << audioDevice.getErrorText() << std::endl;
        return false;
    }
}

void AudioEngine::shutdown()
{
    if (!initialized) return;

    try {
        if (audioDevice.isStreamRunning())
            audioDevice.stopStream();
        if (audioDevice.isStreamOpen())
            audioDevice.closeStream();
    }
    catch (RtAudioErrorType &error) {
        std::cerr << "Error closing stream: " << audioDevice.getErrorText() << std::endl;
    }

    initialized = false;
    std::cout << "AudioEngine shut down" << std::endl;
}

bool AudioEngine::isRunning() const
{
    return initialized && audioDevice.isStreamRunning();
}

void AudioEngine::playNote(const Note& note)
{
    // Store current pitch and note duration
    currentPitch.store(note.getPitchHz());
    noteDuration.store(note.getDuration());

    // Record the start sample for timing
    noteStartSample.store(currentSample.load());

    // Check if note's track has a graph
    int trackIndex = note.getTrackIndex();
    bool hasValidGraph = hasGraph(trackIndex);

    if (hasValidGraph) {
        // Reset graph for new note
        std::lock_guard<std::mutex> lock(graphMutex);
        if (trackGraphs.contains(trackIndex)) {
            trackGraphs[trackIndex]->reset();
        }
    } else {
        // Fallback to direct generator
        generator.setFundamentalHz(note.getPitchHz());
        generator.reset();
    }

    // Open gate
    gateOpen = true;

    std::cout << "Playing note: " << note.getPitchHz() << " Hz, duration: "
              << note.getDuration() << " ms (track " << trackIndex << ")"
              << (hasValidGraph ? " (using graph)" : " (direct)") << std::endl;
}

void AudioEngine::stopNote()
{
    gateOpen = false;
    amplitude = 0.0;  // Immediate cutoff for stop button
    useRenderBuffer.store(false);  // Stop segment-based buffer playback
    useTrackPlayback.store(false); // Stop track-based playback
}

bool AudioEngine::buildGraph(Canvas *canvas, int trackIndex)
{
    if (!canvas) {
        clearGraph(trackIndex);
        return false;
    }

    // Lock mutex to prevent audio thread from accessing graph during rebuild
    std::lock_guard<std::mutex> lock(graphMutex);

    // Delete old graph for this track if it exists
    if (trackGraphs.contains(trackIndex)) {
        delete trackGraphs[trackIndex];
        trackGraphs.remove(trackIndex);
    }

    // Create and build graph
    SounitGraph *newGraph = new SounitGraph(static_cast<double>(sampleRate));
    newGraph->buildFromCanvas(canvas);

    // Check if valid
    bool isValid = newGraph->isValid();
    if (isValid) {
        trackGraphs[trackIndex] = newGraph;
        std::cout << "AudioEngine: Graph built successfully for track " << trackIndex
                  << " - using graph mode" << std::endl;
    } else {
        std::cout << "AudioEngine: Graph invalid for track " << trackIndex
                  << " - falling back to direct mode" << std::endl;
        delete newGraph;
    }

    // Invalidate render cache - graph structure changed
    renderCacheDirty.store(true);
    std::cout << "AudioEngine: Render cache invalidated (graph changed)" << std::endl;

    return isValid;
}

void AudioEngine::clearGraph(int trackIndex)
{
    // Lock mutex to prevent audio thread from accessing graph during clear
    std::lock_guard<std::mutex> lock(graphMutex);

    if (trackGraphs.contains(trackIndex)) {
        delete trackGraphs[trackIndex];
        trackGraphs.remove(trackIndex);
        renderCacheDirty.store(true);
        std::cout << "AudioEngine: Graph cleared for track " << trackIndex
                  << " - using direct mode" << std::endl;
    }
}

void AudioEngine::clearAllGraphs()
{
    // Lock mutex to prevent audio thread from accessing graphs during clear
    std::lock_guard<std::mutex> lock(graphMutex);

    for (SounitGraph *graph : trackGraphs) {
        delete graph;
    }
    trackGraphs.clear();
    renderCacheDirty.store(true);
    std::cout << "AudioEngine: All graphs cleared - using direct mode" << std::endl;
}

bool AudioEngine::hasGraph(int trackIndex) const
{
    return trackGraphs.contains(trackIndex) && trackGraphs[trackIndex] != nullptr
           && trackGraphs[trackIndex]->isValid();
}

int AudioEngine::audioCallback(void *outputBuffer, void *inputBuffer,
                               unsigned int nFrames, double streamTime,
                               RtAudioStreamStatus status, void *userData)
{
    AudioEngine *engine = static_cast<AudioEngine*>(userData);
    float *buffer = static_cast<float*>(outputBuffer);

    // Check if we're in track-based playback mode (NEW - note-based rendering)
    if (engine->useTrackPlayback.load()) {
        // Lock track playback mutex
        std::lock_guard<std::mutex> lock(engine->trackPlaybackMutex);

        // Calculate duration of this buffer in milliseconds
        double bufferDurationMs = (static_cast<double>(nFrames) / engine->sampleRate) * 1000.0;

        // Get current playback time
        double currentTimeMs = engine->trackPlaybackTimeMs.load();
        double endTimeMs = engine->trackPlaybackEndMs.load();

        // Check if we've reached the end
        if (currentTimeMs >= endTimeMs) {
            // Playback complete - fill with silence and stop
            for (unsigned int i = 0; i < nFrames * 2; i++) {
                buffer[i] = 0.0f;
            }
            engine->useTrackPlayback.store(false);
            return 0;
        }

        // Mix audio from all tracks for this buffer
        // First, clear the buffer
        for (unsigned int i = 0; i < nFrames * 2; i++) {
            buffer[i] = 0.0f;
        }

        // Mix each track
        for (Track *track : engine->playbackTracks) {
            if (!track || track->isMuted()) continue;

            // Get mixed buffer from track for this time range
            std::vector<float> trackBuffer = track->getMixedBuffer(currentTimeMs, bufferDurationMs);

            // Calculate pan gains (linear pan law)
            // pan: -1.0 = full left, 0.0 = center, 1.0 = full right
            float pan = track->getPan();
            float leftGain = std::min(1.0f, 1.0f - pan);   // 1.0 at center/left, fades at right
            float rightGain = std::min(1.0f, 1.0f + pan);  // 1.0 at center/right, fades at left

            // Mix into output buffer with pan
            size_t samplesToMix = std::min(static_cast<size_t>(nFrames), trackBuffer.size());
            for (size_t i = 0; i < samplesToMix; i++) {
                float sample = trackBuffer[i];
                buffer[i * 2] += sample * leftGain;      // Left channel
                buffer[i * 2 + 1] += sample * rightGain;  // Right channel
            }
        }

        // Clamp final output to prevent clipping
        for (unsigned int i = 0; i < nFrames * 2; i++) {
            buffer[i] = std::clamp(buffer[i], -1.0f, 1.0f);
        }

        // Advance playback time
        engine->trackPlaybackTimeMs.store(currentTimeMs + bufferDurationMs);

    } else if (engine->useRenderBuffer.load()) {
        // LEGACY: Segment-based buffer playback mode
        // Lock render buffer mutex
        std::lock_guard<std::mutex> lock(engine->renderBufferMutex);

        for (unsigned int i = 0; i < nFrames; i++) {
            float sample = 0.0f;

            // Get current segment and position
            size_t segIdx = engine->renderPlaybackSegmentIndex.load();
            size_t globalPos = engine->renderPlaybackPosition.load();

            // Check if we have more segments to play
            if (segIdx < engine->renderSegments.size()) {
                const RenderSegment &currentSegment = engine->renderSegments[segIdx];

                // Calculate cumulative sample count up to current segment
                size_t segmentStartPos = 0;
                for (size_t s = 0; s < segIdx; s++) {
                    segmentStartPos += engine->renderSegments[s].samples.size();
                }

                // Calculate position within current segment
                size_t posInSegment = globalPos - segmentStartPos;

                // Read sample from current segment
                if (posInSegment < currentSegment.samples.size()) {
                    sample = currentSegment.samples[posInSegment];
                    engine->renderPlaybackPosition.fetch_add(1);
                } else {
                    // Reached end of current segment, advance to next
                    engine->renderPlaybackSegmentIndex.fetch_add(1);
                    engine->renderPlaybackPosition.fetch_add(1);

                    // Check if there's a next segment
                    if (engine->renderPlaybackSegmentIndex.load() < engine->renderSegments.size()) {
                        // Next segment exists, read from it
                        const RenderSegment &nextSegment = engine->renderSegments[engine->renderPlaybackSegmentIndex.load()];
                        if (!nextSegment.samples.empty()) {
                            sample = nextSegment.samples[0];
                        }
                    } else {
                        // No more segments, stop playback
                        engine->useRenderBuffer.store(false);
                    }
                }
            } else {
                // Reached end of all segments, stop playback
                engine->useRenderBuffer.store(false);
            }

            // Output to both stereo channels
            buffer[i * 2] = sample;      // Left channel
            buffer[i * 2 + 1] = sample;  // Right channel
        }
    } else {
        // Live synthesis mode
        // Simple envelope: fast attack, slow release
        const double attackRate = 0.01;   // Rise quickly
        const double releaseRate = 0.001; // Fall slowly

        // Lock mutex ONCE per buffer, not per sample
        std::lock_guard<std::mutex> lock(engine->graphMutex);

        for (unsigned int i = 0; i < nFrames; i++)
        {
            // Calculate note progress (0.0 to 1.0 over note duration)
            uint64_t currentSmp = engine->currentSample.load();
            uint64_t startSmp = engine->noteStartSample.load();
            double duration = engine->noteDuration.load();

            // Convert duration from milliseconds to samples
            double durationSamples = (duration / 1000.0) * engine->sampleRate;

            // Calculate progress
            double noteProgress = 0.5;  // Default fallback
            if (durationSamples > 0.0) {
                double elapsedSamples = static_cast<double>(currentSmp - startSmp);
                noteProgress = elapsedSamples / durationSamples;
                // Clamp to [0.0, 1.0]
                noteProgress = std::clamp(noteProgress, 0.0, 1.0);
            }

            // Update amplitude envelope
            if (engine->gateOpen.load()) {
                double currentAmp = engine->amplitude.load();
                currentAmp += (1.0 - currentAmp) * attackRate;
                engine->amplitude.store(currentAmp);
            } else {
                double currentAmp = engine->amplitude.load();
                currentAmp *= (1.0 - releaseRate);
                if (currentAmp < 0.0001) currentAmp = 0.0;
                engine->amplitude.store(currentAmp);
            }

            // Generate sample
            double sample;
            // Note: For live synthesis, we would need to track which track is active
            // For now, fallback to direct generator for live playback
            // Pre-rendered playback (the main use case) is handled in renderNotes()
            sample = engine->generator.generateSample();

            // Apply envelope
            sample *= engine->amplitude.load();

            // Output to both stereo channels (clamp to prevent clipping)
            float outputSample = static_cast<float>(std::clamp(sample * 0.3, -1.0, 1.0));
            buffer[i * 2] = outputSample;      // Left channel
            buffer[i * 2 + 1] = outputSample;  // Right channel

            // Increment sample counter
            engine->currentSample.fetch_add(1);
        }
    }

    return 0;
}

void AudioEngine::renderNotes(const QVector<Note>& notes, int maxNotes)
{
    std::cout << "AudioEngine::renderNotes called with " << notes.size() << " notes" << std::endl;

    if (notes.isEmpty()) {
        std::cout << "AudioEngine: No notes to render" << std::endl;
        return;
    }

    // Limit to maxNotes (-1 means render all notes)
    int notesToRender = (maxNotes < 0) ? notes.size() : qMin(notes.size(), maxNotes);
    std::cout << "AudioEngine: Will render " << notesToRender << " notes" << std::endl;

    // FAST PATH: If cache is valid, note count unchanged, IDs match, AND duration unchanged, skip expensive change detection
    // NOTE: This optimization catches the common case (playing multiple times without edits)
    if (!renderCacheDirty.load() &&
        !renderSegments.empty() &&
        cachedNotes.size() == notesToRender) {

        // Calculate total duration of current notes
        double currentTotalDuration = 0.0;
        for (int i = 0; i < notesToRender; i++) {
            double noteEnd = notes[i].getStartTime() + notes[i].getDuration();
            if (noteEnd > currentTotalDuration) {
                currentTotalDuration = noteEnd;
            }
        }

        // Calculate total duration of cached notes
        double cachedTotalDuration = 0.0;
        for (const Note &cachedNote : cachedNotes) {
            double noteEnd = cachedNote.getStartTime() + cachedNote.getDuration();
            if (noteEnd > cachedTotalDuration) {
                cachedTotalDuration = noteEnd;
            }
        }

        // Quick ID check: if all note IDs match in same order AND duration unchanged, assume unchanged
        bool idsMatch = true;
        for (int i = 0; i < notesToRender; ++i) {
            if (notes[i].getId() != cachedNotes[i].getId()) {
                idsMatch = false;
                break;
            }
        }

        // Only use fast path if IDs match AND total duration unchanged
        if (idsMatch && std::abs(currentTotalDuration - cachedTotalDuration) < 0.1) {
            std::cout << "AudioEngine: Fast-path cache hit (IDs match, duration unchanged, no graph changes) - skipping render" << std::endl;
            // Emit quick progress signals so UI knows we're using cache
            emit renderStarted();
            emit renderProgressChanged(100);
            emit renderCompleted();
            return;
        } else if (!idsMatch) {
            std::cout << "AudioEngine: Note IDs don't match - running detailed change detection" << std::endl;
        } else {
            std::cout << "AudioEngine: Duration changed (" << cachedTotalDuration << "ms -> " << currentTotalDuration
                      << "ms) - running detailed change detection" << std::endl;
        }
    }

    std::cout << "AudioEngine: Building note maps (cached: " << cachedNotes.size()
              << ", current: " << notesToRender << ")" << std::endl;

    // Build a map of cached notes by ID for fast lookup
    QMap<QString, Note> cachedNoteMap;
    try {
        for (const Note &cachedNote : cachedNotes) {
            QString id = cachedNote.getId();
            if (!id.isEmpty()) {
                cachedNoteMap[id] = cachedNote;
            }
        }
    } catch (...) {
        std::cout << "ERROR: Exception while building cached note map!" << std::endl;
        return;
    }

    // Build a map of current notes by ID
    QMap<QString, Note> currentNoteMap;
    try {
        for (int i = 0; i < notesToRender; i++) {
            QString id = notes[i].getId();
            if (!id.isEmpty()) {
                currentNoteMap[id] = notes[i];
            }
        }
    } catch (...) {
        std::cout << "ERROR: Exception while building current note map!" << std::endl;
        return;
    }

    std::cout << "AudioEngine: Note maps built successfully" << std::endl;

    // Detect which notes changed, were added, or were removed
    QSet<QString> changedNoteIds;
    QSet<QString> addedNoteIds;
    QSet<QString> removedNoteIds;

    // Find added and modified notes
    for (const Note &note : notes) {
        QString noteId = note.getId();

        if (!cachedNoteMap.contains(noteId)) {
            // Note is new
            addedNoteIds.insert(noteId);
            changedNoteIds.insert(noteId);
        } else {
            // Note exists in cache, check if it changed
            const Note &cachedNote = cachedNoteMap[noteId];

            bool noteChanged = (cachedNote.getPitchHz() != note.getPitchHz() ||
                               cachedNote.getDuration() != note.getDuration() ||
                               cachedNote.getStartTime() != note.getStartTime() ||
                               cachedNote.getDynamics() != note.getDynamics() ||
                               cachedNote.getPitchCurve().getPoints().size() != note.getPitchCurve().getPoints().size() ||
                               cachedNote.getDynamicsCurve().getPoints().size() != note.getDynamicsCurve().getPoints().size());

            if (noteChanged) {
                changedNoteIds.insert(noteId);
            }
        }
    }

    // Find removed notes
    for (const Note &cachedNote : cachedNotes) {
        QString noteId = cachedNote.getId();
        if (!currentNoteMap.contains(noteId)) {
            removedNoteIds.insert(noteId);
            changedNoteIds.insert(noteId);
        }
    }

    bool anyChanges = !changedNoteIds.isEmpty() || renderCacheDirty.load();

    if (!anyChanges && !renderSegments.empty()) {
        std::cout << "AudioEngine: Using cached render (no changes detected)" << std::endl;
        // Emit quick progress signals so UI knows we're using cache
        emit renderStarted();
        emit renderProgressChanged(100);
        emit renderCompleted();
        return;
    }

    // Log changes detected
    std::cout << "AudioEngine: Change detection: ";
    if (renderCacheDirty.load()) {
        std::cout << "graph changed (all segments dirty)";
    } else {
        std::cout << addedNoteIds.size() << " added, "
                  << removedNoteIds.size() << " removed, "
                  << (changedNoteIds.size() - addedNoteIds.size() - removedNoteIds.size()) << " modified";
    }
    std::cout << std::endl;

    // Show which synthesis mode we're using
    std::cout << " [Loaded graphs for tracks:";
    bool hasAnyGraph = false;
    for (auto it = trackGraphs.constBegin(); it != trackGraphs.constEnd(); ++it) {
        if (it.value() && it.value()->isValid()) {
            std::cout << " " << it.key();
            hasAnyGraph = true;
        }
    }
    if (!hasAnyGraph) {
        std::cout << " NONE - using fallback generator";
    }
    std::cout << "]" << std::endl;

    // Find the total duration (last note's end time)
    double totalDurationMs = 0.0;
    for (int i = 0; i < notesToRender; i++) {
        double noteEndTime = notes[i].getStartTime() + notes[i].getDuration();
        if (noteEndTime > totalDurationMs) {
            totalDurationMs = noteEndTime;
        }
    }

    // Calculate total samples needed
    double totalDurationSeconds = totalDurationMs / 1000.0;
    size_t totalSamples = static_cast<size_t>(totalDurationSeconds * sampleRate);

    std::cout << "AudioEngine: Total duration: " << totalDurationMs << " ms ("
              << totalSamples << " samples)" << std::endl;

    // Lock both mutexes during rendering
    std::lock_guard<std::mutex> graphLock(graphMutex);
    std::lock_guard<std::mutex> renderLock(renderBufferMutex);

    // Calculate number of segments needed
    int numSegments = static_cast<int>(std::ceil(totalDurationMs / segmentDurationMs));
    if (numSegments == 0) numSegments = 1;  // At least one segment

    size_t oldSegmentCount = renderSegments.size();
    bool compositionGrew = (static_cast<size_t>(numSegments) > oldSegmentCount);
    bool compositionShrank = (static_cast<size_t>(numSegments) < oldSegmentCount);
    bool isFirstRender = (oldSegmentCount == 0);

    // Graph changed: mark all existing segments dirty (full re-render)
    if (renderCacheDirty.load()) {
        std::cout << "AudioEngine: Graph changed, marking all " << numSegments << " segment(s) dirty" << std::endl;

        // If we need more segments, add them
        if (compositionGrew || isFirstRender) {
            size_t oldSize = renderSegments.size();
            std::cout << "AudioEngine: Expanding from " << oldSize << " to " << numSegments << " segments..." << std::endl;
            try {
                renderSegments.resize(numSegments);
            } catch (const std::exception &e) {
                std::cout << "ERROR: Exception during renderSegments.resize: " << e.what() << std::endl;
                return;
            }

            // Initialize only the NEW segments
            for (size_t segIdx = oldSize; segIdx < static_cast<size_t>(numSegments); segIdx++) {
                RenderSegment &seg = renderSegments[segIdx];
                seg.startTimeMs = segIdx * segmentDurationMs;
                seg.endTimeMs = std::min((segIdx + 1) * segmentDurationMs, totalDurationMs);
                double segDurationMs = seg.endTimeMs - seg.startTimeMs;
                size_t segSamples = static_cast<size_t>((segDurationMs / 1000.0) * sampleRate);
                seg.samples.resize(segSamples, 0.0f);
                seg.noteIds.clear();
            }
        } else if (compositionShrank) {
            std::cout << "AudioEngine: Shrinking from " << oldSegmentCount << " to " << numSegments << " segments..." << std::endl;
            renderSegments.resize(numSegments);
        }

        // Mark ALL segments dirty (graph changed)
        for (RenderSegment &seg : renderSegments) {
            seg.isDirty = true;
            seg.noteIds.clear();
        }
    } else {
        // No graph change - handle segment growth/shrinkage, then mark only affected segments dirty
        std::cout << "AudioEngine: Partial re-render - ";

        // Handle composition growing (add new segments)
        if (compositionGrew) {
            size_t oldSize = renderSegments.size();
            std::cout << "composition grew from " << oldSize << " to " << numSegments << " segments, ";
            try {
                renderSegments.resize(numSegments);
            } catch (const std::exception &e) {
                std::cout << "ERROR: Exception during renderSegments.resize: " << e.what() << std::endl;
                return;
            }

            // CRITICAL: Resize the LAST OLD segment if it's now a full 1s segment
            // (When composition grows, what was a partial last segment might now be full)
            if (oldSize > 0) {
                size_t lastOldSegIdx = oldSize - 1;
                RenderSegment &lastOldSeg = renderSegments[lastOldSegIdx];
                lastOldSeg.endTimeMs = std::min((lastOldSegIdx + 1) * segmentDurationMs, totalDurationMs);
                double segDurationMs = lastOldSeg.endTimeMs - lastOldSeg.startTimeMs;
                size_t segSamples = static_cast<size_t>((segDurationMs / 1000.0) * sampleRate);

                // Only resize if it grew
                if (segSamples > lastOldSeg.samples.size()) {
                    size_t oldSampleCount = lastOldSeg.samples.size();
                    lastOldSeg.samples.resize(segSamples, 0.0f);
                    lastOldSeg.isDirty = true;  // Mark dirty since it changed size
                    std::cout << "resized last old segment from " << oldSampleCount << " to " << segSamples << " samples, ";
                }
            }

            // Initialize only the NEW segments
            for (size_t segIdx = oldSize; segIdx < static_cast<size_t>(numSegments); segIdx++) {
                RenderSegment &seg = renderSegments[segIdx];
                seg.startTimeMs = segIdx * segmentDurationMs;
                seg.endTimeMs = std::min((segIdx + 1) * segmentDurationMs, totalDurationMs);
                double segDurationMs = seg.endTimeMs - seg.startTimeMs;
                size_t segSamples = static_cast<size_t>((segDurationMs / 1000.0) * sampleRate);
                seg.samples.resize(segSamples, 0.0f);
                seg.noteIds.clear();
                seg.isDirty = true;  // New segments are dirty
            }
        } else if (compositionShrank) {
            // Handle composition shrinking (remove segments)
            size_t oldSize = renderSegments.size();
            std::cout << "composition shrank from " << oldSize << " to " << numSegments << " segments, ";
            renderSegments.resize(numSegments);
        }

        std::cout << "marking affected segments dirty" << std::endl;

        // Clear noteIds from existing (non-new) segments and mark them clean initially
        for (size_t i = 0; i < oldSegmentCount && i < renderSegments.size(); ++i) {
            renderSegments[i].noteIds.clear();
            renderSegments[i].isDirty = false;  // Start with all old segments clean
        }

        // Mark segments affected by changed/removed notes as dirty
        for (const QString &noteId : changedNoteIds) {
            // For modified notes: mark BOTH old and new positions dirty
            // For removed notes: mark old position dirty
            // For added notes: mark new position dirty

            // Mark old position (if exists in cache)
            if (cachedNoteMap.contains(noteId)) {
                const Note &oldNote = cachedNoteMap[noteId];
                int firstSegment = static_cast<int>(oldNote.getStartTime() / segmentDurationMs);
                int lastSegment = static_cast<int>((oldNote.getStartTime() + oldNote.getDuration()) / segmentDurationMs);
                firstSegment = std::max(0, firstSegment);
                lastSegment = std::min(lastSegment, numSegments - 1);

                // Mark old segments dirty (to clear old audio)
                for (int segIdx = firstSegment; segIdx <= lastSegment; segIdx++) {
                    renderSegments[segIdx].isDirty = true;
                }
            }

            // Mark new position (if exists in current notes)
            if (currentNoteMap.contains(noteId)) {
                const Note &newNote = currentNoteMap[noteId];
                int firstSegment = static_cast<int>(newNote.getStartTime() / segmentDurationMs);
                int lastSegment = static_cast<int>((newNote.getStartTime() + newNote.getDuration()) / segmentDurationMs);
                firstSegment = std::max(0, firstSegment);
                lastSegment = std::min(lastSegment, numSegments - 1);

                // Mark new segments dirty (to write new audio)
                for (int segIdx = firstSegment; segIdx <= lastSegment; segIdx++) {
                    renderSegments[segIdx].isDirty = true;
                }
            }
        }

        // Clear samples in dirty segments
        for (RenderSegment &seg : renderSegments) {
            if (seg.isDirty) {
                std::fill(seg.samples.begin(), seg.samples.end(), 0.0f);  // Reset to silence
            }
        }

        // Count dirty segments
        int dirtyCount = 0;
        for (const RenderSegment &seg : renderSegments) {
            if (seg.isDirty) dirtyCount++;
        }
        std::cout << "AudioEngine: " << dirtyCount << " of " << numSegments
                  << " segment(s) marked dirty" << std::endl;
    }

    // Envelope parameters
    const double attackRate = 0.01;
    const double releaseRate = 0.001;

    // Track which notes affect which segments (for all notes)
    for (int noteIdx = 0; noteIdx < notesToRender; noteIdx++) {
        const Note& note = notes[noteIdx];
        int firstSegment = static_cast<int>(note.getStartTime() / segmentDurationMs);
        int lastSegment = static_cast<int>((note.getStartTime() + note.getDuration()) / segmentDurationMs);
        firstSegment = std::max(0, firstSegment);
        lastSegment = std::min(lastSegment, numSegments - 1);

        for (int segIdx = firstSegment; segIdx <= lastSegment; segIdx++) {
            renderSegments[segIdx].noteIds.insert(note.getId());
        }
    }

    // Count notes that need rendering (those affecting dirty segments)
    int notesRendered = 0;

    // DEBUG: Show available track graphs
    std::cout << " [Loaded graphs for tracks:";
    for (int trackIdx : trackGraphs.keys()) {
        std::cout << " " << trackIdx;
    }
    std::cout << "]" << std::endl;

    emit renderStarted();  // Signal that rendering has begun
    emit renderProgressChanged(5);  // Start at 5% to show immediate feedback

    // Render each note (but only into dirty segments)
    for (int noteIdx = 0; noteIdx < notesToRender; noteIdx++) {
        // Emit progress updates every note (or every 5 notes for large compositions)
        int updateFrequency = (notesToRender > 50) ? 5 : 1;
        if (noteIdx % updateFrequency == 0 || noteIdx == notesToRender - 1) {
            // Calculate based on (noteIdx + 1) so we show progress AFTER completing the note
            int percentage = 5 + ((noteIdx + 1) * 95) / notesToRender;  // 5% to 100%
            emit renderProgressChanged(percentage);
        }
        const Note& note = notes[noteIdx];

        // Calculate which segments this note affects
        int firstSegment = static_cast<int>(note.getStartTime() / segmentDurationMs);
        int lastSegment = static_cast<int>((note.getStartTime() + note.getDuration()) / segmentDurationMs);
        firstSegment = std::max(0, firstSegment);
        lastSegment = std::min(lastSegment, numSegments - 1);

        // Check if this note affects any dirty segments
        bool affectsDirtySegment = false;
        for (int segIdx = firstSegment; segIdx <= lastSegment; segIdx++) {
            if (renderSegments[segIdx].isDirty) {
                affectsDirtySegment = true;
                break;
            }
        }

        // Skip rendering if note doesn't affect any dirty segments
        if (!affectsDirtySegment) {
            continue;
        }

        notesRendered++;

        // Safety check: note start time must be non-negative
        if (note.getStartTime() < 0.0) {
            std::cout << "  WARNING: Note has negative start time " << note.getStartTime()
                      << "ms, skipping" << std::endl;
            continue;
        }

        // Calculate sample positions for this note
        size_t noteStartSample = static_cast<size_t>((note.getStartTime() / 1000.0) * sampleRate);
        size_t noteSamples = static_cast<size_t>((note.getDuration() / 1000.0) * sampleRate);

        // Safety check: note duration must be positive
        if (noteSamples == 0) {
            std::cout << "  WARNING: Note has zero duration, skipping" << std::endl;
            continue;
        }

        // Get note's track and check if it has a graph
        int noteTrackIndex = note.getTrackIndex();

        bool noteHasGraph = trackGraphs.contains(noteTrackIndex)
                            && trackGraphs[noteTrackIndex]
                            && trackGraphs[noteTrackIndex]->isValid();

        // Check if graph produces a tail (reverb, string decay, etc.)
        bool graphHasTail = noteHasGraph && trackGraphs[noteTrackIndex]->hasTail();
        size_t noteDurationSamples = noteSamples;

        // DEBUG: Show which graph is being used
        std::cout << "  Note " << noteIdx << ": trackIndex=" << noteTrackIndex
                  << ", hasGraph=" << (noteHasGraph ? "YES" : "NO")
                  << ", pitch=" << note.getPitchHz() << "Hz" << std::endl;

        // Reset synthesis for this note
        try {
            if (noteHasGraph) {
                trackGraphs[noteTrackIndex]->reset();
            } else {
                // For fallback generator, set initial pitch (will be updated per-sample for continuous notes)
                generator.setFundamentalHz(note.getPitchHz());
                generator.reset();
            }
        } catch (const std::exception &e) {
            std::cout << "ERROR: Exception during synthesis reset: " << e.what() << std::endl;
            return;
        } catch (...) {
            std::cout << "ERROR: Unknown exception during synthesis reset!" << std::endl;
            return;
        }

        double amplitude = 0.0;

        // Calculate segment size in samples (for direct sample-based addressing)
        size_t segmentSizeSamples = static_cast<size_t>((segmentDurationMs / 1000.0) * sampleRate);

        // Safety check: ensure segment size is valid
        if (segmentSizeSamples == 0) {
            std::cout << "  WARNING: segmentSizeSamples is 0, skipping note rendering" << std::endl;
            continue;
        }

        // Maximum tail: 30 seconds safety cap
        size_t maxTailSamples = static_cast<size_t>(30.0 * sampleRate);
        // Silence threshold: ~-60dB (high enough to avoid chasing FFT float residue)
        const float silenceThreshold = 0.001f;
        size_t silenceRunRequired = static_cast<size_t>(0.1 * sampleRate);
        size_t silenceRun = 0;
        bool tailFinished = false;

        // Render this note's samples into segments
        // IMPORTANT: We must process ALL samples to maintain envelope/synthesis state continuity,
        // but we only WRITE to dirty segments. This prevents glitches when notes span multiple segments.
        for (size_t i = 0; !tailFinished; i++) {
            bool inTail = (i >= noteSamples);

            // Stop conditions for tail phase
            if (inTail) {
                if (!graphHasTail) break;
                if ((i - noteSamples) >= maxTailSamples) break;
            }

            // Calculate note progress (0.0 to 1.0 during note, clamped at 1.0 during tail)
            double noteProgress;
            if (inTail) {
                noteProgress = 1.0;
            } else if (noteSamples > 1) {
                noteProgress = static_cast<double>(i) / static_cast<double>(noteSamples - 1);
            } else {
                noteProgress = 0.0;  // Single sample note
            }

            // Sample pitch curve at current time (supports continuous pitch variation)
            double currentPitch = note.getPitchAt(std::min(noteProgress, 1.0));

            // Sample dynamics curve at current time (supports continuous dynamics variation)
            // During tail: freeze dynamics at 1.0 so reverb/decay ring-out is not silenced
            // by a fade-to-zero curve (same reasoning as the wetDry bypass in sounitgraph.cpp).
            double currentDynamics = inTail ? 1.0 : note.getDynamicsAt(noteProgress);

            // Apply vibrato if active (only during note, not tail)
            if (!inTail) {
                const Vibrato &vibrato = note.getVibrato();
                if (vibrato.active && noteProgress >= vibrato.onset) {
                    if (i == 0) {
                        std::cout << "  Vibrato ACTIVE: rate=" << vibrato.rate
                                  << " pitchDepth=" << vibrato.pitchDepth
                                  << " ampDepth=" << vibrato.amplitudeDepth << std::endl;
                    }
                    double vibratoProgress = (noteProgress - vibrato.onset) / (1.0 - vibrato.onset);
                    double envelopeIntensity = vibrato.envelopeAt(vibratoProgress);
                    double noteTimeSeconds = (note.getDuration() / 1000.0) * noteProgress;
                    double vibratoPhase = noteTimeSeconds * vibrato.rate * 2.0 * M_PI;

                    double organicMod = 1.0;
                    if (vibrato.regularity > 0.0) {
                        double slowPhase = noteTimeSeconds * vibrato.rate * 0.23 * 2.0 * M_PI;
                        organicMod = 1.0 + vibrato.regularity * 0.3 * std::sin(slowPhase);
                        vibratoPhase += vibrato.regularity * 0.5 * std::sin(slowPhase * 0.7);
                    }

                    double lfoValue = std::sin(vibratoPhase) * organicMod;
                    currentPitch *= 1.0 + (lfoValue * vibrato.pitchDepth * envelopeIntensity);
                    currentDynamics *= std::max(0.0, 1.0 + (lfoValue * vibrato.amplitudeDepth * envelopeIntensity));
                }
            }

            // Update amplitude envelope (ADSR) - MUST do this for ALL samples to maintain continuity
            if (inTail) {
                amplitude = 1.0;  // Let the natural decay handle fading
            } else if (noteProgress < 0.05) {
                amplitude += (1.0 - amplitude) * attackRate;
            } else if (noteProgress < 0.90) {
                amplitude = 1.0;
            } else if (!graphHasTail) {
                // Release phase - only for graphs without tail (reverb/decay handles fade naturally)
                amplitude *= (1.0 - releaseRate);
                if (amplitude < 0.0001) amplitude = 0.0;
            }

            // Build score curve values: index 0 = dynamics (with vibrato), 1+ = additional named curves
            QVector<double> scoreCurveValues;
            scoreCurveValues.append(currentDynamics);  // index 0: dynamics (already has vibrato applied)
            for (int ci = 1; ci < note.getExpressiveCurveCount(); ++ci) {
                const Curve &c = note.getExpressiveCurve(ci);
                double val = inTail ? 1.0 : (c.isEmpty() ? 0.5 : c.valueAt(std::min(noteProgress, 1.0)));
                scoreCurveValues.append(val);
            }

            // Generate sample using note's track graph
            double sample;
            try {
                if (noteHasGraph) {
                    sample = trackGraphs[noteTrackIndex]->generateSample(currentPitch, noteProgress, false, inTail, currentDynamics, scoreCurveValues);
                } else {
                    generator.setFundamentalHz(currentPitch);
                    sample = generator.generateSample();
                }
            } catch (const std::exception &e) {
                std::cout << "\n  ERROR: Exception during sample generation at sample " << i
                          << ": " << e.what() << std::endl;
                return;
            } catch (...) {
                std::cout << "\n  ERROR: Unknown exception during sample generation at sample " << i << std::endl;
                return;
            }

            // Apply envelope and dynamics curve.
            // When the graph has a post-render IR Convolution, apply it now with the
            // pre-outer-dynamics signal as input.  This prevents the reverb from being
            // doubly suppressed by the dynamics curve (once inside the graph via
            // followDynamics, and once here by currentDynamics).
            if (noteHasGraph && trackGraphs[noteTrackIndex]->hasPostRenderIR()) {
                // irInput: pre-outer-dynamics signal that feeds the IR convolution.
                // dryOutput: fully processed dry signal (with outer currentDynamics).
                double irInput  = sample * amplitude;
                double dryOutput = irInput * currentDynamics;
                sample = trackGraphs[noteTrackIndex]->processPostIR(irInput, dryOutput, inTail);
            } else {
                sample *= amplitude * currentDynamics;
            }
            float outputSample = static_cast<float>(std::clamp(sample * 0.3, -1.0, 1.0));

            // Silence detection during tail
            if (inTail) {
                if (std::fabs(outputSample) < silenceThreshold) {
                    silenceRun++;
                    if (silenceRun >= silenceRunRequired) {
                        tailFinished = true;
                    }
                } else {
                    silenceRun = 0;
                }
            }

            // Now check if we should WRITE this sample (only write to dirty segments)
            // Work directly in sample indices to avoid floating point rounding errors
            size_t globalSamplePos = noteStartSample + i;

            // Calculate which segment this sample belongs to (sample-based, no time conversion)
            int segmentIndex = static_cast<int>(globalSamplePos / segmentSizeSamples);

            // Safety check: validate segment index
            if (segmentIndex < 0 || segmentIndex >= numSegments) {
                // Out of bounds - this shouldn't happen but guard against it
                continue;
            }

            // Safety check: ensure renderSegments has this index
            if (segmentIndex >= static_cast<int>(renderSegments.size())) {
                continue;
            }

            // Only write if this segment is dirty
            if (!renderSegments[segmentIndex].isDirty) {
                continue;
            }

            // Calculate position within segment (sample-based)
            size_t posInSegment = globalSamplePos - (static_cast<size_t>(segmentIndex) * segmentSizeSamples);

            // Safety check: ensure we don't overflow segment buffer
            if (posInSegment >= renderSegments[segmentIndex].samples.size()) {
                continue;
            }

            // Mix into segment buffer - ADD samples to support overlapping notes/tracks
            renderSegments[segmentIndex].samples[posInSegment] += outputSample;
        }

    }

    // Clamp all samples to prevent overflow from mixed notes/tracks
    std::cout << "AudioEngine: Clamping mixed samples to prevent overflow" << std::endl;
    for (RenderSegment &seg : renderSegments) {
        for (float &sample : seg.samples) {
            sample = std::clamp(sample, -1.0f, 1.0f);
        }
    }

    // Mark all segments as clean (rendering complete)
    for (RenderSegment &seg : renderSegments) {
        seg.isDirty = false;
    }

    // Cache the rendered notes and mark cache as clean
    cachedNotes.clear();
    for (int i = 0; i < notesToRender; i++) {
        cachedNotes.append(notes[i]);
    }
    renderCacheDirty.store(false);

    // Calculate total samples across all segments
    size_t totalRenderedSamples = 0;
    for (const RenderSegment &seg : renderSegments) {
        totalRenderedSamples += seg.samples.size();
    }

    std::cout << "AudioEngine: Rendered " << notesRendered << " of " << notesToRender
              << " note(s) into " << renderSegments.size() << " segment(s), "
              << totalRenderedSamples << " samples total" << std::endl;

    emit renderCompleted();  // Signal that rendering has finished
}

void AudioEngine::playRenderedBuffer(double startTimeMs)
{
    if (renderSegments.empty()) {
        std::cout << "AudioEngine: No rendered segments to play" << std::endl;
        return;
    }

    // Calculate total samples
    size_t totalSamples = 0;
    for (const RenderSegment &seg : renderSegments) {
        totalSamples += seg.samples.size();
    }

    // Calculate starting sample position from time
    size_t startSample = static_cast<size_t>((startTimeMs / 1000.0) * sampleRate);

    // Clamp to valid range
    if (startSample >= totalSamples) {
        std::cout << "AudioEngine: Start time " << startTimeMs << "ms ("
                  << startSample << " samples) is beyond rendered buffer ("
                  << totalSamples << " samples), clamping to end" << std::endl;
        startSample = totalSamples > 0 ? totalSamples - 1 : 0;
    }

    // Calculate which segment this sample falls into
    size_t segmentSizeSamples = static_cast<size_t>((segmentDurationMs / 1000.0) * sampleRate);
    size_t startSegmentIndex = 0;
    if (segmentSizeSamples > 0) {
        startSegmentIndex = startSample / segmentSizeSamples;
        // Clamp segment index to valid range
        if (startSegmentIndex >= renderSegments.size()) {
            startSegmentIndex = renderSegments.size() > 0 ? renderSegments.size() - 1 : 0;
        }
    }

    // Set playback position and enable buffer playback mode
    renderPlaybackPosition.store(startSample);
    renderPlaybackSegmentIndex.store(startSegmentIndex);
    useRenderBuffer.store(true);

    std::cout << "AudioEngine: Playing rendered segments from " << startTimeMs << "ms"
              << " (sample " << startSample << ", segment " << startSegmentIndex << ")"
              << " - total: " << renderSegments.size() << " segment(s), "
              << totalSamples << " samples, "
              << (totalSamples / static_cast<double>(sampleRate)) << " seconds" << std::endl;
}

// ========== Track-Based Playback (NEW - Note-Based Rendering) ==========

void AudioEngine::playFromTracks(const QList<Track*> &tracks, double startTimeMs)
{
    std::cout << "AudioEngine: playFromTracks called with " << tracks.size()
              << " track(s), start time: " << startTimeMs << "ms" << std::endl;

    if (tracks.isEmpty()) {
        std::cout << "AudioEngine: No tracks to play" << std::endl;
        return;
    }

    // Stop any existing playback
    useRenderBuffer.store(false);
    useTrackPlayback.store(false);

    // Lock and update track list
    {
        std::lock_guard<std::mutex> lock(trackPlaybackMutex);
        playbackTracks = tracks;
    }

    // Calculate end time based on rendered note buffers (includes reverb/decay tails)
    double endTimeMs = 0.0;
    for (Track *track : tracks) {
        if (!track) continue;

        double trackEnd = track->getRenderedEndTimeMs();
        if (trackEnd > endTimeMs) {
            endTimeMs = trackEnd;
        }
    }

    if (endTimeMs <= startTimeMs) {
        std::cout << "AudioEngine: No content to play (end time " << endTimeMs
                  << "ms <= start time " << startTimeMs << "ms)" << std::endl;
        return;
    }

    // Set playback position
    trackPlaybackTimeMs.store(startTimeMs);
    trackPlaybackEndMs.store(endTimeMs);

    // Enable track-based playback
    useTrackPlayback.store(true);

    std::cout << "AudioEngine: Starting track playback from " << startTimeMs
              << "ms to " << endTimeMs << "ms ("
              << (endTimeMs - startTimeMs) / 1000.0 << " seconds)" << std::endl;
}

void AudioEngine::stopTrackPlayback()
{
    useTrackPlayback.store(false);

    std::lock_guard<std::mutex> lock(trackPlaybackMutex);
    playbackTracks.clear();

    std::cout << "AudioEngine: Track playback stopped" << std::endl;
}

// ========== Audio Device Change Detection ==========

void AudioEngine::startDeviceNotifier()
{
#ifdef _WIN32
    if (m_deviceNotifier) return;

    m_deviceNotifier = new AudioDeviceNotifier(this);
    if (!m_deviceNotifier->start()) {
        delete m_deviceNotifier;
        m_deviceNotifier = nullptr;
        std::cerr << "AudioEngine: Failed to start device notifier" << std::endl;
    }
#endif
}

void AudioEngine::stopDeviceNotifier()
{
#ifdef _WIN32
    if (m_deviceNotifier) {
        m_deviceNotifier->stop();
        delete m_deviceNotifier;
        m_deviceNotifier = nullptr;
    }
#endif
}

void AudioEngine::onDefaultDeviceChanged()
{
    // Guard against re-entrant calls from rapid device changes
    bool expected = false;
    if (!m_deviceSwitchInProgress.compare_exchange_strong(expected, true)) {
        std::cout << "AudioEngine: Device switch already in progress, ignoring" << std::endl;
        return;
    }

    std::cout << "AudioEngine: Default audio device changed, restarting stream..." << std::endl;

    // Stop all playback modes
    useRenderBuffer.store(false);
    useTrackPlayback.store(false);
    gateOpen.store(false);

    // Tell UI to stop its playback timers/buttons
    emit playbackStopRequested();

    // Save current sample rate for re-init
    unsigned int savedSampleRate = sampleRate;

    // Shut down current stream
    shutdown();

    // Delay re-init to let the OS finish the device transition
    QTimer::singleShot(300, this, [this, savedSampleRate]() {
        bool success = initialize(savedSampleRate);

        if (success) {
            std::cout << "AudioEngine: Successfully restarted on new device" << std::endl;
            emit audioDeviceChanged(QString("New default device"));
        } else {
            std::cerr << "AudioEngine: Failed to restart on new device" << std::endl;
            emit audioDeviceError(QString("Failed to initialize audio on the new device. "
                                          "Please restart the application."));
        }

        m_deviceSwitchInProgress.store(false);
    });
}
