#include "bandwidthtestdialog.h"
#include "midioutput.h"
#include <RtMidi.h>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTime>
#include <QDebug>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Note: A3 = 57, held for the whole test so parameter changes are audible.
static const int kNoteNum = 57;
static const int kChannel = 1;

// Waveform sample in [-1, 1] from a phase accumulator in radians.
static double waveValue(const QString &wave, double phase)
{
    const double f = std::fmod(phase / (2.0 * M_PI), 1.0);
    if (wave == "ramp") return 2.0 * f - 1.0;
    if (wave == "triangle") return 4.0 * std::abs(f - 0.5) - 1.0;
    return std::sin(phase);  // sine (default)
}

BandwidthTestDialog::BandwidthTestDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("VL70-m Bandwidth Test");
    resize(640, 560);

    // PreciseTimer: coarse timers clamp to ~15 ms on Windows and would wreck
    // every sub-15 ms ladder step.
    m_streamTimer.setTimerType(Qt::PreciseTimer);
    m_baselineTimer.setTimerType(Qt::PreciseTimer);
    m_baselineTimer.setInterval(10);

    m_routeCombo = new QComboBox();
    m_routeCombo->addItems({"NRPN (non-destructive offsets)",
                            "SysEx DEPTH (writes edit buffer)",
                            "CC (assign once, then 3-byte streams)"});

    m_paramCombo = new QComboBox();

    m_waveCombo = new QComboBox();
    m_waveCombo->addItems({"sine", "triangle", "ramp"});

    m_waveRateSpin = new QDoubleSpinBox();
    m_waveRateSpin->setRange(0.1, 20.0);
    m_waveRateSpin->setDecimals(1);
    m_waveRateSpin->setValue(2.0);
    m_waveRateSpin->setSuffix(" Hz");

    m_swingSpin = new QSpinBox();
    m_swingSpin->setRange(10, 100);
    m_swingSpin->setValue(100);
    m_swingSpin->setSuffix(" %");

    m_streamsSpin = new QSpinBox();
    m_streamsSpin->setRange(1, 8);
    m_streamsSpin->setValue(1);
    m_streamsSpin->setSuffix(" streams");

    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(2, 100);
    m_intervalSpin->setValue(12);
    m_intervalSpin->setSuffix(" ms");

    m_baselineCheck = new QCheckBox("Realistic baseline: breath + pitch bend "
                                    "sines @ 10 ms (~600 B/s, RPN range 12 st)");
    m_baselineCheck->setChecked(true);

    m_extremeCheck = new QCheckBox("Include the 1 ms extreme probe step "
                                   "(deep DIN saturation, UI may stutter)");

    QFormLayout *form = new QFormLayout();
    form->addRow("Route:", m_routeCombo);
    form->addRow("Parameter:", m_paramCombo);
    form->addRow("Waveform:", m_waveCombo);
    form->addRow("Wave rate:", m_waveRateSpin);
    form->addRow("Swing:", m_swingSpin);
    form->addRow("Manual streams:", m_streamsSpin);
    form->addRow("Manual interval:", m_intervalSpin);
    form->addRow("", m_baselineCheck);
    form->addRow("", m_extremeCheck);

    m_startBtn = new QPushButton("Start (manual)");
    m_ladderBtn = new QPushButton("Run ladder");
    m_audibilityBtn = new QPushButton("Audibility check");
    m_cleanBtn = new QPushButton("Clean — next");
    m_lagBtn = new QPushButton("Lag — next");
    m_stopBtn = new QPushButton("Stop");
    m_panicBtn = new QPushButton("Panic");
    m_cleanBtn->setEnabled(false);
    m_lagBtn->setEnabled(false);
    m_stopBtn->setEnabled(false);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(m_startBtn);
    btnRow->addWidget(m_ladderBtn);
    btnRow->addWidget(m_audibilityBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_cleanBtn);
    btnRow->addWidget(m_lagBtn);
    btnRow->addWidget(m_stopBtn);
    btnRow->addWidget(m_panicBtn);

    m_statusLabel = new QLabel("Idle. Sustains A3 on channel 1, device number "
                               "assumed 1 (factory default).");
    m_statusLabel->setWordWrap(true);

    m_log = new QPlainTextEdit();
    m_log->setReadOnly(true);

    QLabel *help = new QLabel(
        "Ladder: 1@50ms → 2@25 → 4@12 → 8@6 → 8@3 → [8@1]. DIN leg caps at "
        "3125 B/s — step 3 is the diagnostic boundary (lag on SysEx/NRPN but "
        "not CC there = transport-bound; CC lag at ~1000 B/s = module-bound). "
        "Manual mode streams 1 chosen parameter, or the route's first N with "
        "the streams control — use it to probe in-between rates (e.g. 4 CC "
        "streams @ 25 ms). Judgement: none / slight / obvious / unusable / "
        "silent.");
    help->setStyleSheet("color: gray; font-size: 10px;");
    help->setWordWrap(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addWidget(help);
    mainLayout->addLayout(btnRow);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_log, 1);

    connect(m_routeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BandwidthTestDialog::onRouteChanged);
    connect(m_startBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onStartClicked);
    connect(m_ladderBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onRunLadderClicked);
    connect(m_audibilityBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onAudibilityClicked);
    connect(m_cleanBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onCleanNextClicked);
    connect(m_lagBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onLagNextClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onStopClicked);
    connect(m_panicBtn, &QPushButton::clicked, this, &BandwidthTestDialog::onPanicClicked);
    connect(&m_streamTimer, &QTimer::timeout, this, &BandwidthTestDialog::onStreamTick);
    connect(&m_baselineTimer, &QTimer::timeout, this, &BandwidthTestDialog::onBaselineTick);

    onRouteChanged(0);
}

