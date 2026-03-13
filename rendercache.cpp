#include "rendercache.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

bool RenderCache::saveToFile(const QString &filePath,
                              const QList<Track*> &tracks,
                              double sampleRate)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "RenderCache: Failed to open file for writing:" << filePath;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Count total valid entries first
    uint32_t entryCount = 0;
    for (const Track *track : tracks) {
        const QMap<QString, NoteRender> &renders = track->getNoteRenders();
        for (auto it = renders.constBegin(); it != renders.constEnd(); ++it) {
            if (it.value().valid && !it.value().samples.empty()) {
                entryCount++;
            }
        }
    }

    // Write header
    if (!writeHeader(stream, sampleRate, entryCount)) {
        qWarning() << "RenderCache: Failed to write header";
        file.close();
        return false;
    }

    // Write each entry
    uint32_t writtenCount = 0;
    for (const Track *track : tracks) {
        int32_t trackId = static_cast<int32_t>(track->getTrackId());
        const QMap<QString, NoteRender> &renders = track->getNoteRenders();

        for (auto it = renders.constBegin(); it != renders.constEnd(); ++it) {
            const QString &noteId = it.key();
            const NoteRender &render = it.value();

            // Skip invalid or empty renders
            if (!render.valid || render.samples.empty()) {
                continue;
            }

            // Write track ID
            stream << trackId;

            // Write note ID (length + UTF-8 data)
            QByteArray noteIdUtf8 = noteId.toUtf8();
            uint32_t noteIdLen = static_cast<uint32_t>(noteIdUtf8.size());
            stream << noteIdLen;
            stream.writeRawData(noteIdUtf8.constData(), noteIdUtf8.size());

            // Write hashes
            stream << render.graphHash;
            stream << render.noteHash;

            // Write sample count and samples
            uint64_t sampleCount = static_cast<uint64_t>(render.samples.size());
            stream << sampleCount;

            // Write samples as raw float array
            stream.writeRawData(reinterpret_cast<const char*>(render.samples.data()),
                                static_cast<int>(sampleCount * sizeof(float)));

            writtenCount++;
        }
    }

    file.close();

    qDebug() << "RenderCache: Saved" << writtenCount << "render entries to" << filePath;
    return true;
}

