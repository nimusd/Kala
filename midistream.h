#ifndef MIDISTREAM_H
#define MIDISTREAM_H

#include "midioutput.h"  // MidiEvent

#include <QString>
#include <memory>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

// Serialize a baked, time-sorted MidiEvent list (absolute ms, first event at
// >= startTimeMs) into WinMM stream format: an array of MIDIEVENT records
// {dwDeltaTime, dwStreamID, dwEvent}. This is NOT a Standard MIDI File -
// midiStreamOut does not parse MThd/MTrk, it reads MIDIEVENTs directly.
//
// Timing: deltas are in ticks of llround(timeMs - startTimeMs), i.e. 1 tick =
// 1 ms. That mapping is established by MidiStreamPlayer::start(), which sets
// the stream's time division to 1000 PPQN and its tempo to 1,000,000
// us/quarter via midiStreamProperty - an embedded SMF tempo meta event would
// mean nothing to the stream API.
//
// Messages of 1-3 bytes become MEVT_SHORTMSG; anything longer (SysEx) becomes
// MEVT_LONGMSG with its payload padded to a DWORD boundary.
//
// Returns a LIST of buffers, not one: midiStreamOut caps a single buffer at
// 64K (~5400 short events, well under a real piece's bake), so the stream is
// split at record boundaries into chunks that are queued back-to-back. Delta
// times stay correct across the split - the stream clock runs continuously
// through queued buffers.
std::vector<std::vector<unsigned char>> buildMidiEventStream(
    const std::vector<MidiEvent> &events, double startTimeMs);

// Plays a baked event stream through Windows midiStreamOut. WinMM-only (the
// whole implementation is #ifdef _WIN32). Owns the HMIDISTRM and one MIDIHDR
// per queued buffer; stop() halts the stream, sends stopMessages (baked reset
// neutrals + All Notes Off) via midiOutShortMsg, then closes everything.
// Runs on the main thread only - the audio callback never touches this class.
class MidiStreamPlayer
{
public:
    MidiStreamPlayer() = default;
    ~MidiStreamPlayer();  // calls stop()

    // buffers: from buildMidiEventStream(). deviceId: 0-based WinMM MIDI
    // output device ID (equals the RtMidi port index - see
    // MidiOutput::configuredDeviceIndex). stopMessages: sent on stop() after
    // the stream halts. Returns false with a readable error.
    bool start(const std::vector<std::vector<unsigned char>> &buffers, int deviceId,
               const std::vector<std::vector<unsigned char>> &stopMessages,
               QString *errorOut = nullptr);

    // Idempotent: midiStreamStop -> midiOutReset (returns queued buffers) ->
    // stop messages -> midiOutUnprepareHeader per buffer (errors tolerated) ->
    // midiStreamClose.
    void stop();

    bool isActive() const { return m_active; }

private:
#ifdef _WIN32
    static DWORD packMessage(const std::vector<unsigned char> &msg);  // little-endian DWORD for midiOutShortMsg
    static QString mmErrorText(MMRESULT res);
    void teardown();  // close whatever is open, no stop messages

    HMIDISTRM m_stream = nullptr;
    // unique_ptr so the addresses handed to the driver stay put regardless of
    // what the owning vector does.
    std::vector<std::unique_ptr<MIDIHDR>> m_headers;
#endif
    std::vector<std::vector<unsigned char>> m_buffers;  // owns the memory m_headers point at
    std::vector<std::vector<unsigned char>> m_stopMessages;
    bool m_active = false;
};

#endif // MIDISTREAM_H
