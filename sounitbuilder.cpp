#include "sounitbuilder.h"
#include "ui_sounitbuilder.h"
#include "kalamain.h"
#include "container.h"
#include "scorecanvas.h"
#include "track.h"
#include "sounitbuildercommands.h"
#include <QToolButton>
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QPushButton>
#include <QStatusBar>
#include "scorecanvaswindow.h"
#include "timeline.h"

SounitBuilder::SounitBuilder(AudioEngine *sharedAudioEngine, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SounitBuilderCanvas)
    , audioEngine(sharedAudioEngine)
    , playbackTimer(nullptr)
    , currentNoteIndex(0)
    , playbackStartTime(0.0)
    , noteDuration(0.0)
    , isPlaying(false)
    , scoreCanvas(nullptr)
    , currentEditingTrack(0)  // Default to track 0
    , canvas(nullptr)  // Will be set via setTrackCanvas()
    , currentTrack(nullptr)  // Will be set via setTrackCanvas()
{
    ui->setupUi(this);

    // NOTE: Canvas is now provided by Track via setTrackCanvas()
    // Don't create our own canvas here anymore

    // Top toolbar - Select Mode button
    ui->toolBar->setMovable(false);
    m_selectModeButton = new QPushButton("Select", this);
    m_selectModeButton->setCheckable(true);
    m_selectModeButton->setChecked(false);
    m_selectModeButton->setMinimumWidth(70);
    m_selectModeButton->setStyleSheet(
        "QPushButton { background-color: #dce8ff; border: 1px solid #666; border-radius: 4px; padding: 4px 12px; font-weight: bold; }"
        "QPushButton:checked { background-color: #3366cc; color: white; border: 2px solid #1a3a99; }"
    );
    m_selectModeButton->setToolTip("Selection Mode (S) — drag to lasso-select containers");
    m_selectModeButton->setFocusPolicy(Qt::NoFocus);
    ui->toolBar->addWidget(m_selectModeButton);
    connect(m_selectModeButton, &QPushButton::clicked, this, [this](bool checked) {
        if (canvas) canvas->setSelectionMode(checked);
    });
    ui->toolBar->addSeparator();

    // Top toolbar - Note Mode button
    m_noteModeButton = new QPushButton("Note Mode", this);
    m_noteModeButton->setCheckable(true);
    m_noteModeButton->setChecked(false);
    m_noteModeButton->setMinimumWidth(100);
    m_noteModeButton->setStyleSheet(
        "QPushButton { background-color: #90EE90; border: 1px solid #666; border-radius: 4px; padding: 4px 12px; font-weight: bold; }"
        "QPushButton:checked { background-color: #FF4444; color: white; border: 2px solid #CC0000; }"
    );
    m_noteModeButton->setToolTip("Toggle Note Mode (N) - Edit per-note sound graphs");
    m_noteModeButton->setFocusPolicy(Qt::NoFocus);
    ui->toolBar->addWidget(m_noteModeButton);
    connect(m_noteModeButton, &QPushButton::clicked, this, [this](bool checked) {
        if (checked && !m_noteModeActive) {
            enterNoteMode();
            // If enterNoteMode failed (no notes etc.), uncheck the button
            if (!m_noteModeActive) {
                m_noteModeButton->setChecked(false);
            }
        } else if (!checked && m_noteModeActive) {
            exitNoteMode();
        }
    });

    // Create transport labels
    QLabel *labelPosition = new QLabel("Playback time: 0:00:00  ", this);
    QLabel *labelTempo = new QLabel("Tempo: 120 BPM   ", this);
    QLabel *labelTimeSignature = new QLabel("Time signature: 4/4", this);

    // Add to bottom toolbar
    ui->sounitBuilderBottomToolbar->addWidget(labelPosition);
    ui->sounitBuilderBottomToolbar->addWidget(labelTempo);
    ui->sounitBuilderBottomToolbar->addWidget(labelTimeSignature);

    // Add render progress bar
    renderProgressBar = new QProgressBar(this);
    renderProgressBar->setMinimum(0);
    renderProgressBar->setMaximum(100);
    renderProgressBar->setValue(0);
    renderProgressBar->setMinimumWidth(200);
    renderProgressBar->setMaximumWidth(200);
    renderProgressBar->setTextVisible(true);
    renderProgressBar->setFormat("Render: %p%");
    renderProgressBar->setStyleSheet("QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; } QProgressBar::chunk { background-color: #05B8CC; }");
    renderProgressBar->setVisible(true);  // TESTING: Always visible to verify it exists
    ui->sounitBuilderBottomToolbar->addSeparator();
    ui->sounitBuilderBottomToolbar->addWidget(renderProgressBar);
    qDebug() << "SounitBuilder: Progress bar created and added to toolbar";

    // Connect AudioEngine progress signals
    if (audioEngine) {
        connect(audioEngine, &AudioEngine::renderStarted, this, &SounitBuilder::onRenderStarted);
        connect(audioEngine, &AudioEngine::renderProgressChanged, this, &SounitBuilder::onRenderProgressChanged);
        connect(audioEngine, &AudioEngine::renderCompleted, this, &SounitBuilder::onRenderCompleted);
    }

    // Essential - Blue
    connect(ui->actionHarmonicGenerator, &QAction::triggered, this, [this]() {
        onAddContainer("Harmonic Generator", QColor("#3498db"),
                       {"purity", "drift", "digitWindowOffset"},
                       {"spectrum"});
    });
    connect(ui->actionSpectrum_to_Signal, &QAction::triggered, this, [this]() {
        onAddContainer("Spectrum to Signal", QColor("#3498db"),
                       {"spectrumIn", "pitchMultiplier", "normalize"},
                       {"signalOut"});
    });
    connect(ui->actionKarplus_Strong_Attack, &QAction::triggered, this, [this]() {
        onAddContainer("Karplus Strong", QColor("#3498db"),
                       {"signalIn", "trigger", "mode", "attackPortion", "damping", "pluckPosition", "mix",
                        "brightness", "excitationType", "blendRatio", "pluckHardness", "bodyResonance", "bodyFreq", "pickDirection",
                        "stringDamping", "pitchMultiplier"},
                       {"signalOut"});
    });
    connect(ui->actionSignal_Mixer, &QAction::triggered, this, [this]() {
        onAddContainer("Signal Mixer", QColor("#3498db"),
                       {"signalA", "signalB", "gainA", "gainB"},
                       {"signalOut"});
    });
    connect(ui->actionNote_Tail, &QAction::triggered, this, [this]() {
        onAddContainer("Note Tail", QColor("#3498db"),
                       {"signalIn", "length"},
                       {"signalOut"});
    });

    // Shaping - Orange
    connect(ui->actionRollofff_processor, &QAction::triggered, this, [this]() {
        onAddContainer("Rolloff Processor", QColor("#e67e22"),
                       {"spectrumIn", "lowRolloff", "highRolloff", "crossover", "transition"},
                       {"spectrumOut"});
    });
    connect(ui->actionSpectrum_Blender, &QAction::triggered, this, [this]() {
        onAddContainer("Spectrum Blender", QColor("#e67e22"),
                       {"spectrumA", "spectrumB", "position"},
                       {"spectrumOut"});
    });
    connect(ui->actionFormant_body, &QAction::triggered, this, [this]() {
        onAddContainer("Formant Body", QColor("#e67e22"),
                       {"signalIn", "f1Freq", "f2Freq", "f1Q", "f2Q", "directMix", "f1f2Balance"},
                       {"signalOut"});
    });
    connect(ui->actionBreath_Turbulence, &QAction::triggered, this, [this]() {
        onAddContainer("Breath Turbulence", QColor("#e67e22"),
                       {"voiceIn", "noiseIn", "blend"},
                       {"signalOut"});
    });
    connect(ui->actionNoise_Color_Filter, &QAction::triggered, this, [this]() {
        onAddContainer("Noise Color Filter", QColor("#e67e22"),
                       {"noiseIn", "color", "filterQ", "pitchMultiplier"},
                       {"noiseOut"});
    });

    // Modifiers - Green
    connect(ui->actionPhysics_System, &QAction::triggered, this, [this]() {
        onAddContainer("Physics System", QColor("#27ae60"),
                       {"targetValue", "mass", "springK", "damping", "impulse", "impulseAmount"},
                       {"currentValue"});
    });
    connect(ui->actionEasing_Applicator, &QAction::triggered, this, [this]() {
        onAddContainer("Easing Applicator", QColor("#27ae60"),
                       {"startValue", "endValue", "progress", "easingSelect"},
                       {"easedValue"});
    });
    connect(ui->actionEnvelope_Engine, &QAction::triggered, this, [this]() {
        onAddContainer("Envelope Engine", QColor("#27ae60"),
                       {"timeScale", "valueScale", "valueOffset"},
                       {"envelopeValue"});
    });
    connect(ui->actionDrift_Engine, &QAction::triggered, this, [this]() {
        onAddContainer("Drift Engine", QColor("#27ae60"),
                       {"amount", "rate"},
                       {"driftOut"});
    });
    // Gate Processor hidden from UI - code kept for potential future use
    // connect(ui->actionGate_Processor, &QAction::triggered, this, [this]() {
    //     onAddContainer("Gate Processor", QColor("#27ae60"),
    //                    {"velocity", "attackTime", "releaseTime", "attackCurve", "releaseCurve", "velocitySens"},
    //                    {"envelopeOut", "stateOut", "attackTrigger", "releaseTrigger"});
    // });
    connect(ui->actionLFO, &QAction::triggered, this, [this]() {
        onAddContainer("LFO", QColor("#27ae60"),
                       {"frequency", "amplitude", "waveType"},
                       {"valueOut"});
    });
    connect(ui->actionFrequency_Mapper, &QAction::triggered, this, [this]() {
        onAddContainer("Frequency Mapper", QColor("#27ae60"),
                       {"curve"},
                       {"controlOut"});
    });
    connect(ui->actionAttack, &QAction::triggered, this, [this]() {
        onAddContainer("Attack", QColor("#3498db"),
                       {"signalIn", "trigger", "attackType", "duration", "intensity", "mix",
                        "noiseAmount", "jetRatio", "reedStiffness", "reedAperture", "blowPosition",
                        "lipTension", "buzzQ", "hardness", "brightness", "tightness", "tone"},
                       {"signalOut"});
    });
    connect(ui->actionWavetable_Synth, &QAction::triggered, this, [this]() {
        onAddContainer("Wavetable Synth", QColor("#3498db"),
                       {"position", "pitchMultiplier"},
                       {"spectrum", "signalOut"});
    });
    connect(ui->actionRecorder, &QAction::triggered, this, [this]() {
        onAddContainer("Recorder", QColor("#3498db"),
                       {"breathPressure", "jetRatio", "noiseGain",
                        "vibratoFreq", "vibratoGain",
                        "endReflection", "jetReflection", "pitchMultiplier"},
                       {"signalOut"});
    });
    connect(ui->actionBowed, &QAction::triggered, this, [this]() {
        onAddContainer("Bowed", QColor("#3498db"),
                       {"bowPressure", "bowVelocity", "bowPosition",
                        "nlType", "nlAmount", "nlFreqMod", "nlAttack", "pitchMultiplier"},
                       {"signalOut"});
    });
    connect(ui->actionReed, &QAction::triggered, this, [this]() {
        onAddContainer("Reed", QColor("#3498db"),
                       {"breathPressure", "reedStiffness", "reedAperture",
                        "blowPosition", "noiseGain", "vibratoFreq", "vibratoGain",
                        "nlType", "nlAmount", "nlFreqMod", "nlAttack", "pitchMultiplier"},
                       {"signalOut"});
    });

    // Filters - Purple
    connect(ui->actionBandpass_EQ, &QAction::triggered, this, [this]() {
        onAddContainer("10-Band EQ", QColor("#9b59b6"),
                       {"signalIn", "band1", "band2", "band3", "band4", "band5",
                        "band6", "band7", "band8", "band9", "band10"},
                       {"signalOut"});
    });
    connect(ui->actionComb_Filter, &QAction::triggered, this, [this]() {
        onAddContainer("Comb Filter", QColor("#9b59b6"),
                       {"signalIn", "delayTime", "feedback", "damping"},
                       {"signalOut"});
    });
    connect(ui->actionLP_HP_Filter, &QAction::triggered, this, [this]() {
        onAddContainer("LP/HP Filter", QColor("#9b59b6"),
                       {"signalIn", "cutoff", "resonance"},
                       {"signalOut"});
    });
    connect(ui->actionIR_Convolution, &QAction::triggered, this, [this]() {
        onAddContainer("IR Convolution", QColor("#9b59b6"),
                       {"signalIn", "irIn", "wetDry", "predelay", "highDamp", "lowCut"},
                       {"signalOut"});
    });

    // Initialize playback timer
    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &SounitBuilder::onPlaybackTick);

    // Connect transport controls
    connect(ui->actionplay, &QAction::triggered, this, &SounitBuilder::onPlay);
    connect(ui->actionstop, &QAction::triggered, this, &SounitBuilder::onStop);

    // Connect variation creation
    connect(ui->actionCreateVariation, &QAction::triggered, this, &SounitBuilder::onCreateVariation);

    // Initial graph build (empty)
    rebuildGraph(currentEditingTrack);
}

