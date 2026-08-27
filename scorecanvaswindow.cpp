#include "scorecanvaswindow.h"
#include "ui_scorecanvas.h"
#include "kalamain.h"
#include "compositionsettingsdialog.h"
#include "gotodialog.h"
#include "trackmanager.h"
#include "track.h"
#include "canvas.h"
#include "tempotimesignature.h"
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QSettings>
#include <QCursor>
#include <QTimer>
#include <QElapsedTimer>
#include <QInputDialog>
#include <QMessageBox>
#include <QSignalBlocker>
#include <cmath>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include "midioutput.h"
#include "audiocapture.h"
#include "bakedialog.h"
#include "laneslegend.h"

ScoreCanvasWindow::ScoreCanvasWindow(AudioEngine *sharedAudioEngine, QWidget *parent)
    : QMainWindow{parent}
    , ui(new Ui::scorecanvas)
    , zoomModeActive(false)
    , currentMinHz(27.5)
    , currentMaxHz(4186.0)
    , currentPixelsPerSecond(100.0)
    , isDraggingZoom(false)
    , dragStartMinHz(27.5)
    , dragStartMaxHz(4186.0)
    , dragStartPixelsPerSecond(100.0)
    , zoomCenterHz(0.0)
    , zoomCenterTime(0.0)
    , panModeActive(false)
    , isDraggingPan(false)
    , panStartHorizontalScroll(0)
    , panStartVerticalScroll(0)
    , currentTimeMode(AbsoluteTime)
    , currentTempo(120)
    , currentTimeSigTop(5)
    , currentTimeSigBottom(0)
    , audioEngine(sharedAudioEngine)
    , trackManager(nullptr)  // Will be set via setTrackManager()
    , playbackTimer(nullptr)
    , currentNoteIndex(0)
    , playbackStartTime(0.0)
    , playbackStartPosition(0.0)
    , isPlaying(false)
    , nextColorIndex(0)
{
    qDebug() << "ScoreCanvasWindow: Starting constructor";
    ui->setupUi(this);

    // Restore window geometry from last session
    {
        QSettings settings;
        if (settings.contains("windows/scoreCanvas/geometry"))
            restoreGeometry(settings.value("windows/scoreCanvas/geometry").toByteArray());
    }

    // Initialize track color palette
    trackColorPalette = {
        QColor(0, 102, 204),   // Deep blue
        QColor(204, 0, 102),   // Deep pink
        QColor(102, 204, 0),   // Lime green
        QColor(204, 102, 0),   // Orange
        QColor(102, 0, 204),   // Purple
        QColor(0, 204, 204)    // Cyan
    };

    // Ensure menubar is visible and properly set
    if (ui->menubar) {
        setMenuBar(ui->menubar);
        ui->menubar->setVisible(true);
        ui->menubar->setNativeMenuBar(false);  // Don't use native menubar on macOS/Ubuntu
        qDebug() << "ScoreCanvasWindow: Menubar set and visible";
    } else {
        qDebug() << "ScoreCanvasWindow: WARNING - menubar is null!";
    }

    // Install event filter on application to catch all key events
    qApp->installEventFilter(this);

    // Create status bar labels (according to spec: Cursor time | Cursor pitch | Tempo | Time signature)
    cursorTimeLabel = new QLabel("0:00:000", this);
    cursorTimeLabel->setMinimumWidth(100);
    cursorTimeLabel->setStyleSheet("QLabel { padding: 0 5px; }");

    cursorPitchLabel = new QLabel("-- Hz; --", this);
    cursorPitchLabel->setMinimumWidth(150);
    cursorPitchLabel->setStyleSheet("QLabel { padding: 0 5px; }");

    statusTempoLabel = new QLabel("120 BPM", this);
    statusTempoLabel->setMinimumWidth(80);
    statusTempoLabel->setStyleSheet("QLabel { padding: 0 5px; }");

    statusTimeSigLabel = new QLabel("5 pulses", this);
    statusTimeSigLabel->setMinimumWidth(50);
    statusTimeSigLabel->setStyleSheet("QLabel { padding: 0 5px; }");

    // Create pressure status label (for pen tablet feedback)
    pressureLabel = new QLabel("Pressure: --", this);
    pressureLabel->setMinimumWidth(120);
    pressureLabel->setStyleSheet("QLabel { padding: 0 5px; }");

    // Create navigation mode label
    navigationModeLabel = new QLabel("", this);
    navigationModeLabel->setMinimumWidth(80);
    navigationModeLabel->setStyleSheet("QLabel { padding: 0 5px; font-weight: bold; }");

    // Add to bottom toolbar (status bar)
    ui->scoreCanvasBottomToolbar->addWidget(cursorTimeLabel);
    ui->scoreCanvasBottomToolbar->addSeparator();
    ui->scoreCanvasBottomToolbar->addWidget(cursorPitchLabel);
    ui->scoreCanvasBottomToolbar->addSeparator();
    ui->scoreCanvasBottomToolbar->addWidget(statusTempoLabel);
    ui->scoreCanvasBottomToolbar->addSeparator();
    ui->scoreCanvasBottomToolbar->addWidget(statusTimeSigLabel);
    ui->scoreCanvasBottomToolbar->addSeparator();
    ui->scoreCanvasBottomToolbar->addWidget(pressureLabel);
    ui->scoreCanvasBottomToolbar->addSeparator();
    ui->scoreCanvasBottomToolbar->addWidget(navigationModeLabel);

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
    ui->scoreCanvasBottomToolbar->addSeparator();
    ui->scoreCanvasBottomToolbar->addWidget(renderProgressBar);
    qDebug() << "ScoreCanvasWindow: Progress bar created and added to toolbar";

    // Connect AudioEngine progress signals
    if (audioEngine) {
        connect(audioEngine, &AudioEngine::renderStarted, this, &ScoreCanvasWindow::onRenderStarted);
        connect(audioEngine, &AudioEngine::renderProgressChanged, this, &ScoreCanvasWindow::onRenderProgressChanged);
        connect(audioEngine, &AudioEngine::renderCompleted, this, &ScoreCanvasWindow::onRenderCompleted);
        qDebug() << "ScoreCanvasWindow: Connected to AudioEngine progress signals";
    } else {
        qDebug() << "ScoreCanvasWindow: WARNING - AudioEngine is null, cannot connect progress signals!";
    }

    qDebug() << "ScoreCanvasWindow: Setting up UI components";
    setupToolbarColors();
    setupDrawingTools();
    setupCompositionSettings();
    setupCurveSelector();
    setupTrackSelector();
    setupScoreCanvas();
    setupZoom();
    setupLanesControls();

    // Initialize composition settings with defaults and apply them
    compositionSettings = CompositionSettings::defaults();
    compositionName = compositionSettings.compositionName;
    updateFromSettings(compositionSettings);
    qDebug() << "ScoreCanvasWindow: Applied default composition settings -"
             << compositionSettings.compositionName
             << compositionSettings.tempo << "BPM"
             << compositionSettings.timeSigTop << "/" << compositionSettings.timeSigBottom;

    // Initialize playback timer
    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &ScoreCanvasWindow::onPlaybackTick);

    // Background pre-render debounce timer (fires 400ms after last edit)
    m_renderDebounceTimer = new QTimer(this);
    m_renderDebounceTimer->setSingleShot(true);
    m_renderDebounceTimer->setInterval(400);
    connect(m_renderDebounceTimer, &QTimer::timeout,
            this, &ScoreCanvasWindow::scheduleBackgroundRender);

    // Connect transport controls
    connect(ui->actionPlay, &QAction::triggered, this, [this]() { startPlayback(); });
    connect(ui->actionstop, &QAction::triggered, this, [this]() { stopPlayback(); });

    // Connect track selection to trigger pre-rendering
    connect(trackSelector, &TrackSelector::trackSelected, this, &ScoreCanvasWindow::onTrackSelected);

    // Connect track context menu delete request to main window
    connect(trackSelector, &TrackSelector::deleteTrackRequested, this, [this](int trackIndex) {
        if (m_mainWindow) {
            m_mainWindow->deleteTrackAtIndex(trackIndex);
        }
    });

    qDebug() << "ScoreCanvasWindow: Constructor complete";
}


void ScoreCanvasWindow::setupToolbarColors()
{
    // Get the tool buttons from the top toolbar
    QToolButton* drawDiscreteBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionDrawDiscreteNotes));

    QToolButton* drawContinuousBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionDrawContinuousNotes));

    QToolButton* recordDiscreteBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionRecordDiscreteNotes));

    QToolButton* recordContinuousBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionRecordContinuousNotes));

    QToolButton* recordPhraseBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionRecordPhrase));

    QToolButton* selectBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionScoreCanvasSelect));

    QToolButton* zoomBtn = qobject_cast<QToolButton*>(
        ui->toolBar->widgetForAction(ui->actionScoreCanvasZoom));

    // Apply colors with checked state highlighting
    if (drawDiscreteBtn) {
        drawDiscreteBtn->setStyleSheet(
            "QToolButton { background-color: #90CAF9; }"  // Light blue
            "QToolButton:checked { background-color: #E57373; }"  // Medium red when active
        );
    }
    if (drawContinuousBtn) {
        drawContinuousBtn->setStyleSheet(
            "QToolButton { background-color: #BBDEFB; }"
            "QToolButton:checked { background-color: #E57373; }"  // Medium red when active
        );
    }

    if (recordDiscreteBtn) {
        recordDiscreteBtn->setStyleSheet("QToolButton { background-color: #FFCC80; }");  // Light orange
    }
    if (recordContinuousBtn) {
        recordContinuousBtn->setStyleSheet("QToolButton { background-color: #FFE0B2; }");
    }
    if (recordPhraseBtn) {
        recordPhraseBtn->setStyleSheet("QToolButton { background-color: #FFCC80; }");
    }

    if (selectBtn) {
        selectBtn->setStyleSheet(
            "QToolButton { background-color: #A5D6A7; }"  // Light green
            "QToolButton:checked { background-color: #E57373; }"  // Medium red when active
        );
    }
    if (zoomBtn) {
        zoomBtn->setStyleSheet(
            "QToolButton { background-color: #C8E6C9; }"  // Light green
            "QToolButton:checked { background-color: #E57373; }"  // Medium red when active
        );
    }
}

void ScoreCanvasWindow::setupDrawingTools()
{
    // Create action group for mutually exclusive drawing tools
    drawingToolGroup = new QActionGroup(this);
    drawingToolGroup->setExclusive(true);

    // Make actions checkable and add to group
    ui->actionDrawDiscreteNotes->setCheckable(true);
    ui->actionDrawContinuousNotes->setCheckable(true);
    ui->actionScoreCanvasSelect->setCheckable(true);

    drawingToolGroup->addAction(ui->actionDrawDiscreteNotes);
    drawingToolGroup->addAction(ui->actionDrawContinuousNotes);
    drawingToolGroup->addAction(ui->actionScoreCanvasSelect);

    // Set discrete drawing as default (will be connected to ScoreCanvas after it's created)
    ui->actionDrawDiscreteNotes->setChecked(true);

    // Add keyboard shortcuts (application-level, work regardless of focus)
    ui->actionScoreCanvasSelect->setShortcut(QKeySequence(Qt::Key_S));
    ui->actionScoreCanvasSelect->setShortcutContext(Qt::ApplicationShortcut);

    ui->actionDrawDiscreteNotes->setShortcut(QKeySequence(Qt::Key_D));
    ui->actionDrawDiscreteNotes->setShortcutContext(Qt::ApplicationShortcut);

    ui->actionDrawContinuousNotes->setShortcut(QKeySequence(Qt::Key_C));
    ui->actionDrawContinuousNotes->setShortcutContext(Qt::ApplicationShortcut);

    qDebug() << "ScoreCanvasWindow: Drawing tool group configured with shortcuts";
}

