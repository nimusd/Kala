#include "trackmanager.h"
#include <QDebug>

// Default color palette for tracks (cycles through these colors)
const QList<QColor> TrackManager::s_colorPalette = {
    QColor("#3498db"),  // Deep blue
    QColor("#e91e63"),  // Deep pink
    QColor("#8bc34a"),  // Lime green
    QColor("#ff9800"),  // Orange
    QColor("#9c27b0"),  // Purple
    QColor("#00bcd4")   // Cyan
};

TrackManager::TrackManager(QObject *parent)
    : QObject(parent)
    , m_currentTrackIndex(-1)
    , m_nextTrackId(0)
    , m_nextColorIndex(0)
{
    qDebug() << "TrackManager created";
}

TrackManager::~TrackManager()
{
    qDebug() << "TrackManager destroyed - cleaning up" << m_tracks.size() << "tracks";
    clearAllTracks();
}

// ========== Track Management ==========

Track* TrackManager::addTrack(const QString &name, const QColor &color)
{
    qDebug() << "TrackManager: Adding track:" << name << "color:" << color.name();

    // Create new track
    Track *track = new Track(m_nextTrackId++, name, color, this);

    // Add to list
    int index = m_tracks.size();
    m_tracks.append(track);

    // If this is the first track, make it current
    if (m_currentTrackIndex == -1) {
        m_currentTrackIndex = 0;
        emit currentTrackChanged(track, 0);
    }

    emit trackAdded(track, index);

    qDebug() << "TrackManager: Track added at index" << index << "(ID:" << track->getTrackId() << ")";

    return track;
}

bool TrackManager::removeTrack(int index)
{
    if (index < 0 || index >= m_tracks.size()) {
        qWarning() << "TrackManager: Invalid track index for removal:" << index;
        return false;
    }

    qDebug() << "TrackManager: Removing track at index" << index;

    Track *track = m_tracks[index];
    m_tracks.removeAt(index);

    // Delete the track (also deletes its canvas, graph, notes, etc.)
    delete track;

    emit trackRemoved(index);

    // Update current track index if needed
    if (m_tracks.isEmpty()) {
        // No tracks left
        m_currentTrackIndex = -1;
        emit currentTrackChanged(nullptr, -1);
    } else if (m_currentTrackIndex == index) {
        // Removed current track - switch to track 0
        m_currentTrackIndex = 0;
        emit currentTrackChanged(m_tracks[0], 0);
    } else if (m_currentTrackIndex > index) {
        // Current track shifted down by one
        m_currentTrackIndex--;
    }

    qDebug() << "TrackManager: Track removed. Remaining tracks:" << m_tracks.size();

    return true;
}

Track* TrackManager::getTrack(int index) const
{
    if (index < 0 || index >= m_tracks.size()) {
        return nullptr;
    }
    return m_tracks[index];
}

void TrackManager::clearAllTracks()
{
    qDebug() << "TrackManager: Clearing all tracks";

    // Delete all tracks
    for (Track *track : m_tracks) {
        delete track;
    }

    m_tracks.clear();
    m_currentTrackIndex = -1;
    m_nextTrackId = 0;
    m_nextColorIndex = 0;

    emit allTracksCleared();
    emit currentTrackChanged(nullptr, -1);

    qDebug() << "TrackManager: All tracks cleared";
}

// ========== Current Track Selection ==========

Track* TrackManager::getCurrentTrack() const
{
    if (m_currentTrackIndex < 0 || m_currentTrackIndex >= m_tracks.size()) {
        return nullptr;
    }
    return m_tracks[m_currentTrackIndex];
}

bool TrackManager::setCurrentTrack(int index)
{
    if (index < 0 || index >= m_tracks.size()) {
        qWarning() << "TrackManager: Invalid track index for selection:" << index;
        return false;
    }

    if (m_currentTrackIndex == index) {
        // Already current - no change needed
        return true;
    }

    qDebug() << "TrackManager: Switching current track from" << m_currentTrackIndex << "to" << index;

    m_currentTrackIndex = index;
    emit currentTrackChanged(m_tracks[index], index);

    return true;
}
