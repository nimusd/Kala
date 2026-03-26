#ifndef TRACKCOMMANDS_H
#define TRACKCOMMANDS_H

#include <QUndoCommand>
#include <QPointer>
#include <QDial>
#include <QLabel>
#include "track.h"

// ============================================================================
// Set Track Mixer Command
// Used by the mixer volume/gain/pan dials.
// Merges consecutive changes to the same property so dial-dragging
// produces a single undo step.
// ============================================================================
class SetTrackMixerCommand : public QUndoCommand
{
public:
    enum Property { Volume, Gain, Pan };

    SetTrackMixerCommand(Track *track, Property property,
                         float oldVal, float newVal,
                         QPointer<QDial> dial, QPointer<QLabel> label,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 2001; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    Track *m_track;
    Property m_property;
    float m_oldVal;
    float m_newVal;
    QPointer<QDial> m_dial;
    QPointer<QLabel> m_label;

    void applyValue(float v);
    QString labelText(float v) const;
};

#endif // TRACKCOMMANDS_H
