#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include "harmonicgenerator.h"
#include "sounitgraph.h"
#include "note.h"
#include <RtAudio.h>
#include <memory>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <vector>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QObject>
#include <QList>
#include <QTimer>
#include "rendersegment.h"

// Forward declarations
class Track;

#ifdef _WIN32
class AudioDeviceNotifier;
#endif

/**
 * AudioEngine - RTAudio wrapper for real-time synthesis (Phase 3)
 *
 * Manages audio stream and connects HarmonicGenerator to output.
 * Supports multiple tracks, each with its own sounit graph.
 *
 * Each track (identified by trackIndex) can have its own SounitGraph.
 * During rendering, notes use the graph associated with their trackIndex.
 */
class AudioEngine : public QObject
{
    Q_OBJECT

public:
    AudioEngine();
    ~AudioEngine();

    // Audio stream control
    bool initialize(unsigned int sampleRate = 44100, unsigned int bufferFrames = 256);
    void shutdown();
    bool isRunning() const;

    // Note playback (simple gate on/off for now)
    void playNote(const Note& note);
    void stopNote();

    // Pre-rendering (render notes to buffer, then play from buffer) - LEGACY segment-based
    void renderNotes(const QVector<Note>& notes, int maxNotes = -1);  // -1 = all notes
    void playRenderedBuffer(double startTimeMs = 0.0);  // Start playback from specific time position
    bool isPlayingBuffer() const { return useRenderBuffer.load() || useTrackPlayback.load(); }

    // Track-based playback (NEW - note-based rendering)
    /**
     * Start playback from tracks using note-based rendering
     * Each track provides pre-rendered note audio via getMixedBuffer().
     *
     * @param tracks List of tracks to play from (must have been pre-rendered)
     * @param startTimeMs Start time in milliseconds
     */
    void playFromTracks(const QList<Track*> &tracks, double startTimeMs = 0.0);

    /**
     * Stop track-based playback
     */
    void stopTrackPlayback();

    /**
     * Check if track-based playback is active
     */
    bool isPlayingFromTracks() const { return useTrackPlayback.load(); }

    /**
     * Get current playback time in milliseconds (for track-based playback)
     */
    double getTrackPlaybackTimeMs() const { return trackPlaybackTimeMs.load(); }

    /**
     * Get sample rate
     */
    unsigned int getSampleRate() const { return sampleRate; }

    // Graph-based synthesis (multi-track support)
    bool buildGraph(class Canvas *canvas, int trackIndex);  // Build graph for specific track
    void clearGraph(int trackIndex);   // Clear graph for specific track
    void clearAllGraphs();              // Clear all track graphs
    bool hasGraph(int trackIndex) const;  // Check if track has a valid graph

    // Parameter access
    HarmonicGenerator& getGenerator() { return generator; }

    // Audio device change detection
    void startDeviceNotifier();
    void stopDeviceNotifier();

public slots:
    void onDefaultDeviceChanged();

signals:
    void audioDeviceChanged(QString deviceName);
    void audioDeviceError(QString msg);
    void playbackStopRequested();
    // Render progress signals
    void renderProgressChanged(int percentage);  // 0-100, emitted during renderNotes()
    void renderStarted();                        // Emitted when rendering begins
    void renderCompleted();                      // Emitted when rendering finishes

private:
    // RTAudio callback (must be static)
    static int audioCallback(void *outputBuffer, void *inputBuffer,
                            unsigned int nFrames, double streamTime,
                            RtAudioStreamStatus status, void *userData);

    RtAudio audioDevice;
    HarmonicGenerator generator;  // Fallback for direct playback
    QMap<int, SounitGraph*> trackGraphs;  // Graph-based synthesis (one per track)

    std::atomic<bool> gateOpen;  // Simple gate for now
    std::atomic<double> amplitude;  // Envelope state
    std::atomic<double> currentPitch;  // Current note pitch in Hz

    // Note timing for envelope engine
    std::atomic<double> noteDuration;  // Note duration in milliseconds
    std::atomic<uint64_t> noteStartSample;  // Sample number when note started
    std::atomic<uint64_t> currentSample;  // Current sample number (for timing)

    // Pre-rendered buffer playback (segment-based)
    std::vector<RenderSegment> renderSegments;  // Pre-rendered audio segments
    double segmentDurationMs;  // Duration of each segment in milliseconds (default: 1000ms)
    std::atomic<bool> useRenderBuffer;  // true = play from buffer, false = live synthesis
    std::atomic<size_t> renderPlaybackPosition;  // Current position in render buffer (global sample index)
    std::atomic<size_t> renderPlaybackSegmentIndex;  // Current segment being played
    std::mutex renderBufferMutex;  // Protect render buffer during creation/playback
    std::atomic<bool> renderCacheDirty;  // true = need to re-render, false = can reuse buffer
    QVector<Note> cachedNotes;  // The notes that were last rendered (for comparison)

    unsigned int sampleRate;
    bool initialized;
    std::mutex graphMutex;  // Protect graph access between audio thread and UI thread

    // Track-based playback (NEW - note-based rendering)
    QList<Track*> playbackTracks;           // Tracks to play from
    std::atomic<bool> useTrackPlayback;     // true = play from tracks, false = other modes
    std::atomic<double> trackPlaybackTimeMs; // Current playback position in milliseconds
    std::atomic<double> trackPlaybackEndMs;  // End time of composition (for auto-stop)
    std::mutex trackPlaybackMutex;          // Protect track list access

    // Audio device change detection
    std::atomic<bool> m_deviceSwitchInProgress;
#ifdef _WIN32
    AudioDeviceNotifier *m_deviceNotifier;
#endif
};

#endif // AUDIOENGINE_H