BandwidthTestDialog::~BandwidthTestDialog()
{
    // done() normally runs first (all terminations route through it); this is
    // the belt-and-braces path for abnormal destruction.
    if (!m_cleanedUp) {
        m_streamTimer.stop();
        m_baselineTimer.stop();
        if (m_out) {
            silenceModule();
            m_out->closePort();
            delete m_out;
            m_out = nullptr;
        }
    }
}

void BandwidthTestDialog::done(int result)
{
    if (!m_cleanedUp) {
        m_streamTimer.stop();
        m_baselineTimer.stop();
        if (m_out) {
            silenceModule();
            m_out->closePort();
            delete m_out;
            m_out = nullptr;
        }
        m_cleanedUp = true;
    }
    QDialog::done(result);
}

void BandwidthTestDialog::onRouteChanged(int index)
{
    Q_UNUSED(index);
    m_params.clear();
    m_paramCombo->clear();
    switch (static_cast<Route>(m_routeCombo->currentIndex())) {
    case Route::NRPN:
        // Non-destructive offsets added to voice data; msb/lsb pairs.
        m_params = {
            {"Vibrato Rate", 0x01, 0x08},
            {"Vibrato Depth", 0x01, 0x09},
            {"Filter Cutoff", 0x01, 0x20},
            {"Filter Resonance", 0x01, 0x21},
            {"Filter EG Depth", 0x01, 0x22},
            {"EG Attack", 0x01, 0x63},
            {"EG Decay", 0x01, 0x64},
            {"EG Release", 0x01, 0x66},
        };
        break;
    case Route::SysEx:
        // Table 9 element DEPTH addresses (base 20 00, al below).
        m_params = {
            {"Embouchure Upper", 0x18, 0},
            {"Embouchure Lower", 0x1A, 0},
            {"Growl Depth", 0x2A, 0},
            {"Throat Formant", 0x2E, 0},
            {"Harmonic Enhancer", 0x32, 0},
            {"Damping", 0x36, 0},
            {"Absorption", 0x3A, 0},
            {"Breath Noise", 0x26, 0},
        };
        break;
    case Route::CC:
        // a = CC number, b = CONTROL NO. address (DEPTH lives at b + 1).
        m_params = {
            {"Growl (CC13)", 13, 0x29},
            {"Breath Noise (CC14)", 14, 0x25},
            {"Damping (CC15)", 15, 0x35},
            {"Absorption (CC16)", 16, 0x39},
            {"Throat Formant (CC17)", 17, 0x2D},
            {"Harmonic Enhancer (CC18)", 18, 0x31},
            {"Tonguing (CC19)", 19, 0x1D},
            {"Scream (CC20)", 20, 0x21},
        };
        break;
    }
    for (const ParamDef &p : m_params)
        m_paramCombo->addItem(p.name);
}