void ScoreCanvasWindow::setupCompositionSettings()
{
    // Add separator before composition settings
    ui->toolBar->addSeparator();

    // Time Mode Toggle Button (Absolute ↔ Musical)
    timeModeToggle = new QPushButton("Absolute", this);
    timeModeToggle->setCheckable(true);
    timeModeToggle->setMinimumWidth(80);
    timeModeToggle->setFocusPolicy(Qt::NoFocus);  // Don't steal keyboard focus
    timeModeToggle->setStyleSheet(
        "QPushButton {"
        "    background-color: #B0BEC5;"  // Blue-grey
        "    padding: 5px 10px;"
        "    border: 1px solid #78909C;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #66BB6A;"  // Green when Musical mode
        "}"
    );
    ui->toolBar->addWidget(timeModeToggle);
    connect(timeModeToggle, &QPushButton::clicked, this, &ScoreCanvasWindow::onTimeModeToggled);

    ui->toolBar->addSeparator();

    // Slide Mode Toggle Button (time-axis-only note movement)
    slideModeBtn = new QPushButton("Slide", this);
    slideModeBtn->setCheckable(true);
    slideModeBtn->setMinimumWidth(60);
    slideModeBtn->setFocusPolicy(Qt::NoFocus);
    slideModeBtn->setToolTip("Slide mode: drag notes in time only, pitch is locked (shortcut: A)");
    slideModeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #A5D6A7;"  // Green when inactive
        "    padding: 5px 10px;"
        "    border: 1px solid #66BB6A;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #E57373;"  // Red when active
        "    border-color: #C62828;"
        "}"
    );
    ui->toolBar->addWidget(slideModeBtn);

    // Transform Mode Toggle Button (pitch-curve perimeter handles for the selected note)
    transformModeBtn = new QPushButton("Transform", this);
    transformModeBtn->setCheckable(true);
    transformModeBtn->setMinimumWidth(80);
    transformModeBtn->setFocusPolicy(Qt::NoFocus);
    transformModeBtn->setToolTip("Transform mode: scale/rotate/stretch the pitch curve of the selected note (shortcut: T)");
    transformModeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #B39DDB;"  // Soft purple when inactive
        "    padding: 5px 10px;"
        "    border: 1px solid #7E57C2;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #5E35B1;"  // Deep purple when active
        "    color: white;"
        "    border-color: #311B92;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #ECEFF1;"
        "    color: #B0BEC5;"
        "    border-color: #CFD8DC;"
        "}"
    );
    transformModeBtn->setEnabled(false);  // Requires a single-note selection
    ui->toolBar->addWidget(transformModeBtn);

    ui->toolBar->addSeparator();

    // Auto/Manual Render Mode Toggle
    renderModeToggle = new QPushButton("Auto", this);
    renderModeToggle->setCheckable(true);
    renderModeToggle->setChecked(true);  // Default: Auto
    renderModeToggle->setMinimumWidth(70);
    renderModeToggle->setFocusPolicy(Qt::NoFocus);
    renderModeToggle->setToolTip("Auto: re-render on every edit\nManual: render only when play is pressed");
    renderModeToggle->setStyleSheet(
        "QPushButton {"
        "    background-color: #64B5F6;"  // Blue when Auto
        "    padding: 5px 10px;"
        "    border: 1px solid #42A5F5;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #64B5F6;"  // Blue when Auto
        "}"
        "QPushButton:!checked {"
        "    background-color: #B0BEC5;"  // Grey when Manual
        "    border-color: #78909C;"
        "}"
    );
    ui->toolBar->addWidget(renderModeToggle);
    connect(renderModeToggle, &QPushButton::clicked, this, &ScoreCanvasWindow::onRenderModeToggled);

    // MIDI test button — quick hello-note to verify the MIDI-out setup
    // (currently the VL70-m; stays as a permanent hardware check).
    QPushButton *midiTestBtn = new QPushButton("MIDI test", this);
    midiTestBtn->setMinimumWidth(70);
    midiTestBtn->setFocusPolicy(Qt::NoFocus);
    midiTestBtn->setToolTip("Send a test note to the first MIDI output port");
    midiTestBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #4DB6AC;"  // Teal
        "    padding: 5px 10px;"
        "    border: 1px solid #26A69A;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #80CBC4;"
        "}"
    );
    ui->toolBar->addWidget(midiTestBtn);
    connect(midiTestBtn, &QPushButton::clicked, this, &ScoreCanvasWindow::onMidiTestClicked);

    // Bake button — records the VL70-m's audio input during playback and
    // attaches the recording to a track as a clip (Phase 9).
    QPushButton *bakeBtn = new QPushButton("Bake", this);
    bakeBtn->setMinimumWidth(70);
    bakeBtn->setFocusPolicy(Qt::NoFocus);
    bakeBtn->setToolTip("Record the VL70-m's audio while the composition plays, then attach it to a track");
    bakeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #FFB74D;"  // Amber
        "    padding: 5px 10px;"
        "    border: 1px solid #FF9800;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #FFCC80;"
        "}"
    );
    ui->toolBar->addWidget(bakeBtn);
    connect(bakeBtn, &QPushButton::clicked, this, &ScoreCanvasWindow::onBakeClicked);

    ui->toolBar->addSeparator();

    // Spacer to push time signature UI to the right
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBar->addWidget(spacer);

    // Time signature mode combo + stacked widget
    timeSignatureModeCombo = new QComboBox(this);
    timeSignatureModeCombo->addItem("Kala time signature");
    timeSignatureModeCombo->addItem("Western time signature");
    timeSignatureModeCombo->setMinimumWidth(140);
    timeSignatureModeCombo->setFocusPolicy(Qt::StrongFocus);
    ui->toolBar->addWidget(timeSignatureModeCombo);

    // --- Kala page ---
    kalaTimeSigPage = new QWidget(this);
    QHBoxLayout *kalaLayout = new QHBoxLayout(kalaTimeSigPage);
    kalaLayout->setContentsMargins(4, 0, 4, 0);
    kalaLayout->setSpacing(4);

    QLabel *kalaPulsesLabel = new QLabel("Pulses/bar:", this);
    kalaLayout->addWidget(kalaPulsesLabel);

    kalaPulsesSpin = new QSpinBox(this);
    kalaPulsesSpin->setRange(1, 99);
    kalaPulsesSpin->setValue(cachedKalaPulses);
    kalaPulsesSpin->setMinimumWidth(50);
    kalaPulsesSpin->setFocusPolicy(Qt::StrongFocus);
    kalaLayout->addWidget(kalaPulsesSpin);

    QLabel *kalaDurationLabel = new QLabel("Pulse (ms):", this);
    kalaLayout->addWidget(kalaDurationLabel);

    kalaDurationSpin = new QSpinBox(this);
    kalaDurationSpin->setRange(1, 60000);
    kalaDurationSpin->setValue(cachedKalaDurationMs);
    kalaDurationSpin->setMinimumWidth(70);
    kalaDurationSpin->setFocusPolicy(Qt::StrongFocus);
    kalaLayout->addWidget(kalaDurationSpin);
    kalaLayout->addStretch();

    // --- Western page ---
    westernTimeSigPage = new QWidget(this);
    QHBoxLayout *westernLayout = new QHBoxLayout(westernTimeSigPage);
    westernLayout->setContentsMargins(4, 0, 4, 0);
    westernLayout->setSpacing(4);

    QLabel *westernTempoLabel = new QLabel("Tempo:", this);
    westernLayout->addWidget(westernTempoLabel);

    westernTempoSpin = new QSpinBox(this);
    westernTempoSpin->setRange(1, 300);
    westernTempoSpin->setValue(cachedWesternTempo);
    westernTempoSpin->setSuffix(" BPM");
    westernTempoSpin->setMinimumWidth(80);
    westernTempoSpin->setFocusPolicy(Qt::StrongFocus);
    westernLayout->addWidget(westernTempoSpin);

    QLabel *westernNumLabel = new QLabel("Num:", this);
    westernLayout->addWidget(westernNumLabel);

    westernNumSpin = new QSpinBox(this);
    westernNumSpin->setRange(1, 99);
    westernNumSpin->setValue(cachedWesternNum);
    westernNumSpin->setMinimumWidth(50);
    westernNumSpin->setFocusPolicy(Qt::StrongFocus);
    westernLayout->addWidget(westernNumSpin);

    QLabel *westernDenLabel = new QLabel("Den:", this);
    westernLayout->addWidget(westernDenLabel);

    westernDenSpin = new QSpinBox(this);
    westernDenSpin->setRange(0, 99);
    westernDenSpin->setValue(cachedWesternDen);
    westernDenSpin->setMinimumWidth(50);
    westernDenSpin->setFocusPolicy(Qt::StrongFocus);
    westernLayout->addWidget(westernDenSpin);
    westernLayout->addStretch();

    // --- Stacked widget ---
    timeSigStack = new QStackedWidget(this);
    timeSigStack->addWidget(kalaTimeSigPage);    // index 0
    timeSigStack->addWidget(westernTimeSigPage); // index 1
    timeSigStack->setCurrentIndex(0);  // Default to Kala
    ui->toolBar->addWidget(timeSigStack);

    // Connect signals
    connect(timeSignatureModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScoreCanvasWindow::onTimeSignatureModeChanged);
    connect(kalaPulsesSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSettingsChanged);
    connect(kalaDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSettingsChanged);
    connect(westernTempoSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSettingsChanged);
    connect(westernNumSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSettingsChanged);
    connect(westernDenSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSettingsChanged);

    // Connect File menu action
    connect(ui->actionCompositionSettings, &QAction::triggered,
            this, &ScoreCanvasWindow::onCompositionSettingsTriggered);
    connect(ui->actionAddTrack, &QAction::triggered,
            this, &ScoreCanvasWindow::onAddTrackTriggered);

    qDebug() << "ScoreCanvasWindow: Composition settings configured";
}

void ScoreCanvasWindow::setupCurveSelector()
{
    ui->toolBar->addSeparator();

    QLabel *curveLabel = new QLabel(" Show curve: ", this);
    ui->toolBar->addWidget(curveLabel);

    curveSelectorCombo = new QComboBox(this);
    curveSelectorCombo->setMinimumWidth(140);
    curveSelectorCombo->setFocusPolicy(Qt::StrongFocus);
    curveSelectorCombo->setToolTip("Which expressive curve is drawn under the notes");
    curveSelectorCombo->addItem(QStringLiteral("Dynamics"), QStringLiteral("Dynamics"));
    ui->toolBar->addWidget(curveSelectorCombo);

    connect(curveSelectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScoreCanvasWindow::onCurveSelectorChanged);
}

void ScoreCanvasWindow::setupLanesControls()
{
    // Curve lanes Phase F: toolbar tier control, split-point spin + Auto,
    // dim-others, legend panel, and the real Ctrl+L shortcut. The density
    // tier itself is NEVER persisted (Nimus 2026-08-26: "Non save") - it
    // resets to Off each session. Split, dim-others and legend visibility
    // are restored below and saved in closeEvent.

    // Ctrl+L cycles Off -> Overlay -> Lanes. Application-wide so it works
    // while any toolbar combo holds focus; the canvas keyPressEvent no
    // longer handles it. Hidden action - the tier combo is the display.
    tierCycleAction = new QAction(this);
    tierCycleAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    tierCycleAction->setShortcutContext(Qt::ApplicationShortcut);
    addAction(tierCycleAction);
    connect(tierCycleAction, &QAction::triggered, this, &ScoreCanvasWindow::onCycleTier);

    // Density tier combo. Qt::NoFocus deliberately: the two StrongFocus
    // combos can swallow Ctrl+L, and this one must never.
    tierCombo = new QComboBox(this);
    tierCombo->addItem(QStringLiteral("Off"), int(ScoreCanvas::DensityTier::Off));
    tierCombo->addItem(QStringLiteral("Overlay"), int(ScoreCanvas::DensityTier::Overlay));
    tierCombo->addItem(QStringLiteral("Lanes"), int(ScoreCanvas::DensityTier::Lanes));
    tierCombo->setFocusPolicy(Qt::NoFocus);
    tierCombo->setToolTip("Curve lanes density tier (Ctrl+L cycles)");
    ui->toolBar->addWidget(tierCombo);
    connect(tierCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (!scoreCanvas || index < 0) return;
                scoreCanvas->setDensityTier(static_cast<ScoreCanvas::DensityTier>(index));
            });
    connect(scoreCanvas, &ScoreCanvas::densityTierChanged, this,
            &ScoreCanvasWindow::refreshTierWidget);

    // Split point (D1): how many lanes sit above the hull. Auto = balanced.
    // The Ctrl+Shift+Up/Down keyboard nudge (canvas) also drives this via
    // lanesAboveChanged - first-class control, not a convenience.
    ui->toolBar->addWidget(new QLabel(" Above: ", this));
    lanesAboveSpin = new QSpinBox(this);
    lanesAboveSpin->setRange(0, 22);  // max VL70-m rows (curvelanes.h)
    lanesAboveSpin->setMaximumWidth(60);
    lanesAboveSpin->setFocusPolicy(Qt::NoFocus);
    lanesAboveSpin->setToolTip("Lanes above the note hull (Ctrl+Shift+Up/Down nudges this)");
    ui->toolBar->addWidget(lanesAboveSpin);
    lanesAboveAutoCheck = new QCheckBox("Auto", this);
    lanesAboveAutoCheck->setFocusPolicy(Qt::NoFocus);
    lanesAboveAutoCheck->setToolTip("Auto: balanced split (Ctrl+Shift+Home restores this)");
    ui->toolBar->addWidget(lanesAboveAutoCheck);
    connect(lanesAboveSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                if (scoreCanvas) scoreCanvas->setLanesAbove(value);
            });
    connect(lanesAboveAutoCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked && scoreCanvas) scoreCanvas->setLanesAbove(-1);
    });
    connect(scoreCanvas, &ScoreCanvas::lanesAboveChanged, this,
            &ScoreCanvasWindow::refreshLanesAboveControls);

    // Dim-others (D9): every note except the single selected one drops to
    // low alpha. Orthogonal to the density tier.
    dimOthersBtn = new QPushButton("Dim others", this);
    dimOthersBtn->setCheckable(true);
    dimOthersBtn->setMinimumWidth(70);
    dimOthersBtn->setFocusPolicy(Qt::NoFocus);
    dimOthersBtn->setToolTip("Dim every note except the single selected one");
    dimOthersBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #B0BEC5;"
        "    padding: 5px 10px;"
        "    border: 1px solid #78909C;"
        "    border-radius: 3px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #66BB6A;"
        "}"
    );
    ui->toolBar->addWidget(dimOthersBtn);
    connect(dimOthersBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (scoreCanvas) scoreCanvas->setDimOthers(checked);
    });

    // Legend panel (D6): the selected note's lane set, in order, with family
    // swatches. Toggled here - ScoreCanvasWindow has no View menu of its own.
    legendBtn = new QPushButton("Legend", this);
    legendBtn->setCheckable(true);
    legendBtn->setMinimumWidth(60);
    legendBtn->setFocusPolicy(Qt::NoFocus);
    legendBtn->setToolTip("Show the selected note's curve-lane legend");
    ui->toolBar->addWidget(legendBtn);
    lanesLegend = new LanesLegend(this);
    lanesLegend->setScoreCanvas(scoreCanvas);
    connect(legendBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (lanesLegend) lanesLegend->setVisible(checked);
    });
    connect(lanesLegend, &LanesLegend::visibilityChanged, this, [this](bool visible) {
        if (legendBtn) legendBtn->setChecked(visible);
    });

    // Restore persisted view settings. The tier stays Off - never saved.
    {
        QSettings settings;
        scoreCanvas->setLanesAbove(settings.value("scoreCanvas/lanesAbove", -1).toInt());
        scoreCanvas->setDimOthers(settings.value("scoreCanvas/dimOthers", false).toBool());
        dimOthersBtn->setChecked(scoreCanvas->getDimOthers());
        refreshTierWidget();
        refreshLanesAboveControls();
        lanesLegend->setVisible(settings.value("scoreCanvas/legendVisible", false).toBool());
    }

    // Legend content follows selection, curve applications and undo. Note
    // selection also refreshes the Auto spin's displayed split.
    connect(scoreCanvas, &ScoreCanvas::noteSelectionChanged, this, [this]() {
        if (lanesLegend) lanesLegend->refresh();
        refreshLanesAboveControls();
    });
    connect(scoreCanvas, &ScoreCanvas::expressiveCurveApplied, this, [this](const QString &) {
        if (lanesLegend) lanesLegend->refresh();
    });
}

void ScoreCanvasWindow::onCycleTier()
{
    if (!scoreCanvas) return;
    switch (scoreCanvas->getDensityTier()) {
    case ScoreCanvas::DensityTier::Off:     scoreCanvas->setDensityTier(ScoreCanvas::DensityTier::Overlay); break;
    case ScoreCanvas::DensityTier::Overlay: scoreCanvas->setDensityTier(ScoreCanvas::DensityTier::Lanes);   break;
    case ScoreCanvas::DensityTier::Lanes:   scoreCanvas->setDensityTier(ScoreCanvas::DensityTier::Off);     break;
    }
}

void ScoreCanvasWindow::refreshTierWidget()
{
    if (!tierCombo || !scoreCanvas) return;
    {
        QSignalBlocker blocker(tierCombo);
        tierCombo->setCurrentIndex(static_cast<int>(scoreCanvas->getDensityTier()));
    }
    // The split control only means something in the Lanes tier. The spin's
    // enabled state is tier AND auto - recompute both together so cycling the
    // tier can't re-enable a spin that Auto mode disables.
    refreshLanesAboveControls();
}

void ScoreCanvasWindow::refreshLanesAboveControls()
{
    if (!scoreCanvas || !lanesAboveSpin || !lanesAboveAutoCheck) return;
    const bool tierLanes = scoreCanvas->getDensityTier() == ScoreCanvas::DensityTier::Lanes;
    {
        QSignalBlocker spinBlock(lanesAboveSpin);
        QSignalBlocker checkBlock(lanesAboveAutoCheck);
        if (scoreCanvas->getLanesAbove() < 0) {
            lanesAboveAutoCheck->setChecked(true);
            lanesAboveAutoCheck->setEnabled(tierLanes);
            lanesAboveSpin->setEnabled(false);  // auto owns the split
            // Show the effective balanced split for the selected note -
            // real information, not a fake value.
            const QVector<int> &sel = scoreCanvas->getSelectedNoteIndices();
            const int noteIndex = sel.isEmpty() ? -1 : sel.first();
            lanesAboveSpin->setValue(scoreCanvas->lanesAboveForNote(noteIndex));
        } else {
            lanesAboveAutoCheck->setChecked(false);
            lanesAboveAutoCheck->setEnabled(tierLanes);
            lanesAboveSpin->setEnabled(tierLanes);
            lanesAboveSpin->setValue(scoreCanvas->getLanesAbove());
        }
    }
}