void SounitBuilder::rebuildGraph(int trackIndex, bool rebuildTrackGraph)
{
    if (audioEngine && canvas) {
        qDebug() << "SounitBuilder: Rebuilding AudioEngine preview graph for track" << trackIndex;
        audioEngine->buildGraph(canvas, trackIndex);

        // Check if the preview graph is valid
        QList<Container*> containerList = canvas->findChildren<Container*>();
        bool isValid = audioEngine->hasGraph(trackIndex);
        if (!isValid && !containerList.isEmpty()) {
            QMessageBox::warning(this, "Invalid Graph Connection",
                               "The connection you made creates an invalid graph.\n\n"
                               "The audio engine has reverted to direct mode.\n\n"
                               "Please check that:\n"
                               "• The graph has exactly one Spectrum to Signal container\n"
                               "• All containers are properly connected\n"
                               "• There are no circular dependencies");
        }

        // Rebuild the track's compiled graph only when the sounit actually changed.
        // When switching tracks without edits, skip this to preserve note render caches
        // (avoids full re-render of every note just because the user switched tracks).
        if (rebuildTrackGraph && currentTrack) {
            currentTrack->rebuildGraph(audioEngine->getSampleRate());
        }
    } else {
        qWarning() << "SounitBuilder::rebuildGraph - no audioEngine or canvas!";
    }
}

void SounitBuilder::setCurrentEditingTrack(int trackIndex)
{
    currentEditingTrack = trackIndex;
    qDebug() << "SounitBuilder: Current editing track set to" << trackIndex;
}

void SounitBuilder::setScoreCanvas(ScoreCanvas *canvas)
{
    scoreCanvas = canvas;
    qDebug() << "SounitBuilder: Connected to ScoreCanvas - will play drawn notes";
}

