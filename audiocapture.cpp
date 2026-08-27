#include "audiocapture.h"
#include "dr_wav.h"
#include <windows.h>
#include <mmsystem.h>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace {

constexpr int kBufferCount = 3;
constexpr size_t kBufferFrames = 4096;  // 16 KB per buffer at 2ch/16-bit

// All WinMM state lives here so this moc'd header's .h stays free of
// Windows types.
struct WaveCaptureImpl {
    HWAVEIN hWaveIn = nullptr;
    WAVEHDR headers[kBufferCount] = {};
    bool headersPrepared[kBufferCount] = {false, false, false};
    drwav *wav = nullptr;
    std::deque<short> queue;  // interleaved stereo s16 pairs pending disk write
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> quit{false};
    AudioCapture::ChannelMode mode = AudioCapture::Stereo;
    unsigned int channels = 2;  // channels opened on the device (1 or 2)
    std::thread writer;
    unsigned long long framesWritten = 0;
};

void CALLBACK captureProc(HWAVEIN, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR)
{
    WaveCaptureImpl *impl = reinterpret_cast<WaveCaptureImpl *>(instance);
    if (!impl || impl->quit.load()) return;
    if (msg != WIM_DATA) return;

    WAVEHDR *hdr = reinterpret_cast<WAVEHDR *>(param1);
    const short *data = reinterpret_cast<const short *>(hdr->lpData);
    const size_t frames = hdr->dwBytesRecorded / sizeof(short) / impl->channels;
    {
        std::lock_guard<std::mutex> lock(impl->mtx);
        for (size_t i = 0; i < frames; ++i) {
            short l, r;
            const short *f = data + i * impl->channels;
            if (impl->mode == AudioCapture::Input1) {
                l = r = f[0];
            } else if (impl->mode == AudioCapture::Input2 && impl->channels >= 2) {
                l = r = f[1];
            } else {
                l = f[0];
                r = impl->channels >= 2 ? f[1] : l;
            }
            impl->queue.push_back(l);
            impl->queue.push_back(r);
        }
    }
    impl->cv.notify_one();

    // Hand the buffer back for more data (fails harmlessly during teardown,
    // when waveInReset/close have already reclaimed it).
    waveInAddBuffer(impl->hWaveIn, hdr, sizeof(WAVEHDR));
}

void writerLoop(WaveCaptureImpl *impl)
{
    std::vector<short> chunk;
    chunk.reserve(kBufferFrames * 2 * 2);
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(impl->mtx);
            impl->cv.wait(lock, [impl] { return impl->quit.load() || !impl->queue.empty(); });
            if (impl->queue.empty() && impl->quit.load()) break;
            const size_t n = std::min<size_t>(impl->queue.size(), chunk.capacity());
            chunk.assign(impl->queue.begin(), impl->queue.begin() + n);
            impl->queue.erase(impl->queue.begin(), impl->queue.begin() + n);
        }
        impl->framesWritten += drwav_write_pcm_frames(impl->wav, chunk.size() / 2, chunk.data());
    }
}

QString mmErrorText(MMRESULT r)
{
    switch (r) {
    case MMSYSERR_BADDEVICEID: return "bad device id";
    case MMSYSERR_ALLOCATED:   return "device is already in use";
    case MMSYSERR_NODRIVER:    return "no audio driver present";
    case MMSYSERR_NOMEM:       return "out of memory";
    case WAVERR_BADFORMAT:     return "unsupported format";
    default: return QString("error %1").arg(r);
    }
}

} // namespace

AudioCapture::AudioCapture(QObject *parent)
    : QObject(parent)
{
}

AudioCapture::~AudioCapture()
{
    QString ignored;
    endCapture(ignored);
}

QList<AudioCapture::CaptureDevice> AudioCapture::listInputDevices()
{
    QList<CaptureDevice> out;
    const UINT count = waveInGetNumDevs();
    for (UINT i = 0; i < count; ++i) {
        WAVEINCAPSW caps;
        if (waveInGetDevCapsW(i, &caps, sizeof(caps)) != MMSYSERR_NOERROR) continue;
        CaptureDevice dev;
        dev.name = QString::fromWCharArray(caps.szPname);
        dev.displayName = dev.name;
        out.append(dev);
    }
    return out;
}