void ScoreCanvasWindow::refreshCurveSelector()
{
    if (!curveSelectorCombo) return;

    // Preserve the current selection by name so switching tracks doesn't jolt the display
    QString previousName = curveSelectorCombo->currentData().toString();
    if (previousName.isEmpty()) previousName = QStringLiteral("Dynamics");

    curveSelectorCombo->blockSignals(true);
    curveSelectorCombo->clear();
    curveSelectorCombo->addItem(QStringLiteral("Dynamics"), QStringLiteral("Dynamics"));
    // Union of names declared by the base sounit + every variation's canvas,
    // so notes on any variation can still have their curve rendered.
    // resolveActiveCurveIndex() falls back to Dynamics per-note if a given
    // note doesn't carry the chosen name.
    QStringList unionNames;
    if (m_curveNamesTrack) {
        QStringList seen;
        auto addFrom = [&](Canvas *c) {
            if (!c) return;
            for (const QString &name : c->getExpressiveCurveNames()) {
                if (name.isEmpty() || seen.contains(name)) continue;
                seen.append(name);
                unionNames.append(name);
            }
        };
        addFrom(m_curveNamesTrack->getCanvas());
        int varCount = m_curveNamesTrack->getVariationCount();
        for (int i = 1; i <= varCount; ++i) {
            addFrom(m_curveNamesTrack->getCanvasForVariation(i));
        }
    }

    // If the union contains all 10 "band 1".."band 10" names (case/whitespace
    // insensitive), insert the "EQ Curve" pseudo-entry right after Dynamics.
    // It groups the 10 band curves so the user can view/edit them as one EQ.
    auto hasAllBands = [&unionNames]() {
        for (int b = 1; b <= 10; ++b) {
            QString expected = QString("band %1").arg(b);
            bool found = false;
            for (const QString &n : unionNames) {
                if (n.simplified().toLower() == expected) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    };
    if (hasAllBands()) {
        curveSelectorCombo->addItem(QStringLiteral("EQ Curve"), QStringLiteral("EQ Curve"));
    }

    for (const QString &name : unionNames) {
        curveSelectorCombo->addItem(name, name);
    }
    int idx = curveSelectorCombo->findData(previousName);
    curveSelectorCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    curveSelectorCombo->blockSignals(false);

    // Sync the score to whatever we ended up showing
    if (scoreCanvas) {
        int activeIdx = curveSelectorCombo->currentIndex();
        QString activeName = curveSelectorCombo->currentData().toString();
        scoreCanvas->setActiveExpressiveCurveIndex(activeIdx, activeName);
    }
}

void ScoreCanvasWindow::bindCurveNamesFromTrack(Track *track)
{
    if (m_curveNamesTrack == track) return;
    // Drop previous wiring
    for (const QMetaObject::Connection &c : m_curveNamesConnections) disconnect(c);
    m_curveNamesConnections.clear();
    m_curveNamesTrack = track;
    if (m_curveNamesTrack) {
        Track *track = m_curveNamesTrack;
        // A live-name change (VL70-m row toggled, name edited) reflows the
        // ribbon and the legend: repaint the canvas so lane states update and
        // refresh the legend so its rows follow.
        auto onNamesChanged = [this]() {
            refreshCurveSelector();
            if (scoreCanvas) scoreCanvas->update();
            if (lanesLegend) lanesLegend->refresh();
            refreshLanesAboveControls();
        };
        // Base canvas still emits when user edits names in the Envelope inspector
        if (Canvas *base = track->getCanvas()) {
            m_curveNamesConnections.append(
                connect(base, &Canvas::expressiveCurveNamesChanged,
                        onNamesChanged));
        }
        // Variation canvases emit when VL70-m rows toggle (Phase 6), so the
        // union refreshes live. ("Variations are immutable" no longer holds
        // for the curve-name list.) Hook existing variations now and each
        // new one as it is created.
        auto connectVariation = [this, onNamesChanged](int index) {
            if (Canvas *varCanvas = m_curveNamesTrack->getCanvasForVariation(index)) {
                m_curveNamesConnections.append(
                    connect(varCanvas, &Canvas::expressiveCurveNamesChanged,
                            onNamesChanged));
            }
        };
        for (int i = 1; i <= track->getVariationCount(); ++i)
            connectVariation(i);
        m_curveNamesConnections.append(
            connect(track, &Track::variationCreated,
                    this, [this, connectVariation](int index, const QString&) {
                        connectVariation(index);
                        refreshCurveSelector();
                    }));
        m_curveNamesConnections.append(
            connect(m_curveNamesTrack, &Track::variationDeleted,
                    this, [this](int) { refreshCurveSelector(); }));
        m_curveNamesConnections.append(
            connect(m_curveNamesTrack, &Track::variationRenamed,
                    this, [this](int, const QString&) { refreshCurveSelector(); }));
    }
    refreshCurveSelector();
}

void ScoreCanvasWindow::onCurveSelectorChanged(int index)
{
    if (!curveSelectorCombo || !scoreCanvas) return;
    QString name = curveSelectorCombo->itemData(index).toString();
    scoreCanvas->setActiveExpressiveCurveIndex(index, name);
}

void ScoreCanvasWindow::setupTrackSelector()
{
    // Note: old UI widgets will be deleted in setupScoreCanvas()

    // Create TrackSelector
    trackSelector = new TrackSelector();
    // No max width - let it grow based on number of tracks
    // Scrollbar will appear in trackScrollArea when > 4 tracks
    trackSelector->setMinimumHeight(2000);  // Match ScoreCanvas height for scrolling

    // Set frequency range (typical musical range: A0 to C8)
    trackSelector->setFrequencyRange(currentMinHz, currentMaxHz);

    // Add default track (will be replaced when sounit is loaded)
    trackSelector->addTrack("(No Sounit)", QColor("#999999"), 27.5, 4186.0);
}

void ScoreCanvasWindow::setupScoreCanvas()
{
    // Create ScoreCanvas widget
    scoreCanvas = new ScoreCanvas();
    connect(slideModeBtn, &QPushButton::clicked, scoreCanvas, &ScoreCanvas::toggleSlideMode);
    connect(transformModeBtn, &QPushButton::clicked, scoreCanvas, &ScoreCanvas::toggleTransformMode);

    // Set minimum size for ScoreCanvas to enable scrolling
    // Width: calculated from composition length and pixels per second
    // Height: based on visible octave range
    // Note: Will be updated when composition settings are applied
    int initialWidth = static_cast<int>(currentPixelsPerSecond * (300000.0 / 1000.0));  // 5 minutes default
    scoreCanvas->setMinimumSize(initialWidth, 2000);

    // Set the same frequency range as TrackSelector for synchronization
    scoreCanvas->setFrequencyRange(currentMinHz, currentMaxHz);

    // Set initial horizontal zoom (pixels per second)
    scoreCanvas->setPixelsPerSecond(currentPixelsPerSecond);

    // Create scroll area for ScoreCanvas
    scoreScrollArea = new QScrollArea(ui->centralwidget);
    scoreScrollArea->setWidget(scoreCanvas);
    scoreScrollArea->setWidgetResizable(false);  // Don't auto-resize widget
    scoreScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scoreScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Apply light gold styling to scroll bars
    scoreScrollArea->setStyleSheet(
        "QScrollBar:vertical {"
        "    background: rgb(255, 235, 205);"  // Light gold
        "    width: 15px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgb(200, 180, 150);"  // Darker gold for handle
        "    min-height: 20px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    background: rgb(255, 235, 205);"
        "    height: 0px;"
        "}"
        "QScrollBar:horizontal {"
        "    background: rgb(255, 235, 205);"  // Light gold
        "    height: 15px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: rgb(200, 180, 150);"  // Darker gold for handle
        "    min-width: 20px;"
        "    border-radius: 3px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    background: rgb(255, 235, 205);"
        "    width: 0px;"
        "}"
    );

    // Create scroll area for TrackSelector
    // Max width fits 4 tracks: 4 * (TRACK_BAR_WIDTH + TRACK_SPACING) = 4 * 42 = 168
    // Add a bit of margin for visual comfort
    static constexpr int MAX_VISIBLE_TRACKS = 4;
    static constexpr int TRACK_BAR_WIDTH = 40;
    static constexpr int TRACK_SPACING = 2;
    int trackScrollAreaWidth = MAX_VISIBLE_TRACKS * (TRACK_BAR_WIDTH + TRACK_SPACING) + 4;  // +4 for margin

    trackScrollArea = new QScrollArea(ui->centralwidget);
    trackScrollArea->setWidget(trackSelector);
    trackScrollArea->setWidgetResizable(false);
    trackScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);  // Show when > 4 tracks
    trackScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // Hidden, synced with score
    trackScrollArea->setMinimumWidth(trackScrollAreaWidth);
    trackScrollArea->setMaximumWidth(trackScrollAreaWidth);

    // Setup layouts for proper resizing
    // DELETE the old fixed-geometry widgets from the .ui file
    // (They conflict with the layout system)
    if (ui->horizontalLayoutWidget) {
        delete ui->horizontalLayoutWidget;
        ui->horizontalLayoutWidget = nullptr;
    }
    if (ui->scrollAreatracks) {
        delete ui->scrollAreatracks;
        ui->scrollAreatracks = nullptr;
    }

    // Create main vertical layout for the central widget
    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create FrequencyLabels widget
    frequencyLabels = new FrequencyLabels(ui->centralwidget);
    frequencyLabels->setFrequencyRange(currentMinHz, currentMaxHz);
    frequencyLabels->setMinimumHeight(2000);

    // Create scroll area for FrequencyLabels (scroll controlled only by score canvas sync)
    frequencyScrollArea = new QScrollArea(ui->centralwidget);
    frequencyScrollArea->setWidget(frequencyLabels);
    frequencyScrollArea->setWidgetResizable(false);
    frequencyScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frequencyScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frequencyScrollArea->setMinimumWidth(60);
    frequencyScrollArea->setMaximumWidth(60);
    frequencyScrollArea->setFocusPolicy(Qt::NoFocus);
    frequencyScrollArea->viewport()->installEventFilter(this);
    frequencyLabels->installEventFilter(this);

    // Create Timeline widget
    timeline = new Timeline(ui->centralwidget);
    timeline->setPixelsPerSecond(100.0);  // Initial zoom level

    // Connect timeline signals
    connect(timeline, &Timeline::nowMarkerChanged, this, [this](double timeMs) {
        qDebug() << "ScoreCanvasWindow: Playback start position set to" << timeMs << "ms";
        // Double-clicking timeline sets the START POSITION (where playback will always return to)
        playbackStartPosition = timeMs;
        // Also update paste target time in ScoreCanvas
        scoreCanvas->setPasteTargetTime(timeMs);
        // Sync tempo/time signature spinboxes with the new position
        onNowMarkerChanged(timeMs);
    });

    // Initialize paste target time from timeline's current now marker
    scoreCanvas->setPasteTargetTime(timeline->getNowMarker());

    // Create horizontal layout for timeline with left spacing
    QHBoxLayout *timelineLayout = new QHBoxLayout();
    timelineLayout->setContentsMargins(0, 0, 0, 0);
    timelineLayout->setSpacing(0);

    // Add spacers to align timeline with score canvas
    // Spacer for track selector width - must match trackScrollArea width
    QWidget *trackSpacer = new QWidget();
    trackSpacer->setMinimumWidth(trackScrollAreaWidth);
    trackSpacer->setMaximumWidth(trackScrollAreaWidth);
    trackSpacer->setStyleSheet("background-color: #f0f0f0;");

    // Spacer for frequency labels width
    QWidget *freqSpacer = new QWidget();
    freqSpacer->setMinimumWidth(60);
    freqSpacer->setMaximumWidth(60);
    freqSpacer->setStyleSheet("background-color: #f0f0f0;");

    timelineLayout->addWidget(trackSpacer);
    timelineLayout->addWidget(freqSpacer);
    timelineLayout->addWidget(timeline, 1);  // Timeline takes remaining space

    // Create horizontal layout for track selector + frequency labels + score canvas scroll areas
    QHBoxLayout *scoreLayout = new QHBoxLayout();
    scoreLayout->setContentsMargins(0, 0, 0, 0);
    scoreLayout->setSpacing(0);
    scoreLayout->addWidget(trackScrollArea);
    scoreLayout->addWidget(frequencyScrollArea);
    scoreLayout->addWidget(scoreScrollArea, 1);  // Stretch factor 1 = expands

    // Add layouts to main layout
    mainLayout->addLayout(timelineLayout);
    mainLayout->addLayout(scoreLayout, 1);  // Stretch factor 1 = expands vertically

    // Synchronize vertical scrolling between TrackSelector, FrequencyLabels, and ScoreCanvas
    connect(scoreScrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            [this](int value) {
                // Update scroll positions
                trackScrollArea->verticalScrollBar()->setValue(value);
                frequencyScrollArea->verticalScrollBar()->setValue(value);
                // Update vertical offset for all three widgets
                trackSelector->setVerticalOffset(value);
                frequencyLabels->setVerticalOffset(value);
                scoreCanvas->setVerticalOffset(value);
            });

    // Synchronize canvas height for pixel calculations
    // Both widgets must use the same height for frequencyToPixel() to align correctly
    // Use the ScoreCanvas actual height (set to minimum 2000 in setupScoreCanvas)
    QTimer::singleShot(0, [this]() {
        // Delay to ensure widgets are fully initialized
        frequencyLabels->setCanvasHeight(scoreCanvas->height());
    });

    // Synchronize horizontal scrolling between Timeline and ScoreCanvas
    connect(scoreScrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
            [this](int value) {
                // Update timeline horizontal offset
                timeline->setHorizontalOffset(value);
                // Update score canvas horizontal offset (critical for note positioning!)
                scoreCanvas->setHorizontalScrollOffset(value);
                // Update frequency labels for the scale at the left edge
                updateFrequencyLabels();
            });

    // Auto-scroll: ScoreCanvas asks us to shift the scroll area during lasso
    connect(scoreCanvas, &ScoreCanvas::requestAutoScroll, this, &ScoreCanvasWindow::onScoreAutoScroll);

    // Install event filter on scoreCanvas for zoom
    scoreCanvas->installEventFilter(this);

    // Connect pressure signal to status display
    connect(scoreCanvas, &ScoreCanvas::pressureChanged, this, &ScoreCanvasWindow::onPressureChanged);

    // Connect cursor position signal to status bar
    connect(scoreCanvas, &ScoreCanvas::cursorPositionChanged, this, &ScoreCanvasWindow::onCursorPositionChanged);

    // Update frequency labels when scale settings change
    connect(scoreCanvas, &ScoreCanvas::scaleSettingsChanged, this, &ScoreCanvasWindow::updateFrequencyLabels);

    // Sync slide mode button with ScoreCanvas state (keyboard shortcut keeps button in sync)
    connect(scoreCanvas, &ScoreCanvas::slideModeChanged, this, &ScoreCanvasWindow::onSlideModeChanged);
    connect(scoreCanvas, &ScoreCanvas::transformModeChanged, this, &ScoreCanvasWindow::onTransformModeChanged);

    // Enable transform button only when exactly one note is selected
    connect(scoreCanvas, &ScoreCanvas::noteSelectionChanged, this, [this]() {
        transformModeBtn->setEnabled(scoreCanvas->getSelectedNoteIndices().size() == 1);
    });

    // Keep toolbar spinboxes and status labels in sync when tempo/time sig changes externally
    connect(scoreCanvas, &ScoreCanvas::tempoSettingsChanged, this, &ScoreCanvasWindow::refreshToolbar);

    // When a curve is applied, the toolbar "Show curve" dropdown may need new
    // ITEMS (a freshly applied name the union list didn't know yet) - but it
    // never moves its selection. The combo is the manual picker for the note
    // body's silhouette (D2); auto-switching it to "the last curve edited"
    // both hijacks the dropdown and, through onCurveSelectorChanged, hijacks
    // the note body too. Reverted 2026-08-26 per Nimus.
    connect(scoreCanvas, &ScoreCanvas::expressiveCurveApplied, this,
            [this](const QString &curveName) {
        if (!curveSelectorCombo || curveName.isEmpty()) return;
        if (curveSelectorCombo->findData(curveName) < 0)
            refreshCurveSelector();   // adds missing names, preserves the selection by name
        // Applying a curve can change lane states and the lane set - keep
        // the legend and the Auto split readout true.
        if (lanesLegend) lanesLegend->refresh();
        refreshLanesAboveControls();
    });

    // Connect track selector to score canvas for active track changes
    connect(trackSelector, &TrackSelector::trackSelected, scoreCanvas, &ScoreCanvas::setActiveTrack);

    // Pass track selector reference to score canvas for rendering decisions
    scoreCanvas->setTrackSelector(trackSelector);

    // Connect drawing tool actions to ScoreCanvas input mode
    connect(ui->actionDrawDiscreteNotes, &QAction::triggered, this, [this]() {
        scoreCanvas->setInputMode(ScoreCanvas::DrawModeDiscrete);
        qDebug() << "ScoreCanvasWindow: Switched to Discrete Drawing mode";
    });

    connect(ui->actionDrawContinuousNotes, &QAction::triggered, this, [this]() {
        scoreCanvas->setInputMode(ScoreCanvas::DrawModeContinuous);
        qDebug() << "ScoreCanvasWindow: Switched to Continuous Drawing mode";
    });

    connect(ui->actionScoreCanvasSelect, &QAction::triggered, this, [this]() {
        scoreCanvas->setInputMode(ScoreCanvas::SelectionMode);
        qDebug() << "ScoreCanvasWindow: Switched to Selection mode";
    });

    // Connect snap to scale action
    connect(ui->actionSnapToScale, &QAction::triggered, this, [this]() {
        scoreCanvas->snapSelectedNotesToScale();
        qDebug() << "ScoreCanvasWindow: Snap to Scale action triggered";
    });

    // Restart background pre-render debounce on every committed score edit
    // Only when auto-render is enabled; in manual mode, rendering happens on play
    connect(scoreCanvas->getUndoStack(), &QUndoStack::indexChanged, this, [this](int) {
        if (m_autoRender && m_renderDebounceTimer) m_renderDebounceTimer->start();
        // A lane edit undone/redone changes lane states - keep the legend and
        // the Auto split readout true.
        if (lanesLegend) lanesLegend->refresh();
        refreshLanesAboveControls();
    });
}

void ScoreCanvasWindow::setupZoom()
{
    // Connect zoom toolbar action
    connect(ui->actionScoreCanvasZoom, &QAction::toggled, this, &ScoreCanvasWindow::onZoomToggled);

    // Make the zoom action checkable
    ui->actionScoreCanvasZoom->setCheckable(true);

    // Set keyboard shortcut to 'z' key (application-level)
    ui->actionScoreCanvasZoom->setShortcut(QKeySequence(Qt::Key_Z));
    ui->actionScoreCanvasZoom->setShortcutContext(Qt::ApplicationShortcut);
}

void ScoreCanvasWindow::setZoomMode(bool active)
{
    zoomModeActive = active;

    if (active) {
        // Disable pan mode if it's active
        if (panModeActive) {
            setPanMode(false);
        }

        // Set magnifier cursor
        scoreCanvas->setCursor(Qt::CrossCursor);  // Qt doesn't have magnifier, use cross
        trackSelector->setCursor(Qt::CrossCursor);

        // Update status bar
        navigationModeLabel->setText("ZOOM (z)");
        navigationModeLabel->setStyleSheet("QLabel { padding: 0 5px; font-weight: bold; color: #E57373; }");  // Red
    } else {
        // Restore normal cursor
        scoreCanvas->setCursor(Qt::ArrowCursor);
        trackSelector->setCursor(Qt::ArrowCursor);

        // Clear status bar
        navigationModeLabel->setText("");
        navigationModeLabel->setStyleSheet("QLabel { padding: 0 5px; font-weight: bold; }");
    }

    // Update toolbar button state
    ui->actionScoreCanvasZoom->setChecked(active);
}

void ScoreCanvasWindow::setPanMode(bool active)
{
    panModeActive = active;

    if (active) {
        // Disable zoom mode if it's active
        if (zoomModeActive) {
            setZoomMode(false);
        }

        // Set hand cursor for panning
        scoreCanvas->setCursor(Qt::OpenHandCursor);
        trackSelector->setCursor(Qt::OpenHandCursor);

        // Update status bar
        navigationModeLabel->setText("PAN (Ctrl+Space)");
        navigationModeLabel->setStyleSheet("QLabel { padding: 0 5px; font-weight: bold; color: #66BB6A; }");  // Green
    } else {
        // Restore normal cursor
        scoreCanvas->setCursor(Qt::ArrowCursor);
        trackSelector->setCursor(Qt::ArrowCursor);

        // Clear status bar
        navigationModeLabel->setText("");
        navigationModeLabel->setStyleSheet("QLabel { padding: 0 5px; font-weight: bold; }");
    }
}

void ScoreCanvasWindow::applyZoom(double minHz, double maxHz)
{
    // Constrain to reasonable limits
    if (minHz < 20.0) minHz = 20.0;
    if (maxHz > 8000.0) maxHz = 8000.0;
    if (maxHz <= minHz) return;  // Invalid range

    // Enforce minimum vertical range so zoom can always recover
    static constexpr double MIN_OCTAVE_RANGE = 1.0;
    double octaveRange = std::log2(maxHz / minHz);
    if (octaveRange < MIN_OCTAVE_RANGE) {
        double center = std::sqrt(minHz * maxHz);
        minHz = center / std::pow(2.0, MIN_OCTAVE_RANGE / 2.0);
        maxHz = center * std::pow(2.0, MIN_OCTAVE_RANGE / 2.0);
        // Re-clamp after expansion
        if (minHz < 20.0) { minHz = 20.0; maxHz = 20.0 * std::pow(2.0, MIN_OCTAVE_RANGE); }
        if (maxHz > 8000.0) { maxHz = 8000.0; minHz = 8000.0 / std::pow(2.0, MIN_OCTAVE_RANGE); }
    }

    double oldMinHz = currentMinHz;
    double oldMaxHz = currentMaxHz;
    currentMinHz = minHz;
    currentMaxHz = maxHz;

    // Apply to all three widgets synchronously
    scoreCanvas->setFrequencyRange(minHz, maxHz);
    trackSelector->setFrequencyRange(minHz, maxHz);
    frequencyLabels->setFrequencyRange(minHz, maxHz);

    // Resize widgets so the full frequency range is scrollable.
    // The visible Hz range maps to the viewport; the full range maps to the widget.
    int viewportHeight = scoreScrollArea->viewport()->height();
    if (viewportHeight <= 0) return;

    double fullOctaveRange = std::log2(8000.0 / 20.0);
    double visibleOctaveRange = std::log2(maxHz / minHz);
    int canvasHeight = static_cast<int>(viewportHeight * (fullOctaveRange / visibleOctaveRange));
    canvasHeight = qMax(canvasHeight, viewportHeight);

    scoreCanvas->setFixedHeight(canvasHeight);
    trackSelector->setFixedHeight(canvasHeight);
    frequencyLabels->setFixedHeight(canvasHeight);
    frequencyLabels->setCanvasHeight(canvasHeight);

    // Adjust scroll position to keep the same frequency region visible.
    // The old visible range center should stay at the viewport center.
    double oldVisibleOctaveRange = std::log2(oldMaxHz / oldMinHz);
    if (oldVisibleOctaveRange > 0) {
        double fullMinOctave = std::log2(20.0 / 18.75);  // base freq = 18.75 Hz
        double fullMaxOctave = std::log2(8000.0 / 18.75);
        double fullRange = fullMaxOctave - fullMinOctave;

        // The visible center frequency in octave-space
        double centerOctave = (std::log2(minHz / 25.0) + std::log2(maxHz / 25.0)) / 2.0;

        // Pixel position of center in the new canvas (from top)
        double normalizedCenter = (centerOctave - fullMinOctave) / fullRange;
        int centerPixel = canvasHeight - static_cast<int>(normalizedCenter * canvasHeight);

        // Scroll so that centerPixel is at the middle of the viewport
        int newScroll = centerPixel - viewportHeight / 2;
        newScroll = qBound(0, newScroll, scoreScrollArea->verticalScrollBar()->maximum());
        scoreScrollArea->verticalScrollBar()->setValue(newScroll);
    }
}

void ScoreCanvasWindow::applyHorizontalZoom(double pixelsPerSecond)
{
    // Constrain to reasonable limits (10 to 1000 pixels per second)
    if (pixelsPerSecond < 10.0) pixelsPerSecond = 10.0;
    if (pixelsPerSecond > 1000.0) pixelsPerSecond = 1000.0;

    double oldPixelsPerSecond = currentPixelsPerSecond;
    currentPixelsPerSecond = pixelsPerSecond;

    // Apply to both ScoreCanvas and Timeline
    scoreCanvas->setPixelsPerSecond(pixelsPerSecond);
    timeline->setPixelsPerSecond(pixelsPerSecond);

    // Resize canvas and timeline to match the new zoom level
    int canvasWidth = static_cast<int>(currentPixelsPerSecond * (compositionSettings.lengthMs / 1000.0));
    canvasWidth = qMax(canvasWidth, scoreScrollArea->viewport()->width());
    scoreCanvas->setFixedWidth(canvasWidth);
    timeline->resize(canvasWidth, timeline->height());

    // Adjust scroll position proportionally so the same content stays visible
    if (oldPixelsPerSecond > 0) {
        int oldScroll = scoreScrollArea->horizontalScrollBar()->value();
        int newScroll = static_cast<int>(oldScroll * (pixelsPerSecond / oldPixelsPerSecond));
        scoreScrollArea->horizontalScrollBar()->setValue(newScroll);
    }
}

void ScoreCanvasWindow::onZoomToggled(bool checked)
{
    setZoomMode(checked);
}

void ScoreCanvasWindow::onZoomIn()
{
    // Zoom in: reduce frequency range by 20%
    double center = std::sqrt(currentMinHz * currentMaxHz);  // Geometric mean
    double range = std::log2(currentMaxHz / currentMinHz);
    double newRange = range * 0.8;  // 20% reduction

    double newMinHz = center / std::pow(2.0, newRange / 2.0);
    double newMaxHz = center * std::pow(2.0, newRange / 2.0);

    applyZoom(newMinHz, newMaxHz);
}

void ScoreCanvasWindow::onZoomOut()
{
    // Zoom out: increase frequency range by 25%
    double center = std::sqrt(currentMinHz * currentMaxHz);  // Geometric mean
    double range = std::log2(currentMaxHz / currentMinHz);
    double newRange = range * 1.25;  // 25% increase

    double newMinHz = center / std::pow(2.0, newRange / 2.0);
    double newMaxHz = center * std::pow(2.0, newRange / 2.0);

    applyZoom(newMinHz, newMaxHz);
}

void ScoreCanvasWindow::zoomToFit()
{
    applyZoom(27.5, 4186.0);  // A0 to C8
}

void ScoreCanvasWindow::onPressureChanged(double pressure, bool active)
{
    if (active) {
        // Display pressure as percentage (0-100%)
        int pressurePercent = static_cast<int>(pressure * 100);
        pressureLabel->setText(QString("Pressure: %1%").arg(pressurePercent));
    } else {
        // Inactive - show placeholder
        pressureLabel->setText("Pressure: --");
    }
}

void ScoreCanvasWindow::onCursorPositionChanged(double timeMs, double pitchHz)
{
    // Format time based on current time mode
    QString timeStr;
    if (currentTimeMode == AbsoluteTime) {
        // Absolute: Min:Sec:Ms
        int totalMs = static_cast<int>(timeMs);
        int minutes = totalMs / 60000;
        int seconds = (totalMs % 60000) / 1000;
        int milliseconds = totalMs % 1000;
        timeStr = QString("%1:%2:%3")
                      .arg(minutes)
                      .arg(seconds, 2, 10, QChar('0'))
                      .arg(milliseconds, 3, 10, QChar('0'));
    } else {
        // Musical: Bars:Beats:Ms
        // Calculate bar duration based on bottom number
        double barDurationMs;
        double beatDurationMs;

        if (currentTimeSigBottom == 0) {
            // Option A: Bottom is just a label
            beatDurationMs = 60000.0 / currentTempo;
            barDurationMs = beatDurationMs * currentTimeSigTop;
        } else {
            // Option B: Bottom scales beat duration
            barDurationMs = (60000.0 / currentTempo) * (static_cast<double>(currentTimeSigTop) / currentTimeSigBottom);
            beatDurationMs = barDurationMs / currentTimeSigTop;
        }

        int bar = static_cast<int>(timeMs / barDurationMs) + 1;  // 1-based
        double timeInBar = std::fmod(timeMs, barDurationMs);
        int beat = static_cast<int>(timeInBar / beatDurationMs) + 1;  // 1-based
        int ms = static_cast<int>(std::fmod(timeInBar, beatDurationMs));

        timeStr = QString("%1:%2:%3")
                      .arg(bar)
                      .arg(beat)
                      .arg(ms, 3, 10, QChar('0'));
    }
    cursorTimeLabel->setText(timeStr);

    // Format pitch: Hz; Scale degree (no western note names)
    if (pitchHz > 0) {
        // Find closest scale line to determine scale degree
        // Access the scale lines from ScoreCanvas
        QString scaleDegreeStr = "--";
        double minDistance = 1000000.0;  // Large initial value

        // Get scale lines from ScoreCanvas to find closest degree
        const auto& scaleLines = scoreCanvas->getPhrase();  // We need access to scale lines
        // For now, use a simple calculation based on just intonation C major
        // TODO: This should query ScoreCanvas for actual scale lines

        // Just intonation major scale ratios starting from BASE_FREQUENCY (18.75 Hz)
        const double BASE_FREQ = 18.75;
        const double majorScaleRatios[7] = {1.0, 9.0/8.0, 5.0/4.0, 4.0/3.0, 3.0/2.0, 5.0/3.0, 15.0/8.0};

        // Find which scale degree is closest
        int closestDegree = 0;
        for (int octave = 0; octave <= 9; ++octave) {
            double octaveBase = BASE_FREQ * std::pow(2.0, octave);
            for (int degree = 0; degree < 7; ++degree) {
                double scaleFreq = octaveBase * majorScaleRatios[degree];
                double distance = std::abs(pitchHz - scaleFreq);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestDegree = degree + 1;  // 1-based
                }
            }
        }

        // Only show scale degree if within reasonable tolerance (within ~50 cents)
        if (minDistance < pitchHz * 0.03) {  // ~3% tolerance
            scaleDegreeStr = QString::number(closestDegree);
        }

        QString pitchStr = QString("%1 Hz; %2")
                               .arg(pitchHz, 0, 'f', 1)
                               .arg(scaleDegreeStr);
        cursorPitchLabel->setText(pitchStr);
    } else {
        cursorPitchLabel->setText("-- Hz; --");
    }

    // Update tempo and time signature in status bar
    statusTempoLabel->setText(QString("%1 BPM").arg(currentTempo));
    statusTimeSigLabel->setText(QString("%1/%2").arg(currentTimeSigTop).arg(currentTimeSigBottom));
}