int RenderCache::loadFromFile(const QString &filePath, const QList<Track*> &tracks)
{
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "RenderCache: Cache file does not exist:" << filePath;
        return 0;  // Not an error - normal for new projects
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "RenderCache: Failed to open file for reading:" << filePath;
        return -1;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Read header
    double sampleRate;
    uint32_t entryCount;
    if (!readHeader(stream, sampleRate, entryCount)) {
        qWarning() << "RenderCache: Invalid or corrupted cache file";
        file.close();
        return -1;
    }

    qDebug() << "RenderCache: Loading" << entryCount << "entries at" << sampleRate << "Hz";

    // Build a map of track ID -> Track pointer for fast lookup
    QMap<int, Track*> trackMap;
    for (Track *track : tracks) {
        trackMap[track->getTrackId()] = track;
    }

    // Build a map of note ID -> Note pointer for each track
    // Also store note's current hash for validation
    QMap<int, QMap<QString, const Note*>> notesByTrack;
    for (Track *track : tracks) {
        QMap<QString, const Note*> noteMap;
        for (const Note &note : track->getNotes()) {
            noteMap[note.getId()] = &note;
        }
        notesByTrack[track->getTrackId()] = noteMap;
    }

    int loadedCount = 0;
    int skippedCount = 0;

    for (uint32_t i = 0; i < entryCount && !stream.atEnd(); ++i) {
        // Read track ID
        int32_t trackId;
        stream >> trackId;

        // Read note ID
        uint32_t noteIdLen;
        stream >> noteIdLen;

        if (noteIdLen > 1024) {  // Sanity check
            qWarning() << "RenderCache: Invalid note ID length:" << noteIdLen;
            break;
        }

        QByteArray noteIdUtf8(static_cast<int>(noteIdLen), '\0');
        stream.readRawData(noteIdUtf8.data(), static_cast<int>(noteIdLen));
        QString noteId = QString::fromUtf8(noteIdUtf8);

        // Read hashes
        uint64_t storedGraphHash, storedNoteHash;
        stream >> storedGraphHash;
        stream >> storedNoteHash;

        // Read sample count
        uint64_t sampleCount;
        stream >> sampleCount;

        if (sampleCount > 100000000) {  // Sanity check (~38 minutes at 44100 Hz)
            qWarning() << "RenderCache: Invalid sample count:" << sampleCount;
            break;
        }

        // Read samples
        std::vector<float> samples(static_cast<size_t>(sampleCount));
        stream.readRawData(reinterpret_cast<char*>(samples.data()),
                           static_cast<int>(sampleCount * sizeof(float)));

        // Validate and restore the render
        if (!trackMap.contains(trackId)) {
            qDebug() << "RenderCache: Track" << trackId << "not found, skipping entry";
            skippedCount++;
            continue;
        }

        Track *track = trackMap[trackId];

        // Check if note still exists
        if (!notesByTrack[trackId].contains(noteId)) {
            qDebug() << "RenderCache: Note" << noteId << "not found in track" << trackId << ", skipping";
            skippedCount++;
            continue;
        }

        // Validate graph hash
        if (storedGraphHash != track->getGraphHash()) {
            qDebug() << "RenderCache: Graph hash mismatch for note" << noteId
                     << "(stored:" << storedGraphHash << ", current:" << track->getGraphHash() << ")";
            skippedCount++;
            continue;
        }

        // Validate note hash
        const Note *note = notesByTrack[trackId][noteId];
        uint64_t currentNoteHash = note->computeHash();
        if (storedNoteHash != currentNoteHash) {
            qDebug() << "RenderCache: Note hash mismatch for note" << noteId
                     << "(stored:" << storedNoteHash << ", current:" << currentNoteHash << ")";
            skippedCount++;
            continue;
        }

        // All validations passed - restore the render
        NoteRender render;
        render.samples = std::move(samples);
        render.sampleRate = sampleRate;
        render.graphHash = storedGraphHash;
        render.noteHash = storedNoteHash;
        render.valid = true;

        track->setNoteRender(noteId, render);
        loadedCount++;
    }

    file.close();

    qDebug() << "RenderCache: Loaded" << loadedCount << "renders,"
             << skippedCount << "skipped (stale or missing)";

    return loadedCount;
}

bool RenderCache::writeHeader(QDataStream &stream, double sampleRate, uint32_t entryCount)
{
    // Write magic bytes
    stream.writeRawData(MAGIC, 4);

    // Write version
    stream << VERSION;

    // Write sample rate (as double - 8 bytes)
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream << sampleRate;
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Write entry count
    stream << entryCount;

    // Write reserved bytes (12 bytes to make header 32 bytes total)
    // 4 (magic) + 4 (version) + 8 (sampleRate) + 4 (entryCount) = 20 bytes
    // Need 12 more bytes for padding
    char reserved[12] = {0};
    stream.writeRawData(reserved, 12);

    return stream.status() == QDataStream::Ok;
}

bool RenderCache::readHeader(QDataStream &stream, double &sampleRate, uint32_t &entryCount)
{
    // Read and verify magic bytes
    char magic[4];
    if (stream.readRawData(magic, 4) != 4) {
        return false;
    }

    if (magic[0] != MAGIC[0] || magic[1] != MAGIC[1] ||
        magic[2] != MAGIC[2] || magic[3] != MAGIC[3]) {
        qWarning() << "RenderCache: Invalid magic bytes";
        return false;
    }

    // Read and verify version
    uint32_t version;
    stream >> version;
    if (version != VERSION) {
        qWarning() << "RenderCache: Unsupported version:" << version << "(expected" << VERSION << ")";
        return false;
    }

    // Read sample rate
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream >> sampleRate;
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Read entry count
    stream >> entryCount;

    // Skip reserved bytes
    char reserved[12];
    stream.readRawData(reserved, 12);

    return stream.status() == QDataStream::Ok;
}
