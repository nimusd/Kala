#include "midistream.h"

#include <QDebug>

#ifdef _WIN32

#include <algorithm>
#include <cmath>

namespace {

// midiStreamOut caps a single buffer at 64K, so chunk well below that and
// split only at record boundaries.
constexpr size_t kMaxStreamBufferBytes = 48 * 1024;

// 1 tick = 1 ms: 1000 ticks per quarter at 1,000,000 us per quarter.
constexpr DWORD kStreamTimeDiv = 1000;
constexpr DWORD kStreamTempo = 1000000;

void appendDword(std::vector<unsigned char> &out, DWORD v)
{
    // The driver reads this buffer back as native MIDIEVENTs; x86/x64 is
    // little-endian.
    out.push_back(static_cast<unsigned char>(v & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 24) & 0xFF));
}

// MIDIEVENT: dwDeltaTime, dwStreamID (always 0), dwEvent.
void appendEventHeader(std::vector<unsigned char> &out, DWORD delta, DWORD event)
{
    appendDword(out, delta);
    appendDword(out, 0);
    appendDword(out, event);
}

} // namespace

std::vector<std::vector<unsigned char>> buildMidiEventStream(
    const std::vector<MidiEvent> &events, double startTimeMs)
{
    std::vector<std::vector<unsigned char>> buffers;
    std::vector<unsigned char> current;
    std::vector<unsigned char> record;
    long long prevTick = 0;

    for (const MidiEvent &ev : events) {
        if (ev.message.empty())
            continue;

        // Monotonic: the bake stable_sorts the list non-decreasing in timeMs.
        const long long tick = std::llround(ev.timeMs - startTimeMs);
        const DWORD delta = static_cast<DWORD>(std::max<long long>(tick - prevTick, 0));
        prevTick = tick;

        record.clear();
        if (ev.message.size() <= 3) {
            // MEVT_SHORTMSG (type 0x00): message bytes in the low 24 bits,
            // status byte first.
            DWORD packed = 0;
            for (size_t i = 0; i < ev.message.size(); ++i)
                packed |= static_cast<DWORD>(ev.message[i]) << (8 * i);
            appendEventHeader(record, delta,
                              (static_cast<DWORD>(MEVT_SHORTMSG) << 24) | (packed & 0x00FFFFFF));
        } else {
            // MEVT_LONGMSG (type 0x80): low 24 bits are the byte count, the
            // payload follows, padded out to a DWORD boundary.
            appendEventHeader(record, delta,
                              (static_cast<DWORD>(MEVT_LONGMSG) << 24)
                                  | (static_cast<DWORD>(ev.message.size()) & 0x00FFFFFF));
            record.insert(record.end(), ev.message.begin(), ev.message.end());
            while (record.size() % 4 != 0)
                record.push_back(0);
        }

        if (!current.empty() && current.size() + record.size() > kMaxStreamBufferBytes) {
            buffers.push_back(std::move(current));
            current.clear();
        }
        current.insert(current.end(), record.begin(), record.end());
    }
    if (!current.empty())
        buffers.push_back(std::move(current));
    return buffers;
}

MidiStreamPlayer::~MidiStreamPlayer()
{
    stop();
}