bool ScoreCanvasWindow::event(QEvent *event)
{
    // Intercept Tab key before Qt's focus navigation system
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() == Qt::NoModifier) {
            if (m_mainWindow) {
                m_mainWindow->toggleCanvasFocus();
            }
            return true;  // Event handled
        }
    }
    return QMainWindow::event(event);
}

void ScoreCanvasWindow::keyPressEvent(QKeyEvent *event)
{
    // Escape cancels in-progress render
    if (event->key() == Qt::Key_Escape && m_isRendering && trackManager) {
        for (int i = 0; i < trackManager->getTrackCount(); ++i) {
            Track *track = trackManager->getTrack(i);
            if (track && track->isRendering()) track->cancelRender();
        }
        event->accept();
        return;
    }

    // Ctrl+Space toggles pan mode
    if (event->key() == Qt::Key_Space && event->modifiers() == Qt::ControlModifier) {
        setPanMode(!panModeActive);
        event->accept();
        return;
    }

    // Spacebar alone toggles playback (start/stop)
    if (event->key() == Qt::Key_Space && event->modifiers() == Qt::NoModifier) {
        if (isPlaying) {
            stopPlayback();
        } else {
            startPlayback();
        }
        event->accept();
        return;
    }

    // L key enters loop mode
    if (event->key() == Qt::Key_L && event->modifiers() == Qt::NoModifier) {
        timeline->setLoopModeActive(true);
        event->accept();
        return;
    }

    // N key sets the playback start position to the current now marker
    if (event->key() == Qt::Key_N && event->modifiers() == Qt::NoModifier) {
        playbackStartPosition = timeline->getNowMarker();
        scoreCanvas->setPasteTargetTime(playbackStartPosition);
        event->accept();
        return;
    }

    // G key opens goto dialog
    if (event->key() == Qt::Key_G && event->modifiers() == Qt::NoModifier) {
        // Create and show goto dialog
        CompositionSettings::TimeMode mode = (currentTimeMode == MusicalTime) ?
            CompositionSettings::Musical : CompositionSettings::Absolute;

        GotoDialog dialog(mode, currentTempo, currentTimeSigTop, currentTimeSigBottom, this);
        if (dialog.exec() == QDialog::Accepted) {
            double targetTimeMs = dialog.getTargetTimeMs();

            // Set now time if requested (via "n" key or "Set Now" button)
            if (dialog.getSetNowTime()) {
                playbackStartPosition = targetTimeMs;
                scoreCanvas->setPasteTargetTime(playbackStartPosition);
                timeline->setNowMarker(playbackStartPosition);
            }

            // Convert time to pixel position
            int targetPixel = scoreCanvas->timeToPixel(targetTimeMs);

            // Scroll to position (center the target in the viewport)
            int viewportWidth = scoreScrollArea->viewport()->width();
            int scrollValue = targetPixel - (viewportWidth / 2);

            // Clamp to valid range
            scrollValue = qMax(scoreScrollArea->horizontalScrollBar()->minimum(), scrollValue);
            scrollValue = qMin(scoreScrollArea->horizontalScrollBar()->maximum(), scrollValue);

            scoreScrollArea->horizontalScrollBar()->setValue(scrollValue);
        }
        event->accept();
        return;
    }

    // Shift+Left / Shift+Right: step selection to previous/next note
    if ((event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) &&
        (event->modifiers() & Qt::ShiftModifier)) {
        scoreCanvas->selectNextNote(event->key() == Qt::Key_Right ? 1 : -1);
        event->accept();
        return;
    }

    // Delete key clears loop (when loop end is selected)
    if (event->key() == Qt::Key_Delete && event->modifiers() == Qt::NoModifier) {
        if (timeline->hasLoop()) {
            timeline->clearLoop();
            event->accept();
            return;
        }
    }

    // Let the base class handle other keys (including action shortcuts)
    QMainWindow::keyPressEvent(event);
}