void SounitBuilder::setTrackCanvas(Track *track)
{
    // CRITICAL: Reset pending connection state when switching canvases
    // This prevents stale state from causing buggy connection behavior
    connectionStartContainer = nullptr;
    connectionStartPort.clear();
    connectionStartIsOutput = false;
    if (canvas) {
        canvas->cancelPendingConnection();
    }

    // Handle nullptr - just detach current canvas
    if (!track) {
        qDebug() << "SounitBuilder::setTrackCanvas - detaching current canvas";
        if (canvas) {
            disconnect(canvas, nullptr, this, nullptr);
            canvas->hide();
            canvas->setParent(nullptr);
            canvas = nullptr;
        }
        if (currentTrack) {
            disconnect(currentTrack, &Track::renderStarted, this, &SounitBuilder::onRenderStarted);
            disconnect(currentTrack, &Track::renderProgressChanged, this, &SounitBuilder::onRenderProgressChanged);
            disconnect(currentTrack, &Track::renderCompleted, this, &SounitBuilder::onRenderCompleted);
            disconnect(currentTrack, &Track::renderCancelled, this, &SounitBuilder::onRenderCancelled);
            currentTrack = nullptr;
        }
        return;
    }

    Canvas *newCanvas = track->getCanvas();
    if (!newCanvas) {
        qWarning() << "SounitBuilder::setTrackCanvas - track" << track->getTrackId() << "has null canvas!";
        return;
    }

    qDebug() << "SounitBuilder: Switching to track" << track->getTrackId() << "canvas:" << track->getName();

    // CRITICAL: Detach the OLD canvas BEFORE setting new central widget!
    // When setCentralWidget() is called, Qt deletes the previous central widget.
    // We must reparent the old canvas to prevent Qt from deleting it.
    if (canvas && canvas != newCanvas) {
        qDebug() << "SounitBuilder: Detaching old canvas before switch";
        disconnect(canvas, nullptr, this, nullptr);
        canvas->hide();
        canvas->setParent(nullptr);  // Detach so Qt won't delete it
    }

    // Disconnect from old track's progress signals
    if (currentTrack && currentTrack != track) {
        disconnect(currentTrack, &Track::renderStarted, this, &SounitBuilder::onRenderStarted);
        disconnect(currentTrack, &Track::renderProgressChanged, this, &SounitBuilder::onRenderProgressChanged);
        disconnect(currentTrack, &Track::renderCompleted, this, &SounitBuilder::onRenderCompleted);
        disconnect(currentTrack, &Track::renderCancelled, this, &SounitBuilder::onRenderCancelled);
    }

    // Store current track and get its canvas
    currentTrack = track;
    canvas = newCanvas;

    // Connect to track's render progress signals for the progress bar
    connect(currentTrack, &Track::renderStarted, this, &SounitBuilder::onRenderStarted);
    connect(currentTrack, &Track::renderProgressChanged, this, &SounitBuilder::onRenderProgressChanged);
    connect(currentTrack, &Track::renderCompleted, this, &SounitBuilder::onRenderCompleted);
    connect(currentTrack, &Track::renderCancelled, this, &SounitBuilder::onRenderCancelled);

    // Set as central widget (Qt will NOT delete the old one since we detached it)
    setCentralWidget(canvas);
    canvas->show();

    // Reconnect canvas signals
    connect(canvas, &Canvas::graphChanged, this, [this]() {
        if (!canvas->isLoading()) {
            rebuildGraph(currentEditingTrack);
        } else {
            qDebug() << "Skipping graph rebuild during canvas load";
        }
    });

    // Keep select mode button in sync with canvas state
    if (m_selectModeButton) {
        connect(canvas, &Canvas::selectionModeChanged, m_selectModeButton, &QPushButton::setChecked);
        m_selectModeButton->setChecked(canvas->isSelectionMode());
    }

    // Reconnect all container signals
    QList<Container*> containers = canvas->findChildren<Container*>();
    for (Container *container : containers) {
        connectContainerSignals(container);
    }

    qDebug() << "SounitBuilder: Track canvas switched successfully -" << containers.size() << "containers found";
}

void SounitBuilder::triggerPlay()
{
    onPlay();
}

void SounitBuilder::onAddContainer(const QString &name, const QColor &color,
                                   const QStringList &inputs, const QStringList &outputs)
{
    Container *newContainer = new Container(canvas, name, color, inputs, outputs);
    newContainer->move(100, 100);

    // Generate unique instance name to avoid connection confusion during save/load
    // e.g., "Envelope Engine 1", "Envelope Engine 2", etc.
    QList<Container*> existingContainers = canvas->findChildren<Container*>();
    QSet<QString> existingNames;
    for (Container *c : existingContainers) {
        existingNames.insert(c->getInstanceName());
    }

    // Find the next available number for this container type
    int suffix = 1;
    QString uniqueName;
    do {
        uniqueName = QString("%1 %2").arg(name).arg(suffix++);
    } while (existingNames.contains(uniqueName));

    newContainer->setInstanceName(uniqueName);
    qDebug() << "Created container with unique instance name:" << uniqueName;

    // Set default parameters based on container type (from Container Port Specification v1.2)
    if (name == "Harmonic Generator") {
        newContainer->setParameter("numHarmonics", 64.0);
        newContainer->setParameter("dnaSelect", 0.0);
        newContainer->setParameter("purity", 0.0);  // 0.0 = pure DNA preset, no blending
        newContainer->setParameter("drift", 0.0);
        newContainer->setParameter("digitWindowOffset", 0.0);
        newContainer->setParameter("padEnabled", 0.0);
        newContainer->setParameter("padBandwidth", 40.0);
        newContainer->setParameter("padBandwidthScale", 1.0);
        newContainer->setParameter("padProfileShape", 0.0);
        newContainer->setParameter("padFftSize", 262144.0);
    } else if (name == "Spectrum to Signal") {
        newContainer->setParameter("normalize", 1.0);
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "Rolloff Processor") {
        newContainer->setParameter("lowRolloff", 0.3);
        newContainer->setParameter("highRolloff", 1.0);
        newContainer->setParameter("crossover", 8.0);
        newContainer->setParameter("transition", 4.0);
    } else if (name == "Spectrum Blender") {
        newContainer->setParameter("position", 0.5);
    } else if (name == "Formant Body") {
        newContainer->setParameter("f1Freq", 500.0);
        newContainer->setParameter("f2Freq", 1500.0);
        newContainer->setParameter("f1Q", 8.0);
        newContainer->setParameter("f2Q", 10.0);
        newContainer->setParameter("directMix", 0.3);
        newContainer->setParameter("f1f2Balance", 0.6);
    } else if (name == "Breath Turbulence") {
        newContainer->setParameter("blend", 0.10);
    } else if (name == "Noise Color Filter") {
        newContainer->setParameter("color", 2000.0);
        newContainer->setParameter("filterQ", 1.0);
        newContainer->setParameter("filterType", 1.0);   // 1 = Highpass (existing default)
        newContainer->setParameter("followPitch", 0.0);
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "Physics System") {
        newContainer->setParameter("mass", 0.5);
        newContainer->setParameter("springK", 0.001);
        newContainer->setParameter("damping", 0.995);
        newContainer->setParameter("impulseAmount", 100.0);
    } else if (name == "Easing Applicator") {
        newContainer->setParameter("startValue", 0.0);
        newContainer->setParameter("endValue", 1.0);
        newContainer->setParameter("easingSelect", 0.0);
    } else if (name == "Envelope Engine") {
        newContainer->setParameter("envelopeSelect", 0.0);
        newContainer->setParameter("timeScale", 1.0);
        newContainer->setParameter("valueScale", 1.0);
        newContainer->setParameter("valueOffset", 0.0);
        newContainer->setParameter("followDynamics", 0.0);
    } else if (name == "Drift Engine") {
        newContainer->setParameter("amount", 0.005);
        newContainer->setParameter("rate", 0.5);
    } else if (name == "Gate Processor") {
        newContainer->setParameter("velocity", 1.0);
        newContainer->setParameter("attackTime", 0.01);
        newContainer->setParameter("releaseTime", 0.1);
        newContainer->setParameter("attackCurve", 0.0);
        newContainer->setParameter("releaseCurve", 0.0);
        newContainer->setParameter("velocitySens", 0.5);
    } else if (name == "Karplus Strong") {
        newContainer->setParameter("mode", 0.0);  // 0 = String mode
        newContainer->setParameter("attackPortion", 0.15);
        newContainer->setParameter("damping", 0.5);
        newContainer->setParameter("pluckPosition", 0.5);
        newContainer->setParameter("mix", 0.8);  // Attack mode mix
        newContainer->setParameter("brightness", 0.5);
        newContainer->setParameter("excitationType", 0.0);  // 0=Noise, 1=SoftPluck, 2=Blend
        newContainer->setParameter("blendRatio", 0.5);
        newContainer->setParameter("pluckHardness", 0.0);    // 0=soft/wide
        newContainer->setParameter("pickDirection", 0.5);    // 0.5=neutral
        newContainer->setParameter("bodyResonance", 0.0);    // off by default
        newContainer->setParameter("bodyFreq", 200.0);
        newContainer->setParameter("stringDamping", 0.0);   // off by default (harp mode)
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "Attack") {
        newContainer->setParameter("attackType", 0.0);    // Flute Chiff
        newContainer->setParameter("duration", 0.15);
        newContainer->setParameter("intensity", 0.8);
        newContainer->setParameter("mix", 0.8);
        // Flute params
        newContainer->setParameter("noiseAmount", 0.5);
        newContainer->setParameter("jetRatio", 0.32);
        // Clarinet/Sax params
        newContainer->setParameter("reedStiffness", 0.5);
        newContainer->setParameter("reedAperture", 0.5);
        // Sax only
        newContainer->setParameter("blowPosition", 0.5);
        // Brass params
        newContainer->setParameter("lipTension", 0.5);
        newContainer->setParameter("buzzQ", 5.0);
        // Piano params
        newContainer->setParameter("hardness", 0.5);
        newContainer->setParameter("brightness", 0.5);
        // Drum params
        newContainer->setParameter("tightness", 0.5);
        newContainer->setParameter("tone", 0.5);
    } else if (name == "Signal Mixer") {
        newContainer->setParameter("gainA", 1.0);
        newContainer->setParameter("gainB", 1.0);
    } else if (name == "Note Tail") {
        newContainer->setParameter("tailLength", 200.0);  // 200 ms default
    } else if (name == "LFO") {
        newContainer->setParameter("frequency", 1.0);    // 1 Hz default
        newContainer->setParameter("amplitude", 1.0);    // Full amplitude
        newContainer->setParameter("waveType", 0.0);     // Sine wave
    } else if (name == "Frequency Mapper") {
        newContainer->setParameter("lowFreq", 25.0);
        newContainer->setParameter("highFreq", 8000.0);
        newContainer->setParameter("outputMin", 0.0);
        newContainer->setParameter("outputMax", 1.0);
        newContainer->setParameter("curveType", 1.0);    // Logarithmic
        newContainer->setParameter("invert", 0.0);
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "Wavetable Synth") {
        newContainer->setParameter("wavetableSelect", 0.0);  // Saw preset
        newContainer->setParameter("position", 0.0);
        newContainer->setParameter("cycleLength", 2048.0);
        newContainer->setParameter("outputMode", 0.0);       // Both
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "10-Band EQ") {
        // 10 gain parameters (band1..band10), default unity gain
        for (int i = 1; i <= 10; i++) {
            newContainer->setParameter(QString("band%1").arg(i), 1.0);
        }
        // 10 Q parameters (q1..q10), default Q = 2.0
        for (int i = 1; i <= 10; i++) {
            newContainer->setParameter(QString("q%1").arg(i), 2.0);
        }
    } else if (name == "Comb Filter") {
        newContainer->setParameter("mode", 0.0);        // Feedforward
        newContainer->setParameter("delayTime", 5.0);   // 5 ms
        newContainer->setParameter("feedback", 0.5);
        newContainer->setParameter("damping", 0.0);
    } else if (name == "LP/HP Filter") {
        newContainer->setParameter("mode", 0.0);        // Low Pass
        newContainer->setParameter("cutoff", 1000.0);   // 1000 Hz
        newContainer->setParameter("resonance", 0.707); // Butterworth
    } else if (name == "IR Convolution") {
        newContainer->setParameter("wetDry", 0.5);
        newContainer->setParameter("predelay", 0.0);
        newContainer->setParameter("decay", 1.0);
        newContainer->setParameter("highDamp", 20000.0);
        newContainer->setParameter("lowCut", 20.0);
        newContainer->setParameter("fadeInPct", 10.0);
        newContainer->setParameter("captureLength", 1.0);
    } else if (name == "Recorder") {
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "Bowed") {
        newContainer->setParameter("pitchMultiplier", 1.0);
    } else if (name == "Reed") {
        newContainer->setParameter("breathPressure", 0.7);
        newContainer->setParameter("reedStiffness",  0.5);
        newContainer->setParameter("reedAperture",   0.5);
        newContainer->setParameter("blowPosition",   0.2);
        newContainer->setParameter("noiseGain",      0.2);
        newContainer->setParameter("vibratoFreq",    5.735);
        newContainer->setParameter("vibratoGain",    0.0);
        newContainer->setParameter("nlType",         0.0);
        newContainer->setParameter("nlAmount",       0.0);
        newContainer->setParameter("nlFreqMod",      10.0);
        newContainer->setParameter("nlAttack",       0.1);
        newContainer->setParameter("pitchMultiplier", 1.0);
    }

    newContainer->show();

    // Use connectContainerSignals for consistent signal handling
    connectContainerSignals(newContainer);

    containers.append(newContainer);

    // Push onto undo stack (redo is called immediately, but container is already on canvas)
    canvas->getUndoStack()->push(new AddContainerCommand(newContainer, canvas));
}