void BandwidthTestDialog::startStreaming(bool ladder, bool audibility)
{
    const Route route = static_cast<Route>(m_routeCombo->currentIndex());

    if (!m_out) {
        m_out = new RtMidiOut();
        QString error;
        if (!MidiOutput::openConfiguredPort(*m_out, &error)) {
            QMessageBox::warning(this, "Bandwidth test", error);
            delete m_out;
            m_out = nullptr;
            return;
        }
    }

    // Table 9 writes land in the current voice edit buffer, not stored data.
    // Warn once per route per dialog session.
    const int routeIndex = m_routeCombo->currentIndex();
    if (route != Route::NRPN && !m_warnedRoutes.contains(routeIndex)) {
        const QString what = (route == Route::CC)
            ? "assigns element parameters to CCs"
            : "streams element DEPTH changes";
        const QMessageBox::StandardButton choice = QMessageBox::warning(
            this, "Bandwidth test",
            QString("This route %1 in the module's current voice edit buffer "
                    "(not the stored voice). To restore your patch, re-select "
                    "the voice on the VL70-m or power-cycle it.\n\nContinue?")
                .arg(what),
            QMessageBox::Ok | QMessageBox::Cancel);
        if (choice != QMessageBox::Ok)
            return;
        m_warnedRoutes.insert(routeIndex);
    }

    // One-time per-run setup
    if (m_baselineCheck->isChecked()) {
        // RPN 0,0 = 12 semitones, matching the baseline bend math.
        const std::vector<std::vector<unsigned char>> rpn = {
            MidiOutput::cc(kChannel, 101, 0), MidiOutput::cc(kChannel, 100, 0),
            MidiOutput::cc(kChannel, 6, 12),  MidiOutput::cc(kChannel, 38, 0),
            MidiOutput::cc(kChannel, 101, 127), MidiOutput::cc(kChannel, 100, 127)};
        for (const auto &msg : rpn)
            sendMessage(msg);
        m_baselineTimer.start();
    }
    if (route == Route::CC && !m_ccAssigned) {
        sendCcAssignSetup();
        m_ccAssigned = true;
    }
    if (!m_noteOn) {
        // Re-starting while the note already sounds must not re-send note-on
        // (the VL70-m re-triggers the attack on a new note-on).
        sendMessage(MidiOutput::cc(kChannel, 2, 100));  // static breath
        sendMessage({0x90, static_cast<unsigned char>(kNoteNum), 100});  // note-on
        m_noteOn = true;
    }

    m_running = true;
    m_audibilityMode = audibility;
    m_firstLagStep = -1;
    m_stepClock.restart();
    m_stepTicks = m_stepBytes = m_stepOverruns = 0;

    if (ladder) {
        m_cleanBtn->setEnabled(true);
        m_lagBtn->setEnabled(true);
        startStep(0);
    } else {
        m_step = -1;
        if (audibility) {
            // Single sweep on the selected parameter at a fixed 10 ms / 5 Hz.
            m_streams = {{m_params.at(m_paramCombo->currentIndex()), 0.0}};
            m_intervalMs = 10.0;
        } else {
            // 1 stream = the selected parameter; N streams = the route's
            // first N parameters (same table the ladder uses).
            m_streams.clear();
            const int n = m_streamsSpin->value();
            if (n <= 1) {
                m_streams.append({m_params.at(m_paramCombo->currentIndex()), 0.0});
            } else {
                for (int i = 0; i < n && i < m_params.size(); ++i)
                    m_streams.append({m_params.at(i), static_cast<double>(i) * 0.7});
            }
            m_intervalMs = static_cast<double>(m_intervalSpin->value());
        }
        m_streamTimer.setInterval(static_cast<int>(m_intervalMs));
        m_streamTimer.start();
        logLine(audibility
                    ? QString("Audibility check: %1, sine 5 Hz for 4 s — listen for the sweep")
                          .arg(m_params.at(m_paramCombo->currentIndex()).name)
                    : QString("Manual: %1 stream(s), %2 wave %3 Hz, interval %4 ms, swing %5%")
                          .arg(m_streams.size())
                          .arg(m_waveCombo->currentText())
                          .arg(m_waveRateSpin->value())
                          .arg(m_intervalSpin->value())
                          .arg(m_swingSpin->value()));
    }

    m_stopBtn->setEnabled(true);
    m_startBtn->setEnabled(false);
    m_ladderBtn->setEnabled(false);
    m_audibilityBtn->setEnabled(false);
    m_routeCombo->setEnabled(false);
    m_paramCombo->setEnabled(false);
    m_streamsSpin->setEnabled(false);
    m_intervalSpin->setEnabled(false);
}

