#ifndef RENDERSEGMENT_H
#define RENDERSEGMENT_H

#include <vector>
#include <QSet>
#include <QString>
#include <cstdint>

/**
 * RenderSegment - Fixed-duration segment of pre-rendered audio
 *
 * Enables partial re-rendering: only segments with changed notes need re-rendering.
 * Segments divide the timeline into fixed chunks (e.g., 1 second each).
 *
 * Used by both Track (for per-track rendering) and AudioEngine (for legacy support).
 */
struct RenderSegment {
    double startTimeMs;           // Segment start time in milliseconds
    double endTimeMs;             // Segment end time in milliseconds
    std::vector<float> samples;   // Pre-rendered audio samples for this segment
    QSet<QString> noteIds;        // IDs of notes affecting this segment
    bool isDirty;                 // True if segment needs re-rendering
    uint64_t hash;                // Hash of note states for quick comparison (future use)

    RenderSegment()
        : startTimeMs(0.0)
        , endTimeMs(0.0)
        , isDirty(true)
        , hash(0)
    {}
};

#endif // RENDERSEGMENT_H