// ============================================================================
// Clipboard Operations (Cut / Copy / Paste)
// ============================================================================

void SounitBuilder::performCopy()
{
    if (!canvas) return;

    const QVector<Container*>& selected = canvas->getSelectedContainers();
    QVector<Container*> toSerialize;
    if (!selected.isEmpty()) {
        toSerialize = selected;
    } else if (canvas->selectedContainer) {
        toSerialize.append(canvas->selectedContainer);
    } else {
        return;
    }

    // Build set of selected instance names for filtering connections
    QSet<QString> selectedNames;
    for (Container *c : toSerialize) selectedNames.insert(c->getInstanceName());

    QJsonArray containersArray;
    for (Container *c : toSerialize) {
        containersArray.append(canvas->serializeContainer(c));
    }

    // Include only connections whose both endpoints are in the selection
    QJsonArray connectionsArray;
    for (const Canvas::Connection &conn : canvas->getConnections()) {
        if (selectedNames.contains(conn.fromContainer->getInstanceName()) &&
            selectedNames.contains(conn.toContainer->getInstanceName())) {
            connectionsArray.append(canvas->serializeConnection(conn));
        }
    }

    m_containerClipboard = QJsonObject();
    m_containerClipboard["containers"] = containersArray;
    m_containerClipboard["connections"] = connectionsArray;
    qDebug() << "SounitBuilder: Copied" << toSerialize.size() << "container(s)";
}

void SounitBuilder::performCut()
{
    if (!canvas) return;

    const QVector<Container*>& selected = canvas->getSelectedContainers();
    if (selected.isEmpty() && !canvas->selectedContainer) return;

    // Copy first
    performCopy();

    // Delete via undo command
    if (!selected.isEmpty()) {
        QVector<Container*> toDelete = selected;
        canvas->clearSelection();
        if (toDelete.size() == 1) {
            canvas->getUndoStack()->push(new DeleteContainerCommand(toDelete[0], canvas));
        } else {
            canvas->getUndoStack()->beginMacro(QString("Cut %1 Containers").arg(toDelete.size()));
            for (Container *c : toDelete) {
                canvas->getUndoStack()->push(new DeleteContainerCommand(c, canvas));
            }
            canvas->getUndoStack()->endMacro();
        }
    } else {
        Container *toDelete = canvas->selectedContainer;
        canvas->clearSelection();
        canvas->getUndoStack()->push(new DeleteContainerCommand(toDelete, canvas));
    }
    qDebug() << "SounitBuilder: Cut containers";
}

void SounitBuilder::performPaste()
{
    if (!canvas || m_containerClipboard.isEmpty()) return;

    QJsonArray containersArray = m_containerClipboard["containers"].toArray();
    QJsonArray connectionsArray = m_containerClipboard["connections"].toArray();
    if (containersArray.isEmpty()) return;

    // Collect existing instance names to guarantee uniqueness
    QList<Container*> existingContainerList = canvas->findChildren<Container*>();
    QSet<QString> existingNames;
    for (Container *c : existingContainerList) existingNames.insert(c->getInstanceName());

    QMap<QString, Container*> nameMap;   // old instance name -> new container
    QVector<Container*> pasted;
    QJsonArray updatedContainersArray;   // Positions advanced +20,+20 for next paste

    for (const QJsonValue &val : containersArray) {
        QJsonObject clipObj = val.toObject();

        // Offset position +20,+20 so each paste is visibly stacked
        if (clipObj.contains("position")) {
            QJsonObject pos = clipObj["position"].toObject();
            QJsonObject newPos;
            newPos["x"] = pos["x"].toInt() + 20;
            newPos["y"] = pos["y"].toInt() + 20;
            clipObj["position"] = newPos;
        }
        updatedContainersArray.append(clipObj);

        QString oldName = clipObj["instanceName"].toString();
        QString type = clipObj["type"].toString();

        Container *newContainer = canvas->deserializeContainer(clipObj, canvas);
        if (!newContainer) continue;

        // Generate a unique instance name
        int suffix = 1;
        QString uniqueName;
        do {
            uniqueName = QString("%1 %2").arg(type).arg(suffix++);
        } while (existingNames.contains(uniqueName));
        existingNames.insert(uniqueName);
        newContainer->setInstanceName(uniqueName);

        nameMap[oldName] = newContainer;
        connectContainerSignals(newContainer);
        newContainer->show();
        containers.append(newContainer);
        pasted.append(newContainer);
    }

    // Advance clipboard positions for the next paste
    m_containerClipboard["containers"] = updatedContainersArray;

    if (pasted.isEmpty()) return;

    // Reconstruct connections between pasted containers
    QVector<Canvas::Connection> pastedConnections;
    for (const QJsonValue &val : connectionsArray) {
        QJsonObject connJson = val.toObject();
        QString fromName = connJson["fromContainer"].toString();
        QString toName = connJson["toContainer"].toString();
        if (nameMap.contains(fromName) && nameMap.contains(toName)) {
            Canvas::Connection conn;
            conn.fromContainer = nameMap[fromName];
            conn.fromPort = connJson["fromPort"].toString();
            conn.toContainer = nameMap[toName];
            conn.toPort = connJson["toPort"].toString();
            conn.function = connJson["function"].toString("passthrough");
            conn.weight = connJson["weight"].toDouble(1.0);
            pastedConnections.append(conn);
        }
    }

    // Push undo commands (single container without connections: no macro needed)
    if (pasted.size() == 1 && pastedConnections.isEmpty()) {
        canvas->selectContainer(pasted[0]);
        canvas->getUndoStack()->push(new AddContainerCommand(pasted[0], canvas));
    } else {
        canvas->getUndoStack()->beginMacro(QString("Paste %1 Container(s)").arg(pasted.size()));
        for (Container *c : pasted) {
            canvas->getUndoStack()->push(new AddContainerCommand(c, canvas));
        }
        for (const Canvas::Connection &conn : pastedConnections) {
            canvas->getUndoStack()->push(new AddConnectionCommand(conn, canvas));
        }
        canvas->getUndoStack()->endMacro();
        canvas->selectContainers(pasted);
    }

    qDebug() << "SounitBuilder: Pasted" << pasted.size() << "container(s)"
             << pastedConnections.size() << "connection(s)";
}

