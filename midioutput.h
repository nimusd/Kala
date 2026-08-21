#ifndef MIDIOUTPUT_H
#define MIDIOUTPUT_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <vector>

class RtMidiOut;

// MIDI output helpers for external hardware (VL70-m).

// A single timestamped MIDI message, baked before playback. Consumed by the
// MIDIEVENT stream writer (midistream.h) and the WinMM stream player; kept
// here so midistream does not have to include audioengine.h.
struct MidiEvent {
    double timeMs = 0.0;
    std::vector<unsigned char> message;
};

namespace MidiOutput {

// Names of all currently available MIDI output ports.
QStringList availablePorts();

// Configured output device name (QSettings key "midi/outputDevice").
// Empty if none has been chosen yet.
QString configuredDevice();

// Save the configured device name.
void setConfiguredDevice(const QString &name);

// Index of the configured device within availablePorts() (fallback: 0 when
// nothing is configured). -1 on failure (errorOut filled). The index equals
// the WinMM device ID - RtMidi enumerates the same order - so it feeds
// midiStreamOpen directly.
int configuredDeviceIndex(QString *errorOut = nullptr);

// Open the configured device (or the first available one if none is
// configured) in out. Returns true on success; on failure, fills errorOut.
bool openConfiguredPort(RtMidiOut &out, QString *errorOut = nullptr);

// Send the hello note (breath CC2, Note On A4, Note Off after 500 ms) to the
// configured device, or the first available one if none is configured.
// Returns an empty string on success or a user-readable error message.
QString sendTestNote();

// --- Message constructors (pure, reused by the bake and the bandwidth test) ---

// 3-byte control change on a 1-16 channel.
std::vector<unsigned char> cc(int channel, int ccNum, int value);

// NRPN data entry, as separate 3-byte messages. CC99/CC98 re-select the
// parameter on EVERY update (the module holds one NRPN register per channel,
// and interleaved streams must not land on each other's parameter), then CC6
// (+ optional CC38 LSB). Returned as a list because MIDI backends (RtMidi
// WinMM) reject messages > 3 bytes unless they are SysEx.
std::vector<std::vector<unsigned char>> nrpn(int channel, int msb, int lsb,
                                             int valueMsb, int valueLsb = -1);

// VL70-m native parameter change: F0 43 10 57 ah am al dd... F7 (device
// number 1; parameter changes carry no checksum - VL70mE2.md MIDI format).
// address = 3 bytes (e.g. {0x20, 0x00, 0x29} for GROWL CONTROL NO.),
// data = 1+ bytes.
std::vector<unsigned char> parameterChange(const QByteArray &address, const QByteArray &data);

// Table 9 element DEPTH 2-byte encoding: the signed 8-bit two's-complement
// byte of the value (-127..+127), split MSB-7-bits first (doc prints the
// range as "0101-007F"). The unsigned-char cast keeps >> 7 from
// sign-extending negative values.
std::vector<unsigned char> vlDepthBytes(int value);

} // namespace MidiOutput

#endif // MIDIOUTPUT_H
