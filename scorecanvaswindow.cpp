#include "scorecanvaswindow.h"
#include "ui_scorecanvas.h"
#include "kalamain.h"
#include "compositionsettingsdialog.h"
#include "gotodialog.h"
#include "trackmanager.h"
#include "track.h"
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
#include <QCursor>
#include <QTimer>
#include <QInputDialog>
#include <cmath>

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
    , currentTimeSigTop(4)
    , currentTimeSigBottom(4)
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

    statusTimeSigLabel = new QLabel("4/4", this);
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
    renderProgressBar->setVisible(true);  // TESTING: Always visible to verify it exists
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
    setupTrackSelector();
    setupScoreCanvas();
    setupZoom();

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

    // Connect transport controls
    connect(ui->actionPlay, &QAction::triggered, this, &ScoreCanvasWindow::startPlayback);
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

    ui->toolBar->addSeparator();

    // Tempo Label and SpinBox
    QLabel *tempoLabel = new QLabel(" Tempo: ", this);
    ui->toolBar->addWidget(tempoLabel);

    tempoSpinBox = new QSpinBox(this);
    tempoSpinBox->setRange(20, 300);  // Reasonable BPM range
    tempoSpinBox->setValue(currentTempo);
    tempoSpinBox->setSuffix(" BPM");
    tempoSpinBox->setMinimumWidth(90);
    tempoSpinBox->setFocusPolicy(Qt::StrongFocus);  // Only gets focus when clicked
    ui->toolBar->addWidget(tempoSpinBox);
    connect(tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTempoChanged);

    ui->toolBar->addSeparator();

    // Time Signature Label and SpinBoxes
    QLabel *timeSigLabel = new QLabel(" Time Sig: ", this);
    ui->toolBar->addWidget(timeSigLabel);

    timeSigNumerator = new QSpinBox(this);
    timeSigNumerator->setRange(1, 99);  // Up to 99 for complex rhythms
    timeSigNumerator->setValue(currentTimeSigTop);
    timeSigNumerator->setMinimumWidth(60);
    timeSigNumerator->setMaximumWidth(60);
    timeSigNumerator->setFocusPolicy(Qt::StrongFocus);  // Only gets focus when clicked
    ui->toolBar->addWidget(timeSigNumerator);

    QLabel *slashLabel = new QLabel(" / ", this);
    ui->toolBar->addWidget(slashLabel);

    timeSigDenominator = new QSpinBox(this);
    // 0 = simple mode (beat = tempo), 1-99 = scaled mode
    timeSigDenominator->setRange(0, 99);
    timeSigDenominator->setValue(currentTimeSigBottom);
    timeSigDenominator->setMinimumWidth(60);
    timeSigDenominator->setMaximumWidth(60);
    timeSigDenominator->setFocusPolicy(Qt::StrongFocus);  // Only gets focus when clicked
    ui->toolBar->addWidget(timeSigDenominator);

    connect(timeSigNumerator, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSignatureChanged);
    connect(timeSigDenominator, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ScoreCanvasWindow::onTimeSignatureChanged);

    // Connect File menu action
    connect(ui->actionCompositionSettings, &QAction::triggered,
            this, &ScoreCanvasWindow::onCompositionSettingsTriggered);
    connect(ui->actionAddTrack, &QAction::triggered,
            this, &ScoreCanvasWindow::onAddTrackTriggered);

    qDebug() << "ScoreCanvasWindow: Composition settings configured";
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

    currentMinHz = minHz;
    currentMaxHz = maxHz;

    // Apply to all three widgets synchronously
    scoreCanvas->setFrequencyRange(minHz, maxHz);
    trackSelector->setFrequencyRange(minHz, maxHz);
    frequencyLabels->setFrequencyRange(minHz, maxHz);
}