void BandwidthTestDialog::startStep(int stepIndex)
{
    // Stream count x interval per step. The last step (1 ms) is an opt-in
    // extreme probe - it saturates the DIN leg hard and can stall the UI.
    static const LadderStep kLadder[] = {
        {1, 50}, {2, 25}, {4, 12}, {8, 6}, {8, 3}, {8, 1}
    };
    const int stepCount = m_extremeCheck->isChecked() ? 6 : 5;
    if (stepIndex >= stepCount) {
        // Ladder complete
        m_cleanBtn->setEnabled(false);
        m_lagBtn->setEnabled(false);
        const QString firstLag = (m_firstLagStep >= 0)
            ? QString("step %1").arg(m_firstLagStep + 1)
            : QString("none through step %1").arg(stepCount);
        logLine(QString("SUMMARY | route=%1 | first lag: %2 | use that step's "
                        "configuration as the Phase 5 budget; note kept sounding "
                        "— Stop to close the port")
                    .arg(m_routeCombo->currentText(), firstLag));
        stopStreaming();
        return;
    }

    m_step = stepIndex;
    const LadderStep &s = kLadder[stepIndex];
    m_intervalMs = s.intervalMs;

    m_streams.clear();
    for (int i = 0; i < s.streams && i < m_params.size(); ++i)
        m_streams.append({m_params.at(i), static_cast<double>(i) * 0.7});

    m_stepClock.restart();
    m_stepTicks = m_stepBytes = m_stepOverruns = 0;
    m_streamTimer.setInterval(s.intervalMs);
    m_streamTimer.start();

    const int bytesPerUpdate = (static_cast<Route>(m_routeCombo->currentIndex()) == Route::NRPN) ? 9
                             : (static_cast<Route>(m_routeCombo->currentIndex()) == Route::SysEx) ? 10
                             : 3;
    const double targetBps = bytesPerUpdate * s.streams * (1000.0 / s.intervalMs)
                           + (m_baselineCheck->isChecked() ? 600.0 : 0.0);
    logLine(QString("step %1/%2 | %3 streams @ %4 ms | %5 B/update | target ~%6 B/s "
                    "(DIN cap 3125 B/s) — ~8 s dwell, then judge")
                .arg(stepIndex + 1).arg(stepCount)
                .arg(s.streams).arg(s.intervalMs)
                .arg(bytesPerUpdate).arg(targetBps, 0, 'f', 0));
    m_statusLabel->setText(QString("Step %1/%2 running: %3 streams @ %4 ms")
                               .arg(stepIndex + 1).arg(stepCount)
                               .arg(s.streams).arg(s.intervalMs));
}

void BandwidthTestDialog::sendStreamSample(Stream &stream, double intervalMs)
{
    const double wave = waveValue(m_waveCombo->currentText(), stream.phase);
    stream.phase += 2.0 * M_PI
                  * (m_audibilityMode ? 5.0 : m_waveRateSpin->value())
                  * intervalMs / 1000.0;

    const double swing = m_swingSpin->value() / 100.0;
    switch (static_cast<Route>(m_routeCombo->currentIndex())) {
    case Route::NRPN: {
        const int v = qBound(0, static_cast<int>(std::lround(64.0 + 32.0 * swing * wave)), 127);
        // NRPN ships as three 3-byte CCs (backends reject >3-byte non-SysEx).
        for (const auto &msg : MidiOutput::nrpn(kChannel, stream.param.a, stream.param.b, v))
            sendMessage(msg);
        break;
    }
    case Route::SysEx: {
        const int v = qBound(-127, static_cast<int>(std::lround(80.0 * swing * wave)), 127);
        QByteArray addr(3, 0);
        addr[0] = 0x20;
        addr[1] = 0x00;
        addr[2] = static_cast<char>(stream.param.a);
        const std::vector<unsigned char> depth = MidiOutput::vlDepthBytes(v);
        sendMessage(MidiOutput::parameterChange(addr, QByteArray(reinterpret_cast<const char*>(depth.data()),
                                                                 static_cast<int>(depth.size()))));
        break;
    }
    case Route::CC: {
        const int v = qBound(0, static_cast<int>(std::lround(64.0 + 63.0 * swing * wave)), 127);
        sendMessage(MidiOutput::cc(kChannel, stream.param.a, v));
        break;
    }
    }
}

