#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QObject>
#include <QString>
#include <QList>
#include <atomic>

/**
 * AudioCapture - records an audio input device to a 16-bit PCM WAV file
 * while the transport plays (Phase 9, bake to audio). This is how the
 * VL70-m's audio, arriving through the interface (UR22) input, comes back
 * into Kala and gets attached to a track as a clip.
 *
 * Backend: Windows WinMM waveIn - the same driver stack the VL70-m MIDI
 * path already uses. RtAudio was tried first and rejected: its WASAPI
 * probe can transiently drop or wipe the whole device list (probeDevices
 * clears deviceList_ on its exit error path), while waveInGetNumDevs is a
 * plain stable enumeration - it cannot fail that way.
 *
 * Incoming buffers go into a queue drained by a writer thread that feeds
 * dr_wav - disk I/O never blocks the WinMM callback thread. The waveIn
 * state lives behind m_impl (WinMM types stay out of this moc'd header).
 */
class AudioCapture : public QObject
{
    Q_OBJECT

public:
    enum ChannelMode {
        Stereo,  // L/R as captured
        Input1,  // input 1 copied to both channels
        Input2   // input 2 copied to both channels
    };

    /**
     * An input-capable device, identified by NAME. WinMM device IDs are
     * stable, but selection by name keeps the UI and the capture self-
     * documenting and immune to enumeration-order surprises.
     */
    struct CaptureDevice {
        QString name;         // raw driver name (used for selection)
        QString displayName;  // what the UI shows
    };

    explicit AudioCapture(QObject *parent = nullptr);
    ~AudioCapture();

    /**
     * Snapshot of the system's input-capable waveIn devices.
     */
    static QList<CaptureDevice> listInputDevices();

    /**
     * Open the input device and start writing to filePath.
     * The WAV is always stereo 16-bit PCM; the sample rate is the
     * composition rate if the device accepts it, else 48000/44100
     * (the actual rate is available via getSampleRate()).
     */
    bool beginCapture(const QString &filePath, const QString &deviceName,
                      unsigned int sampleRate, ChannelMode mode, QString &errorOut);

    /**
     * Stop the stream, drain the queue and finalize the WAV.
     * Returns the number of frames captured (0 if capture failed or never ran).
     * Safe to call when not capturing.
     */
    unsigned long long endCapture(QString &errorOut);

    bool isCapturing() const { return m_active.load(); }

    /**
     * The sample rate the recording was actually made at (set after a
     * successful beginCapture).
     */
    unsigned int getSampleRate() const { return m_sampleRate; }

    /**
     * Onset analysis on a finished recording: onsetMs = first sustained
     * crossing of the detection threshold, lastSignalMs = last time the
     * signal is above it. foundOnset is false (and both times 0) if the
     * file is silent or unreadable. Detection runs on a short-block RMS
     * envelope with a threshold derived from the quiet head's noise floor
     * (soft attacks start far below a fixed -40 dBFS and are missed); the
     * scan skips the first 50 ms (stream-open click region) and the onset
     * must persist (~50 ms) so a click does not count as signal. A
     * 20%-of-peak single crossing remains as a fallback. peakFs receives
     * the recording's peak magnitude (0-1), for level feedback in the UI.
     */
    static void analyzeClipBounds(const QString &filePath, double &onsetMs, double &lastSignalMs,
                                  bool &foundOnset, float &peakFs);

private:
    void *m_impl = nullptr;  // WinMM capture state (defined in the .cpp)
    std::atomic<bool> m_active{false};
    unsigned int m_sampleRate = 0;
    unsigned long long m_framesCaptured = 0;
};

#endif // AUDIOCAPTURE_H