void ScoreCanvasWindow::applyHorizontalZoom(double pixelsPerSecond)
{
    // Constrain to reasonable limits (10 to 1000 pixels per second)
    if (pixelsPerSecond < 10.0) pixelsPerSecond = 10.0;
    if (pixelsPerSecond > 1000.0) pixelsPerSecond = 1000.0;

    currentPixelsPerSecond = pixelsPerSecond;

    // Apply to both ScoreCanvas and Timeline
    scoreCanvas->setPixelsPerSecond(pixelsPerSecond);
    timeline->setPixelsPerSecond(pixelsPerSecond);
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

        // Just intonation major scale ratios starting from BASE_FREQUENCY (25 Hz)
        const double BASE_FREQ = 25.0;
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

    // G key opens goto dialog
    if (event->key() == Qt::Key_G && event->modifiers() == Qt::NoModifier) {
        // Create and show goto dialog
        CompositionSettings::TimeMode mode = (currentTimeMode == MusicalTime) ?
            CompositionSettings::Musical : CompositionSettings::Absolute;

        GotoDialog dialog(mode, currentTempo, currentTimeSigTop, currentTimeSigBottom, this);
        if (dialog.exec() == QDialog::Accepted) {
            double targetTimeMs = dialog.getTargetTimeMs();

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
            dragStartPos = tabletEvent->position().toPoint();
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

            // Calculate vertical drag distance (for frequency zoom)
            int deltaY = dragStartPos.y() - tabletEvent->position().y();

            // Calculate horizontal drag distance (for time zoom)
            int deltaX = dragStartPos.x() - tabletEvent->position().x();

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
                dragStartPos = mouseEvent->pos();
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

            // Calculate vertical drag distance (for frequency zoom)
            int deltaY = dragStartPos.y() - mouseEvent->pos().y();

            // Calculate horizontal drag distance (for time zoom)
            int deltaX = dragStartPos.x() - mouseEvent->pos().x();

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

void ScoreCanvasWindow::startPlayback()
{
    if (!audioEngine) return;

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
            audioEngine->playFromTracks(tracksToPlay, playbackStartPosition);
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

void ScoreCanvasWindow::onTempoChanged(int bpm)
{
    // Get current timeline position
    double nowTime = timeline->getNowMarker();

    qDebug() << "ScoreCanvasWindow: Tempo changed to" << bpm << "BPM at time" << nowTime;

    if (nowTime == 0.0) {
        // At time 0, update the default tempo
        scoreCanvas->setDefaultTempo(bpm);

        // Also update composition settings for reference
        compositionSettings.tempo = bpm;
        compositionSettings.referenceTempo = bpm;
    } else {
        // At other positions, add/update a tempo marker
        // Get current settings at this time to preserve time signature and gradual flag
        TempoTimeSignature tts = scoreCanvas->getTempoTimeSignatureAtTime(nowTime);
        tts.bpm = bpm;
        scoreCanvas->addTempoChange(nowTime, tts);
    }

    // Update internal state
    currentTempo = bpm;

    // Update timeline
    timeline->setTempo(currentTempo);

    // If in musical mode, update bar line spacing
    if (currentTimeMode == MusicalTime) {
        scoreCanvas->setMusicalMode(true, currentTempo, currentTimeSigTop, currentTimeSigBottom);
    }

    // Refresh the canvas to show updated note positions
    scoreCanvas->update();

    emit compositionSettingsChanged();
}

void ScoreCanvasWindow::onTimeSignatureChanged()
{
    int newNum = timeSigNumerator->value();
    int newDenom = timeSigDenominator->value();

    // Get current timeline position
    double nowTime = timeline->getNowMarker();

    qDebug() << "ScoreCanvasWindow: Time signature changed to"
             << newNum << "/" << newDenom << "at time" << nowTime;

    if (nowTime == 0.0) {
        // At time 0, update the default time signature
        scoreCanvas->setDefaultTimeSignature(newNum, newDenom);
    } else {
        // At other positions, add/update a tempo marker
        // Get current settings at this time to preserve tempo and gradual flag
        TempoTimeSignature tts = scoreCanvas->getTempoTimeSignatureAtTime(nowTime);
        tts.timeSigNumerator = newNum;
        tts.timeSigDenominator = newDenom;
        scoreCanvas->addTempoChange(nowTime, tts);
    }

    // Update internal state
    currentTimeSigTop = newNum;
    currentTimeSigBottom = newDenom;

    // Update timeline
    timeline->setTimeSignature(currentTimeSigTop, currentTimeSigBottom);

    // If in musical mode, update bar line spacing
    if (currentTimeMode == MusicalTime) {
        scoreCanvas->setMusicalMode(true, currentTempo, currentTimeSigTop, currentTimeSigBottom);
    }

    emit compositionSettingsChanged();
}

void ScoreCanvasWindow::onNowMarkerChanged(double timeMs)
{
    // Get tempo/time signature at the new timeline position
    TempoTimeSignature tts = scoreCanvas->getTempoTimeSignatureAtTime(timeMs);

    // Block signals to prevent triggering onTempoChanged/onTimeSignatureChanged
    tempoSpinBox->blockSignals(true);
    timeSigNumerator->blockSignals(true);
    timeSigDenominator->blockSignals(true);

    // Update spinbox values
    tempoSpinBox->setValue(static_cast<int>(tts.bpm));
    timeSigNumerator->setValue(tts.timeSigNumerator);
    timeSigDenominator->setValue(tts.timeSigDenominator);

    // Restore signals
    tempoSpinBox->blockSignals(false);
    timeSigNumerator->blockSignals(false);
    timeSigDenominator->blockSignals(false);

    // Update internal state
    currentTempo = static_cast<int>(tts.bpm);
    currentTimeSigTop = tts.timeSigNumerator;
    currentTimeSigBottom = tts.timeSigDenominator;
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
    s.syncLengthFromMs();   // Calculate bars from duration
    return s;
}

void ScoreCanvasWindow::updateFromSettings(const CompositionSettings &settings)
{
    // Block signals to prevent circular updates
    tempoSpinBox->blockSignals(true);
    timeSigNumerator->blockSignals(true);
    timeSigDenominator->blockSignals(true);
    timeModeToggle->blockSignals(true);

    // Update toolbar controls
    tempoSpinBox->setValue(settings.tempo);
    timeSigNumerator->setValue(settings.timeSigTop);
    timeSigDenominator->setValue(settings.timeSigBottom);

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
    tempoSpinBox->blockSignals(false);
    timeSigNumerator->blockSignals(false);
    timeSigDenominator->blockSignals(false);
    timeModeToggle->blockSignals(false);

    // Update internal state
    currentTempo = settings.tempo;
    currentTimeSigTop = settings.timeSigTop;
    currentTimeSigBottom = settings.timeSigBottom;

    // IMPORTANT: Update compositionSettings BEFORE calling onTempoChanged
    // This prevents unwanted scaling when loading saved settings
    compositionName = settings.compositionName;
    compositionSettings = settings;

    // Update timeline and scoreCanvas directly (don't call onTimeModeToggled - it's a toggle!)
    timeline->setTimeMode(currentTimeMode == MusicalTime ? Timeline::Musical : Timeline::Absolute);
    timeline->setTempo(currentTempo);
    timeline->setTimeSignature(currentTimeSigTop, currentTimeSigBottom);
    scoreCanvas->setMusicalMode(currentTimeMode == MusicalTime, currentTempo, currentTimeSigTop, currentTimeSigBottom);

    // Note: We don't call onTempoChanged here because:
    // 1. compositionSettings is already set with the correct referenceTempo
    // 2. Calling onTempoChanged would compare referenceTempo to tempo (same values) and skip scaling
    // 3. Timeline and scoreCanvas are already updated above
    onTimeSignatureChanged();

    // Update canvas width based on composition length
    int canvasWidth = static_cast<int>(currentPixelsPerSecond * (settings.lengthMs / 1000.0));
    scoreCanvas->setMinimumSize(canvasWidth, scoreCanvas->minimumHeight());
    timeline->resize(canvasWidth, timeline->height());

    // Update status bar display
    statusTempoLabel->setText(QString("%1 BPM").arg(settings.tempo));
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
        qDebug() << "ScoreCanvasWindow: Set current track pointer for variations -"
                 << (track ? track->getVariationCount() : 0) << "variations available";
    }

    // Pre-rendering removed - now only happens when hitting play button
    // This avoids unnecessary rendering and potential crashes when switching tracks
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
    qDebug() << "ScoreCanvas: onRenderStarted() called!";
    m_isRendering = true;
    renderProgressBar->setFormat("Rendering...");
    renderProgressBar->setMinimum(0);
    renderProgressBar->setMaximum(0);  // Indeterminate mode (animated busy bar)
    renderProgressBar->setVisible(true);
    renderProgressBar->show();
    qDebug() << "ScoreCanvas: Progress bar visible:" << renderProgressBar->isVisible();
}

void ScoreCanvasWindow::onRenderProgressChanged(int percentage)
{
    //qDebug() << "ScoreCanvas: Render progress:" << percentage << "%";
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
    renderProgressBar->setMaximum(100);
    renderProgressBar->setFormat("Render: %p%");
    renderProgressBar->setValue(100);
    qDebug() << "ScoreCanvas: Render completed at 100%, will hide in 3 seconds";
    // Keep progress bar visible for 3 seconds so user can see it
    QTimer::singleShot(3000, this, [this]() {
        renderProgressBar->setVisible(false);
        qDebug() << "ScoreCanvas: Progress bar now hidden";
    });
}

void ScoreCanvasWindow::onRenderCancelled()
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
            qDebug() << "ScoreCanvasWindow: Set initial current track -"
                     << currentTrack->getVariationCount() << "variations";
        }

        // Keep ScoreCanvas current track in sync when track selection changes
        connect(trackManager, &TrackManager::currentTrackChanged, this, [this](Track *track, int) {
            scoreCanvas->setCurrentTrack(track);
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
        labelLines.append(fl);
    }
    frequencyLabels->setScaleLines(labelLines);
}