bool AudioCapture::beginCapture(const QString &filePath, const QString &deviceName,
                                unsigned int sampleRate, ChannelMode mode,
                                QString &errorOut)
{
    if (m_active.load()) {
        errorOut = "capture already in progress";
        return false;
    }
    if (filePath.isEmpty()) {
        errorOut = "no output file";
        return false;
    }

    // Resolve the device by name - WinMM ids are stable, but the name is
    // what the dialog carried and it keeps the pair self-consistent.
    int deviceId = -1;
    const UINT deviceCount = waveInGetNumDevs();
    for (UINT i = 0; i < deviceCount; ++i) {
        WAVEINCAPSW caps;
        if (waveInGetDevCapsW(i, &caps, sizeof(caps)) != MMSYSERR_NOERROR) continue;
        if (QString::fromWCharArray(caps.szPname) == deviceName) {
            deviceId = static_cast<int>(i);
            break;
        }
    }
    if (deviceId < 0) {
        errorOut = QString("input device \"%1\" is not available").arg(deviceName);
        return false;
    }

    WaveCaptureImpl *impl = new WaveCaptureImpl();
    impl->mode = mode;

    // 16-bit PCM. Try the composition rate first, then the standard rates;
    // two channels first, mono as a last resort (mono is upmixed to stereo).
    const unsigned int rates[] = { sampleRate, 48000, 44100 };
    MMRESULT openResult = MMSYSERR_ERROR;
    unsigned int openedRate = 0;
    for (unsigned int rate : rates) {
        for (unsigned int ch = 2; ch >= 1; --ch) {
            WAVEFORMATEX fmt = {};
            fmt.wFormatTag = WAVE_FORMAT_PCM;
            fmt.nChannels = static_cast<WORD>(ch);
            fmt.nSamplesPerSec = rate;
            fmt.wBitsPerSample = 16;
            fmt.nBlockAlign = static_cast<WORD>(ch * 2);
            fmt.nAvgBytesPerSec = rate * fmt.nBlockAlign;
            openResult = waveInOpen(&impl->hWaveIn, static_cast<UINT>(deviceId), &fmt,
                                    reinterpret_cast<DWORD_PTR>(captureProc),
                                    reinterpret_cast<DWORD_PTR>(impl), CALLBACK_FUNCTION);
            if (openResult == MMSYSERR_NOERROR) {
                openedRate = rate;
                impl->channels = ch;
                break;
            }
            impl->hWaveIn = nullptr;
        }
        if (openResult == MMSYSERR_NOERROR) break;
    }
    if (openResult != MMSYSERR_NOERROR) {
        errorOut = QString("cannot open \"%1\": %2").arg(deviceName, mmErrorText(openResult));
        delete impl;
        return false;
    }

    // WAV: stereo 16-bit PCM at the actual capture rate.
    drwav_data_format fmt;
    fmt.container = drwav_container_riff;
    fmt.format = DR_WAVE_FORMAT_PCM;
    fmt.channels = 2;
    fmt.sampleRate = openedRate;
    fmt.bitsPerSample = 16;
    drwav *wav = new drwav();
    if (!drwav_init_file_write(wav, filePath.toLocal8Bit().constData(), &fmt, nullptr)) {
        waveInClose(impl->hWaveIn);
        delete wav;
        delete impl;
        errorOut = QString("cannot create %1").arg(filePath);
        return false;
    }
    impl->wav = wav;

    // Prepare and queue the capture buffers
    bool buffersOk = true;
    for (int i = 0; i < kBufferCount; ++i) {
        WAVEHDR *hdr = &impl->headers[i];
        hdr->lpData = reinterpret_cast<LPSTR>(new short[kBufferFrames * 2]);
        hdr->dwBufferLength = static_cast<DWORD>(kBufferFrames * 2 * sizeof(short));
        hdr->dwFlags = 0;
        if (waveInPrepareHeader(impl->hWaveIn, hdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            buffersOk = false;
            break;
        }
        impl->headersPrepared[i] = true;
        if (waveInAddBuffer(impl->hWaveIn, hdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            buffersOk = false;
            break;
        }
    }
    if (!buffersOk) {
        for (int i = 0; i < kBufferCount; ++i) {
            if (impl->headersPrepared[i]) {
                waveInUnprepareHeader(impl->hWaveIn, &impl->headers[i], sizeof(WAVEHDR));
            }
            delete[] reinterpret_cast<short *>(impl->headers[i].lpData);
        }
        waveInClose(impl->hWaveIn);
        drwav_uninit(wav);
        delete wav;
        delete impl;
        errorOut = "could not prepare capture buffers";
        return false;
    }

    if (waveInStart(impl->hWaveIn) != MMSYSERR_NOERROR) {
        for (int i = 0; i < kBufferCount; ++i) {
            if (impl->headersPrepared[i]) {
                waveInUnprepareHeader(impl->hWaveIn, &impl->headers[i], sizeof(WAVEHDR));
            }
            delete[] reinterpret_cast<short *>(impl->headers[i].lpData);
        }
        waveInClose(impl->hWaveIn);
        drwav_uninit(wav);
        delete wav;
        delete impl;
        errorOut = QString("could not start capture: %1").arg(mmErrorText(openResult));
        return false;
    }

    impl->writer = std::thread(writerLoop, impl);
    m_impl = impl;
    m_sampleRate = openedRate;
    m_framesCaptured = 0;
    m_active.store(true);
    qDebug() << "AudioCapture: recording from" << deviceName
             << "to" << filePath << "at" << openedRate << "Hz";
    return true;
}

unsigned long long AudioCapture::endCapture(QString &errorOut)
{
    if (!m_active.load()) return m_framesCaptured;

    m_active.store(false);
    WaveCaptureImpl *impl = static_cast<WaveCaptureImpl *>(m_impl);
    m_impl = nullptr;
    if (!impl) return m_framesCaptured;

    // Stop feeding the queue BEFORE resetting the device, so the callback
    // (which may still run once) neither pushes nor re-queues buffers.
    impl->quit.store(true);
    waveInReset(impl->hWaveIn);
    waveInStop(impl->hWaveIn);
    waveInClose(impl->hWaveIn);
    impl->cv.notify_all();
    if (impl->writer.joinable()) impl->writer.join();

    for (int i = 0; i < kBufferCount; ++i) {
        if (impl->headersPrepared[i]) {
            waveInUnprepareHeader(impl->hWaveIn, &impl->headers[i], sizeof(WAVEHDR));
        }
        delete[] reinterpret_cast<short *>(impl->headers[i].lpData);
    }
    if (impl->wav) {
        drwav_uninit(impl->wav);
        delete impl->wav;
    }
    m_framesCaptured = impl->framesWritten;
    delete impl;

    qDebug() << "AudioCapture: recorded" << m_framesCaptured << "frames";
    return m_framesCaptured;
}

void AudioCapture::analyzeClipBounds(const QString &filePath, double &onsetMs, double &lastSignalMs,
                                     bool &foundOnset, float &peakFs)
{
    onsetMs = 0.0;
    lastSignalMs = 0.0;
    foundOnset = false;
    peakFs = 0.0f;

    drwav wav;
    if (!drwav_init_file(&wav, filePath.toLocal8Bit().constData(), nullptr)) return;
    const drwav_uint64 frames = wav.totalPCMFrameCount;
    const unsigned int channels = wav.channels;
    if (frames == 0 || channels < 1) {
        drwav_uninit(&wav);
        return;
    }
    std::vector<float> buf(static_cast<size_t>(frames) * channels);
    const drwav_uint64 read = drwav_read_pcm_frames_f32(&wav, frames, buf.data());
    const double rate = wav.sampleRate > 0 ? static_cast<double>(wav.sampleRate) : 48000.0;
    drwav_uninit(&wav);
    if (read == 0) return;

    auto frameMag = [&](drwav_uint64 f) {
        float m = 0.0f;
        for (unsigned int c = 0; c < channels; ++c) {
            m = std::max(m, std::fabs(buf[f * channels + c]));
        }
        return m;
    };

    float peak = 0.0f;
    for (drwav_uint64 i = 0; i < read * channels; ++i) {
        peak = std::max(peak, std::fabs(buf[i]));
    }
    peakFs = peak;
    if (peak < 0.005f) return;  // silent

    // Noise floor from the quiet head. The recording starts before the
    // transport, so the first instants are pure pre-roll silence/hiss; the
    // median is immune to the stream-open click (a short full-scale spike).
    const drwav_uint64 headFrames = std::min<drwav_uint64>(
        read, static_cast<drwav_uint64>(rate * 0.25));
    std::vector<float> head(static_cast<size_t>(headFrames));
    for (drwav_uint64 f = 0; f < headFrames; ++f) head[static_cast<size_t>(f)] = frameMag(f);
    std::nth_element(head.begin(), head.begin() + head.size() / 2, head.end());
    const float noiseFloor = head[head.size() / 2];

    // Detection runs on a short-block RMS envelope: a soft attack spends its
    // first hundreds of ms barely above the sample-wise threshold (the old
    // detector missed the first note of Kala no 19 entirely and fired 344 ms
    // late on the second). RMS smooths the waveform, so the persistence rule
    // tests ENERGY, not instantaneous crossings.
    const float threshold = std::max(noiseFloor * 2.0f, 0.002f);
    const drwav_uint64 hop = std::max<drwav_uint64>(1, static_cast<drwav_uint64>(rate * 0.005));
    std::vector<float> rms;
    rms.reserve(static_cast<size_t>(read / hop) + 2);
    for (drwav_uint64 f = 0; f + hop <= read; f += hop) {
        double acc = 0.0;
        for (drwav_uint64 g = f; g < f + hop; ++g) {
            const double m = static_cast<double>(frameMag(g));
            acc += m * m;
        }
        rms.push_back(static_cast<float>(std::sqrt(acc / static_cast<double>(hop))));
    }

    // The scan starts 50 ms in: the stream-open click (a WinMM waveInStart
    // transient) lives in the first tens of ms, and no module sound can
    // arrive before the transport has started and the MIDI + audio chain has
    // run its course.
    const drwav_uint64 skipFrames = std::min<drwav_uint64>(
        read, static_cast<drwav_uint64>(rate * 0.05));
    const size_t skipHops = static_cast<size_t>(skipFrames / hop);

    // The onset must persist: 6 of the next 10 hops (60% of 50 ms) above
    // threshold. A stream-open click decays within a few tens of ms and is
    // rejected; a note attack is sustained and accepted.
    constexpr size_t kPersistHops = 10;
    constexpr size_t kPersistNeed = 6;
    for (size_t h = skipHops; h < rms.size(); ++h) {
        if (rms[h] < threshold) continue;
        const size_t end = std::min(rms.size(), h + kPersistHops);
        size_t above = 0;
        for (size_t g = h; g < end; ++g) {
            if (rms[g] >= threshold) ++above;
        }
        if (above >= kPersistNeed || end - h < kPersistHops) {
            onsetMs = (static_cast<double>(h * hop) / rate) * 1000.0;
            foundOnset = true;
            break;
        }
    }

    // Safety net for recordings the persistence rule cannot classify (a
    // first note shorter than the window, a decay-only file): single
    // crossing at 20% of peak, the pre-fix behaviour.
    if (!foundOnset) {
        const float fallbackThr = std::max(0.01f, peak * 0.2f);
        for (drwav_uint64 f = skipFrames; f < read; ++f) {
            if (frameMag(f) >= fallbackThr) {
                onsetMs = (static_cast<double>(f) / rate) * 1000.0;
                foundOnset = true;
                break;
            }
        }
    }

    for (size_t h = rms.size(); h-- > 0;) {
        if (rms[h] >= threshold) {
            lastSignalMs = (static_cast<double>(h * hop) / rate) * 1000.0;
            break;
        }
    }
}