void SounitBuilder::onPortClicked(Container *container, const QString &portName, bool isOutput, QPoint globalPos)
{
    if (!connectionStartContainer) {
        // First click — start dragging
        connectionStartContainer = container;
        connectionStartPort = portName;
        connectionStartIsOutput = isOutput;

        canvas->startPendingConnection(container, portName, isOutput);
    } else {
        // Second click — complete or cancel
        if (connectionStartIsOutput != isOutput) {
            Canvas::Connection conn;
            if (connectionStartIsOutput) {
                conn.fromContainer = connectionStartContainer;
                conn.fromPort = connectionStartPort;
                conn.toContainer = container;
                conn.toPort = portName;
            } else {
                conn.fromContainer = container;
                conn.fromPort = portName;
                conn.toContainer = connectionStartContainer;
                conn.toPort = connectionStartPort;
            }
            canvas->getUndoStack()->push(new AddConnectionCommand(conn, canvas));

            qDebug() << "Connection added:" << conn.fromContainer->getName() << conn.fromPort
                     << "->" << conn.toContainer->getName() << conn.toPort;
        }

        canvas->cancelPendingConnection();
        connectionStartContainer = nullptr;
    }
}

void SounitBuilder::onPlay()
{
    startPlayback();
}

void SounitBuilder::onStop()
{
    stopPlayback();
}

void SounitBuilder::startPlayback()
{
    // In note mode, play only the current note
    if (m_noteModeActive) {
        noteModePlayCurrentNote();
        return;
    }

    if (!audioEngine || !scoreCanvas || !currentTrack) return;

    // Get all notes from the ScoreCanvas phrase (shared with ScoreCanvasWindow)
    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    if (notes.isEmpty()) {
        qDebug() << "SounitBuilder: No notes to play";
        return;
    }

    // Stop any currently playing audio first
    if (isPlaying) {
        stopPlayback();
    }

    // Make sure audio is stopped from any other source
    audioEngine->stopNote();

    qDebug() << "SounitBuilder: Pre-rendering notes using Track system...";

    // Rebuild Track's graph from current canvas to include user's edits
    // This ensures preview playback reflects the current state
    qDebug() << "SounitBuilder: Rebuilding Track graph for preview...";
    currentTrack->rebuildGraph(audioEngine->getSampleRate());

    // Filter notes for the current editing track
    QList<Note> trackNotes;
    for (const Note& note : notes) {
        if (note.getTrackIndex() == currentEditingTrack) {
            trackNotes.append(note);
        }
    }

    // Sync notes to the current track
    currentTrack->getNotesRef().clear();
    for (const Note& note : trackNotes) {
        currentTrack->getNotesRef().append(note);
    }

    // Use Track's note-based rendering (same as ScoreCanvas)
    bool rendered = currentTrack->prerenderDirtyNotes(audioEngine->getSampleRate());
    if (!rendered) {
        qWarning() << "SounitBuilder: Track rendering failed";
        return;
    }

    // Play using Track-based playback (same as ScoreCanvas)
    QList<Track*> tracksToPlay;
    tracksToPlay.append(currentTrack);
    audioEngine->playFromTracks(tracksToPlay, 0.0);

    // Calculate total playback duration including rendered tails (reverb/decay ring-out)
    double totalDuration = currentTrack->getRenderedEndTimeMs();

    // Start playback from the beginning
    currentNoteIndex = 0;
    playbackStartTime = 0.0;
    isPlaying = true;
    noteDuration = totalDuration;

    // Start the playback timer (tick every 10ms for smooth timing)
    playbackTimer->start(10);

    qDebug() << "SounitBuilder: Starting playback of" << trackNotes.size()
             << "note(s) using Track system (duration:" << totalDuration << "ms)";

    // Emit signal to stop other windows
    emit playbackStarted();
}

void SounitBuilder::onPlaybackTick()
{
    if (!audioEngine || !isPlaying) return;

    // With pre-rendered buffer playback, we don't need to trigger individual notes
    // The audio callback automatically plays through the entire rendered buffer
    // This timer just checks if playback has finished

    // Check if we've exceeded the total duration
    if (playbackStartTime >= noteDuration) {
        stopPlayback();
        return;
    }

    // Advance playback time (10ms per tick)
    playbackStartTime += 10.0;
}

void SounitBuilder::stopPlayback(bool stopAudioEngine)
{
    if (!audioEngine) return;

    // Stop the timer first
    if (playbackTimer && playbackTimer->isActive()) {
        playbackTimer->stop();
    }

    // Reset playback state BEFORE stopping note (prevents race condition)
    isPlaying = false;
    currentNoteIndex = 0;
    playbackStartTime = 0.0;
    noteDuration = 0.0;

    // Only stop audio engine if requested (don't stop when another window is taking over)
    if (stopAudioEngine) {
        audioEngine->stopNote();
    }

    // Restore notes saved during note-mode single-note playback
    if (m_noteModePlaying && currentTrack) {
        currentTrack->getNotesRef() = m_noteModePlaybackSavedNotes;
        m_noteModePlaybackSavedNotes.clear();
        m_noteModePlaying = false;
    }

    qDebug() << "SounitBuilder: Playback stopped" << (stopAudioEngine ? "(audio stopped)" : "(audio continues)");
}

bool SounitBuilder::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Note Mode key handling (must be in event() to catch keys even when Canvas has focus)
        if (m_noteModeActive) {
            if (keyEvent->key() == Qt::Key_Escape) {
                exitNoteMode();
                return true;
            }
            if (keyEvent->key() == Qt::Key_Left) {
                noteModeNavigate(-1);
                return true;
            }
            if (keyEvent->key() == Qt::Key_Right) {
                noteModeNavigate(+1);
                return true;
            }
            if (keyEvent->key() == Qt::Key_Space && keyEvent->modifiers() == Qt::NoModifier) {
                if (isPlaying) stopPlayback();
                else noteModePlayCurrentNote();
                return true;
            }
        }

        // S key toggles selection mode
        if (keyEvent->key() == Qt::Key_S && keyEvent->modifiers() == Qt::NoModifier) {
            if (canvas) {
                bool newMode = !canvas->isSelectionMode();
                canvas->setSelectionMode(newMode);
                if (m_selectModeButton) m_selectModeButton->setChecked(newMode);
                statusBar()->showMessage(newMode ? "Selection Mode — drag to lasso-select" : "", newMode ? 0 : 1);
            }
            return true;
        }

        // N key toggles note mode
        if (keyEvent->key() == Qt::Key_N && keyEvent->modifiers() == Qt::NoModifier) {
            if (m_noteModeActive) exitNoteMode();
            else enterNoteMode();
            return true;
        }

        // Ctrl+C for copy
        if (keyEvent->matches(QKeySequence::Copy)) {
            performCopy();
            return true;
        }

        // Ctrl+X for cut
        if (keyEvent->matches(QKeySequence::Cut)) {
            performCut();
            return true;
        }

        // Ctrl+V for paste
        if (keyEvent->matches(QKeySequence::Paste)) {
            performPaste();
            return true;
        }

        // Tab key - intercept before Qt's focus navigation
        if (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() == Qt::NoModifier) {
            if (m_noteModeActive) {
                statusBar()->showMessage("Exit Note Mode first (Esc)", 2000);
                return true;  // Block tab switching
            }
            if (m_mainWindow) {
                m_mainWindow->toggleCanvasFocus();
            }
            return true;  // Event handled
        }
    }
    return QMainWindow::event(event);
}

