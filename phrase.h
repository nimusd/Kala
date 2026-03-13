#ifndef PHRASE_H
#define PHRASE_H

#include "note.h"
#include <QString>
#include <QVector>
#include <QUuid>

/**
 * Phrase - Container for notes
 *
 * Unit of pre-rendering (later). Currently just a container for notes.
 * Will expand to include rendered audio buffers when sound engine is implemented.
 */
class Phrase
{
public:
    Phrase();

    // Note management
    void addNote(const Note &note);
    void removeNote(const QString &noteId);
    void removeNoteByIndex(int index);  // Remove note by index
    void clearNotes();
    QVector<Note> getNotes() const { return notes; }
    QVector<Note>& getNotes() { return notes; }  // Non-const version for modification
    QVector<Note> getNotesInRange(double startTime, double endTime) const;
    int getNoteCount() const { return notes.size(); }

    // Getters
    QString getId() const { return id; }
    double getStartTime() const { return startTime; }
    double getDuration() const { return duration; }
    bool isDirtyFlag() const { return isDirty; }

    // State management
    void markDirty() { isDirty = true; }
    void markClean() { isDirty = false; }

    // Automatic calculation of phrase bounds from notes
    void updateBounds();

private:
    QString id;              // Unique identifier
    QVector<Note> notes;     // Notes in this phrase
    double startTime;        // Start time in milliseconds (calculated from notes)
    double duration;         // Duration in milliseconds (calculated from notes)
    bool isDirty;            // True if needs re-rendering (for later audio engine)
};

#endif // PHRASE_H
