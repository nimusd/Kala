#ifndef NOTERENDER_H
#define NOTERENDER_H

#include <vector>
#include <cstdint>

/**
 * NoteRender - Pre-rendered audio for a single note
 *
 * Each note has its own audio buffer. This enables fine-grained caching:
 * - Moving a note doesn't require re-rendering (just changes mix position)
 * - Editing a note only re-renders that one note
 * - Graph changes invalidate all note renders
 *
 * The render is associated with a note by ID in Track's noteRenders map.
 */
struct NoteRender {
    std::vector<float> samples;   // Pre-rendered audio samples for this note
    double sampleRate;            // Sample rate used for rendering
    uint64_t graphHash;           // Graph state hash when rendered (for invalidation)
    uint64_t noteHash;            // Note properties hash when rendered (for change detection)
    bool valid;                   // True if render is usable for playback

    NoteRender()
        : sampleRate(0.0)
        , graphHash(0)
        , noteHash(0)
        , valid(false)
    {}

    /**
     * Clear the render, marking it invalid
     */
    void clear() {
        samples.clear();
        sampleRate = 0.0;
        graphHash = 0;
        noteHash = 0;
        valid = false;
    }

    /**
     * Check if render needs to be regenerated
     * @param currentGraphHash Current graph state hash
     * @param currentNoteHash Current note properties hash
     * @return true if render is stale and needs regeneration
     */
    bool needsRender(uint64_t currentGraphHash, uint64_t currentNoteHash) const {
        if (!valid) return true;
        if (graphHash != currentGraphHash) return true;
        if (noteHash != currentNoteHash) return true;
        return false;
    }

    /**
     * Get duration of rendered audio in milliseconds
     */
    double getDurationMs() const {
        if (sampleRate <= 0 || samples.empty()) return 0.0;
        return (samples.size() / sampleRate) * 1000.0;
    }

    /**
     * Get sample at a specific time offset within the note
     * @param offsetMs Time offset from note start in milliseconds
     * @return Sample value, or 0.0 if out of range
     */
    float getSampleAt(double offsetMs) const {
        if (!valid || samples.empty() || sampleRate <= 0) return 0.0f;

        size_t index = static_cast<size_t>((offsetMs / 1000.0) * sampleRate);
        if (index >= samples.size()) return 0.0f;

        return samples[index];
    }
};

#endif // NOTERENDER_H
