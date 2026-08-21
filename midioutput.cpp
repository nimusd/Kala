#include "midioutput.h"
#include <QSettings>
#include <QTimer>
#include <QSharedPointer>
#include <QDebug>
#include <RtMidi.h>

namespace MidiOutput {

QStringList availablePorts()
{
    QStringList names;
    try {
        RtMidiOut midiOut;
        for (unsigned int i = 0; i < midiOut.getPortCount(); ++i)
            names << QString::fromStdString(midiOut.getPortName(i));
    } catch (RtMidiError &e) {
        qWarning() << "MidiOutput: could not enumerate ports:" << e.what();
    }
    return names;
}

QString configuredDevice()
{
    QSettings s;
    return s.value("midi/outputDevice").toString();
}

void setConfiguredDevice(const QString &name)
{
    QSettings s;
    s.setValue("midi/outputDevice", name);
}

int configuredDeviceIndex(QString *errorOut)
{
    const QStringList ports = availablePorts();
    if (ports.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("No MIDI output ports found.");
        return -1;
    }

    const QString target = configuredDevice();
    if (!target.isEmpty() && !ports.contains(target)) {
        if (errorOut) *errorOut =
            QStringLiteral("Configured MIDI device \"%1\" is not available.\n\n"
                           "Available ports:\n%2\n\n"
                           "Pick the VL70-m's port in Settings (Ctrl+3) → MIDI Setup.")
                .arg(target, ports.join("\n"));
        return -1;
    }

    return target.isEmpty() ? 0 : ports.indexOf(target);
}

bool openConfiguredPort(RtMidiOut &out, QString *errorOut)
{
    QString error;
    const int portIndex = configuredDeviceIndex(&error);
    if (portIndex < 0) {
        if (errorOut) *errorOut = error;
        return false;
    }

    try {
        out.openPort(static_cast<unsigned int>(portIndex));
    } catch (RtMidiError &e) {
        if (errorOut) *errorOut = QStringLiteral("Could not open MIDI output port:\n%1").arg(e.what());
        return false;
    }
    qDebug() << "MidiOutput: opened port" << portIndex;
    return true;
}

QString sendTestNote()
{
    auto midiOut = QSharedPointer<RtMidiOut>::create();
    QString error;
    if (!openConfiguredPort(*midiOut, &error))
        return error;

    // Breath (CC2) before note-on: the VL70-m's physical model is excited by
    // breath, and a voice can be silent without it.
    const std::vector<unsigned char> breath  = {0xB0, 0x02, 100};
    const std::vector<unsigned char> noteOn  = {0x90, 69, 100};  // A4, velocity 100
    const std::vector<unsigned char> noteOff = {0x80, 69, 0};
    midiOut->sendMessage(&breath);
    midiOut->sendMessage(&noteOn);

    // Hold one beat (500 ms at 120 BPM), then note off. The shared pointer
    // keeps the port open until the note-off has been sent.
    QTimer::singleShot(500, [midiOut, noteOff]() {
        midiOut->sendMessage(&noteOff);
    });

    return QString();
}

std::vector<unsigned char> cc(int channel, int ccNum, int value)
{
    return {static_cast<unsigned char>(0xB0 | (channel - 1)),
            static_cast<unsigned char>(ccNum),
            static_cast<unsigned char>(value)};
}

std::vector<std::vector<unsigned char>> nrpn(int channel, int msb, int lsb,
                                             int valueMsb, int valueLsb)
{
    std::vector<std::vector<unsigned char>> msgs;
    msgs.push_back(cc(channel, 99, msb));
    msgs.push_back(cc(channel, 98, lsb));
    msgs.push_back(cc(channel, 6, valueMsb));
    if (valueLsb >= 0)
        msgs.push_back(cc(channel, 38, valueLsb));
    return msgs;
}

std::vector<unsigned char> parameterChange(const QByteArray &address, const QByteArray &data)
{
    std::vector<unsigned char> msg = {0xF0, 0x43, 0x10, 0x57};
    for (char b : address) msg.push_back(static_cast<unsigned char>(b));
    for (char b : data) msg.push_back(static_cast<unsigned char>(b));
    msg.push_back(0xF7);
    return msg;
}

std::vector<unsigned char> vlDepthBytes(int value)
{
    const unsigned char b = static_cast<unsigned char>(static_cast<signed char>(value));
    return {static_cast<unsigned char>((b >> 7) & 0x7F), static_cast<unsigned char>(b & 0x7F)};
}

} // namespace MidiOutput