void ScoreCanvasWindow::keyReleaseEvent(QKeyEvent *event)
{
    QMainWindow::keyReleaseEvent(event);
}

bool ScoreCanvasWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Block wheel events on frequency labels to prevent independent scrolling
    if (event->type() == QEvent::Wheel &&
        (obj == frequencyLabels || obj == frequencyScrollArea->viewport())) {
        return true;  // Eat the event
    }

    // Handle key presses globally for shortcuts
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Only handle if this window is active
        if (isActiveWindow()) {
            // Let keyPressEvent handle it
            keyPressEvent(keyEvent);
            if (keyEvent->isAccepted()) {
                return true;
            }
        }
    }

    if (obj == scoreCanvas) {
        // Handle tablet press to start zoom drag
        if (event->type() == QEvent::TabletPress && zoomModeActive) {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
            isDraggingZoom = true;
            dragStartPos = tabletEvent->globalPosition().toPoint();
            dragStartMinHz = currentMinHz;
            dragStartMaxHz = currentMaxHz;
            dragStartPixelsPerSecond = currentPixelsPerSecond;

            // Calculate frequency at cursor position for zoom center (vertical)
            int y = tabletEvent->position().y();
            zoomCenterHz = scoreCanvas->pixelToFrequency(y);

            // Calculate time at cursor position for zoom center (horizontal)
            int x = tabletEvent->position().x();
            zoomCenterTime = scoreCanvas->pixelToTime(x);

            tabletEvent->accept();
            return true;  // Event handled
        }

        // Handle tablet move for drag zoom
        if (event->type() == QEvent::TabletMove && zoomModeActive && isDraggingZoom) {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);

            // Calculate drag distance using global coordinates (immune to widget resize/scroll)
            QPoint globalPos = tabletEvent->globalPosition().toPoint();
            int deltaY = dragStartPos.y() - globalPos.y();
            int deltaX = dragStartPos.x() - globalPos.x();

            // VERTICAL ZOOM (Frequency)
            double verticalZoomFactor = std::pow(2.0, deltaY / 100.0);
            double currentFreqRange = std::log2(dragStartMaxHz / dragStartMinHz);
            double newFreqRange = currentFreqRange / verticalZoomFactor;
            double newMinHz = zoomCenterHz / std::pow(2.0, newFreqRange / 2.0);
            double newMaxHz = zoomCenterHz * std::pow(2.0, newFreqRange / 2.0);
            applyZoom(newMinHz, newMaxHz);

            // HORIZONTAL ZOOM (Time)
            double horizontalZoomFactor = std::pow(2.0, deltaX / 100.0);
            double newPixelsPerSecond = dragStartPixelsPerSecond * horizontalZoomFactor;
            applyHorizontalZoom(newPixelsPerSecond);

            tabletEvent->accept();
            return true;  // Event handled
        }

        // Handle tablet release to end zoom drag
        if (event->type() == QEvent::TabletRelease && zoomModeActive && isDraggingZoom) {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
            isDraggingZoom = false;
            tabletEvent->accept();
            return true;  // Event handled
        }

        // Handle tablet press to start pan drag
        if (event->type() == QEvent::TabletPress && panModeActive) {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
            isDraggingPan = true;
            panDragStartPos = tabletEvent->globalPosition().toPoint();
            panStartHorizontalScroll = scoreScrollArea->horizontalScrollBar()->value();
            panStartVerticalScroll = scoreScrollArea->verticalScrollBar()->value();

            // Change cursor to closed hand while dragging
            scoreCanvas->setCursor(Qt::ClosedHandCursor);

            tabletEvent->accept();
            return true;  // Event handled
        }

        // Handle tablet move for panning
        if (event->type() == QEvent::TabletMove && panModeActive && isDraggingPan) {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);

            // Calculate delta from drag start position
            QPoint currentPos = tabletEvent->globalPosition().toPoint();
            QPoint delta = currentPos - panDragStartPos;

            // Apply inverse delta to scroll bars for horizontal
            int newHorizontalScroll = panStartHorizontalScroll - delta.x();
            scoreScrollArea->horizontalScrollBar()->setValue(newHorizontalScroll);

            // Vertical panning: set scroll bar value (auto-clamps to valid range).
            // The valueChanged signal syncs FrequencyLabels and TrackSelector automatically.
            int newVerticalOffset = panStartVerticalScroll - delta.y();
            scoreScrollArea->verticalScrollBar()->setValue(newVerticalOffset);

            tabletEvent->accept();
            return true;  // Event handled
        }

        // Handle tablet release to end pan drag
        if (event->type() == QEvent::TabletRelease && panModeActive && isDraggingPan) {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
            isDraggingPan = false;

            // Restore open hand cursor
            scoreCanvas->setCursor(Qt::OpenHandCursor);

            tabletEvent->accept();
            return true;  // Event handled
        }

        // Handle mouse press to start pan drag
        if (event->type() == QEvent::MouseButtonPress && panModeActive) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDraggingPan = true;
                panDragStartPos = mouseEvent->globalPosition().toPoint();
                panStartHorizontalScroll = scoreScrollArea->horizontalScrollBar()->value();
                panStartVerticalScroll = scoreScrollArea->verticalScrollBar()->value();

                // Change cursor to closed hand while dragging
                scoreCanvas->setCursor(Qt::ClosedHandCursor);

                return true;  // Event handled
            }
        }

        // Handle mouse move for panning
        if (event->type() == QEvent::MouseMove && panModeActive && isDraggingPan) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            // Calculate delta from drag start position
            QPoint currentPos = mouseEvent->globalPosition().toPoint();
            QPoint delta = currentPos - panDragStartPos;

            // Apply inverse delta to scroll bars for horizontal
            int newHorizontalScroll = panStartHorizontalScroll - delta.x();
            scoreScrollArea->horizontalScrollBar()->setValue(newHorizontalScroll);

            // Vertical panning: set scroll bar value (auto-clamps to valid range).
            // The valueChanged signal syncs FrequencyLabels and TrackSelector automatically.
            int newVerticalOffset = panStartVerticalScroll - delta.y();
            scoreScrollArea->verticalScrollBar()->setValue(newVerticalOffset);

            return true;  // Event handled
        }

        // Handle mouse release to end pan drag
        if (event->type() == QEvent::MouseButtonRelease && panModeActive && isDraggingPan) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDraggingPan = false;

                // Restore open hand cursor
                scoreCanvas->setCursor(Qt::OpenHandCursor);

                return true;  // Event handled
            }
        }

        // Handle mouse press to start drag zoom
        if (event->type() == QEvent::MouseButtonPress && zoomModeActive) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDraggingZoom = true;
                dragStartPos = mouseEvent->globalPosition().toPoint();
                dragStartMinHz = currentMinHz;
                dragStartMaxHz = currentMaxHz;
                dragStartPixelsPerSecond = currentPixelsPerSecond;

                // Calculate frequency at cursor position for zoom center (vertical)
                int y = mouseEvent->pos().y();
                zoomCenterHz = scoreCanvas->pixelToFrequency(y);

                // Calculate time at cursor position for zoom center (horizontal)
                int x = mouseEvent->pos().x();
                zoomCenterTime = scoreCanvas->pixelToTime(x);

                return true;  // Event handled
            }
        }

        // Handle mouse move for drag zoom
        if (event->type() == QEvent::MouseMove && zoomModeActive && isDraggingZoom) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            // Calculate drag distance using global coordinates (immune to widget resize/scroll)
            QPoint globalPos = mouseEvent->globalPosition().toPoint();
            int deltaY = dragStartPos.y() - globalPos.y();
            int deltaX = dragStartPos.x() - globalPos.x();

            // VERTICAL ZOOM (Frequency)
            // Drag up = zoom in, drag down = zoom out
            // 100 pixels = 2x zoom
            double verticalZoomFactor = std::pow(2.0, deltaY / 100.0);

            // Calculate new frequency range centered at cursor
            double currentFreqRange = std::log2(dragStartMaxHz / dragStartMinHz);
            double newFreqRange = currentFreqRange / verticalZoomFactor;

            // Apply vertical zoom centered at cursor position
            double newMinHz = zoomCenterHz / std::pow(2.0, newFreqRange / 2.0);
            double newMaxHz = zoomCenterHz * std::pow(2.0, newFreqRange / 2.0);

            applyZoom(newMinHz, newMaxHz);

            // HORIZONTAL ZOOM (Time)
            // Drag left = zoom in, drag right = zoom out
            // 100 pixels = 2x zoom
            double horizontalZoomFactor = std::pow(2.0, deltaX / 100.0);

            // Calculate new pixels per second
            double newPixelsPerSecond = dragStartPixelsPerSecond * horizontalZoomFactor;

            applyHorizontalZoom(newPixelsPerSecond);

            return true;  // Event handled
        }

        // Handle mouse release to end drag zoom
        if (event->type() == QEvent::MouseButtonRelease && zoomModeActive && isDraggingZoom) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDraggingZoom = false;
                return true;  // Event handled
            }
        }

        // Handle mouse wheel zoom when zoom mode is active (both axes)
        if (event->type() == QEvent::Wheel && zoomModeActive) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);

            // Zoom in/out based on wheel direction (both vertical and horizontal)
            if (wheelEvent->angleDelta().y() > 0) {
                onZoomIn();
                // Also zoom in horizontally
                applyHorizontalZoom(currentPixelsPerSecond * 1.25);
            } else {
                onZoomOut();
                // Also zoom out horizontally
                applyHorizontalZoom(currentPixelsPerSecond * 0.8);
            }

            return true;  // Event handled
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void ScoreCanvasWindow::startPlayback(Track *forceMidiTrack)
{
    if (!audioEngine) return;

    // Cancel debounce so no new background render starts after we begin
    m_renderDebounceTimer->stop();

    // If a background render is in progress, wait for it before proceeding
    // (the progress bar is already visible — signal handlers manage it)
    if (m_backgroundRenderCount > 0) {
        QElapsedTimer waitTimer;
        waitTimer.start();
        while (m_backgroundRenderCount > 0 && waitTimer.elapsed() < 30000) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        }
        if (m_backgroundRenderCount > 0) {
            qWarning() << "ScoreCanvas: background render timed out — resetting counter";
            m_backgroundRenderCount = 0;
        }
    }

    // Get all notes from the canvas phrase
    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    if (notes.isEmpty()) {
        qDebug() << "ScoreCanvas: No notes to play";
        return;
    }

    // Stop any currently playing audio first
    if (isPlaying) {
        stopPlayback();
    }

    // Make sure audio is stopped from any other source
    audioEngine->stopNote();

    // Always start playback from the START POSITION (set by double-clicking timeline)
    // This returns to the same position every time, like a "playback anchor"
    playbackStartTime = playbackStartPosition;
    currentNoteIndex = 0;

    // Move the now marker to the start position
    timeline->setNowMarker(playbackStartTime);

    // Scroll the view to show the playback start position
    int startPixel = static_cast<int>((playbackStartTime / 1000.0) * currentPixelsPerSecond);
    int viewportWidth = scoreScrollArea->viewport()->width();
    int newScroll = startPixel - viewportWidth / 4;  // Position marker at 25% from left
    newScroll = qMax(0, newScroll);  // Don't scroll to negative
    scoreScrollArea->horizontalScrollBar()->setValue(newScroll);

    // PRE-RENDER using Track's note-based rendering system (MULTI-TRACK)
    qDebug() << "=== ScoreCanvas: Pre-rendering" << notes.size() << "notes across all tracks ===";

    bool useTrackPlayback = false;

    if (trackManager && trackManager->getTrackCount() > 0) {
        qDebug() << "ScoreCanvas: TrackManager has" << trackManager->getTrackCount() << "track(s)";

        // Distribute notes to their respective tracks based on trackIndex
        QMap<int, QList<Note>> notesByTrack;

        for (const Note &note : notes) {
            int trackIdx = note.getTrackIndex();
            notesByTrack[trackIdx].append(note);
        }

        qDebug() << "ScoreCanvas: Distributing notes to" << notesByTrack.keys().size() << "track(s)";
        qDebug() << "ScoreCanvas: Note distribution by trackIndex:" << notesByTrack.keys();

        // Pre-render each track that has notes
        QList<Track*> tracksToPlay;
        bool allRendered = true;

        for (int trackIdx = 0; trackIdx < trackManager->getTrackCount(); ++trackIdx) {
            Track *track = trackManager->getTrack(trackIdx);
            if (!track) continue;

            // Get notes for this track
            const QList<Note> &trackNotes = notesByTrack.value(trackIdx);

            // Sync notes to track (thread-safe to prevent race with audio callback)
            track->syncNotes(trackNotes);

            // Load any baked clip PCM on the UI thread before playback -
            // the audio callback must never touch the disk (Phase 9).
            track->prepareClipForPlayback();

            // Debug output for synced notes
            for (const Note &note : trackNotes) {
                qDebug() << "ScoreCanvas: Synced note" << note.getId()
                         << "to track" << trackIdx
                         << "with variationIndex=" << note.getVariationIndex();
            }

            // Pre-render if track has notes
            if (!trackNotes.isEmpty()) {
                qDebug() << "ScoreCanvas: Rendering track" << trackIdx
                         << "(" << track->getName() << ") with" << trackNotes.size() << "notes";

                bool rendered = track->prerenderDirtyNotes(audioEngine->getSampleRate());
                if (!rendered) {
                    qWarning() << "ScoreCanvas: Track" << trackIdx << "rendering failed or cancelled";
                    allRendered = false;
                    break;
                }
            }

            // Add track to playback list (even if empty - AudioEngine will handle muting)
            // Skip muted tracks
            if (!track->isMuted()) {
                tracksToPlay.append(track);
            }
        }

        // Abort playback if any track's render was cancelled or failed
        if (!allRendered) {
            qWarning() << "ScoreCanvas: Render cancelled or failed, aborting playback";
            return;
        }

        if (!tracksToPlay.isEmpty()) {
            qDebug() << "ScoreCanvas: Starting playback with" << tracksToPlay.size() << "track(s)";

            // Use track-based playback
            audioEngine->playFromTracks(tracksToPlay, playbackStartPosition, forceMidiTrack);
            useTrackPlayback = true;
        } else {
            qWarning() << "ScoreCanvas: No tracks to play (all muted or empty)";
        }
    }

    // Fallback to legacy segment-based rendering if track-based failed
    if (!useTrackPlayback) {
        qWarning() << "ScoreCanvas: Using legacy segment-based rendering";
        audioEngine->renderNotes(notes, notes.size());
        audioEngine->playRenderedBuffer(playbackStartPosition);
    }

    // Calculate total playback duration from all notes
    double totalDuration = 0.0;
    for (int i = 0; i < notes.size(); i++) {
        double noteEndTime = notes[i].getStartTime() + notes[i].getDuration();
        if (noteEndTime > totalDuration) {
            totalDuration = noteEndTime;
        }
    }

    isPlaying = true;

    // Start the playback timer (tick every 10ms for smooth timing)
    playbackTimer->start(10);

    qDebug() << "ScoreCanvas: Starting playback of" << notes.size() << "note(s) from" << playbackStartPosition
             << "ms (rendered mode, total duration:" << totalDuration << "ms)";

    // Emit signal to stop other windows
    emit playbackStarted();
}