void BandwidthTestDialog::onStreamTick()
{
    if (!m_running) return;

    for (Stream &s : m_streams)
        sendStreamSample(s, m_intervalMs);

    ++m_stepTicks;
    const double expectedMs = static_cast<double>(m_stepTicks) * m_intervalMs;
    const double actualMs = m_stepClock.nsecsElapsed() / 1e6;
    if (actualMs - expectedMs > m_intervalMs * 0.5)
        ++m_stepOverruns;

    const double elapsedSec = m_stepClock.nsecsElapsed() / 1e9;
    m_statusLabel->setText(QString("%1 — elapsed %2 s | updates %3 | %4 B/s | overruns %5")
                               .arg(m_audibilityMode ? "Audibility check" : (m_step >= 0 ? "Ladder" : "Manual"))
                               .arg(elapsedSec, 0, 'f', 1)
                               .arg(m_stepTicks * m_streams.size())
                               .arg(elapsedSec > 0.0 ? static_cast<qint64>(m_stepBytes / elapsedSec) : 0)
                               .arg(m_stepOverruns));

    if (m_audibilityMode && m_stepClock.elapsed() > 4000) {
        logLine("Sweep done — heard? If silent, check the part's Rcv CONTROL "
                "CHANGE gate (08 0p 33, default ON) and the patch's routing.");
        stopStreaming();
    }
}

void BandwidthTestDialog::onBaselineTick()
{
    if (!m_running || !m_baselineCheck->isChecked() || !m_noteOn) return;

    // Breath sine 70-110, bend sine ±25% of the 12-st RPN range.
    const int breath = qBound(1, static_cast<int>(std::lround(90.0 + 20.0 * std::sin(m_baselinePhase))), 127);
    m_baselinePhase += 2.0 * M_PI * 0.5 * 0.010;

    const int bend = qBound(0, static_cast<int>(8192 + 2048.0 * std::sin(m_baselineBendPhase)), 16383);
    m_baselineBendPhase += 2.0 * M_PI * 0.33 * 0.010;

    sendMessage(MidiOutput::cc(kChannel, 2, breath));
    sendMessage({0xE0, static_cast<unsigned char>(bend & 0x7F),
                 static_cast<unsigned char>((bend >> 7) & 0x7F)});
}

void BandwidthTestDialog::sendCcAssignSetup()
{
    logLine("CC-assign setup: Table 9 CONTROL NO. = CC, DEPTH = 127 (edit buffer):");
    for (const ParamDef &p : m_params) {
        QByteArray cnAddr(3, 0), depAddr(3, 0);
        cnAddr[0] = depAddr[0] = 0x20;
        cnAddr[1] = depAddr[1] = 0x00;
        cnAddr[2] = static_cast<char>(p.b);
        depAddr[2] = static_cast<char>(p.b + 1);
        QByteArray depData(2, 0);
        depData[0] = 0x00;
        depData[1] = 0x7F;  // DEPTH = +127
        const auto cnMsg = MidiOutput::parameterChange(cnAddr, QByteArray(1, static_cast<char>(p.a)));
        const auto depMsg = MidiOutput::parameterChange(depAddr, depData);
        logLine(QString("  %1: CN %2 | DEPTH %3")
                    .arg(p.name, hexString(cnMsg), hexString(depMsg)));
        sendMessage(cnMsg);
        sendMessage(depMsg);
    }
}

void BandwidthTestDialog::onStartClicked()
{
    startStreaming(false, false);
}

void BandwidthTestDialog::onRunLadderClicked()
{
    startStreaming(true, false);
}

void BandwidthTestDialog::onAudibilityClicked()
{
    startStreaming(false, true);
}

void BandwidthTestDialog::onCleanNextClicked()
{
    if (!m_running || m_step < 0) return;
    const double elapsedSec = m_stepClock.nsecsElapsed() / 1e9;
    logLine(QString("step %1 verdict: clean | %2 B/s measured | %3 overruns")
                .arg(m_step + 1)
                .arg(elapsedSec > 0.0 ? static_cast<qint64>(m_stepBytes / elapsedSec) : 0)
                .arg(m_stepOverruns));
    startStep(m_step + 1);
}

