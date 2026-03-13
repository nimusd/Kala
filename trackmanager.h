#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include <QObject>
#include <QList>
#include <QColor>
#include "track.h"

/**
 * TrackManager - Manages collection of Track objects
 *
 * Simple manager for track lifecycle and current track selection.
 * Owns all Track objects and handles their creation/deletion.
 *
 * Usage:
 *   TrackManager *manager = new TrackManager();
 *   Track *track = manager->addTrack("Piano", Qt::blue);
 *   manager->setCurrentTrack(0);
 *   Track *current = manager->getCurrentTrack();
 */
class TrackManager : public QObject
{
    Q_OBJECT

public:
    explicit TrackManager(QObject *parent = nullptr);
    ~TrackManager();

    // ========== Track Management ==========

    /**
     * Add a new track
     * @param name Track display name
     * @param color Track color for UI
     * @return Pointer to newly created track (owned by manager)
     */
    Track* addTrack(const QString &name, const QColor &color);

    /**
     * Remove a track by index
     * Deletes the track and all its data.
     * If removing current track, switches to track 0 (if exists).
     *
     * @param index Track index (0-based)
     * @return true if removed successfully
     */
    bool removeTrack(int index);

    /**
     * Get track by index
     * @param index Track index (0-based)
     * @return Pointer to track, or nullptr if invalid index
     */
    Track* getTrack(int index) const;

    /**
     * Get all tracks
     * @return List of all track pointers
     */
    const QList<Track*>& getTracks() const { return m_tracks; }

    /**
     * Get number of tracks
     */
    int getTrackCount() const { return m_tracks.size(); }

    // ========== Current Track Selection ==========

    /**
     * Get current track index
     * @return Index of current track, or -1 if no tracks
     */
    int getCurrentTrackIndex() const { return m_currentTrackIndex; }

    /**
     * Get current track
     * @return Pointer to current track, or nullptr if no tracks
     */
    Track* getCurrentTrack() const;

    /**
     * Set current track by index
     * @param index Track index (0-based)
     * @return true if successful, false if invalid index
     */
    bool setCurrentTrack(int index);

    /**
     * Clear all tracks
     * Deletes all tracks and resets state.
     */
    void clearAllTracks();

signals:
    // ========== Signals ==========

    /**
     * Emitted when a track is added
     * @param track Pointer to newly added track
     * @param index Index where track was added
     */
    void trackAdded(Track *track, int index);

    /**
     * Emitted when a track is removed
     * @param index Index of removed track
     */
    void trackRemoved(int index);

    /**
     * Emitted when current track selection changes
     * @param track Pointer to new current track (may be nullptr)
     * @param index Index of new current track (-1 if no tracks)
     */
    void currentTrackChanged(Track *track, int index);

    /**
     * Emitted when all tracks are cleared
     */
    void allTracksCleared();

private:
    // ========== Member Variables ==========

    QList<Track*> m_tracks;          // All tracks (owned by manager)
    int m_currentTrackIndex;         // Index of current track (-1 if none)
    int m_nextTrackId;               // Auto-incrementing track ID counter

    // Default color palette for new tracks
    static const QList<QColor> s_colorPalette;
    int m_nextColorIndex;            // Cycles through palette
};

#endif // TRACKMANAGER_H
