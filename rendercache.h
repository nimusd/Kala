#ifndef RENDERCACHE_H
#define RENDERCACHE_H

#include <QString>
#include <QList>
#include "track.h"

/**
 * RenderCache - Binary file handler for pre-rendered audio cache
 *
 * Saves and loads NoteRender data to/from binary .kalarender files.
 * This eliminates the need to re-render all notes when loading a project.
 *
 * File format (.kalarender):
 *
 * Header (32 bytes):
 *   - Magic: "CLMR" (4 bytes)
 *   - Version: uint32_t (4 bytes)
 *   - Sample rate: double (8 bytes)
 *   - Entry count: uint32_t (4 bytes)
 *   - Reserved: 12 bytes
 *
 * Per-entry header (variable size):
 *   - Track ID: int32_t (4 bytes)
 *   - Note ID length: uint32_t (4 bytes)
 *   - Note ID: UTF-8 string (variable)
 *   - Graph hash: uint64_t (8 bytes)
 *   - Note hash: uint64_t (8 bytes)
 *   - Sample count: uint64_t (8 bytes)
 *
 * Per-entry audio data:
 *   - Samples: float[] (sample_count * 4 bytes)
 *
 * Usage:
 *   // Save render cache alongside project
 *   RenderCache::saveToFile("project.kalarender", tracks, 44100.0);
 *
 *   // Load render cache when opening project
 *   int loaded = RenderCache::loadFromFile("project.kalarender", tracks);
 */
class RenderCache
{
public:
    /**
     * Save all track render caches to binary file
     *
     * Iterates through all tracks and their note renders, writing
     * valid renders to the binary file. Skips invalid/empty renders.
     *
     * @param filePath Path to .kalarender file
     * @param tracks List of tracks to save renders from
     * @param sampleRate Sample rate of the renders
     * @return true if saved successfully, false on error
     */
    static bool saveToFile(const QString &filePath,
                           const QList<Track*> &tracks,
                           double sampleRate);

    /**
     * Load render caches from binary file into tracks
     *
     * Reads the cache file and restores valid renders to matching notes.
     * Validation:
     *   1. Note ID must exist in a track
     *   2. Graph hash must match track's current graphHash
     *   3. Note hash must match note's computeHash()
     *
     * Notes that don't match are marked as renderDirty for re-rendering.
     *
     * @param filePath Path to .kalarender file
     * @param tracks List of tracks to load renders into
     * @return Number of valid entries loaded, or -1 on file error
     */
    static int loadFromFile(const QString &filePath,
                            const QList<Track*> &tracks);

private:
    // File format constants
    static constexpr char MAGIC[4] = {'C', 'L', 'M', 'R'};
    static constexpr uint32_t VERSION = 1;
    static constexpr size_t HEADER_SIZE = 32;

    // Helper methods
    static bool writeHeader(QDataStream &stream, double sampleRate, uint32_t entryCount);
    static bool readHeader(QDataStream &stream, double &sampleRate, uint32_t &entryCount);
};

#endif // RENDERCACHE_H