void BandwidthTestDialog::onLagNextClicked()
{
    if (!m_running || m_step < 0) return;
    const double elapsedSec = m_stepClock.nsecsElapsed() / 1e9;
    logLine(QString("step %1 verdict: LAG | %2 B/s measured | %3 overruns")
                .arg(m_step + 1)
                .arg(elapsedSec > 0.0 ? static_cast<qint64>(m_stepBytes / elapsedSec) : 0)
                .arg(m_stepOverruns));
    if (m_firstLagStep < 0)
        m_firstLagStep = m_step;
    startStep(m_step + 1);
}

void BandwidthTestDialog::stopStreaming()
{
    if (!m_running) return;
    m_streamTimer.stop();
    m_baselineTimer.stop();
    m_running = false;
    m_cleanBtn->setEnabled(false);
    m_lagBtn->setEnabled(false);
    if (m_stepTicks > 0) {
        const double elapsedSec = m_stepClock.nsecsElapsed() / 1e9;
        logLine(QString("streams stopped | %1 updates | %2 B/s measured | %3 overruns")
                    .arg(m_stepTicks * m_streams.size())
                    .arg(elapsedSec > 0.0 ? static_cast<qint64>(m_stepBytes / elapsedSec) : 0)
                    .arg(m_stepOverruns));
    }
    m_statusLabel->setText("Streams stopped — note still sounding. Start another "
                           "run, or Stop to silence and close the port.");
    m_startBtn->setEnabled(true);
    m_ladderBtn->setEnabled(true);
    m_audibilityBtn->setEnabled(true);
    m_routeCombo->setEnabled(true);
    m_paramCombo->setEnabled(true);
    m_streamsSpin->setEnabled(true);
    m_intervalSpin->setEnabled(true);
}

void BandwidthTestDialog::onStopClicked()
{
    stopStreaming();
    silenceModule();
    if (m_out) {
        m_out->closePort();
        delete m_out;
        m_out = nullptr;
    }
    m_noteOn = false;
    m_ccAssigned = false;
    m_audibilityMode = false;
    m_step = -1;
    m_startBtn->setEnabled(true);
    m_ladderBtn->setEnabled(true);
    m_audibilityBtn->setEnabled(true);
    m_routeCombo->setEnabled(true);
    m_paramCombo->setEnabled(true);
    m_streamsSpin->setEnabled(true);
    m_intervalSpin->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_statusLabel->setText("Stopped, port closed, module silenced.");
}

void BandwidthTestDialog::onPanicClicked()
{
    logLine("PANIC: silence sequence");
    m_streamTimer.stop();
    m_baselineTimer.stop();
    m_running = false;
    silenceModule();
}

void BandwidthTestDialog::silenceModule()
{
    if (!m_out) return;
    // Order matters: silence FIRST — CC121 resets breath to MAX (127) and
    // re-centers CC13, so a sounding note must already be dead.
    sendMessage(MidiOutput::cc(kChannel, 123, 0));  // all notes off
    sendMessage(MidiOutput::cc(kChannel, 120, 0));  // all sound off
    for (int c = 13; c <= 20; ++c)
        sendMessage(MidiOutput::cc(kChannel, c, 64));  // re-center assigned CCs
    sendMessage(MidiOutput::cc(kChannel, 121, 0));  // reset all controllers
    sendMessage({0x80, static_cast<unsigned char>(kNoteNum), 0});  // note-off
}

void BandwidthTestDialog::sendMessage(const std::vector<unsigned char> &msg)
{
    if (!m_out) return;
    try {
        m_out->sendMessage(&msg);
        m_stepBytes += static_cast<qint64>(msg.size());
    } catch (RtMidiError &e) {
        logLine(QString("send error: %1").arg(e.what()));
    }
}

void BandwidthTestDialog::logLine(const QString &text)
{
    const QString line = QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss.zzz"), text);
    m_log->appendPlainText(line);
    qDebug().noquote() << "BandwidthTest:" << line;
}

QString BandwidthTestDialog::hexString(const std::vector<unsigned char> &msg)
{
    QString s;
    for (unsigned char b : msg)
        s += QString("%1 ").arg(b, 2, 16, QLatin1Char('0'));
    return s.trimmed().toUpper();
}
