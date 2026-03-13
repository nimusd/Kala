#include "phrase.h"
#include <algorithm>

Phrase::Phrase()
    : id(QUuid::createUuid().toString())
    , startTime(0.0)
    , duration(0.0)
    , isDirty(true)
{
}

void Phrase::addNote(const Note &note)
{
    notes.append(note);
    updateBounds();
    markDirty();
}

void Phrase::removeNote(const QString &noteId)
{
    notes.erase(
        std::remove_if(notes.begin(), notes.end(),
                      [&noteId](const Note &n) { return n.getId() == noteId; }),
        notes.end()
    );
    updateBounds();
    markDirty();
}

void Phrase::removeNoteByIndex(int index)
{
    if (index >= 0 && index < notes.size()) {
        notes.removeAt(index);
        updateBounds();
        markDirty();
    }
}

void Phrase::clearNotes()
{
    notes.clear();
    startTime = 0.0;
    duration = 0.0;
    markDirty();
}

QVector<Note> Phrase::getNotesInRange(double startTime, double endTime) const
{
    QVector<Note> result;
    for (const Note &note : notes) {
        // Include note if it overlaps with the range
        if (note.getStartTime() < endTime && note.getEndTime() > startTime) {
            result.append(note);
        }
    }
    return result;
}

void Phrase::updateBounds()
{
    if (notes.isEmpty()) {
        startTime = 0.0;
        duration = 0.0;
        return;
    }

    // Find earliest start and latest end
    double minStart = notes.first().getStartTime();
    double maxEnd = notes.first().getEndTime();

    for (const Note &note : notes) {
        minStart = std::min(minStart, note.getStartTime());
        maxEnd = std::max(maxEnd, note.getEndTime());
    }

    startTime = minStart;
    duration = maxEnd - minStart;
}