void ScoreCanvasWindow::onPlaybackTick()
{
    if (!audioEngine || !isPlaying) return;

    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    // Check if audio engine has stopped buffer playback (reached end of buffer)
    if (!audioEngine->isPlayingBuffer()) {
        qDebug() << "ScoreCanvas: Reached end of buffer, stopping playback";
        stopPlayback(false);  // Stop UI but don't try to stop already-stopped audio
        return;
    }

    // Check if we've hit the loop end - if so, jump back to loop start
    if (timeline->hasLoop()) {
        double loopEnd = timeline->getLoopEnd();
        if (playbackStartTime >= loopEnd) {
            // Jump back to loop start
            playbackStartTime = timeline->getLoopStart();

            // Seek the audio engine back to loop start position
            audioEngine->playRenderedBuffer(playbackStartTime);

            // Reset note index to find notes at loop start
            currentNoteIndex = 0;
            for (int i = 0; i < notes.size(); ++i) {
                if (notes[i].getStartTime() >= playbackStartTime) {
                    currentNoteIndex = i;
                    break;
                }
            }

            qDebug() << "ScoreCanvas: Looping back to" << playbackStartTime << "ms (seeking in buffer)";
        }
    }

    // Update timeline now marker (always advance, even if no notes)
    timeline->setNowMarker(playbackStartTime);

    // Auto-scroll to keep now marker visible during playback
    int nowPixel = static_cast<int>((playbackStartTime / 1000.0) * currentPixelsPerSecond);
    int viewportWidth = scoreScrollArea->viewport()->width();
    int currentScroll = scoreScrollArea->horizontalScrollBar()->value();
    int rightThreshold = currentScroll + static_cast<int>(viewportWidth * 0.8);

    if (nowPixel > rightThreshold) {
        int newScroll = nowPixel - viewportWidth / 4;  // Keep marker at 25% from left
        scoreScrollArea->horizontalScrollBar()->setValue(newScroll);
    }

    // Note: In pre-rendered mode, notes are already playing from the buffer
    // We just need to update the timeline marker for visual feedback

    // Always advance playback time (10ms per tick)
    playbackStartTime += 10.0;
}

void ScoreCanvasWindow::stopPlayback(bool stopAudioEngine)
{
    if (!audioEngine) return;

    // Stop the timer first
    if (playbackTimer && playbackTimer->isActive()) {
        playbackTimer->stop();
    }

    // A bake pass ends with the playback - finalize the recording and
    // attach it to the target track (Phase 9).
    if (m_capture) {
        finishBakeCapture();
    }

    // Reset playback state BEFORE stopping note (prevents race condition)
    isPlaying = false;
    currentNoteIndex = 0;
    // DON'T reset playbackStartTime - leave it where playback stopped
    // DON'T reset timeline now marker - leave it at current position so user can see where it stopped

    // Only stop audio engine if requested (don't stop when another window is taking over)
    if (stopAudioEngine) {
        audioEngine->stopNote();
    }

    qDebug() << "ScoreCanvas: Playback stopped" << (stopAudioEngine ? "(audio stopped)" : "(audio continues)");
}

void ScoreCanvasWindow::play()
{
    startPlayback();
}

void ScoreCanvasWindow::onSlideModeChanged(bool active)
{
    slideModeBtn->setChecked(active);
}

void ScoreCanvasWindow::onTransformModeChanged(bool active)
{
    transformModeBtn->setChecked(active);
}

void ScoreCanvasWindow::onRenderModeToggled()
{
    m_autoRender = renderModeToggle->isChecked();
    renderModeToggle->setText(m_autoRender ? "Auto" : "Manual");
    qDebug() << "ScoreCanvasWindow: Render mode set to" << (m_autoRender ? "Auto" : "Manual");
}

void ScoreCanvasWindow::onMidiTestClicked()
{
    // Phase 1 hello-note: fire one hardcoded note at the configured MIDI
    // output device — proves the wire from Kala to external hardware works.
    const QString error = MidiOutput::sendTestNote();
    if (!error.isEmpty())
        QMessageBox::warning(this, "MIDI test", error);
}

void ScoreCanvasWindow::onBakeClicked()
{
    // Phase 9: record the VL70-m's audio input while the composition plays,
    // then attach the recording to a track as a clip.
    if (m_capture) {
        QMessageBox::information(this, "Bake", "A bake is already in progress.");
        return;
    }
    if (isPlaying) {
        stopPlayback(true);
    }

    // A track is a VL70-m track when its base graph or any variation graph
    // carries a MIDI-out container.
    auto hasMidiGraph = [](Track *t) {
        if (!t) return false;
        if (t->getGraph() && t->getGraph()->hasMidiOut()) return true;
        for (int v = 1; v <= t->getVariationCount(); ++v) {
            SounitGraph *g = t->getGraphForVariation(v);
            if (g && g->hasMidiOut()) return true;
        }
        return false;
    };

    // The combo index becomes m_captureTrackIndex, which indexes trackManager
    // again (and picks forceMidiTrack) - so the list must stay index-aligned
    // with trackManager, nulls included.
    QList<Track *> tracksList;
    int firstMidiTrack = -1;
    for (int i = 0; i < trackManager->getTrackCount(); ++i) {
        Track *t = trackManager->getTrack(i);
        tracksList << t;
        if (firstMidiTrack < 0 && hasMidiGraph(t)) firstMidiTrack = i;
    }
    if (tracksList.isEmpty()) {
        QMessageBox::warning(this, "Bake", "No tracks to bake into.");
        return;
    }

    // Target track default: the score's active track when it is a VL70-m
    // track — what the user has selected is what they mean to bake. Picking
    // the wrong target is expensive and quiet: the target is exempt from the
    // freeze (forceMidiTrack), so an already-baked track would play its MIDI
    // live alongside the intended one and then have its clip overwritten.
    // Falls back to the first VL70-m track.
    const int activeTrack = scoreCanvas ? scoreCanvas->getActiveTrack() : -1;
    int defaultTrack = 0;
    if (activeTrack >= 0 && activeTrack < tracksList.size()
        && hasMidiGraph(tracksList[activeTrack])) {
        defaultTrack = activeTrack;
    } else if (firstMidiTrack >= 0) {
        defaultTrack = firstMidiTrack;
    }

    const QList<AudioCapture::CaptureDevice> devices = AudioCapture::listInputDevices();
    if (devices.isEmpty()) {
        QMessageBox::warning(this, "Bake",
                             "No audio input devices found.\n"
                             "Check that the interface is connected and an input is enabled.");
        return;
    }

    // Default WAV name: "<project>-bake-<N>.wav" with the next free number,
    // so successive bakes never overwrite each other - every clip keeps its
    // own file and a re-bake leaves the earlier take intact (Phase 9).
    QString projectName = compositionName.trimmed();
    if (projectName.isEmpty()) projectName = "Kala";
    QString safeName = projectName;
    for (QChar &c : safeName) {
        if (QString("\\/:*?\"<>|").contains(c)) c = QLatin1Char('-');
    }
    const QString musicDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    const QString bakeBase = safeName + "-bake-";
    int nextNum = 1;
    const QStringList existing = QDir(musicDir).entryList({bakeBase + "*.wav"}, QDir::Files);
    for (const QString &entry : existing) {
        const QString middle = entry.mid(bakeBase.size(), entry.size() - bakeBase.size() - 4);
        bool ok = false;
        const int n = middle.toInt(&ok);
        if (ok && n >= nextNum) nextNum = n + 1;
    }
    QString defaultPath = musicDir + "/" + bakeBase + QString::number(nextNum) + ".wav";
    BakeDialog dialog(tracksList, defaultTrack, devices, defaultPath, this);
    if (dialog.exec() != QDialog::Accepted) return;

    m_capture = new AudioCapture(this);
    QString error;
    if (!m_capture->beginCapture(dialog.filePath(), dialog.selectedDeviceName(),
                                 audioEngine->getSampleRate(), dialog.channelMode(), error)) {
        QMessageBox::warning(this, "Bake", "Could not start recording: " + error);
        delete m_capture;
        m_capture = nullptr;
        return;
    }

    m_captureTrackIndex = dialog.selectedTrack();
    m_capturePath = dialog.filePath();

    // Transport starts at playbackStartPosition (the play anchor); startPlayback
    // leaves the member as-is, so finishBakeCapture can read it for alignment.
    // The target track's MIDI must play even though it has a clip - the
    // module needs to sound to be re-recorded (other baked tracks stay frozen).
    startPlayback(trackManager->getTrack(m_captureTrackIndex));
}

void ScoreCanvasWindow::finishBakeCapture()
{
    if (!m_capture) return;

    AudioCapture *cap = m_capture;
    m_capture = nullptr;
    const QString path = m_capturePath;
    const int trackIdx = m_captureTrackIndex;
    const double transportStart = playbackStartPosition;
    m_capturePath.clear();
    m_captureTrackIndex = -1;

    QString error;
    const unsigned long long frames = cap->endCapture(error);
    const unsigned int captureRate = cap->getSampleRate();
    cap->deleteLater();

    Track *target = trackManager ? trackManager->getTrack(trackIdx) : nullptr;
    if (!target) {
        QMessageBox::warning(this, "Bake", "The target track no longer exists.");
        return;
    }
    if (frames == 0) {
        QMessageBox::warning(this, "Bake",
                             "No audio was captured." + (error.isEmpty() ? QString() : " (" + error + ")"));
        return;
    }

    // Onset analysis on the finished recording. The interface round-trip
    // (MIDI out + module + audio in) and the capture pre-roll are absorbed
    // here: the recording's first sound is aligned to the first VL70-m
    // note-on of the baked span.
    double onsetMs = 0.0, lastSignalMs = 0.0;
    bool foundOnset = false;
    float peakFs = 0.0f;
    AudioCapture::analyzeClipBounds(path, onsetMs, lastSignalMs, foundOnset, peakFs);

    // First VL70-m note-on of the span = the EARLIEST note at/after the
    // transport start with a MIDI-out graph. Notes are stored in drawing
    // order, not time order, so take the minimum rather than the first in
    // the list.
    double firstNoteMs = -1.0;
    for (const Note &note : target->getNotes()) {
        if (note.getStartTime() < transportStart) continue;
        SounitGraph *g = target->getGraphForVariation(note.getVariationIndex());
        if (!g || !g->hasMidiOut()) continue;
        if (firstNoteMs < 0.0 || note.getStartTime() < firstNoteMs) {
            firstNoteMs = note.getStartTime();
        }
    }

    const double rate = captureRate > 0 ? static_cast<double>(captureRate)
                                        : static_cast<double>(audioEngine->getSampleRate());
    TrackClip clip;
    clip.filePath = path;
    clip.manualNudgeMs = 0.0;
    clip.gain = 1.0f;
    clip.enabled = true;
    if (foundOnset && firstNoteMs >= 0.0) {
        // The recording's pre-roll (and any stream-open click) is trimmed
        // away at load time (clip.onsetMs), so the clip sits right at the
        // first note. Keep 500 ms of pad for the module's reverb tail.
        clip.onsetMs = onsetMs;
        clip.offsetMs = firstNoteMs;
        clip.durationMs = (lastSignalMs > onsetMs)
                              ? (lastSignalMs - onsetMs) + 500.0
                              : (static_cast<double>(frames) / rate) * 1000.0;
    } else {
        // No signal found (or no MIDI-out note): keep the whole recording
        // at time 0, the pre-fix behaviour.
        clip.onsetMs = 0.0;
        clip.offsetMs = 0.0;
        clip.durationMs = (static_cast<double>(frames) / rate) * 1000.0;
    }
    target->setClip(clip);
    target->prepareClipForPlayback();

    // Recording level feedback so the UR22 input gain can be dialed in
    // without exporting to Audacity each time (Nimus's gain trial-and-error).
    QString peakLine;
    if (peakFs > 0.0f) {
        const double peakDb = 20.0 * std::log10(peakFs);
        peakLine = QString("\nRecording peak: %1 dBFS%2")
                       .arg(peakDb, 0, 'f', 1)
                       .arg(peakDb < -20.0 ? " - quiet, raise the input gain"
                            : peakDb > -1.0 ? " - close to clipping, lower the input gain"
                                            : QString());
    }

    QMessageBox::information(this, "Bake",
        foundOnset && firstNoteMs >= 0.0
            ? QString("Recording attached to track \"%1\".\n"
                      "Onset detected at %2 ms; clip placed at the first VL70-m note (%3 ms).%4\n\n"
                      "This track now plays its recording instead of MIDI - "
                      "the module is free for the next track's bake.")
                  .arg(target->getName())
                  .arg(onsetMs, 0, 'f', 1)
                  .arg(firstNoteMs, 0, 'f', 1)
                  .arg(peakLine)
            : QString("Recording attached to track \"%1\" without onset alignment "
                      "(no signal found in the recording).%2\n\n"
                      "This track now plays its recording instead of MIDI - "
                      "the module is free for the next track's bake.")
                  .arg(target->getName())
                  .arg(peakLine));
}

