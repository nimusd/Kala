#include "trackcommands.h"
#include <QDebug>

static const char* propertyName(SetTrackMixerCommand::Property p)
{
    switch (p) {
    case SetTrackMixerCommand::Volume: return "Volume";
    case SetTrackMixerCommand::Gain:   return "Gain";
    case SetTrackMixerCommand::Pan:    return "Pan";
    }
    return "";
}

SetTrackMixerCommand::SetTrackMixerCommand(Track *track, Property property,
                                           float oldVal, float newVal,
                                           QPointer<QDial> dial, QPointer<QLabel> label,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_track(track)
    , m_property(property)
    , m_oldVal(oldVal)
    , m_newVal(newVal)
    , m_dial(dial)
    , m_label(label)
{
    setText(QString("Set Track %1").arg(propertyName(property)));
}

QString SetTrackMixerCommand::labelText(float v) const
{
    int pct = static_cast<int>(v * 100);
    switch (m_property) {
    case Volume: return QString("V:%1%").arg(pct);
    case Gain:   return QString("G:%1%").arg(pct);
    case Pan:    return QString("P:%1").arg(pct);
    }
    return {};
}

void SetTrackMixerCommand::applyValue(float v)
{
    switch (m_property) {
    case Volume: m_track->setVolume(v); break;
    case Gain:   m_track->setGain(v);   break;
    case Pan:    m_track->setPan(v);    break;
    }
    if (m_dial)  m_dial->blockSignals(true);
    if (m_dial)  m_dial->setValue(static_cast<int>(v * 100));
    if (m_dial)  m_dial->blockSignals(false);
    if (m_label) m_label->setText(labelText(v));
}

void SetTrackMixerCommand::undo()
{
    applyValue(m_oldVal);
    qDebug() << "Undo: Set track" << propertyName(m_property) << "to" << m_oldVal;
}

void SetTrackMixerCommand::redo()
{
    applyValue(m_newVal);
    qDebug() << "Redo: Set track" << propertyName(m_property) << "to" << m_newVal;
}

bool SetTrackMixerCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id()) return false;
    const SetTrackMixerCommand *cmd = static_cast<const SetTrackMixerCommand*>(other);
    if (cmd->m_track != m_track || cmd->m_property != m_property) return false;
    m_newVal = cmd->m_newVal;  // Keep original m_oldVal, adopt latest value
    return true;
}