bool MidiStreamPlayer::start(const std::vector<std::vector<unsigned char>> &buffers, int deviceId,
                             const std::vector<std::vector<unsigned char>> &stopMessages,
                             QString *errorOut)
{
    stop();  // defensive re-entry

    if (buffers.empty()) {
        if (errorOut) *errorOut = QStringLiteral("Empty MIDI stream data.");
        return false;
    }

    m_buffers = buffers;
    m_stopMessages = stopMessages;

    UINT devId = static_cast<UINT>(deviceId);

    // Only the stream handle is opened: most drivers are exclusive-access, so
    // a second midiOutOpen on the same device risks MMSYSERR_ALLOCATED.
    // HMIDISTRM casts to HMIDIOUT for the midiOut* calls.
    MMRESULT res = midiStreamOpen(&m_stream, &devId, 1, 0, 0, CALLBACK_NULL);
    if (res != MMSYSERR_NOERROR) {
        m_stream = nullptr;
        if (errorOut) *errorOut = mmErrorText(res);
        teardown();
        return false;
    }

    // Tick rate. Without these the stream runs at the 96 PPQN / 500,000 us
    // default and every baked timestamp is wrong.
    MIDIPROPTIMEDIV timeDiv{sizeof(MIDIPROPTIMEDIV), kStreamTimeDiv};
    res = midiStreamProperty(m_stream, reinterpret_cast<LPBYTE>(&timeDiv),
                             MIDIPROP_SET | MIDIPROP_TIMEDIV);
    if (res != MMSYSERR_NOERROR) {
        if (errorOut) *errorOut = mmErrorText(res);
        teardown();
        return false;
    }

    MIDIPROPTEMPO tempo{sizeof(MIDIPROPTEMPO), kStreamTempo};
    res = midiStreamProperty(m_stream, reinterpret_cast<LPBYTE>(&tempo),
                             MIDIPROP_SET | MIDIPROP_TEMPO);
    if (res != MMSYSERR_NOERROR) {
        if (errorOut) *errorOut = mmErrorText(res);
        teardown();
        return false;
    }

    // Prepare and queue every chunk before restarting; the driver plays them
    // back-to-back on one continuous clock, so deltas carry across the splits.
    m_headers.reserve(m_buffers.size());
    for (std::vector<unsigned char> &buf : m_buffers) {
        auto hdr = std::make_unique<MIDIHDR>();
        ZeroMemory(hdr.get(), sizeof(MIDIHDR));
        hdr->lpData = reinterpret_cast<LPSTR>(buf.data());
        hdr->dwBufferLength = static_cast<DWORD>(buf.size());
        // Output reads dwBytesRecorded for how much to actually send.
        hdr->dwBytesRecorded = static_cast<DWORD>(buf.size());

        res = midiOutPrepareHeader(reinterpret_cast<HMIDIOUT>(m_stream), hdr.get(), sizeof(MIDIHDR));
        if (res != MMSYSERR_NOERROR) {
            if (errorOut) *errorOut = mmErrorText(res);
            teardown();
            return false;
        }

        res = midiStreamOut(m_stream, hdr.get(), sizeof(MIDIHDR));
        if (res != MMSYSERR_NOERROR) {
            if (errorOut) *errorOut = mmErrorText(res);
            midiOutUnprepareHeader(reinterpret_cast<HMIDIOUT>(m_stream), hdr.get(), sizeof(MIDIHDR));
            teardown();
            return false;
        }
        m_headers.push_back(std::move(hdr));
    }

    // midiStreamOpen leaves the device paused - without this, nothing plays.
    res = midiStreamRestart(m_stream);
    if (res != MMSYSERR_NOERROR) {
        if (errorOut) *errorOut = mmErrorText(res);
        teardown();
        return false;
    }

    m_active = true;
    return true;
}

void MidiStreamPlayer::stop()
{
    if (m_stream) {
        // Always stop before close - closing a playing stream can hang some
        // drivers - and midiStreamStop itself turns off all notes as a
        // backstop. midiOutReset then hands back every queued buffer, so the
        // unprepare in teardown() cannot fail with MIDIERR_STILLPLAYING.
        midiStreamStop(m_stream);
        midiOutReset(reinterpret_cast<HMIDIOUT>(m_stream));
        // Data-driven close reset (rows architecture): reset neutrals + All
        // Notes Off go out before the port closes.
        for (const auto &msg : m_stopMessages)
            midiOutShortMsg(reinterpret_cast<HMIDIOUT>(m_stream), packMessage(msg));
    }
    teardown();
}

void MidiStreamPlayer::teardown()
{
    if (m_stream) {
        for (auto &hdr : m_headers)
            midiOutUnprepareHeader(reinterpret_cast<HMIDIOUT>(m_stream), hdr.get(), sizeof(MIDIHDR));
        midiStreamClose(m_stream);
        m_stream = nullptr;
    }
    m_headers.clear();
    m_buffers.clear();
    m_stopMessages.clear();
    m_active = false;
}

DWORD MidiStreamPlayer::packMessage(const std::vector<unsigned char> &msg)
{
    DWORD packed = 0;
    for (size_t i = 0; i < msg.size() && i < 4; ++i)
        packed |= static_cast<DWORD>(msg[i]) << (8 * i);
    return packed;
}

QString MidiStreamPlayer::mmErrorText(MMRESULT res)
{
    wchar_t buf[MAXERRORLENGTH] = {0};
    midiOutGetErrorTextW(res, buf, MAXERRORLENGTH);
    return QStringLiteral("MIDI stream error %1: %2").arg(res).arg(QString::fromWCharArray(buf));
}

#else // !_WIN32

std::vector<std::vector<unsigned char>> buildMidiEventStream(
    const std::vector<MidiEvent> &, double)
{
    return {};
}

MidiStreamPlayer::~MidiStreamPlayer() {}

bool MidiStreamPlayer::start(const std::vector<std::vector<unsigned char>> &, int,
                             const std::vector<std::vector<unsigned char>> &,
                             QString *errorOut)
{
    if (errorOut) *errorOut = QStringLiteral("MIDI stream playback is Windows-only.");
    return false;
}

void MidiStreamPlayer::stop()
{
    m_active = false;
    m_buffers.clear();
    m_stopMessages.clear();
}

#endif // _WIN32