void SounitBuilder::keyPressEvent(QKeyEvent *event)
{
    // Escape cancels in-progress render
    if (event->key() == Qt::Key_Escape && m_isRendering && currentTrack && currentTrack->isRendering()) {
        currentTrack->cancelRender();
        event->accept();
        return;
    }

    // Spacebar toggles playback (start/stop)
    if (event->key() == Qt::Key_Space && event->modifiers() == Qt::NoModifier) {
        if (isPlaying) {
            stopPlayback();
        } else {
            startPlayback();
        }
        event->accept();
        return;
    }

    // T key plays test tone at 262 Hz (middle C)
    if (event->key() == Qt::Key_T && event->modifiers() == Qt::NoModifier) {
        if (audioEngine && currentTrack) {
            qDebug() << "SounitBuilder: Rendering test tone using Track graph: 261.63 Hz (Middle C)";

            // Create a test note: Middle C (261.63 Hz), 1 second duration
            Note testNote(0.0, 1000.0, 261.63, 0.7);

            // Clear track's notes and add test note
            currentTrack->clearNotes();
            currentTrack->addNote(testNote);

            // Render using Track (uses Track's graph!)
            bool rendered = currentTrack->prerender(48000.0, 1000.0);

            if (rendered) {
                // Get Track's rendered segments and copy to AudioEngine for playback
                // (AudioEngine still handles playback, but uses Track's audio data)
                const QVector<RenderSegment>& trackSegments = currentTrack->getRenderSegments();

                // For now, render using AudioEngine too (for playback compatibility)
                // TODO: Make AudioEngine read directly from Track's segments
                QVector<Note> testNotes;
                testNotes.append(testNote);
                audioEngine->renderNotes(testNotes, 1);
                audioEngine->playRenderedBuffer();

                qDebug() << "Test tone rendered successfully -" << trackSegments.size() << "segments in Track";
            } else {
                qWarning() << "Failed to render test tone - check graph validity";
            }
        }
        event->accept();
        return;
    }

    // Z key toggles zoom mode
    if (event->key() == Qt::Key_Z && event->modifiers() == Qt::NoModifier) {
        if (canvas) {
            bool newZoomMode = !canvas->isZoomMode();
            canvas->setZoomMode(newZoomMode);

            // Update status bar or visual feedback
            if (newZoomMode) {
                statusBar()->showMessage("ZOOM MODE - Use mouse wheel to zoom in/out. Press 'z' to exit.", 0);
            } else {
                statusBar()->clearMessage();
            }
        }
        event->accept();
        return;
    }

    // R key resets zoom to 100%
    if (event->key() == Qt::Key_R && event->modifiers() == Qt::NoModifier) {
        if (canvas) {
            canvas->resetZoom();
            statusBar()->showMessage("Zoom reset to 100%", 2000);
        }
        event->accept();
        return;
    }

    // Ctrl+Space toggles pan mode
    if (event->key() == Qt::Key_Space && event->modifiers() == Qt::ControlModifier) {
        if (canvas) {
            bool newPanMode = !canvas->isPanMode();
            canvas->setPanMode(newPanMode);

            // Update status bar
            if (newPanMode) {
                statusBar()->showMessage("PAN MODE - Drag to pan the canvas. Press Ctrl+Space to exit.", 0);
            } else {
                statusBar()->clearMessage();
            }
        }
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void SounitBuilder::connectContainerSignals(Container *container)
{
    // Use Qt::UniqueConnection to prevent duplicate signal connections
    // which can cause bugs like port clicks being processed twice
    connect(container, &Container::portClicked, this, &SounitBuilder::onPortClicked, Qt::UniqueConnection);
    connect(container, &Container::clicked, canvas, &Canvas::selectContainer, Qt::UniqueConnection);
    connect(container, &Container::moved, canvas, QOverload<>::of(&QWidget::update), Qt::UniqueConnection);

    // Envelope Engine: add the "Follow Dynamics" toggle button to the header
    if (container->getName() == "Envelope Engine")
        container->addFollowDynamicsButton();

    // Note: Lambda connections cannot use Qt::UniqueConnection, so we disconnect first
    disconnect(container, &Container::parameterChanged, nullptr, nullptr);
    connect(container, &Container::parameterChanged, this, [this]() {
        rebuildGraph(currentEditingTrack);
    });

    // Connect dragStarted to set up multi-drag companions and record undo positions
    disconnect(container, &Container::dragStarted, nullptr, nullptr);
    connect(container, &Container::dragStarted, this, [this](Container *leader) {
        if (!canvas) return;
        const QVector<Container*>& selected = canvas->getSelectedContainers();
        if (selected.size() > 1 && selected.contains(leader)) {
            // Record start positions for all selected containers for undo
            m_multiDragStartPositions.clear();
            for (Container *c : selected) {
                m_multiDragStartPositions[c] = c->pos();
            }
            // Set companions on the leader (everyone except the leader itself)
            QVector<Container*> companions;
            for (Container *c : selected) {
                if (c != leader) companions.append(c);
            }
            leader->setMultiDragCompanions(companions);
        } else {
            m_multiDragStartPositions.clear();
        }
    });

    // Connect move completed for undo support
    disconnect(container, &Container::moveCompleted, nullptr, nullptr);
    connect(container, &Container::moveCompleted, this, [this](Container *c, const QPoint &oldPos, const QPoint &newPos) {
        if (!canvas) return;
        if (!m_multiDragStartPositions.isEmpty() && m_multiDragStartPositions.contains(c)) {
            // Multi-drag: one macro entry covers all containers that moved
            canvas->getUndoStack()->beginMacro(
                QString("Move %1 Containers").arg(m_multiDragStartPositions.size()));
            for (auto it = m_multiDragStartPositions.begin(); it != m_multiDragStartPositions.end(); ++it) {
                Container *movedContainer = it.key();
                QPoint startPos = it.value();
                QPoint endPos = movedContainer->pos();
                if (startPos != endPos) {
                    canvas->getUndoStack()->push(
                        new MoveContainerCommand(movedContainer, startPos, endPos, canvas));
                }
            }
            canvas->getUndoStack()->endMacro();
            m_multiDragStartPositions.clear();
        } else {
            // Single container move
            canvas->getUndoStack()->push(new MoveContainerCommand(c, oldPos, newPos, canvas));
        }
    });
}

void SounitBuilder::onRenderStarted()
{
    m_isRendering = true;
    renderProgressBar->setFormat("Rendering...");
    renderProgressBar->setMinimum(0);
    renderProgressBar->setMaximum(0);  // Indeterminate mode (animated busy bar)
    renderProgressBar->setVisible(true);
    qDebug() << "SounitBuilder: Render started, progress bar visible";
}

void SounitBuilder::onRenderProgressChanged(int percentage)
{
    //qDebug() << "SounitBuilder: Render progress:" << percentage << "%";
    if (percentage <= 0) return;  // Stay in indeterminate "Rendering..." mode until real progress
    if (renderProgressBar->maximum() == 0) {
        // Switch from indeterminate to determinate mode
        renderProgressBar->setMaximum(100);
        renderProgressBar->setFormat("Render: %p%");
    }
    renderProgressBar->setValue(percentage);
    renderProgressBar->update();
}

void SounitBuilder::onRenderCompleted()
{
    m_isRendering = false;
    renderProgressBar->setMaximum(100);
    renderProgressBar->setFormat("Render: %p%");
    renderProgressBar->setValue(100);
    qDebug() << "SounitBuilder: Render completed at 100%, will hide in 3 seconds";
    // Keep progress bar visible for 3 seconds so user can see it
    QTimer::singleShot(3000, this, [this]() {
        renderProgressBar->setVisible(false);
        qDebug() << "SounitBuilder: Progress bar now hidden";
    });
}

void SounitBuilder::onRenderCancelled()
{
    m_isRendering = false;
    renderProgressBar->setMaximum(100);
    renderProgressBar->setValue(0);
    renderProgressBar->setFormat("Render cancelled");
    QTimer::singleShot(2000, this, [this]() {
        renderProgressBar->setVisible(false);
        renderProgressBar->setFormat("Render: %p%");
    });
}

void SounitBuilder::onCreateVariation()
{
    if (!currentTrack) {
        QMessageBox::warning(this, "No Track",
                            "No track is currently being edited.\n\n"
                            "Please load a sounit first.");
        return;
    }

    // Rebuild graph if needed (may have been invalidated by parameter changes)
    if (!currentTrack->hasValidGraph()) {
        qDebug() << "SounitBuilder: Rebuilding Track graph before creating variation";
        if (!currentTrack->rebuildGraph(audioEngine ? audioEngine->getSampleRate() : 44100.0)) {
            QMessageBox::warning(this, "Invalid Graph",
                                "The current graph is invalid.\n\n"
                                "Please ensure the sounit has a valid graph before creating a variation.");
            return;
        }
    }

    // Prompt for variation name
    bool ok;
    QString defaultName = QString("Variation %1").arg(currentTrack->getVariationCount() + 1);
    QString name = QInputDialog::getText(this, "Create Variation",
                                         "Variation name:", QLineEdit::Normal,
                                         defaultName, &ok);

    if (ok && !name.isEmpty()) {
        // Check if a variation with this name already exists
        int existingIndex = currentTrack->findVariationByName(name);
        bool isUpdate = false;
        if (existingIndex > 0) {
            // Ask user if they want to update the existing variation
            QMessageBox::StandardButton reply = QMessageBox::question(this,
                "Update Variation?",
                QString("A variation named '%1' already exists.\n\n"
                        "Do you want to update it with the current edits?").arg(name),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // Delete the old variation first
                currentTrack->deleteVariation(existingIndex);
                isUpdate = true;
            } else {
                return;  // User cancelled
            }
        }

        int index = currentTrack->createVariation(name);
        if (index > 0) {
            emit variationsChanged();

            // Ask if user wants to reload the original sounit
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(isUpdate ? "Variation Updated" : "Variation Created");
            msgBox.setText(QString("%1 variation %2: %3")
                          .arg(isUpdate ? "Updated" : "Created")
                          .arg(index)
                          .arg(name));
            msgBox.setInformativeText(
                "Your edits are now saved in this variation.\n\n"
                "Would you like to reload the original sounit? This resets the canvas "
                "to the base sound, allowing clean edits for new variations.\n\n"
                "If you keep current edits, they will affect the base sounit (variation 0) "
                "in composition playback.");

            QPushButton *reloadBtn = msgBox.addButton("Reload Original", QMessageBox::ActionRole);
            QPushButton *keepBtn = msgBox.addButton("Keep Current Edits", QMessageBox::AcceptRole);
            msgBox.setDefaultButton(keepBtn);

            msgBox.exec();

            if (msgBox.clickedButton() == static_cast<QAbstractButton*>(reloadBtn)) {
                // Reload the original sounit from file
                QString filePath = currentTrack->getSounitFilePath();
                if (!filePath.isEmpty() && QFile::exists(filePath)) {
                    if (currentTrack->loadSounit(filePath)) {
                        qDebug() << "SounitBuilder: Reloaded original sounit";
                        // Refresh the canvas display
                        setTrackCanvas(currentTrack);
                        rebuildGraph(currentEditingTrack);
                        QMessageBox::information(this, "Original Restored",
                            "The sounit has been reset to its original state.\n"
                            "Your variation is preserved and can be applied to notes.");
                    }
                } else {
                    // No file to reload from - create new sounit
                    currentTrack->newSounit();
                    setTrackCanvas(currentTrack);
                    rebuildGraph(currentEditingTrack);
                    QMessageBox::information(this, "Reset to New",
                        "No original sounit file found. Reset to a new empty sounit.\n"
                        "Your variation is preserved and can be applied to notes.");
                }
            }
        } else {
            QMessageBox::warning(this, "Failed",
                                "Failed to create variation.\n\n"
                                "Check the debug output for details.");
        }
    }
}

// ========== Note Mode ==========

void SounitBuilder::enterNoteMode()
{
    if (m_noteModeActive || !currentTrack || !scoreCanvas || !m_scoreCanvasWindow) {
        statusBar()->showMessage("Cannot enter Note Mode - no track or score canvas", 3000);
        return;
    }

    // Save current canvas state before entering note mode
    QJsonObject state;
    QJsonArray containersArray;
    for (Container* c : canvas->findChildren<Container*>()) {
        containersArray.append(canvas->serializeContainer(c));
    }
    state["containers"] = containersArray;
    QJsonArray connectionsArray;
    for (const Canvas::Connection& conn : canvas->getConnections()) {
        connectionsArray.append(canvas->serializeConnection(conn));
    }
    state["connections"] = connectionsArray;
    m_preModeCanvasState = state;

    // Require exactly one selected note in the score canvas
    const QVector<int>& selectedIndices = scoreCanvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) {
        QMessageBox::warning(this, "Note Mode",
            "Please select exactly one note before entering Note Mode.\n\n"
            "No note is currently selected.");
        return;
    }
    if (selectedIndices.size() > 1) {
        QMessageBox::warning(this, "Note Mode",
            "Please select exactly one note before entering Note Mode.\n\n"
            "Multiple notes are currently selected.");
        return;
    }
    int noteIndex = selectedIndices[0];

    m_noteModeActive = true;
    m_noteModeButton->setChecked(true);
    emit noteModeChanged(true);

    // Ensure base canvas state is saved for the track (needed for restoring base variation)
    if (!currentTrack->hasBaseCanvasState()) {
        currentTrack->saveBaseCanvasState();
    }

    noteModeLoadNote(noteIndex);
    statusBar()->showMessage("NOTE MODE - Left/Right: navigate | Space: play note | Esc: exit", 0);
}

void SounitBuilder::exitNoteMode()
{
    if (!m_noteModeActive) return;

    // Stop any active note-mode playback first (restores track notes)
    if (isPlaying) {
        stopPlayback();
    }

    // Save current note before exiting
    noteModeSaveCurrentNote();

    m_noteModeActive = false;
    m_noteModeNoteId.clear();
    m_noteModeNoteIndex = -1;

    // Restore canvas from pre-mode state
    if (!m_preModeCanvasState.isEmpty() && canvas) {
        // Clear current canvas
        canvas->getConnections().clear();
        QList<Container*> existingContainers = canvas->findChildren<Container*>();
        for (Container* c : existingContainers) {
            c->setParent(nullptr);
            c->deleteLater();
        }
        canvas->clearSelection();
        QCoreApplication::processEvents();

        canvas->setLoading(true);

        // Deserialize containers
        QJsonArray containersArray = m_preModeCanvasState["containers"].toArray();
        QMap<QString, Container*> containerMap;
        for (const QJsonValue& val : containersArray) {
            QJsonObject containerJson = val.toObject();
            Container* container = canvas->deserializeContainer(containerJson, canvas);
            if (container) {
                container->setParent(canvas);
                container->show();
                containerMap[container->getInstanceName()] = container;
            }
        }

        // Deserialize connections
        QJsonArray connsArray = m_preModeCanvasState["connections"].toArray();
        for (const QJsonValue& val : connsArray) {
            QJsonObject connJson = val.toObject();
            Container* fromC = containerMap.value(connJson["fromContainer"].toString(), nullptr);
            Container* toC = containerMap.value(connJson["toContainer"].toString(), nullptr);
            if (fromC && toC) {
                Canvas::Connection conn;
                conn.fromContainer = fromC;
                conn.fromPort = connJson["fromPort"].toString();
                conn.toContainer = toC;
                conn.toPort = connJson["toPort"].toString();
                conn.function = connJson["function"].toString("passthrough");
                conn.weight = connJson["weight"].toDouble(1.0);
                canvas->getConnections().append(conn);
            }
        }

        canvas->setLoading(false);
        canvas->update();

        // Reconnect container signals
        for (Container* c : canvas->findChildren<Container*>()) {
            connectContainerSignals(c);
        }

        // Rebuild preview graph
        rebuildGraph(currentEditingTrack);
    }

    m_preModeCanvasState = QJsonObject();
    statusBar()->clearMessage();
    m_noteModeButton->setChecked(false);

    // Clear dirty flag - canvas was restored to its pre-mode state, no actual user edits
    if (currentTrack) {
        currentTrack->clearSounitDirty();
    }

    emit noteModeChanged(false);

    qDebug() << "SounitBuilder: Exited Note Mode";
}

void SounitBuilder::noteModeLoadNote(int noteIndex)
{
    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    const Note& note = notes[noteIndex];
    m_noteModeNoteIndex = noteIndex;
    m_noteModeNoteId = note.getId();

    int varIndex = note.getVariationIndex();

    if (varIndex > 0) {
        // Load the variation's graph into the canvas
        currentTrack->loadVariationToCanvas(varIndex);
    } else {
        // Load the base canvas (pre-mode state)
        if (!m_preModeCanvasState.isEmpty()) {
            // Restore from pre-mode state (same as base graph)
            canvas->getConnections().clear();
            QList<Container*> existingContainers = canvas->findChildren<Container*>();
            for (Container* c : existingContainers) {
                c->setParent(nullptr);
                c->deleteLater();
            }
            canvas->clearSelection();
            QCoreApplication::processEvents();

            canvas->setLoading(true);

            QJsonArray containersArray = m_preModeCanvasState["containers"].toArray();
            QMap<QString, Container*> containerMap;
            for (const QJsonValue& val : containersArray) {
                QJsonObject containerJson = val.toObject();
                Container* container = canvas->deserializeContainer(containerJson, canvas);
                if (container) {
                    container->setParent(canvas);
                    container->show();
                    containerMap[container->getInstanceName()] = container;
                }
            }

            QJsonArray connsArray = m_preModeCanvasState["connections"].toArray();
            for (const QJsonValue& val : connsArray) {
                QJsonObject connJson = val.toObject();
                Container* fromC = containerMap.value(connJson["fromContainer"].toString(), nullptr);
                Container* toC = containerMap.value(connJson["toContainer"].toString(), nullptr);
                if (fromC && toC) {
                    Canvas::Connection conn;
                    conn.fromContainer = fromC;
                    conn.fromPort = connJson["fromPort"].toString();
                    conn.toContainer = toC;
                    conn.toPort = connJson["toPort"].toString();
                    conn.function = connJson["function"].toString("passthrough");
                    conn.weight = connJson["weight"].toDouble(1.0);
                    canvas->getConnections().append(conn);
                }
            }

            canvas->setLoading(false);
            canvas->update();
        }
    }

    // Reconnect container signals
    for (Container* c : canvas->findChildren<Container*>()) {
        connectContainerSignals(c);
    }

    // Rebuild preview graph
    rebuildGraph(currentEditingTrack);

    // Update now marker to note position
    m_scoreCanvasWindow->getTimeline()->setNowMarker(note.getStartTime());

    // Update status bar with note info
    QString info = QString("NOTE MODE [%1/%2] - Pitch: %3 Hz | Start: %4 ms | Var: %5")
        .arg(noteIndex + 1)
        .arg(notes.size())
        .arg(note.getPitchHz(), 0, 'f', 1)
        .arg(note.getStartTime(), 0, 'f', 0)
        .arg(varIndex > 0 ? QString::number(varIndex) : "Base");
    statusBar()->showMessage(info, 0);

    qDebug() << "SounitBuilder: Note Mode loaded note" << noteIndex
             << "id:" << note.getId() << "var:" << varIndex;
}

void SounitBuilder::noteModeSaveCurrentNote()
{
    if (m_noteModeNoteId.isEmpty() || !currentTrack) return;

    QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    // Find current note by ID
    int noteIdx = -1;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].getId() == m_noteModeNoteId) {
            noteIdx = i;
            break;
        }
    }
    if (noteIdx < 0) return;

    Note& note = notes[noteIdx];
    int currentVarIndex = note.getVariationIndex();

    // Rebuild track graph from current canvas state so the internal variation captures latest edits
    currentTrack->rebuildGraph(audioEngine ? audioEngine->getSampleRate() : 44100.0);

    // Create or update internal variation from current canvas
    int newVarIndex = currentTrack->createOrUpdateInternalVariation(currentVarIndex);

    if (newVarIndex > 0 && newVarIndex != currentVarIndex) {
        note.setVariationIndex(newVarIndex);
        note.setRenderDirty(true);
        qDebug() << "SounitBuilder: Note" << m_noteModeNoteId
                 << "assigned to internal variation" << newVarIndex;
    }
}