void ScoreCanvasWindow::onTimeModeToggled()
{
    // Toggle between Absolute and Musical time modes
    if (currentTimeMode == AbsoluteTime) {
        currentTimeMode = MusicalTime;
        timeModeToggle->setText("Musical");
        timeModeToggle->setChecked(true);
        qDebug() << "ScoreCanvasWindow: Switched to Musical time mode";
    } else {
        currentTimeMode = AbsoluteTime;
        timeModeToggle->setText("Absolute");
        timeModeToggle->setChecked(false);
        qDebug() << "ScoreCanvasWindow: Switched to Absolute time mode";
    }

    // Update timeline display
    timeline->setTimeMode(currentTimeMode == MusicalTime ? Timeline::Musical : Timeline::Absolute);
    timeline->setTempo(currentTempo);
    timeline->setTimeSignature(currentTimeSigTop, currentTimeSigBottom);

    // Update ScoreCanvas musical mode to show/hide bar lines
    scoreCanvas->setMusicalMode(currentTimeMode == MusicalTime, currentTempo, currentTimeSigTop, currentTimeSigBottom);
}

void ScoreCanvasWindow::onTimeSignatureModeChanged(int index)
{
    if (m_suppressTimeSigModeSwitch) return;

    if (index == 0) {
        // Switching to Kala — save Western cache, restore Kala cache
        cachedWesternTempo = westernTempoSpin->value();
        cachedWesternNum = westernNumSpin->value();
        cachedWesternDen = westernDenSpin->value();

        kalaPulsesSpin->setValue(cachedKalaPulses);
        kalaDurationSpin->setValue(cachedKalaDurationMs);
        timeSigStack->setCurrentIndex(0);
    } else {
        // Switching to Western — save Kala cache, restore Western cache
        cachedKalaPulses = kalaPulsesSpin->value();
        cachedKalaDurationMs = kalaDurationSpin->value();

        westernTempoSpin->setValue(cachedWesternTempo);
        westernNumSpin->setValue(cachedWesternNum);
        westernDenSpin->setValue(cachedWesternDen);
        timeSigStack->setCurrentIndex(1);
    }

    // Push to backend
    onTimeSettingsChanged();
}

void ScoreCanvasWindow::onTimeSettingsChanged()
{
    if (m_suppressTimeSigModeSwitch) return;
    if (!timeline) return;  // Not yet initialized during constructor

    double nowTime = timeline->getNowMarker();
    int newTempo, newTop, newBottom;

    if (timeSignatureModeCombo->currentIndex() == 0) {
        // Kala mode: tempo derived from pulse duration
        newTop = kalaPulsesSpin->value();
        newBottom = 0;
        newTempo = static_cast<int>(60000.0 / kalaDurationSpin->value());
        if (newTempo < 1) newTempo = 1;
        if (newTempo > 300) newTempo = 300;

        cachedKalaPulses = newTop;
        cachedKalaDurationMs = kalaDurationSpin->value();
    } else {
        // Western mode
        newTempo = westernTempoSpin->value();
        newTop = westernNumSpin->value();
        newBottom = westernDenSpin->value();

        cachedWesternTempo = newTempo;
        cachedWesternNum = newTop;
        cachedWesternDen = newBottom;
    }

    qDebug() << "ScoreCanvasWindow: Time settings changed — tempo:" << newTempo
             << "time sig:" << newTop << "/" << newBottom << "at time" << nowTime;

    // Update the marker at the current position (or default at time 0)
    if (nowTime == 0.0) {
        scoreCanvas->setDefaultTempo(newTempo);
        scoreCanvas->setDefaultTimeSignature(newTop, newBottom);
        compositionSettings.tempo = newTempo;
        compositionSettings.referenceTempo = newTempo;
    } else {
        TempoTimeSignature tts = scoreCanvas->getTempoTimeSignatureAtTime(nowTime);
        tts.bpm = newTempo;
        tts.timeSigNumerator = newTop;
        tts.timeSigDenominator = newBottom;
        scoreCanvas->addTempoChange(nowTime, tts);
    }

    // Update internal state
    currentTempo = newTempo;
    currentTimeSigTop = newTop;
    currentTimeSigBottom = newBottom;

    // Update status bar
    statusTempoLabel->setText(QString("%1 BPM").arg(currentTempo));
    if (currentTimeSigBottom == 0)
        statusTimeSigLabel->setText(QString("%1 pulses").arg(currentTimeSigTop));
    else
        statusTimeSigLabel->setText(QString("%1/%2").arg(currentTimeSigTop).arg(currentTimeSigBottom));

    // Update timeline
    timeline->setTempo(currentTempo);
    timeline->setTimeSignature(currentTimeSigTop, currentTimeSigBottom);

    // Update ScoreCanvas musical mode
    if (currentTimeMode == MusicalTime)
        scoreCanvas->setMusicalMode(true, currentTempo, currentTimeSigTop, currentTimeSigBottom);

    scoreCanvas->update();
    emit compositionSettingsChanged();
}

void ScoreCanvasWindow::onNowMarkerChanged(double timeMs)
{
    if (!scoreCanvas) return;

    TempoTimeSignature tts = scoreCanvas->getTempoTimeSignatureAtTime(timeMs);

    m_suppressTimeSigModeSwitch = true;

    // Block all spinbox signals
    kalaPulsesSpin->blockSignals(true);
    kalaDurationSpin->blockSignals(true);
    westernTempoSpin->blockSignals(true);
    westernNumSpin->blockSignals(true);
    westernDenSpin->blockSignals(true);

    int tempo = static_cast<int>(tts.bpm);

    if (tts.timeSigDenominator == 0) {
        // Kala mode
        timeSignatureModeCombo->setCurrentIndex(0);
        timeSigStack->setCurrentIndex(0);
        kalaPulsesSpin->setValue(tts.timeSigNumerator);
        kalaDurationSpin->setValue(static_cast<int>(60000.0 / tempo));
    } else {
        // Western mode
        timeSignatureModeCombo->setCurrentIndex(1);
        timeSigStack->setCurrentIndex(1);
        westernTempoSpin->setValue(tempo);
        westernNumSpin->setValue(tts.timeSigNumerator);
        westernDenSpin->setValue(tts.timeSigDenominator);
    }

    // Restore signals
    kalaPulsesSpin->blockSignals(false);
    kalaDurationSpin->blockSignals(false);
    westernTempoSpin->blockSignals(false);
    westernNumSpin->blockSignals(false);
    westernDenSpin->blockSignals(false);

    m_suppressTimeSigModeSwitch = false;

    // Update internal state
    currentTempo = tempo;
    currentTimeSigTop = tts.timeSigNumerator;
    currentTimeSigBottom = tts.timeSigDenominator;
}

void ScoreCanvasWindow::refreshToolbar()
{
    onNowMarkerChanged(timeline->getNowMarker());
    statusTempoLabel->setText(QString("%1 BPM").arg(currentTempo));
    if (currentTimeSigBottom == 0)
        statusTimeSigLabel->setText(QString("%1 pulses").arg(currentTimeSigTop));
    else
        statusTimeSigLabel->setText(QString("%1/%2").arg(currentTimeSigTop).arg(currentTimeSigBottom));
}

void ScoreCanvasWindow::scaleNoteTimes(double scaleFactor)
{
    // Scale all note start times and durations by the given factor
    // This is called when tempo changes to keep notes at the same musical positions

    if (scaleFactor <= 0.0 || scaleFactor == 1.0) {
        return;  // No scaling needed
    }

    // Get mutable reference to notes
    QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    int noteCount = notes.size();
    if (noteCount == 0) {
        qDebug() << "ScoreCanvasWindow::scaleNoteTimes: No notes to scale";
        return;
    }

    qDebug() << "ScoreCanvasWindow::scaleNoteTimes: Scaling" << noteCount << "notes by factor" << scaleFactor;

    for (Note& note : notes) {
        // Scale start time
        double oldStartTime = note.getStartTime();
        double newStartTime = oldStartTime * scaleFactor;
        note.setStartTime(newStartTime);

        // Scale duration
        double oldDuration = note.getDuration();
        double newDuration = oldDuration * scaleFactor;
        note.setDuration(newDuration);

        // Mark note as dirty for re-rendering
        note.setRenderDirty(true);
    }

    // Also scale any scale changes (modulations) in the composition
    QMap<double, QPair<Scale, double>> scaleChanges = scoreCanvas->getScaleChanges();
    if (!scaleChanges.isEmpty()) {
        scoreCanvas->clearScaleChanges();
        for (auto it = scaleChanges.begin(); it != scaleChanges.end(); ++it) {
            double newTime = it.key() * scaleFactor;
            scoreCanvas->addScaleChange(newTime, it.value().first, it.value().second);
        }
        qDebug() << "ScoreCanvasWindow::scaleNoteTimes: Also scaled" << scaleChanges.size() << "scale change markers";
    }

    // Also scale tempo change marker times (but not their BPM values)
    QMap<double, TempoTimeSignature> tempoChanges = scoreCanvas->getTempoChanges();
    if (!tempoChanges.isEmpty()) {
        scoreCanvas->clearTempoChanges();
        for (auto it = tempoChanges.begin(); it != tempoChanges.end(); ++it) {
            double newTime = it.key() * scaleFactor;
            scoreCanvas->addTempoChange(newTime, it.value());
        }
        qDebug() << "ScoreCanvasWindow::scaleNoteTimes: Also scaled" << tempoChanges.size() << "tempo change markers";
    }

    // Mark phrase as dirty
    scoreCanvas->getPhrase().markDirty();

    qDebug() << "ScoreCanvasWindow::scaleNoteTimes: Scaling complete";
}

void ScoreCanvasWindow::updateSounitTrack(int trackIndex, const QString &sounitName, const QColor &sounitColor)
{
    // Update the specified track with the loaded sounit information
    // For now, all sounits use the full register (lowest to highest note)
    const double LOWEST_NOTE = 27.5;   // A0
    const double HIGHEST_NOTE = 4186.0; // C8

    // Check if the track exists
    const QVector<TrackSelector::Track>& tracks = trackSelector->getTracks();
    if (trackIndex >= tracks.size()) {
        qWarning() << "ScoreCanvasWindow: Invalid track index" << trackIndex << "- only" << tracks.size() << "tracks exist";
        return;
    }

    // Update the specified track
    trackSelector->updateTrack(trackIndex, sounitName, sounitColor);

    qDebug() << "ScoreCanvasWindow: Track" << trackIndex << "updated with sounit:" << sounitName;
}

void ScoreCanvasWindow::onCompositionSettingsTriggered()
{
    qDebug() << "ScoreCanvasWindow: Composition Settings menu item clicked!";

    // Get current settings from toolbar state
    CompositionSettings current = getSettings();
    qDebug() << "ScoreCanvasWindow: Current settings retrieved - Name:" << current.compositionName
             << "Tempo:" << current.tempo;

    // Show modal dialog
    qDebug() << "ScoreCanvasWindow: Creating dialog...";
    CompositionSettingsDialog dialog(current, this);
    qDebug() << "ScoreCanvasWindow: Dialog created, showing...";

    if (dialog.exec() == QDialog::Accepted) {
        // User clicked OK - apply new settings
        CompositionSettings newSettings = dialog.getSettings();

        // Check if tempo changed - if so, scale note times
        int oldTempo = compositionSettings.referenceTempo;
        int newTempo = newSettings.tempo;
        if (oldTempo != newTempo && oldTempo > 0) {
            double scaleFactor = static_cast<double>(oldTempo) / static_cast<double>(newTempo);
            scaleNoteTimes(scaleFactor);
            qDebug() << "ScoreCanvasWindow: Dialog tempo change - scaled notes by" << scaleFactor;
        }

        // Update referenceTempo to match new tempo
        newSettings.referenceTempo = newSettings.tempo;

        updateFromSettings(newSettings);

        qDebug() << "ScoreCanvasWindow: Composition settings updated to:"
                 << newSettings.compositionName
                 << "tempo:" << newSettings.tempo
                 << "time sig:" << newSettings.timeSigTop << "/" << newSettings.timeSigBottom;
    }
    // User clicked Cancel - do nothing
}

CompositionSettings ScoreCanvasWindow::getSettings() const
{
    // Start with stored settings to preserve all fields (including sampleRate, bitDepth)
    CompositionSettings s = compositionSettings;

    // Update from current UI/internal state
    s.compositionName = compositionName.isEmpty() ? "Untitled" : compositionName;
    s.timeMode = (currentTimeMode == MusicalTime) ?
                 CompositionSettings::Musical : CompositionSettings::Absolute;
    s.tempo = currentTempo;
    s.referenceTempo = compositionSettings.referenceTempo;  // Preserve reference tempo
    s.timeSigTop = currentTimeSigTop;
    s.timeSigBottom = currentTimeSigBottom;
    s.autoRender = m_autoRender;
    s.syncLengthFromMs();   // Calculate bars from duration
    return s;
}

