#ifndef BANDWIDTHTESTDIALOG_H
#define BANDWIDTHTESTDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QSet>
#include <QString>
#include <vector>

class RtMidiOut;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPlainTextEdit;
class QPushButton;
class QLabel;

/**
 * VL70-m bandwidth ceiling test (Phase 4).
 *
 * Pushes simultaneous NRPN / SysEx / CC parameter streams at the configured
 * MIDI output while a note sustains, ramping stream count x interval until
 * audible lag appears. The output is a measured budget for Phase 5: how many
 * streams, at what interval, before anything lags. Diagnostic tool only -
 * owns its own RtMidiOut and does not touch playback.
 *
 * Route notes:
 *  - NRPN: non-destructive offsets added to voice data.
 *  - SysEx DEPTH: streams Table 9 element DEPTH changes; leaves the current
 *    voice edit buffer at the last streamed value (restore = re-select the
 *    voice or power-cycle).
 *  - CC: writes CONTROL NO. = CC + DEPTH = max into the edit buffer once at
 *    start (same restore story), then streams plain 3-byte CCs.
 */
class BandwidthTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandwidthTestDialog(QWidget *parent = nullptr);
    ~BandwidthTestDialog() override;

protected:
    // All termination (X, Esc, Stop) routes through here: idempotent cleanup.
    void done(int result) override;

private slots:
    void onRouteChanged(int index);
    void onStartClicked();
    void onRunLadderClicked();
    void onAudibilityClicked();
    void onCleanNextClicked();
    void onLagNextClicked();
    void onStopClicked();
    void onPanicClicked();
    void onStreamTick();
    void onBaselineTick();

private:
    enum class Route { NRPN, SysEx, CC };

    struct ParamDef {
        QString name;
        int a;  // NRPN: msb | SysEx: al (base 20 00) | CC: cc number
        int b;  // NRPN: lsb | CC: Table 9 CONTROL NO. address | SysEx: unused
    };

    struct Stream {
        ParamDef param;
        double phase;  // waveform phase accumulator (radians)
    };

    struct LadderStep {
        int streams;
        int intervalMs;
    };

    // One tick of a single stream: waveform sample -> MIDI bytes -> port.
    void sendStreamSample(Stream &stream, double intervalMs);

    void startStreaming(bool ladder, bool audibility);  // common Start path
    void startStep(int stepIndex);      // -1 = manual
    void stopStreaming();               // stop timers, log stats
    void silenceModule();               // CC123, CC120, CC13-20=64, CC121, note-off
    void sendCcAssignSetup();           // Table 9 CONTROL NO. + DEPTH writes, hex-logged
    void sendMessage(const std::vector<unsigned char> &msg);
    void logLine(const QString &text);
    static QString hexString(const std::vector<unsigned char> &msg);

    RtMidiOut *m_out = nullptr;
    QTimer m_streamTimer;      // one tick per step interval
    QTimer m_baselineTimer;    // breath + bend sines, fixed 10 ms
    QElapsedTimer m_stepClock;

    QComboBox *m_routeCombo;
    QComboBox *m_paramCombo;
    QComboBox *m_waveCombo;
    QDoubleSpinBox *m_waveRateSpin;   // Hz
    QSpinBox *m_swingSpin;            // % of full swing
    QSpinBox *m_streamsSpin;          // manual stream count (1 = selected param, N = first N)
    QSpinBox *m_intervalSpin;         // manual interval, ms
    QCheckBox *m_baselineCheck;
    QCheckBox *m_extremeCheck;        // allow the 1 ms probe step
    QPushButton *m_startBtn;
    QPushButton *m_ladderBtn;
    QPushButton *m_audibilityBtn;
    QPushButton *m_cleanBtn;
    QPushButton *m_lagBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_panicBtn;
    QLabel *m_statusLabel;
    QPlainTextEdit *m_log;

    QVector<ParamDef> m_params;   // table for the current route
    QVector<Stream> m_streams;    // active streams
    QSet<int> m_warnedRoutes;     // edit-buffer warning shown once per route
    int m_step = -1;              // ladder step index, -1 = manual/audibility
    int m_firstLagStep = -1;      // first ladder step marked as lagging
    double m_intervalMs = 12.0;   // current stream interval
    bool m_running = false;
    bool m_audibilityMode = false;
    bool m_noteOn = false;
    bool m_ccAssigned = false;
    bool m_cleanedUp = false;

    // Per-step stats
    qint64 m_stepTicks = 0;
    qint64 m_stepBytes = 0;
    qint64 m_stepOverruns = 0;
    double m_baselinePhase = 0.0;
    double m_baselineBendPhase = 0.0;
};

#endif // BANDWIDTHTESTDIALOG_H