void SounitBuilder::noteModeNavigate(int direction)
{
    // Save current note's graph
    noteModeSaveCurrentNote();

    int trackIndex = scoreCanvas->getActiveTrack();
    int newIndex = findAdjacentNote(m_noteModeNoteIndex, direction, trackIndex);

    if (newIndex < 0) {
        statusBar()->showMessage(
            direction < 0 ? "Already at first note" : "Already at last note", 2000);
        // Re-show note mode status after timeout
        QTimer::singleShot(2100, this, [this]() {
            if (m_noteModeActive) {
                const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();
                statusBar()->showMessage(
                    QString("NOTE MODE [%1/%2] - Left/Right: navigate | Space: play | Esc: exit")
                        .arg(m_noteModeNoteIndex + 1).arg(notes.size()), 0);
            }
        });
        return;
    }

    noteModeLoadNote(newIndex);
}

void SounitBuilder::noteModePlayCurrentNote()
{
    if (!audioEngine || !currentTrack || m_noteModeNoteId.isEmpty()) return;

    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();
    if (m_noteModeNoteIndex < 0 || m_noteModeNoteIndex >= notes.size()) return;

    const Note& originalNote = notes[m_noteModeNoteIndex];

    // Rebuild track graph from current canvas (captures latest edits)
    currentTrack->rebuildGraph(audioEngine->getSampleRate());

    // Create a temporary note for playback: start at time 0, use base variation (current canvas IS the graph)
    Note playNote(0.0, originalNote.getDuration(), originalNote.getPitchHz(), originalNote.getDynamics());
    playNote.setPitchCurve(originalNote.getPitchCurve());
    playNote.setDynamicsCurve(originalNote.getDynamicsCurve());
    playNote.setBottomCurve(originalNote.getBottomCurve());
    playNote.setVibrato(originalNote.getVibrato());
    playNote.setVariationIndex(0);  // Use base = current canvas graph
    playNote.setTrackIndex(currentEditingTrack);

    // Save current notes - will be restored when playback stops
    m_noteModePlaybackSavedNotes = currentTrack->getNotes();

    // Replace track notes with just this one
    currentTrack->clearNotes();
    currentTrack->addNote(playNote);

    // Render and play
    bool rendered = currentTrack->prerenderDirtyNotes(audioEngine->getSampleRate());
    if (rendered) {
        m_noteModePlaying = true;

        QList<Track*> tracksToPlay;
        tracksToPlay.append(currentTrack);
        audioEngine->playFromTracks(tracksToPlay, 0.0);

        double totalDuration = currentTrack->getRenderedEndTimeMs();
        isPlaying = true;
        noteDuration = totalDuration;
        playbackStartTime = 0.0;
        playbackTimer->start(10);
    } else {
        // Render failed - restore notes immediately
        currentTrack->getNotesRef() = m_noteModePlaybackSavedNotes;
        m_noteModePlaybackSavedNotes.clear();
    }

    qDebug() << "SounitBuilder: Note Mode playing note" << m_noteModeNoteId;
}