void ScoreCanvasWindow::updateFromSettings(const CompositionSettings &settings)
{
    // Block signals to prevent circular updates
    m_suppressTimeSigModeSwitch = true;
    kalaPulsesSpin->blockSignals(true);
    kalaDurationSpin->blockSignals(true);
    westernTempoSpin->blockSignals(true);
    westernNumSpin->blockSignals(true);
    westernDenSpin->blockSignals(true);
    timeModeToggle->blockSignals(true);

    // Update toolbar controls based on time signature mode
    int tempo = settings.tempo;
    if (settings.timeSigBottom == 0) {
        // Kala mode
        timeSignatureModeCombo->setCurrentIndex(0);
        timeSigStack->setCurrentIndex(0);
        kalaPulsesSpin->setValue(settings.timeSigTop);
        int pulseMs = static_cast<int>(60000.0 / tempo);
        if (pulseMs < 1) pulseMs = 1;
        if (pulseMs > 60000) pulseMs = 60000;
        kalaDurationSpin->setValue(pulseMs);
        cachedKalaPulses = settings.timeSigTop;
        cachedKalaDurationMs = pulseMs;
    } else {
        // Western mode
        timeSignatureModeCombo->setCurrentIndex(1);
        timeSigStack->setCurrentIndex(1);
        westernTempoSpin->setValue(tempo);
        westernNumSpin->setValue(settings.timeSigTop);
        westernDenSpin->setValue(settings.timeSigBottom);
        cachedWesternTempo = tempo;
        cachedWesternNum = settings.timeSigTop;
        cachedWesternDen = settings.timeSigBottom;
    }

    // Time mode toggle
    if (settings.timeMode == CompositionSettings::Musical) {
        timeModeToggle->setChecked(true);
        timeModeToggle->setText("Musical");
        timeModeToggle->setStyleSheet("background-color: #66BB6A; color: white;");
        currentTimeMode = MusicalTime;
    } else {
        timeModeToggle->setChecked(false);
        timeModeToggle->setText("Absolute");
        timeModeToggle->setStyleSheet("background-color: #B0BEC5; color: white;");
        currentTimeMode = AbsoluteTime;
    }

    // Restore signal processing
    kalaPulsesSpin->blockSignals(false);
    kalaDurationSpin->blockSignals(false);
    westernTempoSpin->blockSignals(false);
    westernNumSpin->blockSignals(false);
    westernDenSpin->blockSignals(false);
    timeModeToggle->blockSignals(false);
    m_suppressTimeSigModeSwitch = false;

    // Update internal state
    currentTempo = settings.tempo;
    currentTimeSigTop = settings.timeSigTop;
    currentTimeSigBottom = settings.timeSigBottom;

    // IMPORTANT: Update compositionSettings BEFORE calling anything that might scale
    compositionName = settings.compositionName;
    compositionSettings = settings;

    // Apply pre-render debounce delay
    if (m_renderDebounceTimer)
        m_renderDebounceTimer->setInterval(settings.preRenderDelayMs);

    // Update timeline and scoreCanvas directly
    timeline->setTimeMode(currentTimeMode == MusicalTime ? Timeline::Musical : Timeline::Absolute);
    timeline->setTempo(currentTempo);
    timeline->setTimeSignature(currentTimeSigTop, currentTimeSigBottom);
    scoreCanvas->setDefaultTempo(currentTempo);
    scoreCanvas->setDefaultTimeSignature(currentTimeSigTop, currentTimeSigBottom);
    scoreCanvas->setMusicalMode(currentTimeMode == MusicalTime, currentTempo, currentTimeSigTop, currentTimeSigBottom);

    // Restore render mode toggle
    m_autoRender = settings.autoRender;
    renderModeToggle->setChecked(m_autoRender);
    renderModeToggle->setText(m_autoRender ? "Auto" : "Manual");

    // Update canvas width based on composition length
    int canvasWidth = static_cast<int>(currentPixelsPerSecond * (settings.lengthMs / 1000.0));
    scoreCanvas->setMinimumSize(canvasWidth, scoreCanvas->minimumHeight());
    timeline->resize(canvasWidth, timeline->height());

    // Update status bar display
    statusTempoLabel->setText(QString("%1 BPM").arg(settings.tempo));
    if (settings.timeSigBottom == 0)
        statusTimeSigLabel->setText(QString("%1 pulses").arg(settings.timeSigTop));
    else
        statusTimeSigLabel->setText(QString("%1/%2").arg(settings.timeSigTop).arg(settings.timeSigBottom));

    qDebug() << "ScoreCanvasWindow: Canvas resized to" << canvasWidth << "pixels for"
             << settings.lengthMs << "ms composition (" << (settings.lengthMs / 60000.0) << "minutes)";
}

void ScoreCanvasWindow::onAddTrackTriggered()
{
    qDebug() << "ScoreCanvasWindow: Add Track menu item clicked";

    // Get track name from user
    bool ok;
    QString trackName = QInputDialog::getText(this, "Add Track",
                                             "Track name:",
                                             QLineEdit::Normal,
                                             "New Track", &ok);
    if (!ok || trackName.isEmpty()) {
        return;  // User canceled
    }

    // Get next color from palette
    QColor trackColor = getNextTrackColor();

    // Add track to TrackManager (data) - THIS IS CRITICAL FOR MULTI-TRACK PLAYBACK
    if (trackManager) {
        Track *newTrack = trackManager->addTrack(trackName, trackColor);
        qDebug() << "ScoreCanvasWindow: Added track to TrackManager, now has"
                 << trackManager->getTrackCount() << "tracks";
    } else {
        qWarning() << "ScoreCanvasWindow: TrackManager not set, track won't be playable!";
    }

    // Add track to TrackSelector (visual)
    // For now, use full frequency range (A0 to C8)
    const double LOWEST_NOTE = 27.5;    // A0
    const double HIGHEST_NOTE = 4186.0; // C8
    trackSelector->addTrack(trackName, trackColor, LOWEST_NOTE, HIGHEST_NOTE);

    qDebug() << "ScoreCanvasWindow: Added track" << trackName << "with color" << trackColor.name();
}

void ScoreCanvasWindow::onTrackSelected(int trackIndex)
{
    qDebug() << "ScoreCanvasWindow: Track" << trackIndex << "selected";

    // Set the active track in ScoreCanvas so new notes go to this track
    scoreCanvas->setActiveTrack(trackIndex);

    // Set the current Track pointer so ScoreCanvas can access variations
    if (trackManager) {
        Track* track = trackManager->getTrack(trackIndex);
        scoreCanvas->setCurrentTrack(track);
        bindCurveNamesFromTrack(track);
        qDebug() << "ScoreCanvasWindow: Set current track pointer for variations -"
                 << (track ? track->getVariationCount() : 0) << "variations available";
    }

    // Pre-rendering removed - now only happens when hitting play button
    // This avoids unnecessary rendering and potential crashes when switching tracks
}

void ScoreCanvasWindow::scheduleBackgroundRender()
{
    if (!audioEngine || !trackManager || m_backgroundRenderCount > 0) return;

    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();
    if (notes.isEmpty()) return;

    // Distribute notes to their respective tracks
    QMap<int, QList<Note>> notesByTrack;
    for (const Note &note : notes) {
        notesByTrack[note.getTrackIndex()].append(note);
    }

    for (int trackIdx = 0; trackIdx < trackManager->getTrackCount(); ++trackIdx) {
        Track *track = trackManager->getTrack(trackIdx);
        if (!track) continue;

        const QList<Note> &trackNotes = notesByTrack.value(trackIdx);
        track->syncNotes(trackNotes);

        if (!trackNotes.isEmpty()) {
            ++m_backgroundRenderCount;  // pre-increment before signal fires in startBackgroundRender
            bool started = track->startBackgroundRender(audioEngine->getSampleRate());
            if (!started) --m_backgroundRenderCount;  // undo if nothing was started
        }
    }
}

void ScoreCanvasWindow::prerenderNotes()
{
    if (!audioEngine) return;

    // Get all notes from the canvas phrase
    const QVector<Note>& notes = scoreCanvas->getPhrase().getNotes();

    if (notes.isEmpty()) {
        qDebug() << "ScoreCanvas: No notes to pre-render";
        return;
    }

    qDebug() << "=== ScoreCanvas: Auto pre-rendering" << notes.size() << "notes (multi-track) ===";

    // Use Track's note-based rendering if available
    if (trackManager && trackManager->getTrackCount() > 0) {
        // Distribute notes to their respective tracks based on trackIndex
        QMap<int, QList<Note>> notesByTrack;

        for (const Note &note : notes) {
            int trackIdx = note.getTrackIndex();
            notesByTrack[trackIdx].append(note);
        }

        // Pre-render each track
        for (int trackIdx = 0; trackIdx < trackManager->getTrackCount(); ++trackIdx) {
            Track *track = trackManager->getTrack(trackIdx);
            if (!track) continue;

            // Get notes for this track
            const QList<Note> &trackNotes = notesByTrack.value(trackIdx);

            // Sync notes to track (thread-safe to prevent race with audio callback)
            track->syncNotes(trackNotes);

            // Pre-render if track has notes
            if (!trackNotes.isEmpty()) {
                bool rendered = track->prerenderDirtyNotes(audioEngine->getSampleRate());
                if (!rendered) {
                    qWarning() << "ScoreCanvas: Track" << trackIdx << "pre-rendering failed";
                }
            }
        }

        qDebug() << "=== ScoreCanvas: Pre-rendering complete (ready for playback) ===";
        return;
    }

    // Fallback to legacy segment-based rendering
    qWarning() << "ScoreCanvas: Using legacy segment-based pre-rendering";
    audioEngine->renderNotes(notes, notes.size());

    qDebug() << "=== ScoreCanvas: Pre-rendering complete (ready for playback) ===";
}

QColor ScoreCanvasWindow::getNextTrackColor()
{
    // Get the next color from the palette and increment the index
    QColor color = trackColorPalette[nextColorIndex % trackColorPalette.size()];
    nextColorIndex++;
    qDebug() << "ScoreCanvasWindow: Assigned color" << color.name() << "(index" << (nextColorIndex - 1) << ")";
    return color;
}

void ScoreCanvasWindow::onRenderStarted()
{
    m_isRendering = true;
    renderProgressBar->setFormat("Rendering...");
    renderProgressBar->setMinimum(0);
    renderProgressBar->setMaximum(0);  // Indeterminate mode (animated busy bar)
    renderProgressBar->setVisible(true);
    renderProgressBar->show();
    qDebug() << "ScoreCanvas: onRenderStarted() called, backgroundCount=" << m_backgroundRenderCount;
}

void ScoreCanvasWindow::onRenderProgressChanged(int percentage)
{
    if (percentage <= 0) return;  // Stay in indeterminate "Rendering..." mode until real progress
    if (renderProgressBar->maximum() == 0) {
        // Switch from indeterminate to determinate mode
        renderProgressBar->setMaximum(100);
        renderProgressBar->setFormat("Render: %p%");
    }
    renderProgressBar->setValue(percentage);
    renderProgressBar->update();
}

void ScoreCanvasWindow::onRenderCompleted()
{
    m_isRendering = false;
    if (m_backgroundRenderCount > 0) {
        --m_backgroundRenderCount;
        if (m_backgroundRenderCount > 0) return;  // More background renders still in progress
    }
    // All renders done — show 100% then hide
    renderProgressBar->setMaximum(100);
    renderProgressBar->setFormat("Render: %p%");
    renderProgressBar->setValue(100);
    qDebug() << "ScoreCanvas: All renders completed, will hide in 3 seconds";
    QTimer::singleShot(3000, this, [this]() {
        renderProgressBar->setVisible(false);
    });
}

void ScoreCanvasWindow::onRenderCancelled()
{
    m_isRendering = false;
    if (m_backgroundRenderCount > 0) {
        --m_backgroundRenderCount;
        if (m_backgroundRenderCount > 0) return;  // More background renders still in progress
    }
    renderProgressBar->setMaximum(100);
    renderProgressBar->setValue(0);
    renderProgressBar->setFormat("Render cancelled");
    QTimer::singleShot(2000, this, [this]() {
        renderProgressBar->setVisible(false);
        renderProgressBar->setFormat("Render: %p%");
    });
}

void ScoreCanvasWindow::onScoreAutoScroll(int dx, int dy)
{
    if (dx != 0) {
        QScrollBar *hsb = scoreScrollArea->horizontalScrollBar();
        hsb->setValue(hsb->value() + dx);
    }
    if (dy != 0) {
        QScrollBar *vsb = scoreScrollArea->verticalScrollBar();
        vsb->setValue(vsb->value() + dy);
    }
}

ScoreCanvasWindow::~ScoreCanvasWindow()
{
    // Remove event filter
    qApp->removeEventFilter(this);

    // Don't delete audioEngine - it's owned by KalaMain
    delete ui;
}

void ScoreCanvasWindow::setTrackManager(TrackManager *manager)
{
    trackManager = manager;
    qDebug() << "ScoreCanvasWindow: TrackManager set -" << (manager ? manager->getTrackCount() : 0) << "tracks available";

    if (trackManager) {
        // Connect to existing tracks' progress signals
        for (int i = 0; i < trackManager->getTrackCount(); ++i) {
            Track *track = trackManager->getTrack(i);
            if (track) {
                connectTrackProgressSignals(track);
            }
        }

        // Connect to trackAdded signal to catch future tracks
        connect(trackManager, &TrackManager::trackAdded, this, [this](Track *track, int) {
            if (track) {
                connectTrackProgressSignals(track);
            }
        });

        // Set initial current track on ScoreCanvas so variations work from startup
        Track *currentTrack = trackManager->getCurrentTrack();
        if (currentTrack) {
            scoreCanvas->setCurrentTrack(currentTrack);
            bindCurveNamesFromTrack(currentTrack);
            qDebug() << "ScoreCanvasWindow: Set initial current track -"
                     << currentTrack->getVariationCount() << "variations";
        }

        // Keep ScoreCanvas current track in sync when track selection changes
        connect(trackManager, &TrackManager::currentTrackChanged, this, [this](Track *track, int) {
            scoreCanvas->setCurrentTrack(track);
            bindCurveNamesFromTrack(track);
            qDebug() << "ScoreCanvasWindow: Current track changed -"
                     << (track ? track->getVariationCount() : 0) << "variations";
        });
    }
}

void ScoreCanvasWindow::connectTrackProgressSignals(Track *track)
{
    if (!track) return;

    connect(track, &Track::renderStarted, this, &ScoreCanvasWindow::onRenderStarted);
    connect(track, &Track::renderProgressChanged, this, &ScoreCanvasWindow::onRenderProgressChanged);
    connect(track, &Track::renderCompleted, this, &ScoreCanvasWindow::onRenderCompleted);
    connect(track, &Track::renderCancelled, this, &ScoreCanvasWindow::onRenderCancelled);

    qDebug() << "ScoreCanvasWindow: Connected progress signals for track" << track->getName();
}

void ScoreCanvasWindow::updateFrequencyLabels()
{
    // Use the scale active at the left edge of the visible viewport
    int scrollOffset = scoreScrollArea->horizontalScrollBar()->value();
    double leftEdgeTime = scoreCanvas->pixelToTime(scrollOffset);
    Scale scale = scoreCanvas->getScaleAtTime(leftEdgeTime);
    double baseFreq = scoreCanvas->getBaseFrequencyAtTime(leftEdgeTime);

    auto canvasLines = scoreCanvas->generateScaleLinesForScale(scale, baseFreq);

    QVector<FrequencyLabels::ScaleLine> labelLines;
    for (const auto &cl : canvasLines) {
        FrequencyLabels::ScaleLine fl;
        fl.frequencyHz = cl.frequencyHz;
        fl.scaleDegree = cl.scaleDegree;
        fl.color = cl.color;
        fl.noteName = cl.noteName;
        fl.isThicker = cl.isThicker;
        fl.isAccidental = cl.isAccidental;
        labelLines.append(fl);
    }
    frequencyLabels->setScaleLines(labelLines);
}

void ScoreCanvasWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("windows/scoreCanvas/geometry", saveGeometry());
    // Curve lanes view settings (Phase F). The density tier is deliberately
    // NOT here - it resets to Off each session ("Non save", Nimus 2026-08-26).
    if (scoreCanvas) {
        settings.setValue("scoreCanvas/lanesAbove", scoreCanvas->getLanesAbove());
        settings.setValue("scoreCanvas/dimOthers", scoreCanvas->getDimOthers());
    }
    settings.setValue("scoreCanvas/legendVisible", lanesLegend && lanesLegend->isVisible());
    QMainWindow::closeEvent(event);
}