int SounitBuilder::findNoteAtOrAfterTime(double timeMs, int trackIndex)
{
    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    // First pass: note where startTime <= timeMs < endTime on target track
    for (int i = 0; i < notes.size(); ++i) {
        const Note& note = notes[i];
        if (note.getTrackIndex() != trackIndex) continue;
        if (note.getStartTime() <= timeMs && timeMs < note.getEndTime()) {
            return i;
        }
    }

    // Second pass: first note with startTime >= timeMs
    int bestIndex = -1;
    double bestTime = std::numeric_limits<double>::max();
    for (int i = 0; i < notes.size(); ++i) {
        const Note& note = notes[i];
        if (note.getTrackIndex() != trackIndex) continue;
        if (note.getStartTime() >= timeMs && note.getStartTime() < bestTime) {
            bestTime = note.getStartTime();
            bestIndex = i;
        }
    }
    if (bestIndex >= 0) return bestIndex;

    // Third pass: last note before timeMs (fallback)
    double latestTime = -1.0;
    for (int i = 0; i < notes.size(); ++i) {
        const Note& note = notes[i];
        if (note.getTrackIndex() != trackIndex) continue;
        if (note.getStartTime() > latestTime) {
            latestTime = note.getStartTime();
            bestIndex = i;
        }
    }
    return bestIndex;
}

int SounitBuilder::findAdjacentNote(int currentIndex, int direction, int trackIndex)
{
    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    // Build sorted list of (startTime, originalIndex) for notes on this track
    QVector<QPair<double, int>> sorted;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].getTrackIndex() == trackIndex) {
            sorted.append({notes[i].getStartTime(), i});
        }
    }

    // Sort by start time
    std::sort(sorted.begin(), sorted.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first < b.first;
              });

    // Find current note's position in sorted list
    int pos = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i].second == currentIndex) {
            pos = i;
            break;
        }
    }

    if (pos < 0) return -1;

    int newPos = pos + direction;
    if (newPos < 0 || newPos >= sorted.size()) return -1;

    return sorted[newPos].second;
}

SounitBuilder::~SounitBuilder()
{
    delete ui;
}
