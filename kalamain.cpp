#include "kalamain.h"
#include "ui_kalamain.h"
#include "kalatools.h"
#include "kalaagent.h"
#include "companionpanel.h"
#include <QDockWidget>
#include <QSettings>
#include "scorecanvaswindow.h"
#include "canvas.h"
#include "container.h"
#include "spectrumvisualizer.h"
#include "envelopevisualizer.h"
#include "dnaeditordialog.h"
#include "envelopelibraryDialog.h"
#include "scaledialog.h"
#include "scorecanvascommands.h"
#include "tempodialog.h"
#include "easing.h"
#include "easingapplicator.h"
#include "compositionsettings.h"
#include "scale.h"
#include "track.h"
#include "rendercache.h"
#include "exportaudiodialog.h"
#include "containersettings.h"
#include "wavetablesynth.h"
#include "vibratoeditordialog.h"
#include "helpdialog.h"
#include "dynamicscurvedialog.h"
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QScrollArea>
#include <QFrame>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QDial>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTimeEdit>
#include <QTime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QRegularExpression>
#include <cmath>
#include <algorithm>

KalaMain::KalaMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::KalaMain)
    , sounitBuilder(nullptr)    // Initialize pointers
    , scoreCanvasWindow(nullptr) // Initialize pointers
    , audioEngine(nullptr)
    , currentEditingTrack(-1)  // No sounit loaded initially
    , m_isLoadingSounit(false)  // ***  Initialize flag ***
{
    ui->setupUi(this);

    // Initialize shared audio engine FIRST
    audioEngine = new AudioEngine();
    if (!audioEngine->initialize()) {
        qWarning("Failed to initialize audio engine - playback will not work");
    } else {
        qDebug() << "Shared AudioEngine initialized successfully";
        audioEngine->startDeviceNotifier();
    }

    // Connect audio device change signals
    connect(audioEngine, &AudioEngine::playbackStopRequested, this, &KalaMain::stopAllPlayback);
    connect(audioEngine, &AudioEngine::audioDeviceChanged, this, [](const QString &name) {
        qDebug() << "Audio device changed:" << name;
    });
    connect(audioEngine, &AudioEngine::audioDeviceError, this, [this](const QString &msg) {
        QMessageBox::warning(this, "Audio Device Error", msg);
    });

    //  Create TrackManager and add default track
    trackManager = new TrackManager(this);
    Track *defaultTrack = trackManager->addTrack("Track 1", QColor("#3498db"));
    qDebug() << "Created default track:" << defaultTrack->getName();

    // Create SounitBuilder with shared audio engine - NOT parented to make it independent
    sounitBuilder = new SounitBuilder(audioEngine);
    sounitBuilder->hide();  // Hide initially

    // Set window attribute so it doesn't prevent app quit
    sounitBuilder->setAttribute(Qt::WA_QuitOnClose, false);

    // Set main window reference for Tab key toggle
    sounitBuilder->setMainWindow(this);

    // Set SounitBuilder to display the default track's canvas
    sounitBuilder->setTrackCanvas(defaultTrack);

    // Create ScoreCanvasWindow - NOT parented to this to make it independent
    scoreCanvasWindow = new ScoreCanvasWindow(audioEngine);
    scoreCanvasWindow->hide();  // Hide initially

    // Set window attribute so it doesn't prevent app quit
    scoreCanvasWindow->setAttribute(Qt::WA_QuitOnClose, false);

    // NEW: Give ScoreCanvasWindow access to TrackManager for rendering
    scoreCanvasWindow->setTrackManager(trackManager);

    // Set main window reference for Tab key toggle
    scoreCanvasWindow->setMainWindow(this);

    // Connect TrackManager signals for mixer updates
    connect(trackManager, &TrackManager::trackAdded, this, [this](Track*, int) {
        rebuildMixer();
    });
    connect(trackManager, &TrackManager::trackRemoved, this, [this](int) {
        rebuildMixer();
    });

    // Build initial mixer UI
    rebuildMixer();

    // Connect SounitBuilder to ScoreCanvas so both windows play the same notes
    sounitBuilder->setScoreCanvas(scoreCanvasWindow->getScoreCanvas());
    sounitBuilder->setScoreCanvasWindow(scoreCanvasWindow);

    // NEW: Connect Canvas selection signals to inspector update slots
    // Now get canvas from the Track
    Canvas *canvas = defaultTrack->getCanvas();
    connect(canvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
    connect(canvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
    connect(canvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);

    // Connect inspector edit signals
    connect(ui->editInstanceName, &QLineEdit::editingFinished, this, &KalaMain::onInstanceNameChanged);
    connect(ui->comboConnectionFunction, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KalaMain::onConnectionFunctionChanged);
    connect(ui->spinConnectionWeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KalaMain::onConnectionWeightChanged);
    connect(ui->buttonDisconnect, &QPushButton::clicked, this, &KalaMain::onDisconnectClicked);

    // Initialize inspectors as empty
    onSelectionCleared();

    connect(ui->MainTab, &QTabWidget::currentChanged, this, &KalaMain::onTabChanged);

    // Connect playback start signals to stop the other window
    connect(sounitBuilder, &SounitBuilder::playbackStarted, this, [this]() {
        if (scoreCanvasWindow) {
            scoreCanvasWindow->stopPlayback(false);  // Don't stop audio - SounitBuilder is taking over
        }
    });
    connect(scoreCanvasWindow, &ScoreCanvasWindow::playbackStarted, this, [this]() {
        if (sounitBuilder) {
            sounitBuilder->stopPlayback(false);  // Don't stop audio - ScoreCanvas is taking over
        }
    });

    // Connect track selector to switch Sound Engine canvas when track is selected
    TrackSelector *trackSelector = scoreCanvasWindow->getTrackSelector();
    connect(trackSelector, &TrackSelector::trackSelected, this, [this](int trackIndex) {
        // Switch Sound Engine canvas to show this track's containers
        if (trackSounitFiles.contains(trackIndex)) {
            qDebug() << "Track" << trackIndex << "selected in composition - switching Sound Engine canvas";
            switchToSounit(trackIndex);
        } else {
            qDebug() << "Track" << trackIndex << "selected but no sounit loaded for this track";
            trackManager->setCurrentTrack(trackIndex);  // Keep current track in sync even without sounit
        }
    });
    connect(trackSelector, &TrackSelector::multiSelectionChanged, this, [this](QVector<int> indices) {
        m_selectedTrackIndices = indices;
        updateMixerSelection();
        qDebug() << "KalaMain: multi-selection updated:" << indices;
    });

    // Connect File menu actions (Project)
    connect(ui->actionNew_project_Ctrl_N, &QAction::triggered, this, &KalaMain::onNewProject);
    connect(ui->actionOpen_ctrl_o, &QAction::triggered, this, &KalaMain::onOpenProject);
    connect(ui->actionSave_ctrl_S, &QAction::triggered, this, &KalaMain::onSaveProject);
    connect(ui->actionSave_AS_Ctrl_SHift_S, &QAction::triggered, this, &KalaMain::onSaveProjectAs);
    connect(ui->actionExport_Audio_Ctrl_E, &QAction::triggered, this, &KalaMain::onExportAudio);

    //  /Set up keyboard shortcuts for File menu
    ui->actionNew_project_Ctrl_N->setShortcut(QKeySequence::New);        // Ctrl+N //
    ui->actionOpen_ctrl_o->setShortcut(QKeySequence::Open);              // Ctrl+O
    ui->actionSave_ctrl_S->setShortcut(QKeySequence::Save);              // Ctrl+S
    ui->actionSave_AS_Ctrl_SHift_S->setShortcut(QKeySequence::SaveAs);   // Ctrl+Shift+S
    ui->actionExport_Audio_Ctrl_E->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));  // Ctrl+E

    // Make all File menu shortcuts fire regardless of which window has focus
    ui->actionNew_project_Ctrl_N->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionOpen_ctrl_o->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionSave_ctrl_S->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionSave_AS_Ctrl_SHift_S->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionExport_Audio_Ctrl_E->setShortcutContext(Qt::ApplicationShortcut);

    //  Connect Sound menu actions
    connect(ui->actionNew_Sounit, &QAction::triggered, this, &KalaMain::onNewSounit);
    connect(ui->actionLoad_Sounit, &QAction::triggered, this, &KalaMain::onLoadSounit);
    connect(ui->actionSave_Sounit_As, &QAction::triggered, this, &KalaMain::onSaveSounitAs);
    connect(canvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
    connect(canvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);

    // Connect Sounit Info UI fields
    connect(ui->comboSounitSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KalaMain::onSounitSelectorChanged);
    connect(ui->editSounitName, &QLineEdit::editingFinished,
            this, &KalaMain::onSounitNameEdited);
    connect(ui->editSounitComment, &QPlainTextEdit::textChanged, this, &KalaMain::onSounitCommentEdited);

    // Connect track22 management buttons
    connect(ui->btnAddTrack, &QPushButton::clicked, this, &KalaMain::onAddTrack);

    // Connect menu actions for track management
    connect(ui->actionNew_track, &QAction::triggered, this, &KalaMain::onAddTrack);
    connect(ui->actionDelete_Track, &QAction::triggered, this, &KalaMain::onDeleteTrack);

    // Connect variation management UI
    connect(ui->listVariations, &QListWidget::itemSelectionChanged, this, &KalaMain::onVariationSelectionChanged);
    connect(ui->btnDeleteVariation, &QPushButton::clicked, this, &KalaMain::onDeleteVariation);
    connect(ui->btnRenameVariation, &QPushButton::clicked, this, &KalaMain::onRenameVariation);
    connect(ui->editVariationsTrackName, &QLineEdit::editingFinished, this, &KalaMain::onVariationsTrackNameEdited);

    // Connect SounitBuilder variations changed signal
    connect(sounitBuilder, &SounitBuilder::variationsChanged, this, &KalaMain::refreshVariationsList);
    connect(sounitBuilder, &SounitBuilder::variationsChanged, this, &KalaMain::updateVariationSelector);

    // Refresh variations list whenever the active track changes (composition tab track selection)
    connect(trackManager, &TrackManager::currentTrackChanged, this, [this](Track*, int) {
        refreshVariationsList();
    });

    // Connect Sound Engine variation selector
    connect(ui->comboVariationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KalaMain::onVariationSelectorChanged);

    // Initialize variation selector
    updateVariationSelector();

    // Connect scale management buttons
    connect(ui->btnScaleEdit, &QPushButton::clicked, this, &KalaMain::onScaleEditClicked);
    connect(ui->btnScaleAdd, &QPushButton::clicked, this, &KalaMain::onScaleAddClicked);
    connect(ui->spinScaleMarker, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KalaMain::onScaleMarkerChanged);

    // Connect timeline marker changes for scale display updates
    connect(scoreCanvasWindow->getTimeline(), &Timeline::nowMarkerChanged,
            this, &KalaMain::updateScaleDisplay);

    // Connect ScoreCanvas scale changes for display updates
    connect(scoreCanvasWindow->getScoreCanvas(), &ScoreCanvas::scaleSettingsChanged,
            this, &KalaMain::updateScaleDisplay);

    // Initialize scale display
    updateScaleDisplay();

    // Connect tempo management buttons
    connect(ui->btnTempoEdit, &QPushButton::clicked, this, &KalaMain::onTempoEditClicked);
    connect(ui->btnTempoAdd, &QPushButton::clicked, this, &KalaMain::onTempoAddClicked);
    connect(ui->spinTempoMarker, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KalaMain::onTempoMarkerChanged);

    // Connect timeline marker changes for tempo display updates
    connect(scoreCanvasWindow->getTimeline(), &Timeline::nowMarkerChanged,
            this, &KalaMain::updateTempoDisplay);

    // Connect ScoreCanvas tempo changes for display updates
    connect(scoreCanvasWindow->getScoreCanvas(), &ScoreCanvas::tempoSettingsChanged,
            this, &KalaMain::updateTempoDisplay);

    // Initialize tempo display
    updateTempoDisplay();

    // Connect ScoreCanvas note selection signal for note inspector
    connect(scoreCanvasWindow->getScoreCanvas(), &ScoreCanvas::noteSelectionChanged,
            this, &KalaMain::onNoteSelectionChanged);

    // Connect note inspector widgets
    connect(ui->spinNoteStart, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KalaMain::onNoteStartChanged);
    connect(ui->spinNoteDuration, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &KalaMain::onNoteDurationChanged);
    connect(ui->spinNotePitch, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KalaMain::onNotePitchChanged);
    connect(ui->noteVariationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KalaMain::onNoteVariationChanged);

    // Vibrato controls
    connect(ui->checkNoteVibrato, &QCheckBox::toggled,
            this, &KalaMain::onNoteVibratoToggled);
    connect(ui->btnNoteVibratoEdit, &QPushButton::clicked,
            this, &KalaMain::onNoteVibratoEditClicked);

    // Expressive curves controls
    connect(ui->comboExpressiveCurve, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KalaMain::onExpressiveCurveChanged);
    connect(ui->btnExpressiveCurveAdd, &QPushButton::clicked,
            this, &KalaMain::onExpressiveCurveAdd);
    connect(ui->btnExpressiveCurveDelete, &QPushButton::clicked,
            this, &KalaMain::onExpressiveCurveDelete);
    connect(ui->btnExpressiveCurveApply, &QPushButton::clicked,
            this, &KalaMain::onExpressiveCurveApply);

    // Initial state for expressive curves
    ui->comboExpressiveCurve->setEnabled(false);
    ui->btnExpressiveCurveAdd->setEnabled(false);
    ui->btnExpressiveCurveDelete->setEnabled(false);
    ui->btnExpressiveCurveApply->setEnabled(false);

    // Configure note inspector spinboxes
    ui->spinNoteStart->setRange(0.0, 3600000.0);  // 0 to 1 hour in ms
    ui->spinNoteStart->setDecimals(1);
    ui->spinNoteStart->setSuffix(" ms");
    ui->spinNoteDuration->setRange(1.0, 60000.0);  // 1ms to 1 minute
    ui->spinNoteDuration->setDecimals(1);
    ui->spinNoteDuration->setSuffix(" ms");
    ui->spinNotePitch->setRange(20, 20000);  // 20 Hz to 20 kHz
    ui->spinNotePitch->setSuffix(" Hz");

    // Initialize note inspector as disabled (no selection)
    ui->spinNoteStart->setEnabled(false);
    ui->spinNoteDuration->setEnabled(false);
    ui->spinNotePitch->setEnabled(false);
    ui->noteVariationComboBox->setEnabled(false);

    // ========== Project Dirty Tracking ==========

    // Track note changes via undo stack (covers add, delete, move, resize, curve edits)
    connect(scoreCanvasWindow->getScoreCanvas()->getUndoStack(), &QUndoStack::indexChanged,
            this, &KalaMain::markProjectDirty);

    // Track track additions/removals
    connect(trackManager, &TrackManager::trackAdded, this, [this](Track*, int) {
        markProjectDirty();
    });
    connect(trackManager, &TrackManager::trackRemoved, this, [this](int) {
        markProjectDirty();
    });

    // Track composition settings changes (tempo, time signature)
    connect(scoreCanvasWindow, &ScoreCanvasWindow::compositionSettingsChanged,
            this, &KalaMain::markProjectDirty);

    // ========== Hook Up Remaining Menu Actions ==========

    // File > Exit
    connect(ui->actionExit_Alt_F4, &QAction::triggered, this, &QWidget::close);

    // Edit menu actions (context-sensitive based on active tab)
    connect(ui->actionUndo_Ctrl_Z, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 0) {  // Sound Engine tab
            Canvas *c = sounitBuilder->getCanvas();
            if (c) c->getUndoStack()->undo();
        } else if (ui->MainTab->currentIndex() == 1) {  // Composition tab
            scoreCanvasWindow->getScoreCanvas()->getUndoStack()->undo();
        }
    });
    connect(ui->actionRedo_Ctrl_Y, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 0) {  // Sound Engine tab
            Canvas *c = sounitBuilder->getCanvas();
            if (c) c->getUndoStack()->redo();
        } else if (ui->MainTab->currentIndex() == 1) {  // Composition tab
            scoreCanvasWindow->getScoreCanvas()->getUndoStack()->redo();
        }
    });
    connect(ui->actionCut_Ctrl_X, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->getScoreCanvas()->performCut();
        }
    });
    connect(ui->actionCopy_Ctrl_C, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->getScoreCanvas()->performCopy();
        }
    });
    connect(ui->actionPaste_Ctrl_V, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->getScoreCanvas()->performPaste();
        }
    });
    connect(ui->actionDelete_Del, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 0) {  // Sound Engine tab
            Canvas *c = sounitBuilder->getCanvas();
            if (c) c->deleteSelected();
        } else if (ui->MainTab->currentIndex() == 1) {  // Composition tab
            scoreCanvasWindow->getScoreCanvas()->performDelete();
        }
    });
    connect(ui->actionSelect_All_ctrl_A, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->getScoreCanvas()->performSelectAll();
        }
    });
    connect(ui->actionDeselect_Esc, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 0) {  // Sound Engine tab
            Canvas *c = sounitBuilder->getCanvas();
            if (c) c->clearSelection();
        } else if (ui->MainTab->currentIndex() == 1) {  // Composition tab
            scoreCanvasWindow->getScoreCanvas()->deselectAll();
        }
    });

    // Sound > Add Container actions
    connect(ui->actionHarmonic_Generator, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Harmonic Generator", QColor("#3498db"),
            {"purity", "drift", "digitWindowOffset"}, {"spectrum"});
    });
    connect(ui->actionSpectrum_To_Signal, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Spectrum to Signal", QColor("#3498db"),
            {"spectrumIn", "pitch", "normalize"}, {"signalOut"});
    });
    connect(ui->actionRolloff_Processor, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Rolloff Processor", QColor("#e67e22"),
            {"spectrumIn", "lowRolloff", "highRolloff", "crossover", "transition"}, {"spectrumOut"});
    });
    connect(ui->actionFormant_Body, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Formant Body", QColor("#e67e22"),
            {"signalIn", "f1Freq", "f2Freq", "f1Q", "f2Q", "directMix", "f1f2Balance"}, {"signalOut"});
    });
    connect(ui->actionBreth_Turbulance, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Breath Turbulence", QColor("#e67e22"),
            {"voiceIn", "noiseIn", "blend"}, {"signalOut"});
    });
    connect(ui->actionNoise_Color_Filter, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Noise Color Filter", QColor("#e67e22"),
            {"noiseIn", "color", "filterQ"}, {"noiseOut"});
    });
    connect(ui->actionPhysics_System, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Physics System", QColor("#27ae60"),
            {"targetValue", "mass", "springK", "damping", "impulse", "impulseAmount"}, {"currentValue"});
    });
    connect(ui->actionEasing_Applicator, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Easing Applicator", QColor("#27ae60"),
            {"startValue", "endValue", "progress", "easingSelect"}, {"easedValue"});
    });
    connect(ui->actionEnvelope_Engine, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Envelope Engine", QColor("#27ae60"),
            {"timeScale", "valueScale", "valueOffset"}, {"envelopeValue"});
    });
    connect(ui->actionDrift_Engine, &QAction::triggered, this, [this]() {
        sounitBuilder->onAddContainer("Drift Engine", QColor("#27ae60"),
            {"amount", "rate"}, {"driftOut"});
    });

    // Sound > Gate Processor (hidden/disabled)
    ui->actionGate_Processor->setEnabled(false);
    ui->actionGate_Processor->setVisible(false);

    // Sound > Delete Container
    connect(ui->actionDelete_Container_2, &QAction::triggered, this, [this]() {
        Canvas *c = sounitBuilder->getCanvas();
        if (c) c->deleteSelected();
    });

    // Compose > Scale Settings
    connect(ui->actionScale_Settings, &QAction::triggered, this, &KalaMain::onScaleEditClicked);

    // Compose > Assign Sounit to Track
    connect(ui->actionAssign_Sounit_to_Track, &QAction::triggered, this, &KalaMain::onLoadSounit);

    // View > Canvas Window
    connect(ui->actionCanvas_Window, &QAction::triggered, this, &KalaMain::toggleCanvasFocus);

    // View > Full Screen Canvas (F11)
    connect(ui->actionFull_Screen_Canvas_F11, &QAction::triggered, this, [this]() {
        QWidget *activeCanvas = nullptr;
        if (ui->MainTab->currentIndex() == 0 && sounitBuilder) {
            activeCanvas = sounitBuilder;
        } else if (ui->MainTab->currentIndex() == 1 && scoreCanvasWindow) {
            activeCanvas = scoreCanvasWindow;
        }
        if (activeCanvas) {
            if (activeCanvas->isMaximized()) {
                activeCanvas->showNormal();
            } else {
                activeCanvas->showMaximized();
            }
        }
    });
    ui->actionFull_Screen_Canvas_F11->setShortcut(Qt::Key_F11);
    ui->actionFull_Screen_Canvas_F11->setShortcutContext(Qt::ApplicationShortcut);

    // View > Zoom In/Out/Fit (Composition tab only)
    connect(ui->actionZoom_In_Ctrl, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->onZoomIn();
        }
    });
    connect(ui->actionZoom_Out_Crl, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->onZoomOut();
        }
    });
    connect(ui->actionZoom_to_fit_Ctrl_0, &QAction::triggered, this, [this]() {
        if (ui->MainTab->currentIndex() == 1) {
            scoreCanvasWindow->zoomToFit();
        }
    });
    ui->actionZoom_In_Ctrl->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
    ui->actionZoom_Out_Crl->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    ui->actionZoom_to_fit_Ctrl_0->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    ui->actionZoom_In_Ctrl->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionZoom_Out_Crl->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionZoom_to_fit_Ctrl_0->setShortcutContext(Qt::ApplicationShortcut);

    // View > Tab switching
    connect(ui->actionSound_Engine_Tab_Ctrl_1, &QAction::triggered, this, [this]() {
        ui->MainTab->setCurrentIndex(0);
    });
    connect(ui->actionCompisition_Tab_Ctrl_2, &QAction::triggered, this, [this]() {
        ui->MainTab->setCurrentIndex(1);
    });
    connect(ui->actionPreferences_Tab_Ctrl_3, &QAction::triggered, this, [this]() {
        ui->MainTab->setCurrentIndex(2);
    });
    ui->actionSound_Engine_Tab_Ctrl_1->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    ui->actionCompisition_Tab_Ctrl_2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    ui->actionPreferences_Tab_Ctrl_3->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_3));
    ui->actionSound_Engine_Tab_Ctrl_1->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionCompisition_Tab_Ctrl_2->setShortcutContext(Qt::ApplicationShortcut);
    ui->actionPreferences_Tab_Ctrl_3->setShortcutContext(Qt::ApplicationShortcut);

    // View > AI Companion toggle (Ctrl+Shift+A) — added programmatically
    // (The companion dock is created later in the constructor; the action is
    //  wired via a lambda that references m_companionDock after it's created.)
    QAction *actionCompanion = new QAction("AI Companion", this);
    actionCompanion->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
    actionCompanion->setShortcutContext(Qt::ApplicationShortcut);
    actionCompanion->setCheckable(true);
    ui->menuView->addSeparator();
    ui->menuView->addAction(actionCompanion);
    connect(actionCompanion, &QAction::triggered, this, [this, actionCompanion](bool checked) {
        if (m_companionDock) {
            m_companionDock->setVisible(checked);
            actionCompanion->setChecked(m_companionDock->isVisible());
        }
    });

    // Help menu
    connect(ui->actionKeyboard_Shortcuts, &QAction::triggered, this, &KalaMain::onKeyboardShortcuts);
    connect(ui->actionDocumentation_F1, &QAction::triggered, this, &KalaMain::onDocumentation);
    ui->actionDocumentation_F1->setShortcut(QKeySequence(Qt::Key_F1));
    ui->actionDocumentation_F1->setShortcutContext(Qt::ApplicationShortcut);

    // Initialize settings tab
    populateSettingsTab();

    // Show initial tab content
    if(ui->MainTab->currentIndex() == 0)  {
        sounitBuilder->show();
    }
    else if (ui->MainTab->currentIndex() == 1)  {
        scoreCanvasWindow->show();
    }

    // Install event filter to catch Tab key from all widgets
    qApp->installEventFilter(this);

    // ── AI Companion ─────────────────────────────────────────────────────────
    // Load persisted LLM config (falls back to Ollama defaults if never saved)
    {
        QSettings s;
        int providerInt = s.value("llm/provider", 0).toInt();
        m_llmConfig.provider  = (providerInt == 1) ? LLMProvider::Anthropic
                                                    : LLMProvider::OpenAICompatible;
        m_llmConfig.baseUrl   = s.value("llm/baseUrl",  "http://localhost:11434/v1").toString();
        m_llmConfig.apiKey    = s.value("llm/apiKey",   "ollama").toString();
        m_llmConfig.model     = s.value("llm/model",    "qwen3-coder:30b").toString();
        m_llmConfig.maxTokens = s.value("llm/maxTokens", 4096).toInt();
    }

    m_kalaTools    = new KalaTools(sounitBuilder, trackManager, scoreCanvasWindow, this);
    m_kalaTools->setMainWindow(this);
    m_kalaAgent    = new KalaAgent(m_llmConfig, m_kalaTools, this);
    m_companionPanel = new CompanionPanel(this);

    m_companionDock = new QDockWidget("AI Companion", this);
    m_companionDock->setWidget(m_companionPanel);
    m_companionDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    m_companionDock->setMinimumWidth(280);
    addDockWidget(Qt::RightDockWidgetArea, m_companionDock);
    m_companionDock->hide();  // Hidden by default — user opens via View menu

    // Wire panel → agent → panel
    connect(m_companionPanel, &CompanionPanel::userMessageSubmitted,
            m_kalaAgent,       &KalaAgent::sendUserMessage);
    connect(m_kalaAgent, &KalaAgent::messageReady,
            m_companionPanel,  &CompanionPanel::appendMessage);
    connect(m_kalaAgent, &KalaAgent::thinkingStarted,
            m_companionPanel,  [this]() { m_companionPanel->setThinking(true); });
    connect(m_kalaAgent, &KalaAgent::thinkingFinished,
            m_companionPanel,  [this]() { m_companionPanel->setThinking(false); });
    connect(m_companionPanel, &CompanionPanel::cancelRequested,
            m_kalaAgent,       &KalaAgent::cancel);
    connect(m_companionPanel, &CompanionPanel::clearRequested,
            m_kalaAgent,       &KalaAgent::clearHistory);

    // ── Initialize window title
    updateWindowTitle();
}

void KalaMain::onTabChanged(int index)
{
    if (index == 0) {  // Sound Engine tab
        if (scoreCanvasWindow) {
            // Save state before hiding
            m_scoreCanvasWasMaximized = scoreCanvasWindow->isMaximized();
            scoreCanvasWindow->hide();  // Hide other window (don't close - it destroys the window!)
        }
        if (sounitBuilder) {
            // Restore previous state
            if (m_sounitBuilderWasMaximized) {
                sounitBuilder->showMaximized();
            } else {
                sounitBuilder->show();
            }
            sounitBuilder->raise();
            sounitBuilder->activateWindow();
        }

        // NEW: With Track-based architecture, each Track has its own canvas
        // Just ensure SounitBuilder is showing the correct Track's canvas
        if (currentEditingTrack >= 0) {
            Track *track = trackManager->getTrack(currentEditingTrack);
            if (track) {
                // setTrackCanvas handles showing the right canvas
                sounitBuilder->setTrackCanvas(track);
                qDebug() << "Switched to Sound Engine tab - showing Track" << currentEditingTrack
                         << "canvas (" << track->getName() << ")";
            }
        }
    } else if (index == 1) {  // Composition tab
        // Check if current sounit has unsaved edits (skip in Note Mode - it manages its own state)
        if (currentEditingTrack >= 0 && !(sounitBuilder && sounitBuilder->isNoteModeActive())) {
            Track *track = trackManager->getTrack(currentEditingTrack);
            if (track && track->isSounitDirty()) {
                // Show warning dialog
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Unsaved Sounit Edits");
                msgBox.setText("You have unsaved edits to the sounit.");
                msgBox.setInformativeText(
                    "Notes using the base sounit (variation 0) will play with your "
                    "current edits, not the original sound.\n\n"
                    "What would you like to do?");

                QPushButton *createVarBtn = msgBox.addButton("Create Variation First", QMessageBox::ActionRole);
                QPushButton *reloadBtn = msgBox.addButton("Reload Original", QMessageBox::ActionRole);
                QPushButton *continueBtn = msgBox.addButton("Continue Anyway", QMessageBox::AcceptRole);
                msgBox.setDefaultButton(continueBtn);

                msgBox.exec();

                if (msgBox.clickedButton() == static_cast<QAbstractButton*>(createVarBtn)) {
                    // Switch back to Sound Engine tab and trigger variation creation
                    ui->MainTab->setCurrentIndex(0);
                    sounitBuilder->show();
                    sounitBuilder->onCreateVariation();
                    return;  // Don't switch to ScoreCanvas yet
                } else if (msgBox.clickedButton() == static_cast<QAbstractButton*>(reloadBtn)) {
                    // Reload the original sounit from file
                    QString filePath = track->getSounitFilePath();
                    if (!filePath.isEmpty() && QFile::exists(filePath)) {
                        if (track->loadSounit(filePath)) {
                            qDebug() << "Reloaded original sounit";
                            sounitBuilder->rebuildGraph(currentEditingTrack);
                        }
                    } else {
                        // No file to reload from - create new sounit
                        track->newSounit();
                        sounitBuilder->rebuildGraph(currentEditingTrack);
                        QMessageBox::information(this, "Reset to New",
                            "No original sounit file found. Reset to a new empty sounit.");
                    }
                }
                // Continue anyway - just proceed to ScoreCanvas
            }
        }

        if (sounitBuilder) {
            // Stop any active note-mode playback before hiding
            // (track notes may be temporarily replaced with a single play note)
            if (sounitBuilder->isNoteModeActive()) {
                sounitBuilder->stopPlayback();
            }
            // Save state before hiding
            m_sounitBuilderWasMaximized = sounitBuilder->isMaximized();
            sounitBuilder->hide();  // Hide, don't close
        }
        if (scoreCanvasWindow) {
            // Restore previous state
            if (m_scoreCanvasWasMaximized) {
                scoreCanvasWindow->showMaximized();
            } else {
                scoreCanvasWindow->show();
            }
            scoreCanvasWindow->raise();
            scoreCanvasWindow->activateWindow();
        }
    }
    // index == 2 (Preferences) — leave as-is
}

void KalaMain::stopAllPlayback()
{
    if (sounitBuilder) {
        sounitBuilder->stopPlayback();
    }
    if (scoreCanvasWindow) {
        scoreCanvasWindow->stopPlayback();
    }
}

bool KalaMain::loadProjectFile(const QString &filePath)
{
    return loadProject(filePath);
}

bool KalaMain::saveProjectFile(const QString &filePath)
{
    return saveProject(filePath);
}

void KalaMain::onContainerSelected(Container *container)
{
    if (!container) return;

    //  Store current container
    currentContainer = container;

    //  Show container inspector, hide connection inspector
    ui->groupContainerInspector->setVisible(true);
    ui->groupConnectionInspector->setVisible(false);

    // Enable container inspector controls
    ui->editInstanceName->setEnabled(true);
    // Populate container inspector
    ui->editInstanceName->setText(container->getInstanceName());
    ui->labelContainerType->setText(container->getName());

    // Set description
    ui->labelContainerDescription->setText(getContainerDescription(container->getName()));

    // Populate inputs list
    ui->listWidgetInputs->clear();
    for (const QString &input : container->getInputPorts()) {
        ui->listWidgetInputs->addItem(input);
    }

    // Populate outputs list
    ui->listWidgetOutputs->clear();
    for (const QString &output : container->getOutputPorts()) {
        ui->listWidgetOutputs->addItem(output);
    }

    // Clear previous config controls and populate container-specific inspector
    clearConfigInspector();

    // Create container-specific inspector controls
    if (container->getName() == "Harmonic Generator") {
        populateHarmonicGeneratorInspector();
    } else if (container->getName() == "Rolloff Processor") {
        populateRolloffProcessorInspector();
    } else if (container->getName() == "Spectrum to Signal") {
        populateSpectrumToSignalInspector();
    } else if (container->getName() == "Formant Body") {
        populateFormantBodyInspector();
    } else if (container->getName() == "Breath Turbulence") {
        populateBreathTurbulenceInspector();
    } else if (container->getName() == "Noise Color Filter") {
        populateNoiseColorFilterInspector();
    } else if (container->getName() == "Physics System") {
        populatePhysicsSystemInspector();
    } else if (container->getName() == "Envelope Engine") {
        populateEnvelopeEngineInspector();
    } else if (container->getName() == "Drift Engine") {
        populateDriftEngineInspector();
    } else if (container->getName() == "Gate Processor") {
        populateGateProcessorInspector();
    } else if (container->getName() == "Easing Applicator") {
        populateEasingApplicatorInspector();
    } else if (container->getName() == "Karplus Strong") {
        populateKarplusStrongInspector();
    } else if (container->getName() == "Attack") {
        populateAttackInspector();
    } else if (container->getName() == "LFO") {
        populateLFOInspector();
    } else if (container->getName() == "Frequency Mapper") {
        populateFrequencyMapperInspector();
    } else if (container->getName() == "Signal Mixer") {
        populateSignalMixerInspector();
    } else if (container->getName() == "Spectrum Blender") {
        populateSpectrumBlenderInspector();
    } else if (container->getName() == "Wavetable Synth") {
        populateWavetableSynthInspector();
    } else if (container->getName() == "10-Band EQ") {
        populateBandpassEQInspector();
    } else if (container->getName() == "Comb Filter") {
        populateCombFilterInspector();
    } else if (container->getName() == "LP/HP Filter") {
        populateLowHighPassFilterInspector();
    } else if (container->getName() == "IR Convolution") {
        populateIRConvolutionInspector();
    } else if (container->getName() == "Note Tail") {
        populateNoteTailInspector();
    } else if (container->getName() == "Recorder") {
        populateRecorderInspector();
    } else if (container->getName() == "Bowed") {
        populateBowedInspector();
    } else if (container->getName() == "Saxophone" || container->getName() == "Reed") {
        populateReedInspector();
    }
}

void KalaMain::onConnectionSelected(int connectionIndex)
{
    Canvas *canvas = sounitBuilder->getCanvas();
    Canvas::Connection *conn = canvas->getSelectedConnection();
    if (!conn) return;

    // Hide container inspector, show connection inspector
    ui->groupContainerInspector->setVisible(false);
    ui->groupConnectionInspector->setVisible(true);

    // Enable connection inspector controls
    ui->comboConnectionFunction->setEnabled(true);
    ui->spinConnectionWeight->setEnabled(true);
    ui->buttonDisconnect->setEnabled(true);

    // Populate connection inspector
    QString fromText = QString("%1.%2").arg(conn->fromContainer->getInstanceName(), conn->fromPort);
    QString toText = QString("%1.%2").arg(conn->toContainer->getInstanceName(), conn->toPort);

    ui->labelConnectionFrom->setText(fromText);
    ui->labelConnectionTo->setText(toText);

    // Set function combo box (block signals to avoid triggering change event)
    ui->comboConnectionFunction->blockSignals(true);
    int functionIndex = ui->comboConnectionFunction->findText(conn->function);
    if (functionIndex >= 0) {
        ui->comboConnectionFunction->setCurrentIndex(functionIndex);
    }
    ui->comboConnectionFunction->blockSignals(false);

    // Set weight (block signals to avoid triggering change event)
    ui->spinConnectionWeight->blockSignals(true);
    ui->spinConnectionWeight->setValue(conn->weight);
    ui->spinConnectionWeight->blockSignals(false);
}

void KalaMain::onSelectionCleared()
{
    // Clear current container reference
    currentContainer = nullptr;

    // Hide both inspectors when nothing is selected
    ui->groupContainerInspector->setVisible(false);
    ui->groupConnectionInspector->setVisible(false);

    // Disable connection inspector controls
    ui->comboConnectionFunction->setEnabled(false);
    ui->spinConnectionWeight->setEnabled(false);
    ui->buttonDisconnect->setEnabled(false);

    // Clear fields
    ui->editInstanceName->clear();
    ui->labelContainerType->clear();
    ui->labelContainerDescription->clear();
    ui->listWidgetInputs->clear();
    ui->listWidgetOutputs->clear();
    clearConfigInspector();
    ui->labelConnectionFrom->clear();
    ui->labelConnectionTo->clear();
}

void KalaMain::onInstanceNameChanged()
{
    Canvas *canvas = sounitBuilder->getCanvas();
    if (canvas->selectedContainer) {
        canvas->selectedContainer->setInstanceName(ui->editInstanceName->text());
    }
}

void KalaMain::onConnectionFunctionChanged(int index)
{
    Canvas *canvas = sounitBuilder->getCanvas();
    Canvas::Connection *conn = canvas->getSelectedConnection();
    if (conn) {
        conn->function = ui->comboConnectionFunction->currentText();
        qDebug() << "Connection function changed to:" << conn->function;

        // Trigger graph rebuild to apply new connection function
        emit canvas->graphChanged();
    }
}

void KalaMain::onConnectionWeightChanged(double value)
{
    Canvas *canvas = sounitBuilder->getCanvas();
    Canvas::Connection *conn = canvas->getSelectedConnection();
    if (conn) {
        conn->weight = value;
        qDebug() << "Connection weight changed to:" << conn->weight;

        // Trigger graph rebuild to apply new connection weight
        emit canvas->graphChanged();
    }
}

void KalaMain::onDisconnectClicked()
{
    Canvas *canvas = sounitBuilder->getCanvas();
    Canvas::Connection *conn = canvas->getSelectedConnection();
    if (conn) {
        // Find and remove the connection
        QVector<Canvas::Connection> &connections = canvas->getConnections();
        for (int i = 0; i < connections.size(); ++i) {
            Canvas::Connection &c = connections[i];
            if (&c == conn) {
                connections.removeAt(i);
                canvas->clearSelection();
                canvas->update();
                qDebug() << "Connection disconnected";

                // Trigger graph rebuild to apply connection removal
                emit canvas->graphChanged();
                break;
            }
        }
    }
}

void KalaMain::onConfigParameterChanged(QListWidgetItem *item)
{
    // This method is deprecated - now using container-specific inspectors
    // Kept for compatibility but does nothing
}

// Helper method to clear all dynamic inspector controls
void KalaMain::clearConfigInspector()
{
    // Get the layout from the scroll area contents
    QLayout *layout = ui->scrollAreaConfigContents->layout();
    if (!layout) return;

    // Delete all widgets in the layout
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Disconnect container→preview connection before destroying widgets
    QObject::disconnect(m_envelopePreviewConnection);

    // Clear member variable pointers
    comboDnaSelect = nullptr;
    sliderNumHarmonics = nullptr;
    spinNumHarmonics = nullptr;
    spectrumViz = nullptr;
    dnaNameLabel = nullptr;
    comboEnvelopeSelect = nullptr;
    comboScoreCurve = nullptr;
    envelopeParamsWidget = nullptr;
    envelopeViz = nullptr;
    envelopeNameLabel = nullptr;
    checkPadEnabled = nullptr;
    padParamsWidget = nullptr;
}

// Get description text for each container type
QString KalaMain::getContainerDescription(const QString &containerType)
{
    if (containerType == "Harmonic Generator") {
        return "Generates harmonic spectrum using additive synthesis. "
               "Creates rich, controllable timbres from fundamental frequency and harmonic series.";
    } else if (containerType == "Spectrum to Signal") {
        return "Converts harmonic spectrum to audio signal using additive synthesis.";
    } else if (containerType == "Rolloff Processor") {
        return "Applies rolloff curve to attenuate higher harmonics, shaping spectral brightness.";
    } else if (containerType == "Formant Body") {
        return "Resonant filter pair simulating vocal tract formants for vocal-like timbres.";
    } else if (containerType == "Breath Turbulence") {
        return "Blends voice signal with noise for breathiness and air texture.";
    } else if (containerType == "Noise Color Filter") {
        return "Generates and filters colored noise with adjustable spectral characteristics.";
    } else if (containerType == "Physics System") {
        return "Spring-mass-damper system for natural, physics-based parameter evolution.";
    } else if (containerType == "Easing Applicator") {
        return "Applies easing curves to interpolate smoothly between parameter values.";
    } else if (containerType == "Envelope Engine") {
        return "Multi-stage envelope generator for shaping parameter evolution over note lifetime.";
    } else if (containerType == "Drift Engine") {
        return "Generates subtle random variations for organic, living sound textures.";
    } else if (containerType == "Gate Processor") {
        return "Note gate with velocity-sensitive attack/release envelopes.";
    } else if (containerType == "Signal Mixer") {
        return "Mixes two audio signals with independent gain controls for each input.";
    } else if (containerType == "Spectrum Blender") {
        return "Crossfades between two harmonic spectra using a blend position parameter.";
    } else if (containerType == "Attack") {
        return "Attack transient generator with six excitation types: "
               "air jet, reed chirp, sub bloom, lip bend, FM strike, and membrane sweep.";
    } else if (containerType == "Wavetable Synth") {
        return "Wavetable oscillator with built-in presets and WAV import. "
               "Outputs both direct signal and FFT spectrum for the shaping pipeline.";
    } else if (containerType == "10-Band EQ") {
        return "10-band parametric equalizer with ISO center frequencies (31 Hz to 16 kHz). "
               "Each band has independent gain and Q controls. Band gains are modulatable via input ports.";
    } else if (containerType == "Comb Filter") {
        return "Comb filter with feedforward (FIR, notches) and feedback (IIR, resonant peaks) modes. "
               "Delay time, feedback, and damping are modulatable for flanging and pitched comb effects.";
    } else if (containerType == "LP/HP Filter") {
        return "Biquad low-pass / high-pass filter. Mode selector switches between LP and HP. "
               "Cutoff and resonance (Q) are modulatable via input ports for sweeping filter effects.";
    } else if (containerType == "IR Convolution") {
        return "Convolves the input signal with a loaded IR WAV file or a live graph signal. "
               "Connect any signal to irIn to use it as the IR for the next note "
               "(one-note lag: irIn captured during note N becomes the room for note N+1). "
               "Capture Length controls how many seconds of irIn are used as the IR. "
               "Pre-delay adds temporal separation. Decay trim shortens the IR tail. "
               "High damp applies a lowpass filter for warmer reverb.";
    } else if (containerType == "Note Tail") {
        return "Extends note rendering with a smooth cosine fade-out after the note ends, "
               "preventing click artifacts caused by abrupt signal cutoffs. "
               "Place at the end of the signal chain. Set tail length in milliseconds.";
    }
    return "";
}

// Create Harmonic Generator-specific inspector controls
void KalaMain::populateHarmonicGeneratorInspector()
{
    if (!currentContainer) return;

    // Get the existing layout from the scroll area contents
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) {
        qDebug() << "Warning: scrollAreaConfigContents has no layout!";
        return;
    }

    // Create a widget to hold the form layout
    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // DNA Select dropdown
    QLabel *labelDna = new QLabel("DNA Select:");
    comboDnaSelect = new QComboBox();
    comboDnaSelect->addItem("Vocal (Bright)", 0);
    comboDnaSelect->addItem("Vocal (Warm)", 1);
    comboDnaSelect->addItem("Brass (Trumpet-like)", 2);
    comboDnaSelect->addItem("Reed (Clarinet-like)", 3);
    comboDnaSelect->addItem("String (Violin-like)", 4);
    comboDnaSelect->addItem("Pure Sine (Test)", 5);
    comboDnaSelect->addItem("Custom...", -1);

    // Set current value (block signals to prevent triggering dialog on initial load)
    int dnaValue = static_cast<int>(currentContainer->getParameter("dnaSelect", 0.0));
    int index = comboDnaSelect->findData(dnaValue);
    if (index >= 0) {
        comboDnaSelect->blockSignals(true);
        comboDnaSelect->setCurrentIndex(index);
        comboDnaSelect->blockSignals(false);
    }

    // Use 'activated' instead of 'currentIndexChanged' so clicking Custom again opens the dialog
    connect(comboDnaSelect, QOverload<int>::of(&QComboBox::activated),
            this, &KalaMain::onDnaSelectChanged);
    formLayout->addRow(labelDna, comboDnaSelect);

    // numHarmonics slider + spinbox
    QLabel *labelNumHarmonics = new QLabel("Num Harmonics:");
    QHBoxLayout *layoutNumHarmonics = new QHBoxLayout();
    sliderNumHarmonics = new QSlider(Qt::Horizontal);
    int maxHarmonics = ContainerSettings::instance().harmonicGenerator.maxHarmonics;
    sliderNumHarmonics->setRange(1, maxHarmonics);
    sliderNumHarmonics->setValue(static_cast<int>(currentContainer->getParameter("numHarmonics", 64.0)));
    spinNumHarmonics = new QSpinBox();
    spinNumHarmonics->setRange(1, maxHarmonics);
    spinNumHarmonics->setValue(sliderNumHarmonics->value());
    layoutNumHarmonics->addWidget(sliderNumHarmonics);
    layoutNumHarmonics->addWidget(spinNumHarmonics);

    connect(sliderNumHarmonics, &QSlider::valueChanged, spinNumHarmonics, &QSpinBox::setValue);
    connect(spinNumHarmonics, QOverload<int>::of(&QSpinBox::valueChanged),
            sliderNumHarmonics, &QSlider::setValue);
    connect(spinNumHarmonics, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KalaMain::onNumHarmonicsChanged);
    formLayout->addRow(labelNumHarmonics, layoutNumHarmonics);

    // Digit window offset (visible only when a digit formula is loaded)
    if (!currentContainer->getDigitString().isEmpty()) {
        QLabel *labelOffset = new QLabel("Window Offset:");
        QSpinBox *spinOffset = new QSpinBox();
        spinOffset->setRange(0, 9999);
        spinOffset->setValue(static_cast<int>(currentContainer->getParameter("digitWindowOffset", 0.0)));
        spinOffset->setToolTip("Base digit offset into the formula's fractional expansion.\n"
                               "Connect a curve to the digitWindowOffset port to animate this.");
        connect(spinOffset, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int value) {
                    if (!currentContainer) return;
                    currentContainer->setParameter("digitWindowOffset", static_cast<double>(value));
                });
        formLayout->addRow(labelOffset, spinOffset);
    }

    // Add the form widget to the main layout
    mainLayout->addWidget(formWidget);

    // Add spectrum visualizer
    QLabel *labelSpectrum = new QLabel("Spectrum Preview:");
    labelSpectrum->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    mainLayout->addWidget(labelSpectrum);

    spectrumViz = new SpectrumVisualizer();
    int numHarmonics = static_cast<int>(currentContainer->getParameter("numHarmonics", 64.0));

    // Check if this is a custom DNA pattern (dnaValue == -1)
    if (dnaValue == -1) {
        // Load custom DNA pattern from container
        int customDnaCount = static_cast<int>(currentContainer->getParameter("customDnaCount", 0.0));
        if (customDnaCount > 0) {
            QVector<double> customAmplitudes;
            for (int i = 0; i < customDnaCount; i++) {
                QString paramName = QString("customDna_%1").arg(i);
                double amp = currentContainer->getParameter(paramName, 0.0);
                customAmplitudes.append(amp);
            }
            spectrumViz->setSpectrum(customAmplitudes);
            qDebug() << "Loaded custom DNA pattern with" << customDnaCount << "harmonics for display";
        } else {
            // No custom pattern stored, show default rolloff
            spectrumViz->setDnaPreset(-1, numHarmonics, 0.0);
        }
    } else {
        // Using preset DNA
        spectrumViz->setDnaPreset(dnaValue, numHarmonics, 0.0);
    }

    mainLayout->addWidget(spectrumViz);

    dnaNameLabel = new QLabel();
    dnaNameLabel->setAlignment(Qt::AlignCenter);
    dnaNameLabel->setStyleSheet("color: #888; font-style: italic;");
    mainLayout->addWidget(dnaNameLabel);

    // Set initial DNA name
    if (dnaValue == -1) {
        QString name = currentContainer->getCustomDnaName();
        dnaNameLabel->setText(name.isEmpty() ? "Custom" : name);
    } else if (comboDnaSelect) {
        dnaNameLabel->setText(comboDnaSelect->currentText());
    }

    // --- PADsynth section ---
    QFrame *padSeparator = new QFrame();
    padSeparator->setFrameShape(QFrame::HLine);
    padSeparator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(padSeparator);

    QLabel *padTitle = new QLabel("PADsynth");
    padTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
    mainLayout->addWidget(padTitle);

    bool padOn = currentContainer->getParameter("padEnabled", 0.0) > 0.5;

    checkPadEnabled = new QCheckBox("Enable PADsynth");
    checkPadEnabled->setChecked(padOn);
    mainLayout->addWidget(checkPadEnabled);

    // PAD parameters widget (hidden when PAD is off)
    padParamsWidget = new QWidget();
    QFormLayout *padLayout = new QFormLayout(padParamsWidget);
    padLayout->setSpacing(8);

    const auto &hgSettings = ContainerSettings::instance().harmonicGenerator;
    addParameterSlider(padLayout, "Bandwidth (cents)", "padBandwidth", hgSettings.padBandwidthMin, hgSettings.padBandwidthMax, 40.0, 1.0, 0);
    addParameterSlider(padLayout, "Bandwidth Scale", "padBandwidthScale", hgSettings.padBandwidthScaleMin, hgSettings.padBandwidthScaleMax, 1.0, 0.01, 2);

    // Profile shape dropdown
    QComboBox *comboProfile = new QComboBox();
    comboProfile->addItem("Gaussian", 0);
    comboProfile->addItem("Raised Cosine", 1);
    comboProfile->addItem("Square", 2);
    int profileVal = static_cast<int>(currentContainer->getParameter("padProfileShape", 0.0));
    comboProfile->setCurrentIndex(std::clamp(profileVal, 0, 2));
    connect(comboProfile, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("padProfileShape", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    padLayout->addRow("Profile Shape:", comboProfile);

    // FFT Size dropdown
    QComboBox *comboFftSize = new QComboBox();
    comboFftSize->addItem("65536 (64K)", 65536);
    comboFftSize->addItem("131072 (128K)", 131072);
    comboFftSize->addItem("262144 (256K)", 262144);
    comboFftSize->addItem("524288 (512K)", 524288);
    comboFftSize->addItem("1048576 (1M)", 1048576);
    int fftVal = static_cast<int>(currentContainer->getParameter("padFftSize", 262144.0));
    int fftIndex = comboFftSize->findData(fftVal);
    if (fftIndex >= 0) comboFftSize->setCurrentIndex(fftIndex);
    else comboFftSize->setCurrentIndex(2); // default 256K
    connect(comboFftSize, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, comboFftSize](int index) {
        if (currentContainer) {
            int size = comboFftSize->itemData(index).toInt();
            currentContainer->setParameter("padFftSize", static_cast<double>(size));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    padLayout->addRow("FFT Size:", comboFftSize);

    padParamsWidget->setVisible(padOn);
    mainLayout->addWidget(padParamsWidget);

    connect(checkPadEnabled, &QCheckBox::toggled, this, [this](bool checked) {
        if (currentContainer) {
            currentContainer->setParameter("padEnabled", checked ? 1.0 : 0.0);
            if (padParamsWidget) padParamsWidget->setVisible(checked);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    mainLayout->addStretch();  // Push controls to top

    // CRITICAL: Initialize any unset parameters to their UI defaults
    // This ensures newly created containers have proper default values
    bool needsRebuild = false;
    if (currentContainer->getParameter("dnaSelect", -999.0) == -999.0) {
        currentContainer->setParameter("dnaSelect", 0.0);  // Default to Vocal (Bright)
        needsRebuild = true;
    }
    if (currentContainer->getParameter("numHarmonics", -999.0) == -999.0) {
        currentContainer->setParameter("numHarmonics", 64.0);
        needsRebuild = true;
    }

    // Rebuild graph to apply defaults if any were set
    if (needsRebuild) {
        int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
        sounitBuilder->rebuildGraph(trackIndex);
        qDebug() << "Applied default parameters to new Harmonic Generator for track" << trackIndex;
    }
}

// Helper to update spectrum visualization
void KalaMain::updateSpectrumVisualization()
{
    if (!spectrumViz || !currentContainer) return;

    int numHarmonics = static_cast<int>(currentContainer->getParameter("numHarmonics", 64.0));
    int dnaValue = static_cast<int>(currentContainer->getParameter("dnaSelect", 0.0));

    // If custom DNA, load and resize the custom pattern
    if (dnaValue == -1) {
        int customDnaCount = static_cast<int>(currentContainer->getParameter("customDnaCount", 0.0));
        if (customDnaCount > 0) {
            QVector<double> customAmplitudes;
            for (int i = 0; i < customDnaCount; i++) {
                QString paramName = QString("customDna_%1").arg(i);
                double amp = currentContainer->getParameter(paramName, 0.0);
                customAmplitudes.append(amp);
            }

            // Resize the pattern to match new numHarmonics
            if (customAmplitudes.size() < numHarmonics) {
                // Add zeros at the end
                customAmplitudes.resize(numHarmonics, 0.0);
            } else if (customAmplitudes.size() > numHarmonics) {
                // Truncate
                customAmplitudes.resize(numHarmonics);
            }

            spectrumViz->setSpectrum(customAmplitudes);
        } else {
            // No custom pattern stored, use rolloff
            spectrumViz->setDnaPreset(-1, numHarmonics, 0.0);
        }
    } else {
        // Using preset DNA
        spectrumViz->setDnaPreset(dnaValue, numHarmonics, 0.0);
    }
}

// Harmonic Generator parameter change slots
void KalaMain::onDnaSelectChanged(int index)
{
    if (!currentContainer) return;

    int dnaValue = comboDnaSelect->itemData(index).toInt();

    if (dnaValue == -1) {
        // Custom option selected - open DNA editor dialog
        int numHarmonics = static_cast<int>(currentContainer->getParameter("numHarmonics", 64.0));
        DnaEditorDialog dialog(numHarmonics, this);

        // If there's an existing custom pattern, load it into the dialog
        int customDnaCount = static_cast<int>(currentContainer->getParameter("customDnaCount", 0.0));
        if (customDnaCount > 0) {
            QVector<double> existingPattern;
            for (int i = 0; i < customDnaCount; i++) {
                QString paramName = QString("customDna_%1").arg(i);
                double amp = currentContainer->getParameter(paramName, 0.0);
                existingPattern.append(amp);
            }
            // Load the existing pattern into the dialog
            dialog.setCustomPattern(existingPattern);
            qDebug() << "Loaded existing custom DNA pattern with" << customDnaCount << "harmonics into editor";
        }

        if (dialog.exec() == QDialog::Accepted) {
            // User clicked OK - get the custom DNA pattern
            QVector<double> customDna = dialog.getCustomDnaPattern();
            qDebug() << "Custom DNA created with" << customDna.size() << "harmonics";

            // Update spectrum visualization with custom pattern
            if (spectrumViz) {
                spectrumViz->setSpectrum(customDna);
            }

            // Batch parameter updates to avoid multiple graph rebuilds
            currentContainer->beginParameterUpdate();

            // Store dnaSelect = -1 to indicate custom mode
            currentContainer->setParameter("dnaSelect", -1.0);

            // Store custom DNA pattern - using individual parameters for each harmonic
            // (In the future, we could add QMap<QString, QString> to Container for JSON storage)
            for (int i = 0; i < customDna.size(); i++) {
                QString paramName = QString("customDna_%1").arg(i);
                currentContainer->setParameter(paramName, customDna[i]);
            }
            // Store the count
            currentContainer->setParameter("customDnaCount", static_cast<double>(customDna.size()));

            // End batch update - this triggers a single graph rebuild
            currentContainer->endParameterUpdate();

            // If this was a digit formula, store the digit string and base offset
            // so the digitWindowOffset port can modulate it live during rendering
            if (dialog.isDigitFormulaMode()) {
                QString digits = dialog.getComputedDigitString(1000);
                currentContainer->setDigitString(digits);
                currentContainer->setParameter("digitWindowOffset",
                                               static_cast<double>(dialog.getFormulaStartDigit()));
            } else {
                currentContainer->setDigitString({});  // clear any previous digit formula
            }

            // Store custom DNA name (filename if loaded from file)
            QString dnaName = dialog.getLoadedName();
            if (dnaName.isEmpty()) dnaName = dialog.isDigitFormulaMode() ? "Digit Formula" : "Custom";
            currentContainer->setCustomDnaName(dnaName);
            if (dnaNameLabel) dnaNameLabel->setText(dnaName);

            qDebug() << "DNA Select changed to: Custom (-1) with" << customDna.size() << "harmonics stored";

            // CRITICAL: Update the spectrum visualizer in the config section to show the new pattern
            if (spectrumViz) {
                spectrumViz->setSpectrum(customDna);
            }
        } else {
            // User cancelled - revert to previous selection
            // Find the current dnaSelect value and set combo box back to it
            int currentDna = static_cast<int>(currentContainer->getParameter("dnaSelect", 0.0));
            int comboIndex = comboDnaSelect->findData(currentDna);
            if (comboIndex >= 0) {
                comboDnaSelect->blockSignals(true);
                comboDnaSelect->setCurrentIndex(comboIndex);
                comboDnaSelect->blockSignals(false);
            }
        }
        return;
    }

    currentContainer->setParameter("dnaSelect", static_cast<double>(dnaValue));
    updateSpectrumVisualization();  // Update spectrum preview
    if (dnaNameLabel && comboDnaSelect) {
        dnaNameLabel->setText(comboDnaSelect->currentText());
    }
    int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
    sounitBuilder->rebuildGraph(trackIndex);  // Rebuild graph to apply changes
    qDebug() << "DNA Select changed to:" << dnaValue << "for track" << trackIndex;

    // Auto re-render notes so changes are heard immediately on next playback
    if (scoreCanvasWindow) {
        scoreCanvasWindow->prerenderNotes();
        qDebug() << "Auto re-rendered notes with new DNA for track" << trackIndex;
    }
}

void KalaMain::onNumHarmonicsChanged(int value)
{
    if (!currentContainer) return;

    // If custom DNA is loaded, resize the custom pattern
    int dnaValue = static_cast<int>(currentContainer->getParameter("dnaSelect", 0.0));
    if (dnaValue == -1) {
        int customDnaCount = static_cast<int>(currentContainer->getParameter("customDnaCount", 0.0));
        if (customDnaCount > 0) {
            QVector<double> customAmplitudes;
            for (int i = 0; i < customDnaCount; i++) {
                QString paramName = QString("customDna_%1").arg(i);
                double amp = currentContainer->getParameter(paramName, 0.0);
                customAmplitudes.append(amp);
            }

            // Resize the pattern
            if (customAmplitudes.size() < value) {
                // Add zeros at the end
                customAmplitudes.resize(value, 0.0);
            } else if (customAmplitudes.size() > value) {
                // Truncate
                customAmplitudes.resize(value);
            }

            // Batch parameter updates to avoid multiple graph rebuilds
            currentContainer->beginParameterUpdate();

            // Store the resized pattern back
            for (int i = 0; i < customAmplitudes.size(); i++) {
                QString paramName = QString("customDna_%1").arg(i);
                currentContainer->setParameter(paramName, customAmplitudes[i]);
            }
            currentContainer->setParameter("customDnaCount", static_cast<double>(customAmplitudes.size()));

            // End batch update
            currentContainer->endParameterUpdate();

            qDebug() << "Resized custom DNA pattern to" << value << "harmonics";
        }
    }

    currentContainer->setParameter("numHarmonics", static_cast<double>(value));
    updateSpectrumVisualization();
    qDebug() << "Num Harmonics changed to:" << value;
}

// Helper function to add a parameter slider to the inspector
void KalaMain::addParameterSlider(QFormLayout *layout, const QString &label,
                                     const QString &paramName, double minVal, double maxVal,
                                     double defaultVal, double step, int decimals)
{
    if (!currentContainer) return;

    QHBoxLayout *paramLayout = new QHBoxLayout();
    QSlider *slider = new QSlider(Qt::Horizontal);
    QDoubleSpinBox *spinBox = new QDoubleSpinBox();

    // Calculate slider range (map double range to int)
    int sliderMin = static_cast<int>(minVal / step);
    int sliderMax = static_cast<int>(maxVal / step);
    int sliderVal = static_cast<int>(currentContainer->getParameter(paramName, defaultVal) / step);

    slider->setRange(sliderMin, sliderMax);
    slider->setValue(sliderVal);

    spinBox->setRange(minVal, maxVal);
    spinBox->setSingleStep(step);
    spinBox->setDecimals(decimals);
    spinBox->setValue(currentContainer->getParameter(paramName, defaultVal));

    paramLayout->addWidget(slider);
    paramLayout->addWidget(spinBox);

    // Connect slider and spinbox
    connect(slider, &QSlider::valueChanged, this, [this, spinBox, slider, paramName, step](int value) {
        double doubleVal = value * step;
        spinBox->blockSignals(true);
        spinBox->setValue(doubleVal);
        spinBox->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter(paramName, doubleVal);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, slider, paramName, step](double value) {
        slider->blockSignals(true);
        slider->setValue(static_cast<int>(value / step));
        slider->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter(paramName, value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    layout->addRow(label + ":", paramLayout);
}

// Rolloff Processor Inspector
void KalaMain::populateRolloffProcessorInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &rolloffSettings = ContainerSettings::instance().rolloffProcessor;
    addParameterSlider(formLayout, "Low Rolloff", "lowRolloff",
                       rolloffSettings.lowRolloffMin, rolloffSettings.lowRolloffMax, 0.3, 0.01, 2);
    addParameterSlider(formLayout, "High Rolloff", "highRolloff",
                       rolloffSettings.highRolloffMin, rolloffSettings.highRolloffMax, 1.0, 0.01, 2);
    addParameterSlider(formLayout, "Crossover Harmonic", "crossover",
                       rolloffSettings.crossoverMin, rolloffSettings.crossoverMax, 8.0, 1.0, 0);
    addParameterSlider(formLayout, "Transition Width", "transition",
                       rolloffSettings.transitionMin, rolloffSettings.transitionMax, 4.0, 1.0, 0);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Spectrum to Signal Inspector
void KalaMain::populateSpectrumToSignalInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    addParameterSlider(formLayout, "Normalize", "normalize", 0.0, 1.0, 1.0, 0.01, 2);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescStoS = new QLabel("1.0 = note pitch · 2.0 = octave up · 0.5 = octave down");
    pitchMultDescStoS->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescStoS);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Formant Body Inspector
void KalaMain::populateFormantBodyInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &formantSettings = ContainerSettings::instance().formantBody;
    addParameterSlider(formLayout, "F1 Frequency (Hz)", "f1Freq",
                       formantSettings.f1FreqMin, formantSettings.f1FreqMax, 500.0, 1.0, 0);
    addParameterSlider(formLayout, "F2 Frequency (Hz)", "f2Freq",
                       formantSettings.f2FreqMin, formantSettings.f2FreqMax, 1500.0, 1.0, 0);
    addParameterSlider(formLayout, "F1 Q", "f1Q",
                       formantSettings.qMin, formantSettings.qMax, 8.0, 0.1, 1);
    addParameterSlider(formLayout, "F2 Q", "f2Q",
                       formantSettings.qMin, formantSettings.qMax, 10.0, 0.1, 1);
    addParameterSlider(formLayout, "Direct Mix", "directMix",
                       formantSettings.mixMin, formantSettings.mixMax, 0.3, 0.01, 2);
    addParameterSlider(formLayout, "F1/F2 Balance", "f1f2Balance",
                       formantSettings.mixMin, formantSettings.mixMax, 0.6, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Breath Turbulence Inspector
void KalaMain::populateBreathTurbulenceInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    addParameterSlider(formLayout, "Blend", "blend", 0.0, 1.0, 0.10, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Noise Color Filter Inspector
void KalaMain::populateNoiseColorFilterInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // Noise Type dropdown
    QComboBox *comboNoiseType = new QComboBox();
    comboNoiseType->addItem("White Noise", 0);
    comboNoiseType->addItem("Pink Noise", 1);
    comboNoiseType->addItem("Brown Noise", 2);
    int noiseType = static_cast<int>(currentContainer->getParameter("noiseType", 0.0));
    comboNoiseType->setCurrentIndex(noiseType);
    connect(comboNoiseType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("noiseType", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Noise Type:", comboNoiseType);

    // Add note about noise type
    QLabel *noteLabel = new QLabel("(Only used when audioIn not connected)");
    noteLabel->setStyleSheet("color: gray; font-size: 9pt;");
    formLayout->addRow("", noteLabel);

    // Filter Type dropdown
    QComboBox *comboFilterType = new QComboBox();
    comboFilterType->addItem("Lowpass", 0);
    comboFilterType->addItem("Highpass", 1);
    comboFilterType->addItem("Bandpass", 2);
    int filterTypeVal = static_cast<int>(currentContainer->getParameter("filterType", 1.0));
    comboFilterType->setCurrentIndex(filterTypeVal);
    connect(comboFilterType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("filterType", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Filter Type:", comboFilterType);

    const auto &noiseSettings = ContainerSettings::instance().noiseColorFilter;
    addParameterSlider(formLayout, "Color (Hz)", "color",
                       noiseSettings.colorMin, noiseSettings.colorMax, 2000.0, 10.0, 0);
    addParameterSlider(formLayout, "Filter Q", "filterQ",
                       noiseSettings.filterQMin, noiseSettings.filterQMax, 1.0, 0.1, 1);

    // Follow Pitch toggle
    QCheckBox *checkFollowPitch = new QCheckBox();
    checkFollowPitch->setChecked(currentContainer->getParameter("followPitch", 0.0) > 0.5);
    connect(checkFollowPitch, &QCheckBox::toggled, this, [this](bool checked) {
        if (currentContainer) {
            currentContainer->setParameter("followPitch", checked ? 1.0 : 0.0);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Follow Note Pitch:", checkFollowPitch);

    QLabel *pitchNote = new QLabel("When on, Color is ignored. Use Bandpass + high Q for pitched noise.");
    pitchNote->setWordWrap(true);
    pitchNote->setStyleSheet("color: gray; font-size: 9pt;");
    formLayout->addRow("", pitchNote);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier",
                       0.5, 8.0, 1.0, 0.5, 1);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Physics System Inspector
void KalaMain::populatePhysicsSystemInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &physicsSettings = ContainerSettings::instance().physicsSystem;
    addParameterSlider(formLayout, "Mass", "mass",
                       physicsSettings.massMin, physicsSettings.massMax, 0.5, 0.1, 1);
    addParameterSlider(formLayout, "Spring K", "springK",
                       physicsSettings.springKMin, physicsSettings.springKMax, 0.001, 0.001, 3);
    addParameterSlider(formLayout, "Damping", "damping",
                       physicsSettings.dampingMin, physicsSettings.dampingMax, 0.995, 0.001, 3);
    addParameterSlider(formLayout, "Impulse Amount", "impulseAmount",
                       physicsSettings.impulseMin, physicsSettings.impulseMax, 100.0, 1.0, 0);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Envelope Engine Inspector
void KalaMain::populateEnvelopeEngineInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // Envelope Select dropdown
    comboEnvelopeSelect = new QComboBox();
    comboEnvelopeSelect->addItem("Linear", 0);
    comboEnvelopeSelect->addItem("Attack-Decay", 1);
    comboEnvelopeSelect->addItem("ADSR", 2);
    comboEnvelopeSelect->addItem("Fade In", 3);
    comboEnvelopeSelect->addItem("Fade Out", 4);
    comboEnvelopeSelect->addItem("Log Fade In", 5);
    comboEnvelopeSelect->addItem("Log Fade Out", 6);
    comboEnvelopeSelect->addItem("Custom...", 7);
    int envValue = static_cast<int>(currentContainer->getParameter("envelopeSelect", 0.0));
    comboEnvelopeSelect->setCurrentIndex(envValue);

    // Lambda to open custom envelope dialog
    auto openCustomEnvelopeDialog = [this]() {
        if (!currentContainer) return;

        // Open Envelope Library dialog
        EnvelopeLibraryDialog dialog(this);

        // If already using custom envelope, load it into the dialog
        if (currentContainer->hasCustomEnvelopeData()) {
            dialog.setEnvelope(currentContainer->getCustomEnvelopeData());
        }

        if (dialog.exec() == QDialog::Accepted) {
            EnvelopeData selectedEnvelope = dialog.getSelectedEnvelope();
            // Store custom envelope data in container
            currentContainer->setParameter("envelopeSelect", 7.0);
            currentContainer->setCustomEnvelopeData(selectedEnvelope);
            // Update the UI and visualizer
            updateEnvelopeParameters(7);
            // Rebuild graph to apply the custom envelope
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        } else {
            // User cancelled - revert to previous selection if not already custom
            int previousValue = static_cast<int>(currentContainer->getParameter("envelopeSelect", 0.0));
            if (previousValue != 7) {
                comboEnvelopeSelect->blockSignals(true);
                comboEnvelopeSelect->setCurrentIndex(previousValue);
                comboEnvelopeSelect->blockSignals(false);
            }
        }
    };

    // Use activated signal to handle clicks even when already selected
    connect(comboEnvelopeSelect, QOverload<int>::of(&QComboBox::activated),
            this, [this, openCustomEnvelopeDialog](int index) {
        if (!currentContainer) return;

        if (index == 7) {
            openCustomEnvelopeDialog();
        } else {
            // Standard envelope type
            currentContainer->setParameter("envelopeSelect", static_cast<double>(index));
            updateEnvelopeParameters(index);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Envelope Shape:", comboEnvelopeSelect);

    // Score Curve dropdown: which expressive curve from the note to use when followDynamics is on.
    // "Dynamics" (index 0) is always available; additional named curves are collected from the
    // score canvas phrase (where the actual expressive curves are stored and edited).
    comboScoreCurve = new QComboBox();
    comboScoreCurve->addItem("Dynamics", 0);

    {
        QSet<QString> seenNames;
        seenNames.insert("Dynamics");
        const QVector<Note> &canvasNotes = scoreCanvasWindow->getScoreCanvas()->getPhrase().getNotes();
        for (const Note &note : canvasNotes) {
            for (int ci = 1; ci < note.getExpressiveCurveCount(); ++ci) {
                QString name = note.getExpressiveCurveName(ci);
                if (!name.isEmpty() && !seenNames.contains(name)) {
                    seenNames.insert(name);
                    comboScoreCurve->addItem(name, ci);
                }
            }
        }
    }

    int currentScoreIdx = static_cast<int>(currentContainer->getParameter("scoreCurveIndex", 0.0));
    // Select the combo item whose stored data matches currentScoreIdx
    for (int i = 0; i < comboScoreCurve->count(); ++i) {
        if (comboScoreCurve->itemData(i).toInt() == currentScoreIdx) {
            comboScoreCurve->setCurrentIndex(i);
            break;
        }
    }

    connect(comboScoreCurve, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int comboIndex) {
        if (!currentContainer) return;
        int curveIndex = comboScoreCurve->itemData(comboIndex).toInt();
        currentContainer->setParameter("scoreCurveIndex", static_cast<double>(curveIndex));
        int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
        sounitBuilder->rebuildGraph(trackIndex);
        updateEnvelopePreview();
    });
    formLayout->addRow("Score Curve:", comboScoreCurve);

    mainLayout->addWidget(formWidget);

    // Create container for contextual parameters
    envelopeParamsWidget = new QWidget();
    mainLayout->addWidget(envelopeParamsWidget);

    // Envelope preview visualizer
    QLabel *labelPreview = new QLabel("Envelope Preview:");
    labelPreview->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(labelPreview);

    envelopeViz = new EnvelopeVisualizer();
    mainLayout->addWidget(envelopeViz);

    envelopeNameLabel = new QLabel();
    envelopeNameLabel->setAlignment(Qt::AlignCenter);
    envelopeNameLabel->setStyleSheet("color: #888; font-style: italic;");
    mainLayout->addWidget(envelopeNameLabel);

    // Populate parameters based on current envelope type
    updateEnvelopeParameters(envValue);

    // Refresh preview whenever the container's followDynamics toggle changes.
    // Store the handle so clearConfigInspector() can disconnect it cleanly.
    m_envelopePreviewConnection = connect(currentContainer, &Container::parameterChanged,
                                          this, [this]() {
        if (!currentContainer || !envelopeViz) return;
        updateEnvelopePreview();
    });

    mainLayout->addStretch();
}

// Drift Engine Inspector
void KalaMain::populateDriftEngineInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // Drift Pattern dropdown
    QComboBox *comboDriftPattern = new QComboBox();
    comboDriftPattern->addItem("Sine", 0);
    comboDriftPattern->addItem("Random", 1);
    comboDriftPattern->addItem("Perlin (Smooth)", 2);
    int driftPattern = static_cast<int>(currentContainer->getParameter("driftPattern", 2.0));
    comboDriftPattern->setCurrentIndex(driftPattern);
    connect(comboDriftPattern, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("driftPattern", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Pattern:", comboDriftPattern);

    const auto &driftSettings = ContainerSettings::instance().driftEngine;
    addParameterSlider(formLayout, "Amount", "amount",
                       driftSettings.amountMin, driftSettings.amountMax, 0.005, 0.001, 3);
    addParameterSlider(formLayout, "Rate (Hz)", "rate",
                       driftSettings.rateMin, driftSettings.rateMax, 0.5, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Gate Processor Inspector
void KalaMain::populateGateProcessorInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &gateSettings = ContainerSettings::instance().gateProcessor;
    addParameterSlider(formLayout, "Velocity", "velocity",
                       gateSettings.velocityMin, gateSettings.velocityMax, 1.0, 0.01, 2);
    addParameterSlider(formLayout, "Attack Time (s)", "attackTime",
                       gateSettings.attackTimeMin, gateSettings.attackTimeMax, 0.01, 0.001, 3);
    addParameterSlider(formLayout, "Release Time (s)", "releaseTime",
                       gateSettings.releaseTimeMin, gateSettings.releaseTimeMax, 0.1, 0.001, 3);
    addParameterSlider(formLayout, "Velocity Sens", "velocitySens",
                       gateSettings.velocitySensMin, gateSettings.velocitySensMax, 0.5, 0.01, 2);

    // Attack/Release Curve dropdowns
    QComboBox *comboAttack = new QComboBox();
    comboAttack->addItem("Linear", 0);
    comboAttack->addItem("Exponential", 1);
    comboAttack->addItem("Logarithmic", 2);
    int attackCurve = static_cast<int>(currentContainer->getParameter("attackCurve", 0.0));
    comboAttack->setCurrentIndex(attackCurve);
    connect(comboAttack, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("attackCurve", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Attack Curve:", comboAttack);

    QComboBox *comboRelease = new QComboBox();
    comboRelease->addItem("Linear", 0);
    comboRelease->addItem("Exponential", 1);
    comboRelease->addItem("Logarithmic", 2);
    int releaseCurve = static_cast<int>(currentContainer->getParameter("releaseCurve", 0.0));
    comboRelease->setCurrentIndex(releaseCurve);
    connect(comboRelease, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("releaseCurve", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Release Curve:", comboRelease);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Easing Applicator Inspector
void KalaMain::populateEasingApplicatorInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // Start Value spinbox
    QDoubleSpinBox *startValueSpin = new QDoubleSpinBox();
    startValueSpin->setRange(-10000.0, 10000.0);
    startValueSpin->setSingleStep(0.1);
    startValueSpin->setDecimals(4);
    startValueSpin->setValue(currentContainer->getParameter("startValue", 0.0));
    formLayout->addRow("Start Value:", startValueSpin);

    connect(startValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double value) {
        if (currentContainer) {
            currentContainer->setParameter("startValue", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    // End Value spinbox
    QDoubleSpinBox *endValueSpin = new QDoubleSpinBox();
    endValueSpin->setRange(-10000.0, 10000.0);
    endValueSpin->setSingleStep(0.1);
    endValueSpin->setDecimals(4);
    endValueSpin->setValue(currentContainer->getParameter("endValue", 1.0));
    formLayout->addRow("End Value:", endValueSpin);

    connect(endValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double value) {
        if (currentContainer) {
            currentContainer->setParameter("endValue", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    // Easing Select dropdown - populated from Easing class
    QComboBox *comboEasing = new QComboBox();

    // Get all easings and add them organized by category
    QString currentCategory = "";
    for (const Easing &easing : Easing::getAllEasings()) {
        QString category = easing.getCategory();

        // Add category separator when category changes
        if (category != currentCategory) {
            if (!currentCategory.isEmpty()) {
                // Add a separator item (disabled, acts as visual separator)
                comboEasing->insertSeparator(comboEasing->count());
            }
            currentCategory = category;
        }

        // Add the easing with its ID as data
        QString displayName = easing.getName();
        if (easing.canOvershoot()) {
            displayName += " *";  // Mark overshooting easings
        }
        comboEasing->addItem(displayName, easing.getId());
    }

    // Set current value from container parameter
    int easingValue = static_cast<int>(currentContainer->getParameter("easingSelect", 0.0));

    // Find the index for this easing ID (accounting for separators)
    for (int i = 0; i < comboEasing->count(); ++i) {
        if (comboEasing->itemData(i).toInt() == easingValue) {
            comboEasing->setCurrentIndex(i);
            break;
        }
    }

    formLayout->addRow("Easing Function:", comboEasing);

    // Add info label about overshooting easings
    QLabel *infoLabel = new QLabel("* Can overshoot 0-1 range");
    infoLabel->setStyleSheet("color: #666; font-size: 10px; font-style: italic;");
    formLayout->addRow("", infoLabel);

    // ========== Parameters Section ==========
    // Create a container widget for parameters that can be shown/hidden
    QWidget *paramsWidget = new QWidget();
    QFormLayout *paramsLayout = new QFormLayout(paramsWidget);
    paramsLayout->setSpacing(8);
    paramsLayout->setContentsMargins(0, 10, 0, 0);

    QLabel *paramsHeader = new QLabel("Parameters");
    paramsHeader->setStyleSheet("font-weight: bold; color: #333;");
    paramsLayout->addRow(paramsHeader);

    // --- Back Parameters: Overshoot ---
    QWidget *overshootWidget = new QWidget();
    QHBoxLayout *overshootLayout = new QHBoxLayout(overshootWidget);
    overshootLayout->setContentsMargins(0, 0, 0, 0);

    QSlider *overshootSlider = new QSlider(Qt::Horizontal);
    overshootSlider->setRange(0, 500);  // 0.0 to 5.0 with 0.01 step
    QDoubleSpinBox *overshootSpin = new QDoubleSpinBox();
    overshootSpin->setRange(0.0, 5.0);
    overshootSpin->setSingleStep(0.1);
    overshootSpin->setDecimals(2);

    double overshootVal = currentContainer->getParameter("easingOvershoot", Easing::getDefaultOvershoot());
    overshootSlider->setValue(static_cast<int>(overshootVal * 100));
    overshootSpin->setValue(overshootVal);

    overshootLayout->addWidget(overshootSlider);
    overshootLayout->addWidget(overshootSpin);
    paramsLayout->addRow("Overshoot:", overshootWidget);

    // --- Elastic Parameters: Amplitude ---
    QWidget *amplitudeWidget = new QWidget();
    QHBoxLayout *amplitudeLayout = new QHBoxLayout(amplitudeWidget);
    amplitudeLayout->setContentsMargins(0, 0, 0, 0);

    QSlider *amplitudeSlider = new QSlider(Qt::Horizontal);
    amplitudeSlider->setRange(10, 300);  // 0.1 to 3.0 with 0.01 step
    QDoubleSpinBox *amplitudeSpin = new QDoubleSpinBox();
    amplitudeSpin->setRange(0.1, 3.0);
    amplitudeSpin->setSingleStep(0.1);
    amplitudeSpin->setDecimals(2);

    double amplitudeVal = currentContainer->getParameter("easingAmplitude", Easing::getDefaultAmplitude());
    amplitudeSlider->setValue(static_cast<int>(amplitudeVal * 100));
    amplitudeSpin->setValue(amplitudeVal);

    amplitudeLayout->addWidget(amplitudeSlider);
    amplitudeLayout->addWidget(amplitudeSpin);
    paramsLayout->addRow("Amplitude:", amplitudeWidget);

    // --- Elastic Parameters: Period ---
    QWidget *periodWidget = new QWidget();
    QHBoxLayout *periodLayout = new QHBoxLayout(periodWidget);
    periodLayout->setContentsMargins(0, 0, 0, 0);

    QSlider *periodSlider = new QSlider(Qt::Horizontal);
    periodSlider->setRange(10, 200);  // 0.1 to 2.0 with 0.01 step
    QDoubleSpinBox *periodSpin = new QDoubleSpinBox();
    periodSpin->setRange(0.1, 2.0);
    periodSpin->setSingleStep(0.05);
    periodSpin->setDecimals(2);

    double periodVal = currentContainer->getParameter("easingPeriod", Easing::getDefaultPeriod());
    periodSlider->setValue(static_cast<int>(periodVal * 100));
    periodSpin->setValue(periodVal);

    periodLayout->addWidget(periodSlider);
    periodLayout->addWidget(periodSpin);
    paramsLayout->addRow("Period:", periodWidget);

    // --- Wobble Parameters: Frequency ---
    QWidget *frequencyWidget = new QWidget();
    QHBoxLayout *frequencyLayout = new QHBoxLayout(frequencyWidget);
    frequencyLayout->setContentsMargins(0, 0, 0, 0);

    QSlider *frequencySlider = new QSlider(Qt::Horizontal);
    frequencySlider->setRange(10, 100);  // 1.0 to 10.0 with 0.1 step
    QDoubleSpinBox *frequencySpin = new QDoubleSpinBox();
    frequencySpin->setRange(1.0, 10.0);
    frequencySpin->setSingleStep(0.5);
    frequencySpin->setDecimals(1);

    double frequencyVal = currentContainer->getParameter("easingFrequency", Easing::getDefaultFrequency());
    frequencySlider->setValue(static_cast<int>(frequencyVal * 10));
    frequencySpin->setValue(frequencyVal);

    frequencyLayout->addWidget(frequencySlider);
    frequencyLayout->addWidget(frequencySpin);
    paramsLayout->addRow("Frequency:", frequencyWidget);

    // --- Wobble Parameters: Decay ---
    QWidget *decayWidget = new QWidget();
    QHBoxLayout *decayLayout = new QHBoxLayout(decayWidget);
    decayLayout->setContentsMargins(0, 0, 0, 0);

    QSlider *decaySlider = new QSlider(Qt::Horizontal);
    decaySlider->setRange(10, 200);  // 0.1 to 2.0 with 0.01 step
    QDoubleSpinBox *decaySpin = new QDoubleSpinBox();
    decaySpin->setRange(0.1, 2.0);
    decaySpin->setSingleStep(0.1);
    decaySpin->setDecimals(2);

    double decayVal = currentContainer->getParameter("easingDecay", Easing::getDefaultDecay());
    decaySlider->setValue(static_cast<int>(decayVal * 100));
    decaySpin->setValue(decayVal);

    decayLayout->addWidget(decaySlider);
    decayLayout->addWidget(decaySpin);
    paramsLayout->addRow("Decay:", decayWidget);

    // Helper lambda to determine which parameters to show based on easing type
    auto updateParamsVisibility = [=](int easingId) {
        Easing easing = Easing::getById(easingId);
        QString category = easing.getCategory();

        bool showOvershoot = (category == "Back");
        bool showElastic = (category == "Elastic");
        bool showWobble = (category == "Wobble");

        overshootWidget->setVisible(showOvershoot);
        amplitudeWidget->setVisible(showElastic);
        periodWidget->setVisible(showElastic);
        frequencyWidget->setVisible(showWobble);
        decayWidget->setVisible(showWobble);

        // Show params section if any parameter is visible
        paramsHeader->setVisible(showOvershoot || showElastic || showWobble);
    };

    // Set initial visibility
    updateParamsVisibility(easingValue);

    // Connect combo box to update visibility and parameter
    connect(comboEasing, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, comboEasing, updateParamsVisibility](int index) {
        if (currentContainer && index >= 0) {
            QVariant data = comboEasing->itemData(index);
            if (data.isValid()) {
                int easingId = data.toInt();
                currentContainer->setParameter("easingSelect", static_cast<double>(easingId));
                updateParamsVisibility(easingId);
                int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
                sounitBuilder->rebuildGraph(trackIndex);
            }
        }
    });

    // Connect overshoot slider/spinbox
    connect(overshootSlider, &QSlider::valueChanged, this, [this, overshootSpin](int value) {
        double doubleVal = value / 100.0;
        overshootSpin->blockSignals(true);
        overshootSpin->setValue(doubleVal);
        overshootSpin->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingOvershoot", doubleVal);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    connect(overshootSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, overshootSlider](double value) {
        overshootSlider->blockSignals(true);
        overshootSlider->setValue(static_cast<int>(value * 100));
        overshootSlider->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingOvershoot", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    // Connect amplitude slider/spinbox
    connect(amplitudeSlider, &QSlider::valueChanged, this, [this, amplitudeSpin](int value) {
        double doubleVal = value / 100.0;
        amplitudeSpin->blockSignals(true);
        amplitudeSpin->setValue(doubleVal);
        amplitudeSpin->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingAmplitude", doubleVal);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    connect(amplitudeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, amplitudeSlider](double value) {
        amplitudeSlider->blockSignals(true);
        amplitudeSlider->setValue(static_cast<int>(value * 100));
        amplitudeSlider->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingAmplitude", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    // Connect period slider/spinbox
    connect(periodSlider, &QSlider::valueChanged, this, [this, periodSpin](int value) {
        double doubleVal = value / 100.0;
        periodSpin->blockSignals(true);
        periodSpin->setValue(doubleVal);
        periodSpin->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingPeriod", doubleVal);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    connect(periodSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, periodSlider](double value) {
        periodSlider->blockSignals(true);
        periodSlider->setValue(static_cast<int>(value * 100));
        periodSlider->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingPeriod", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    // Connect frequency slider/spinbox (Wobble)
    connect(frequencySlider, &QSlider::valueChanged, this, [this, frequencySpin](int value) {
        double doubleVal = value / 10.0;
        frequencySpin->blockSignals(true);
        frequencySpin->setValue(doubleVal);
        frequencySpin->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingFrequency", doubleVal);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    connect(frequencySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, frequencySlider](double value) {
        frequencySlider->blockSignals(true);
        frequencySlider->setValue(static_cast<int>(value * 10));
        frequencySlider->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingFrequency", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    // Connect decay slider/spinbox (Wobble)
    connect(decaySlider, &QSlider::valueChanged, this, [this, decaySpin](int value) {
        double doubleVal = value / 100.0;
        decaySpin->blockSignals(true);
        decaySpin->setValue(doubleVal);
        decaySpin->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingDecay", doubleVal);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    connect(decaySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, decaySlider](double value) {
        decaySlider->blockSignals(true);
        decaySlider->setValue(static_cast<int>(value * 100));
        decaySlider->blockSignals(false);
        if (currentContainer) {
            currentContainer->setParameter("easingDecay", value);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });

    mainLayout->addWidget(formWidget);
    mainLayout->addWidget(paramsWidget);
    mainLayout->addStretch();
}

// Karplus Strong Inspector
void KalaMain::populateKarplusStrongInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &ks = ContainerSettings::instance().karplusStrong;

    // Mode combo box (String / Attack)
    QComboBox *comboMode = new QComboBox();
    comboMode->addItem("String", 0);   // K-S controls entire sound
    comboMode->addItem("Attack", 1);   // K-S adds attack to input signal
    int modeValue = static_cast<int>(currentContainer->getParameter("mode", 0.0));
    comboMode->setCurrentIndex(modeValue);
    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("mode", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Mode:", comboMode);

    // Mode description
    QLabel *modeDescLabel = new QLabel("String: K-S is entire sound\nAttack: K-S adds pluck to input");
    modeDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", modeDescLabel);

    // Decay slider (remapped damping: 0=longest sustain, 1=quickest decay)
    addParameterSlider(formLayout, "Decay", "damping", ks.dampingMin, ks.dampingMax, 0.5, 0.01, 2);
    QLabel *decayDescLabel = new QLabel("0 = let ring (~8s), 1 = dead stop (~15ms)");
    decayDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", decayDescLabel);

    // Pluck Position slider (0-100%, stored as 0-1)
    addParameterSlider(formLayout, "Pluck Position", "pluckPosition", ks.pluckPositionMin, ks.pluckPositionMax, 0.5, 0.01, 2);
    QLabel *pluckDescLabel = new QLabel("Bridge (0) = Bright, Neck (1) = Mellow");
    pluckDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pluckDescLabel);

    // Attack-mode-only parameters — hidden in String mode
    QWidget *attackOnlyParams = new QWidget();
    QFormLayout *attackOnlyLayout = new QFormLayout(attackOnlyParams);
    attackOnlyLayout->setContentsMargins(0, 0, 0, 0);
    attackOnlyLayout->setSpacing(10);

    addParameterSlider(attackOnlyLayout, "Attack Portion (%)", "attackPortion", ks.attackPortionMin, ks.attackPortionMax, 0.15, 0.01, 2);

    addParameterSlider(attackOnlyLayout, "Attack Mix", "mix", ks.mixMin, ks.mixMax, 0.8, 0.01, 2);
    QLabel *mixDescLabel = new QLabel("0=subtle, 1=prominent pluck");
    mixDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    attackOnlyLayout->addRow("", mixDescLabel);

    formLayout->addRow(attackOnlyParams);
    attackOnlyParams->setVisible(modeValue == 1);  // Show only in Attack mode

    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            attackOnlyParams, [attackOnlyParams](int index) {
        attackOnlyParams->setVisible(index == 1);
    });

    // ========== Extended KS Parameters ==========
    QLabel *eksHeader = new QLabel("Extended KS Parameters");
    eksHeader->setStyleSheet("font-weight: bold; font-size: 11px; margin-top: 8px;");
    formLayout->addRow(eksHeader);

    // Brightness slider
    addParameterSlider(formLayout, "Brightness", "brightness", ks.brightnessMin, ks.brightnessMax, 0.5, 0.01, 2);
    QLabel *brightnessDescLabel = new QLabel("Loop filter: 0=warm/dark, 1=bright/ringing");
    brightnessDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", brightnessDescLabel);

    // Excitation Type combo
    QComboBox *comboExcitation = new QComboBox();
    comboExcitation->addItem("Noise", 0);
    comboExcitation->addItem("Soft Pluck", 1);
    comboExcitation->addItem("Blend", 2);
    comboExcitation->addItem("Finger Pluck", 3);
    int excitationValue = static_cast<int>(currentContainer->getParameter("excitationType", 0.0));
    comboExcitation->setCurrentIndex(excitationValue);
    connect(comboExcitation, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("excitationType", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Excitation:", comboExcitation);

    // Blend Ratio slider — only visible when Blend excitation is selected
    QWidget *blendWidget = new QWidget();
    QFormLayout *blendLayout = new QFormLayout(blendWidget);
    blendLayout->setContentsMargins(0, 0, 0, 0);
    blendLayout->setSpacing(6);
    addParameterSlider(blendLayout, "Blend Ratio", "blendRatio",
                       ks.excitationSoftnessMin, ks.excitationSoftnessMax, 0.5, 0.01, 2);
    QLabel *blendDescLabel = new QLabel("0 = noise only, 1 = soft pluck only");
    blendDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    blendLayout->addRow("", blendDescLabel);
    formLayout->addRow(blendWidget);
    blendWidget->setVisible(excitationValue == 2);
    connect(comboExcitation, QOverload<int>::of(&QComboBox::currentIndexChanged),
            blendWidget, [blendWidget](int index) {
        blendWidget->setVisible(index == 2);
    });

    // Pluck Hardness slider
    addParameterSlider(formLayout, "Pluck Hardness", "pluckHardness", ks.pluckHardnessMin, ks.pluckHardnessMax, 0.0, 0.01, 2);
    QLabel *hardnessDescLabel = new QLabel("0=soft wide thumb, 1=hard narrow pick");
    hardnessDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", hardnessDescLabel);

    // Pick Direction slider
    addParameterSlider(formLayout, "Pick Direction", "pickDirection", ks.pickDirectionMin, ks.pickDirectionMax, 0.5, 0.01, 2);
    QLabel *pickDescLabel2 = new QLabel("0=downstroke, 0.5=neutral, 1=upstroke");
    pickDescLabel2->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pickDescLabel2);

    // ========== Body Resonance ==========
    QLabel *bodyHeader = new QLabel("Body Resonance");
    bodyHeader->setStyleSheet("font-weight: bold; font-size: 11px; margin-top: 8px;");
    formLayout->addRow(bodyHeader);

    // Body Resonance mix slider
    addParameterSlider(formLayout, "Resonance Mix", "bodyResonance", ks.bodyResonanceMin, ks.bodyResonanceMax, 0.0, 0.01, 2);
    QLabel *bodyMixDescLabel = new QLabel("0=off, adds warmth from resonant body");
    bodyMixDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", bodyMixDescLabel);

    // Body Frequency slider
    addParameterSlider(formLayout, "Body Frequency (Hz)", "bodyFreq", ks.bodyFreqMin, ks.bodyFreqMax, 200.0, 1.0, 0);
    QLabel *bodyFreqDescLabel = new QLabel("Center frequency of body resonator");
    bodyFreqDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", bodyFreqDescLabel);

    // ========== String Behavior ==========
    QLabel *stringHeader = new QLabel("String Behavior");
    stringHeader->setStyleSheet("font-weight: bold; font-size: 11px; margin-top: 8px;");
    formLayout->addRow(stringHeader);

    QCheckBox *stringDampingCheck = new QCheckBox("Fretted Mode");
    stringDampingCheck->setChecked(currentContainer->getParameter("stringDamping", 0.0) >= 0.5);
    connect(stringDampingCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (currentContainer) {
            currentContainer->setParameter("stringDamping", checked ? 1.0 : 0.0);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow(stringDampingCheck);
    QLabel *dampingDescLabel = new QLabel("On: each new note cuts the previous (guitar/fretted). Off: notes ring freely (harp/open string).");
    dampingDescLabel->setStyleSheet("color: gray; font-size: 10px;");
    dampingDescLabel->setWordWrap(true);
    formLayout->addRow("", dampingDescLabel);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescKS = new QLabel("1.0 = note pitch · 2.0 = octave up · 0.5 = octave down");
    pitchMultDescKS->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescKS);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Attack Inspector
void KalaMain::populateAttackInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &attackSettings = ContainerSettings::instance().attack;

    // Attack Type dropdown
    QComboBox *comboAttackType = new QComboBox();
    comboAttackType->addItem("Air Jet", 0);
    comboAttackType->addItem("Reed Chirp", 1);
    comboAttackType->addItem("Sub Bloom", 2);
    comboAttackType->addItem("Lip Bend", 3);
    comboAttackType->addItem("FM Strike", 4);
    comboAttackType->addItem("Membrane Sweep", 5);
    int typeValue = static_cast<int>(currentContainer->getParameter("attackType", 0.0));
    comboAttackType->setCurrentIndex(typeValue);
    formLayout->addRow("Attack Type:", comboAttackType);

    // Common sliders
    addParameterSlider(formLayout, "Duration (s)", "duration",
                       attackSettings.durationMin, attackSettings.durationMax, 0.15, 0.001, 3);
    addParameterSlider(formLayout, "Intensity", "intensity", attackSettings.intensityMin, attackSettings.intensityMax, 0.8, 0.01, 2);
    addParameterSlider(formLayout, "Mix", "mix", attackSettings.mixMin, attackSettings.mixMax, 0.8, 0.01, 2);

    QLabel *mixDesc = new QLabel("0 = input only, 1 = burst dominant");
    mixDesc->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", mixDesc);

    // === Type-specific parameter groups ===

    // Flute group
    QWidget *fluteGroup = new QWidget();
    QFormLayout *fluteLayout = new QFormLayout(fluteGroup);
    fluteLayout->setSpacing(6);
    QLabel *fluteHeader = new QLabel("Air Jet Parameters");
    fluteHeader->setStyleSheet("font-weight: bold; color: #3498db;");
    fluteLayout->addRow(fluteHeader);
    addParameterSlider(fluteLayout, "Noise Amount", "noiseAmount", attackSettings.noiseAmountMin, attackSettings.noiseAmountMax, 0.5, 0.01, 2);
    addParameterSlider(fluteLayout, "Jet Ratio", "jetRatio", attackSettings.jetRatioMin, attackSettings.jetRatioMax, 0.32, 0.01, 2);
    formLayout->addRow(fluteGroup);

    // Clarinet group
    QWidget *clarinetGroup = new QWidget();
    QFormLayout *clarinetLayout = new QFormLayout(clarinetGroup);
    clarinetLayout->setSpacing(6);
    QLabel *clarinetHeader = new QLabel("Reed Parameters");
    clarinetHeader->setStyleSheet("font-weight: bold; color: #3498db;");
    clarinetLayout->addRow(clarinetHeader);
    addParameterSlider(clarinetLayout, "Reed Stiffness", "reedStiffness", attackSettings.reedStiffnessMin, attackSettings.reedStiffnessMax, 0.5, 0.01, 2);
    addParameterSlider(clarinetLayout, "Reed Aperture", "reedAperture", attackSettings.reedApertureMin, attackSettings.reedApertureMax, 0.5, 0.01, 2);
    formLayout->addRow(clarinetGroup);

    // Sax blow position
    QWidget *saxGroup = new QWidget();
    QFormLayout *saxLayout = new QFormLayout(saxGroup);
    saxLayout->setSpacing(6);
    QLabel *saxHeader = new QLabel("Sub Bloom Parameters");
    saxHeader->setStyleSheet("font-weight: bold; color: #3498db;");
    saxLayout->addRow(saxHeader);
    addParameterSlider(saxLayout, "Blow Position", "blowPosition", attackSettings.blowPositionMin, attackSettings.blowPositionMax, 0.5, 0.01, 2);
    formLayout->addRow(saxGroup);

    // Brass group
    QWidget *brassGroup = new QWidget();
    QFormLayout *brassLayout = new QFormLayout(brassGroup);
    brassLayout->setSpacing(6);
    QLabel *brassHeader = new QLabel("Lip Bend Parameters");
    brassHeader->setStyleSheet("font-weight: bold; color: #3498db;");
    brassLayout->addRow(brassHeader);
    addParameterSlider(brassLayout, "Lip Tension", "lipTension", attackSettings.lipTensionMin, attackSettings.lipTensionMax, 0.5, 0.01, 2);
    addParameterSlider(brassLayout, "Buzz Q", "buzzQ",
                       attackSettings.buzzQMin, attackSettings.buzzQMax, 5.0, 0.1, 1);
    formLayout->addRow(brassGroup);

    // Piano group
    QWidget *pianoGroup = new QWidget();
    QFormLayout *pianoLayout = new QFormLayout(pianoGroup);
    pianoLayout->setSpacing(6);
    QLabel *pianoHeader = new QLabel("FM Strike Parameters");
    pianoHeader->setStyleSheet("font-weight: bold; color: #3498db;");
    pianoLayout->addRow(pianoHeader);
    addParameterSlider(pianoLayout, "Hardness", "hardness", attackSettings.hardnessMin, attackSettings.hardnessMax, 0.5, 0.01, 2);
    addParameterSlider(pianoLayout, "Brightness", "brightness", attackSettings.brightnessMin, attackSettings.brightnessMax, 0.5, 0.01, 2);
    formLayout->addRow(pianoGroup);

    // Drum group
    QWidget *drumGroup = new QWidget();
    QFormLayout *drumLayout = new QFormLayout(drumGroup);
    drumLayout->setSpacing(6);
    QLabel *drumHeader = new QLabel("Membrane Sweep Parameters");
    drumHeader->setStyleSheet("font-weight: bold; color: #3498db;");
    drumLayout->addRow(drumHeader);
    addParameterSlider(drumLayout, "Tightness", "tightness", attackSettings.tightnessMin, attackSettings.tightnessMax, 0.5, 0.01, 2);
    addParameterSlider(drumLayout, "Tone", "tone", attackSettings.toneMin, attackSettings.toneMax, 0.5, 0.01, 2);
    formLayout->addRow(drumGroup);

    // Show/hide groups based on attack type
    auto updateVisibility = [=](int index) {
        fluteGroup->setVisible(index == 0);
        // Clarinet and Sax both use reed params
        clarinetGroup->setVisible(index == 1 || index == 2);
        saxGroup->setVisible(index == 2);
        brassGroup->setVisible(index == 3);
        pianoGroup->setVisible(index == 4);
        drumGroup->setVisible(index == 5);
    };
    updateVisibility(typeValue);

    connect(comboAttackType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, updateVisibility](int index) {
        if (currentContainer) {
            currentContainer->setParameter("attackType", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
        updateVisibility(index);
    });

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// LFO Inspector
void KalaMain::populateLFOInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &lfoSettings = ContainerSettings::instance().lfo;
    // Frequency slider
    addParameterSlider(formLayout, "Frequency (Hz)", "frequency",
                       lfoSettings.frequencyMin, lfoSettings.frequencyMax, 1.0, 0.01, 2);

    // Amplitude slider
    addParameterSlider(formLayout, "Amplitude", "amplitude",
                       lfoSettings.amplitudeMin, lfoSettings.amplitudeMax, 1.0, 0.01, 2);

    // Wave Type dropdown
    QComboBox *comboWaveType = new QComboBox();
    comboWaveType->addItem("Sine", 0);
    comboWaveType->addItem("Triangle", 1);
    comboWaveType->addItem("Square", 2);
    comboWaveType->addItem("Sawtooth", 3);
    int waveType = static_cast<int>(currentContainer->getParameter("waveType", 0.0));
    comboWaveType->setCurrentIndex(waveType);
    connect(comboWaveType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("waveType", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Wave Type:", comboWaveType);

    // Add helpful description
    QLabel *descLabel = new QLabel("Output: -amplitude to +amplitude");
    descLabel->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", descLabel);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Frequency Mapper Inspector
void KalaMain::populateFrequencyMapperInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    const auto &fm = ContainerSettings::instance().frequencyMapper;

    // Low Frequency
    addParameterSlider(formLayout, "Low Freq (Hz)", "lowFreq",
                       fm.lowFreqMin, fm.lowFreqMax, 25.0, 1.0, 0);

    // High Frequency
    addParameterSlider(formLayout, "High Freq (Hz)", "highFreq",
                       fm.highFreqMin, fm.highFreqMax, 8000.0, 100.0, 0);

    // Output Min
    addParameterSlider(formLayout, "Output Min", "outputMin",
                       fm.outputMinMin, fm.outputMinMax, 0.0, 0.01, 2);

    // Output Max
    addParameterSlider(formLayout, "Output Max", "outputMax",
                       fm.outputMaxMin, fm.outputMaxMax, 1.0, 0.01, 2);

    // Curve Type dropdown
    QComboBox *comboCurveType = new QComboBox();
    comboCurveType->addItem("Linear", 0);
    comboCurveType->addItem("Logarithmic", 1);
    comboCurveType->addItem("Exponential", 2);
    comboCurveType->addItem("S-Curve", 3);
    int curveType = static_cast<int>(currentContainer->getParameter("curveType", 1.0));
    comboCurveType->setCurrentIndex(curveType);
    connect(comboCurveType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("curveType", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Curve Type:", comboCurveType);

    // Invert checkbox
    QCheckBox *invertCheck = new QCheckBox("Invert output");
    invertCheck->setChecked(currentContainer->getParameter("invert", 0.0) > 0.5);
    connect(invertCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (currentContainer) {
            currentContainer->setParameter("invert", checked ? 1.0 : 0.0);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("", invertCheck);

    QLabel *descLabel = new QLabel("Maps note pitch to a 0\u20131 control value.\nConnect 'curve' input for external shaping.");
    descLabel->setStyleSheet("color: gray; font-size: 10px;");
    descLabel->setWordWrap(true);
    formLayout->addRow("", descLabel);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescFM = new QLabel("Scales input pitch before mapping · 2.0 = octave up");
    pitchMultDescFM->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescFM);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Signal Mixer Inspector
void KalaMain::populateSignalMixerInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    addParameterSlider(formLayout, "Gain A", "gainA", 0.0, 2.0, 1.0, 0.01, 2);
    addParameterSlider(formLayout, "Gain B", "gainB", 0.0, 2.0, 1.0, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Spectrum Blender Inspector
void KalaMain::populateSpectrumBlenderInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    addParameterSlider(formLayout, "Position", "position", 0.0, 1.0, 0.5, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Wavetable Synth Inspector
void KalaMain::populateWavetableSynthInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // Wavetable preset dropdown
    QComboBox *comboPreset = new QComboBox();
    comboPreset->addItem("Saw", 0);
    comboPreset->addItem("Square", 1);
    comboPreset->addItem("Triangle", 2);
    comboPreset->addItem("Pulse", 3);
    comboPreset->addItem("SuperSaw", 4);
    comboPreset->addItem("Formant", 5);
    comboPreset->addItem("Load WAV...", 6);

    int presetVal = static_cast<int>(currentContainer->getParameter("wavetableSelect", 0.0));
    // If container has WAV data loaded, show "Load WAV..." as selected
    if (currentContainer->hasWavetableData()) {
        comboPreset->setCurrentIndex(6);
    } else {
        comboPreset->setCurrentIndex(presetVal);
    }

    // Loaded file path label
    QLabel *filePathLabel = new QLabel();
    filePathLabel->setStyleSheet("color: gray; font-size: 10px;");
    filePathLabel->setWordWrap(true);
    if (currentContainer->hasWavetableData()) {
        QString path = currentContainer->getWavetableFilePath();
        filePathLabel->setText("Loaded: " + (path.isEmpty() ? "(embedded data)" : path));
        filePathLabel->setVisible(true);
    } else {
        filePathLabel->setVisible(false);
    }

    connect(comboPreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, comboPreset, filePathLabel](int index) {
        if (!currentContainer) return;

        if (index == 6) {
            // Load WAV file
            QString filePath = QFileDialog::getOpenFileName(this, "Load Wavetable WAV",
                                                             QString(), "WAV Files (*.wav)");
            if (!filePath.isEmpty()) {
                std::vector<float> wavData;
                if (WavetableSynth::loadWavFile(filePath, wavData)) {
                    currentContainer->setWavetableData(wavData);
                    currentContainer->setWavetableFilePath(filePath);
                    filePathLabel->setText("Loaded: " + filePath);
                    filePathLabel->setVisible(true);
                    int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
                    sounitBuilder->rebuildGraph(trackIndex);
                } else {
                    QMessageBox::warning(this, "Load Error",
                                        "Failed to load WAV file:\n" + filePath);
                    // Revert to previous preset
                    int prev = static_cast<int>(currentContainer->getParameter("wavetableSelect", 0.0));
                    comboPreset->blockSignals(true);
                    comboPreset->setCurrentIndex(currentContainer->hasWavetableData() ? 6 : prev);
                    comboPreset->blockSignals(false);
                }
            } else {
                // User cancelled - revert
                int prev = static_cast<int>(currentContainer->getParameter("wavetableSelect", 0.0));
                comboPreset->blockSignals(true);
                comboPreset->setCurrentIndex(currentContainer->hasWavetableData() ? 6 : prev);
                comboPreset->blockSignals(false);
            }
        } else {
            // Built-in preset selected - clear any loaded WAV data
            currentContainer->setWavetableData({});
            currentContainer->setWavetableFilePath("");
            currentContainer->setParameter("wavetableSelect", static_cast<double>(index));
            filePathLabel->setVisible(false);
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Wavetable:", comboPreset);
    formLayout->addRow("", filePathLabel);

    // Position slider
    const auto &wtSettings = ContainerSettings::instance().wavetableSynth;
    addParameterSlider(formLayout, "Position", "position",
                       wtSettings.positionMin, wtSettings.positionMax, 0.0, 0.01, 2);

    // Cycle length dropdown
    QComboBox *comboCycleLen = new QComboBox();
    comboCycleLen->addItem("256", 256);
    comboCycleLen->addItem("512", 512);
    comboCycleLen->addItem("1024", 1024);
    comboCycleLen->addItem("2048", 2048);
    comboCycleLen->addItem("4096", 4096);
    int currentCycleLen = static_cast<int>(currentContainer->getParameter("cycleLength", 2048.0));
    int cycleLenIndex = comboCycleLen->findData(currentCycleLen);
    if (cycleLenIndex >= 0) comboCycleLen->setCurrentIndex(cycleLenIndex);
    else comboCycleLen->setCurrentIndex(3); // default to 2048

    connect(comboCycleLen, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, comboCycleLen](int index) {
        if (currentContainer) {
            int val = comboCycleLen->itemData(index).toInt();
            currentContainer->setParameter("cycleLength", static_cast<double>(val));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Cycle Length:", comboCycleLen);

    // Output mode dropdown
    QComboBox *comboOutputMode = new QComboBox();
    comboOutputMode->addItem("Both (Spectrum + Signal)", 0);
    comboOutputMode->addItem("Signal Only", 1);
    comboOutputMode->addItem("Spectrum Only", 2);
    int outputMode = static_cast<int>(currentContainer->getParameter("outputMode", 0.0));
    comboOutputMode->setCurrentIndex(outputMode);

    connect(comboOutputMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (currentContainer) {
            currentContainer->setParameter("outputMode", static_cast<double>(index));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    formLayout->addRow("Output Mode:", comboOutputMode);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescWT = new QLabel("1.0 = note pitch · 2.0 = octave up · 0.5 = octave down");
    pitchMultDescWT->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescWT);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

void KalaMain::populateBandpassEQInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &eqSettings = ContainerSettings::instance().bandpassEQ;

    static const char *bandLabels[10] = {
        "31 Hz", "63 Hz", "125 Hz", "250 Hz", "500 Hz",
        "1 kHz", "2 kHz", "4 kHz", "8 kHz", "16 kHz"
    };

    // 10 gain sliders
    QLabel *gainHeader = new QLabel("<b>Band Gains</b>");
    formLayout->addRow(gainHeader);
    for (int i = 0; i < 10; i++) {
        QString label = QString("%1 Gain").arg(bandLabels[i]);
        QString param = QString("band%1").arg(i + 1);
        addParameterSlider(formLayout, label, param,
                           eqSettings.gainMin, eqSettings.gainMax, 1.0, 0.01, 2);
    }

    // 10 Q sliders
    QLabel *qHeader = new QLabel("<b>Band Q (Width)</b>");
    formLayout->addRow(qHeader);
    for (int i = 0; i < 10; i++) {
        QString label = QString("%1 Q").arg(bandLabels[i]);
        QString param = QString("q%1").arg(i + 1);
        addParameterSlider(formLayout, label, param,
                           eqSettings.qMin, eqSettings.qMax, 2.0, 0.1, 1);
    }

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

void KalaMain::populateCombFilterInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &cfSettings = ContainerSettings::instance().combFilter;

    // Mode dropdown (not modulatable)
    QComboBox *comboMode = new QComboBox();
    comboMode->addItem("Feedforward (FIR - Notches)");
    comboMode->addItem("Feedback (IIR - Resonant)");
    comboMode->setCurrentIndex(static_cast<int>(currentContainer->getParameter("mode", 0.0)));
    formLayout->addRow("Mode:", comboMode);

    Container *container = currentContainer;
    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [container, this](int index) {
        container->setParameter("mode", static_cast<double>(index));
        sounitBuilder->rebuildGraph(currentEditingTrack);
    });

    // Parameter sliders
    addParameterSlider(formLayout, "Delay Time (ms)", "delayTime",
                       cfSettings.delayTimeMin, cfSettings.delayTimeMax, 5.0, 0.1, 1);
    addParameterSlider(formLayout, "Feedback", "feedback",
                       cfSettings.feedbackMin, cfSettings.feedbackMax, 0.5, 0.01, 2);
    addParameterSlider(formLayout, "Damping", "damping",
                       cfSettings.dampingMin, cfSettings.dampingMax, 0.0, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

void KalaMain::populateLowHighPassFilterInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &lpSettings = ContainerSettings::instance().lowHighPassFilter;

    // Mode dropdown (not modulatable)
    QComboBox *comboMode = new QComboBox();
    comboMode->addItem("Low Pass");
    comboMode->addItem("High Pass");
    comboMode->setCurrentIndex(static_cast<int>(currentContainer->getParameter("mode", 0.0)));
    formLayout->addRow("Mode:", comboMode);

    Container *container = currentContainer;
    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [container, this](int index) {
        container->setParameter("mode", static_cast<double>(index));
        sounitBuilder->rebuildGraph(currentEditingTrack);
    });

    // Parameter sliders
    addParameterSlider(formLayout, "Cutoff (Hz)", "cutoff",
                       lpSettings.cutoffMin, lpSettings.cutoffMax, 1000.0, 1.0, 0);
    addParameterSlider(formLayout, "Resonance (Q)", "resonance",
                       lpSettings.resonanceMin, lpSettings.resonanceMax, 0.707, 0.01, 2);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

void KalaMain::populateIRConvolutionInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &irSettings = ContainerSettings::instance().irConvolution;

    // File path label showing currently loaded IR
    QLabel *filePathLabel = new QLabel();
    filePathLabel->setStyleSheet("color: gray; font-size: 10px;");
    filePathLabel->setWordWrap(true);
    if (currentContainer->hasIRData()) {
        QString path = currentContainer->getIRFilePath();
        QString displayName = path.isEmpty() ? "(embedded data)" : QFileInfo(path).fileName();
        filePathLabel->setText("Loaded: " + displayName);
        filePathLabel->setVisible(true);
    } else {
        filePathLabel->setText("No IR loaded");
        filePathLabel->setVisible(true);
    }

    // Load IR button
    QPushButton *loadButton = new QPushButton("Load IR File...");
    Container *container = currentContainer;
    connect(loadButton, &QPushButton::clicked, this, [this, container, filePathLabel]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Load Impulse Response",
                                                         QString(), "WAV Files (*.wav)");
        if (!filePath.isEmpty()) {
            std::vector<float> wavData;
            if (WavetableSynth::loadWavFile(filePath, wavData)) {
                container->setIRData(wavData);
                container->setIRFilePath(filePath);
                filePathLabel->setText("Loaded: " + QFileInfo(filePath).fileName());
                int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
                sounitBuilder->rebuildGraph(trackIndex);
            } else {
                QMessageBox::warning(this, "Load Error",
                                    "Failed to load WAV file:\n" + filePath);
            }
        }
    });

    formLayout->addRow("IR File:", loadButton);
    formLayout->addRow("", filePathLabel);

    // Wet/Dry slider
    addParameterSlider(formLayout, "Wet/Dry Mix", "wetDry",
                       irSettings.wetDryMin, irSettings.wetDryMax, 0.5, 0.01, 2);

    // Pre-delay slider (0-500 ms)
    addParameterSlider(formLayout, "Pre-delay (ms)", "predelay",
                       irSettings.predelayMin, irSettings.predelayMax, 0.0, 1.0, 0);

    // Fade-in ramp slider (% of pre-delay)
    addParameterSlider(formLayout, "Fade-in (%)", "fadeInPct",
                       irSettings.fadeInPctMin, irSettings.fadeInPctMax, 10.0, 1.0, 0);

    // Decay trim slider (0.0-1.0) — inspector only, triggers graph rebuild
    addParameterSlider(formLayout, "Decay Trim", "decay",
                       irSettings.decayMin, irSettings.decayMax, 1.0, 0.01, 2);

    // Low Cut slider (20-2000 Hz)
    addParameterSlider(formLayout, "Low Cut (Hz)", "lowCut",
                       irSettings.lowCutMin, irSettings.lowCutMax, 20.0, 1.0, 0);

    // High Damp slider (200-20000 Hz)
    addParameterSlider(formLayout, "High Damp (Hz)", "highDamp",
                       irSettings.highDampMin, irSettings.highDampMax, 20000.0, 10.0, 0);

    // Capture Length slider (only relevant when irIn is connected)
    addParameterSlider(formLayout, "Capture Length (s)", "captureLength",
                       irSettings.captureLengthMin, irSettings.captureLengthMax, 1.0, 0.1, 1);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Note Tail Inspector
void KalaMain::populateNoteTailInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);

    // Tail Length: QHBoxLayout with QSpinBox (same pattern as addParameterSlider)
    QHBoxLayout *paramLayout = new QHBoxLayout();
    QSpinBox *spinTailLength = new QSpinBox();
    spinTailLength->setRange(1, 9999);
    spinTailLength->setSuffix(" ms");
    spinTailLength->setValue(static_cast<int>(currentContainer->getParameter("tailLength", 200.0)));
    connect(spinTailLength, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
        if (currentContainer) {
            currentContainer->setParameter("tailLength", static_cast<double>(value));
            int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
            sounitBuilder->rebuildGraph(trackIndex);
        }
    });
    paramLayout->addWidget(spinTailLength);
    paramLayout->addStretch();
    formLayout->addRow("Tail Length:", paramLayout);

    QLabel *descLabel = new QLabel("Cosine fade-out after note ends.\nPlace last in the signal chain.");
    descLabel->setStyleSheet("color: gray; font-size: 10px;");
    descLabel->setWordWrap(true);
    formLayout->addRow("", descLabel);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Recorder Inspector
void KalaMain::populateRecorderInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &rec = ContainerSettings::instance().recorder;

    addParameterSlider(formLayout, "Breath Pressure", "breathPressure",
                       rec.breathPressureMin, rec.breathPressureMax, 0.81, 0.01, 2);

    addParameterSlider(formLayout, "Jet Ratio", "jetRatio",
                       rec.jetRatioMin, rec.jetRatioMax, 0.35, 0.01, 2);

    addParameterSlider(formLayout, "Noise Gain", "noiseGain",
                       rec.noiseGainMin, rec.noiseGainMax, 0.015, 0.001, 3);

    addParameterSlider(formLayout, "Vibrato Freq (Hz)", "vibratoFreq",
                       rec.vibratoFreqMin, rec.vibratoFreqMax, 0.0, 0.1, 2);

    addParameterSlider(formLayout, "Vibrato Gain", "vibratoGain",
                       rec.vibratoGainMin, rec.vibratoGainMax, 0.0, 0.01, 2);

    addParameterSlider(formLayout, "End Reflection", "endReflection",
                       rec.endReflectionMin, rec.endReflectionMax, 0.73, 0.01, 2);

    addParameterSlider(formLayout, "Jet Reflection", "jetReflection",
                       rec.jetReflectionMin, rec.jetReflectionMax, 0.59, 0.01, 2);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescRec = new QLabel("1.0 = note pitch · 2.0 = octave up · 0.5 = octave down");
    pitchMultDescRec->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescRec);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Bowed Inspector
void KalaMain::populateBowedInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &b = ContainerSettings::instance().bowed;

    addParameterSlider(formLayout, "Bow Pressure", "bowPressure",
                       b.bowPressureMin, b.bowPressureMax, 0.75, 0.01, 2);

    addParameterSlider(formLayout, "Bow Velocity", "bowVelocity",
                       b.bowVelocityMin, b.bowVelocityMax, 0.5, 0.01, 2);

    addParameterSlider(formLayout, "Bow Position", "bowPosition",
                       b.bowPositionMin, b.bowPositionMax, 0.127, 0.001, 3);

    addParameterSlider(formLayout, "NL Type (0–4)", "nlType",
                       b.nlTypeMin, b.nlTypeMax, 0.0, 1.0, 0);

    addParameterSlider(formLayout, "NL Amount", "nlAmount",
                       b.nlAmountMin, b.nlAmountMax, 0.0, 0.01, 2);

    addParameterSlider(formLayout, "NL Freq Mod (Hz)", "nlFreqMod",
                       b.nlFreqModMin, b.nlFreqModMax, 10.0, 0.1, 1);

    addParameterSlider(formLayout, "NL Attack (s)", "nlAttack",
                       b.nlAttackMin, b.nlAttackMax, 0.1, 0.01, 2);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescBow = new QLabel("1.0 = note pitch · 2.0 = octave up · 0.5 = octave down");
    pitchMultDescBow->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescBow);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Reed Inspector
void KalaMain::populateReedInspector()
{
    if (!currentContainer) return;
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaConfigContents->layout());
    if (!mainLayout) return;

    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(6);

    const auto &s = ContainerSettings::instance().reed;

    addParameterSlider(formLayout, "Breath Pressure", "breathPressure",
                       s.breathPressureMin, s.breathPressureMax, 0.7, 0.01, 2);

    addParameterSlider(formLayout, "Reed Stiffness", "reedStiffness",
                       s.reedStiffnessMin, s.reedStiffnessMax, 0.5, 0.01, 2);

    addParameterSlider(formLayout, "Reed Aperture", "reedAperture",
                       s.reedApertureMin, s.reedApertureMax, 0.5, 0.01, 2);

    addParameterSlider(formLayout, "Blow Position", "blowPosition",
                       s.blowPositionMin, s.blowPositionMax, 0.2, 0.01, 2);

    addParameterSlider(formLayout, "Noise Gain", "noiseGain",
                       s.noiseGainMin, s.noiseGainMax, 0.2, 0.01, 2);

    addParameterSlider(formLayout, "Vibrato Freq (Hz)", "vibratoFreq",
                       s.vibratoFreqMin, s.vibratoFreqMax, 5.735, 0.1, 2);

    addParameterSlider(formLayout, "Vibrato Gain", "vibratoGain",
                       s.vibratoGainMin, s.vibratoGainMax, 0.0, 0.01, 2);

    addParameterSlider(formLayout, "NL Type (0–4)", "nlType",
                       s.nlTypeMin, s.nlTypeMax, 0.0, 1.0, 0);

    addParameterSlider(formLayout, "NL Amount", "nlAmount",
                       s.nlAmountMin, s.nlAmountMax, 0.0, 0.01, 2);

    addParameterSlider(formLayout, "NL Freq Mod (Hz)", "nlFreqMod",
                       s.nlFreqModMin, s.nlFreqModMax, 10.0, 0.1, 1);

    addParameterSlider(formLayout, "NL Attack (s)", "nlAttack",
                       s.nlAttackMin, s.nlAttackMax, 0.1, 0.01, 2);

    addParameterSlider(formLayout, "Pitch Multiplier", "pitchMultiplier", 0.25, 4.0, 1.0, 0.01, 2);
    QLabel *pitchMultDescReed = new QLabel("1.0 = note pitch · 2.0 = octave up · 0.5 = octave down");
    pitchMultDescReed->setStyleSheet("color: gray; font-size: 10px;");
    formLayout->addRow("", pitchMultDescReed);

    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();
}

// Update envelope parameters based on envelope type (contextual sliders)
void KalaMain::updateEnvelopeParameters(int envelopeType)
{
    if (!currentContainer || !envelopeParamsWidget || !envelopeViz) return;

    // Hide widget to prevent visual artifacts during rebuild
    envelopeParamsWidget->setVisible(false);

    // Clear existing parameters
    if (envelopeParamsWidget->layout()) {
        QLayout *oldLayout = envelopeParamsWidget->layout();
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->setVisible(false);
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete oldLayout;
    }

    // Create new layout for parameters
    QFormLayout *paramLayout = new QFormLayout(envelopeParamsWidget);
    paramLayout->setSpacing(10);
    paramLayout->setContentsMargins(0, 0, 0, 0);

    // Helper to create slider+spinbox and connect to preview
    auto addEnvelopeParam = [this, paramLayout](const QString &label, const QString &paramName,
                                                 double minVal, double maxVal, double defaultVal,
                                                 double step, int decimals) {
        if (!currentContainer) return;

        QHBoxLayout *layout = new QHBoxLayout();
        QSlider *slider = new QSlider(Qt::Horizontal);
        QDoubleSpinBox *spinBox = new QDoubleSpinBox();

        int sliderMin = static_cast<int>(minVal / step);
        int sliderMax = static_cast<int>(maxVal / step);
        int sliderVal = static_cast<int>(currentContainer->getParameter(paramName, defaultVal) / step);

        slider->setRange(sliderMin, sliderMax);
        slider->setValue(sliderVal);

        spinBox->setRange(minVal, maxVal);
        spinBox->setSingleStep(step);
        spinBox->setDecimals(decimals);
        spinBox->setValue(currentContainer->getParameter(paramName, defaultVal));

        layout->addWidget(slider);
        layout->addWidget(spinBox);

        // Connect slider and spinbox
        connect(slider, &QSlider::valueChanged, this, [this, spinBox, slider, paramName, step](int value) {
            double doubleVal = value * step;
            spinBox->blockSignals(true);
            spinBox->setValue(doubleVal);
            spinBox->blockSignals(false);
            if (currentContainer) {
                currentContainer->setParameter(paramName, doubleVal);
                updateEnvelopePreview();
                int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
                sounitBuilder->rebuildGraph(trackIndex);
            }
        });

        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this, slider, paramName, step](double value) {
            slider->blockSignals(true);
            slider->setValue(static_cast<int>(value / step));
            slider->blockSignals(false);
            if (currentContainer) {
                currentContainer->setParameter(paramName, value);
                updateEnvelopePreview();
                int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
                sounitBuilder->rebuildGraph(trackIndex);
            }
        });

        paramLayout->addRow(label + ":", layout);
    };

    // Add parameters based on envelope type
    QString envelopeName;
    switch (envelopeType) {
    case 0: // Linear - no parameters
        envelopeName = "Linear";
        break;

    case 1: // Attack-Decay
        envelopeName = "Attack-Decay";
        addEnvelopeParam("Attack Time (s)", "envAttack", 0.001, 2.0, 0.1, 0.001, 3);
        addEnvelopeParam("Decay Time (s)", "envDecay", 0.001, 2.0, 0.2, 0.001, 3);
        break;

    case 2: // ADSR
        envelopeName = "ADSR";
        addEnvelopeParam("Attack Time (s)", "envAttack", 0.001, 2.0, 0.1, 0.001, 3);
        addEnvelopeParam("Decay Time (s)", "envDecay", 0.001, 2.0, 0.1, 0.001, 3);
        addEnvelopeParam("Sustain Level", "envSustain", 0.0, 1.0, 0.7, 0.01, 2);
        addEnvelopeParam("Release Time (s)", "envRelease", 0.001, 3.0, 0.2, 0.001, 3);
        break;

    case 3: // Fade In
        envelopeName = "Fade In";
        addEnvelopeParam("Fade Time", "envFadeTime", 0.0, 1.0, 0.5, 0.01, 2);
        break;

    case 4: // Fade Out
        envelopeName = "Fade Out";
        addEnvelopeParam("Fade Time", "envFadeTime", 0.0, 1.0, 0.5, 0.01, 2);
        break;

    case 5: // Log Fade In - no parameters
        envelopeName = "Log Fade In";
        break;

    case 6: // Log Fade Out - no parameters
        envelopeName = "Log Fade Out";
        break;

    case 7: // Custom - no parameters (configured in library dialog)
        // Display custom envelope in visualizer
        if (currentContainer->hasCustomEnvelopeData() && envelopeViz) {
            EnvelopeData customData = currentContainer->getCustomEnvelopeData();
            envelopeViz->setCustomEnvelope(customData.points);
            envelopeName = customData.name.isEmpty() ? "Custom" : customData.name;
        } else {
            envelopeName = "Custom";
        }
        break;
    }

    // Update envelope name label
    if (envelopeNameLabel) {
        envelopeNameLabel->setText(envelopeName);
    }

    // Show widget again after rebuild
    envelopeParamsWidget->setVisible(true);

    // Always show envelope preview
    if (envelopeViz) {
        envelopeViz->setVisible(true);
    }

    // Update preview for non-custom envelopes
    if (envelopeType != 7) {
        updateEnvelopePreview();
    }
}

// Update envelope preview visualization
void KalaMain::updateEnvelopePreview()
{
    if (!currentContainer || !envelopeViz) return;

    // When followDynamics is on, show the selected score curve instead of the envelope shape.
    if (currentContainer->getParameter("followDynamics", 0.0) > 0.5) {
        int scoreIdx = static_cast<int>(currentContainer->getParameter("scoreCurveIndex", 0.0));
        const QVector<Note> &canvasNotes = scoreCanvasWindow->getScoreCanvas()->getPhrase().getNotes();

        // Find the first note that has a curve at the requested index
        const Curve *sourceCurve = nullptr;
        for (const Note &note : canvasNotes) {
            if (scoreIdx < note.getExpressiveCurveCount()) {
                sourceCurve = &note.getExpressiveCurve(scoreIdx);
                break;
            }
        }

        if (sourceCurve && !sourceCurve->isEmpty()) {
            // Convert Curve::Point to EnvelopePoint for the visualizer
            QVector<EnvelopePoint> vizPoints;
            for (const Curve::Point &pt : sourceCurve->getPoints()) {
                vizPoints.append(EnvelopePoint(pt.time, pt.value, 0));
            }
            envelopeViz->setCustomEnvelope(vizPoints);
        } else {
            // No curve data yet — show a flat line at 0.5 as placeholder
            QVector<EnvelopePoint> flat;
            flat.append(EnvelopePoint(0.0, 0.5, 0));
            flat.append(EnvelopePoint(1.0, 0.5, 0));
            envelopeViz->setCustomEnvelope(flat);
        }

        if (envelopeNameLabel) {
            QString name = (comboScoreCurve && comboScoreCurve->currentIndex() >= 0)
                           ? comboScoreCurve->currentText()
                           : "Score Curve";
            envelopeNameLabel->setText(name + " (score)");
        }
        return;
    }

    int envelopeType = static_cast<int>(currentContainer->getParameter("envelopeSelect", 0.0));
    double attack = currentContainer->getParameter("envAttack", 0.1);
    double decay = currentContainer->getParameter("envDecay", 0.2);
    double sustain = currentContainer->getParameter("envSustain", 0.7);
    double release = currentContainer->getParameter("envRelease", 0.2);
    double fadeTime = currentContainer->getParameter("envFadeTime", 0.5);

    envelopeViz->setEnvelope(envelopeType, attack, decay, sustain, release, fadeTime);
}

void KalaMain::closeEvent(QCloseEvent *event)
{
    // Check for unsaved changes before closing
    if (!checkUnsavedChanges()) {
        event->ignore();
        return;
    }

    // Close SounitBuilder when main window closes (no longer a child)
    if (sounitBuilder) {
        sounitBuilder->close();
    }

    // Close ScoreCanvasWindow when main window closes
    if (scoreCanvasWindow) {
        scoreCanvasWindow->close();
    }

    // Accept the close event
    QMainWindow::closeEvent(event);
}

bool KalaMain::eventFilter(QObject *watched, QEvent *event)
{
    // Catch Tab key from any of our windows
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() == Qt::NoModifier) {
            // Check if any of our windows is active
            bool mainActive = isActiveWindow();
            bool sounitActive = sounitBuilder && sounitBuilder->isActiveWindow();
            bool scoreActive = scoreCanvasWindow && scoreCanvasWindow->isActiveWindow();

            if (mainActive || sounitActive || scoreActive) {
                // Only handle if we're on Sound Engine or Composition tab
                int currentTab = ui->MainTab->currentIndex();
                if (currentTab == 0 || currentTab == 1) {
                    toggleCanvasFocus();
                    return true;  // Event handled, don't propagate
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void KalaMain::toggleCanvasFocus()
{
    int currentTab = ui->MainTab->currentIndex();

    // Do nothing for Settings tab (index 2)
    if (currentTab == 2) {
        return;
    }

    // Determine which canvas window is relevant
    QMainWindow *canvasWindow = nullptr;
    bool *wasMaximized = nullptr;
    if (currentTab == 0) {
        canvasWindow = sounitBuilder;
        wasMaximized = &m_sounitBuilderWasMaximized;
    } else if (currentTab == 1) {
        canvasWindow = scoreCanvasWindow;
        wasMaximized = &m_scoreCanvasWasMaximized;
    }

    if (!canvasWindow || !wasMaximized) {
        return;
    }

    // Check which window is currently active
    bool canvasIsActive = canvasWindow->isActiveWindow();

    if (canvasIsActive) {
        // Canvas is active - switch to main window
        *wasMaximized = canvasWindow->isMaximized();

        // Lower the canvas window so main window can come in front
        // This is especially important for sounitBuilder which is parented to main window
        canvasWindow->lower();

        // Restore main window's maximized state
        if (m_mainWindowWasMaximized) {
            showMaximized();
        } else {
            showNormal();
        }
        raise();
        activateWindow();
        setFocus();
    } else {
        // Main window is active - switch to canvas
        // Save main window's maximized state before switching
        m_mainWindowWasMaximized = isMaximized();

        if (*wasMaximized) {
            canvasWindow->showMaximized();
        } else {
            canvasWindow->showNormal();
        }
        canvasWindow->raise();
        canvasWindow->activateWindow();
        canvasWindow->setFocus();
    }
}

// ============================================================================
// Project File Management
// ============================================================================

void KalaMain::updateWindowTitle()
{
    QString title = "Kala";

    // Use composition name from settings as the project name
    CompositionSettings settings = scoreCanvasWindow->getSettings();
    QString projectName = settings.compositionName;

    if (!projectName.isEmpty() && projectName != "Untitled") {
        title = projectName + " - Kala";
    } else if (!m_currentProjectPath.isEmpty()) {
        // Fall back to file name if no composition name set
        QFileInfo fi(m_currentProjectPath);
        title = fi.baseName() + " - Kala";
    } else {
        title = "Untitled - Kala";
    }

    if (m_projectDirty) {
        title = "* " + title;
    }

    setWindowTitle(title);
}

void KalaMain::markProjectDirty()
{
    if (!m_projectDirty) {
        m_projectDirty = true;
        updateWindowTitle();
    }
}

bool KalaMain::checkUnsavedChanges()
{
    if (!m_projectDirty) {
        return true;  // No unsaved changes, proceed
    }

    QMessageBox::StandardButton result = QMessageBox::warning(
        this,
        "Unsaved Changes",
        "The project has unsaved changes. Do you want to save before continuing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );

    if (result == QMessageBox::Save) {
        onSaveProject();
        return !m_projectDirty;  // Return true only if save succeeded
    } else if (result == QMessageBox::Discard) {
        return true;  // Discard changes, proceed
    } else {
        return false;  // Cancel, don't proceed
    }
}

void KalaMain::newProject()
{
    // Detach SounitBuilder from current canvas before clearing tracks
    sounitBuilder->setTrackCanvas(nullptr);

    // Clear all tracks except one default track
    trackManager->clearAllTracks();
    Track *defaultTrack = trackManager->addTrack("Track 1", QColor("#3498db"));

    // Update SounitBuilder to show the new default track's canvas
    sounitBuilder->setTrackCanvas(defaultTrack);

    // Reconnect canvas signals
    Canvas *canvas = defaultTrack->getCanvas();
    connect(canvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
    connect(canvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
    connect(canvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
    connect(canvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
    connect(canvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);

    // Clear sounit tracking maps
    trackSounitFiles.clear();
    trackSounitNames.clear();
    trackCanvasStates.clear();
    trackContainers.clear();
    trackConnections.clear();
    currentEditingTrack = -1;

    // Clear notes from ScoreCanvas
    scoreCanvasWindow->getScoreCanvas()->clearNotes();

    // Reset scale to default
    scoreCanvasWindow->getScoreCanvas()->clearScaleChanges();
    scoreCanvasWindow->getScoreCanvas()->setScale(Scale::justIntonation());
    scoreCanvasWindow->getScoreCanvas()->setBaseFrequency(25.0);

    // Clear timeline scale markers and update display
    scoreCanvasWindow->getTimeline()->setScaleChanges(QMap<double, QString>());
    updateScaleDisplay();

    // Reset tempo/time signature to defaults
    ScoreCanvas *sc = scoreCanvasWindow->getScoreCanvas();
    sc->clearTempoChanges();
    sc->setDefaultTempo(120.0);
    sc->setDefaultTimeSignature(4, 4);

    // Clear timeline tempo markers
    Timeline *timeline = scoreCanvasWindow->getTimeline();
    timeline->setTempoChanges(QMap<double, QString>());

    // Update tempo display
    updateTempoDisplay();

    // Reset composition settings to defaults
    CompositionSettings defaultSettings = CompositionSettings::defaults();
    scoreCanvasWindow->updateFromSettings(defaultSettings);

    // Refresh the settings tab to show reset values
    populateSettingsTab();

    // Reset track selector
    scoreCanvasWindow->getTrackSelector()->clearTracks();
    scoreCanvasWindow->getTrackSelector()->addTrack("(No Sounit)", QColor("#999999"), 25.0, 2000.0);

    // Rebuild mixer
    rebuildMixer();

    // Update sounit selector and variation selector
    updateSounitSelector();
    updateVariationSelector();

    // Clear sounit inspector fields
    ui->editSounitName->blockSignals(true);
    ui->editSounitName->setText("Untitled Sounit");
    ui->editSounitName->blockSignals(false);
    ui->editSounitComment->blockSignals(true);
    ui->editSounitComment->clear();
    ui->editSounitComment->blockSignals(false);

    // Clear container/connection inspector
    onSelectionCleared();

    // Clear project state
    m_currentProjectPath.clear();
    m_projectDirty = false;
    updateWindowTitle();

    qDebug() << "New project created";
}

void KalaMain::onNewProject()
{
    if (!checkUnsavedChanges()) {
        return;
    }

    newProject();
}

void KalaMain::onOpenProject()
{
    if (!checkUnsavedChanges()) {
        return;
    }

    QSettings settings;
    QString lastDir = settings.value("lastDirectory/project", QString()).toString();

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Project",
        lastDir,
        "Kala Project (*.kala);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Remember the directory for next time
    settings.setValue("lastDirectory/project", QFileInfo(filePath).absolutePath());

    if (loadProject(filePath)) {
        m_currentProjectPath = filePath;
        m_projectDirty = false;
        updateWindowTitle();
    }
}

void KalaMain::onSaveProject()
{
    if (m_currentProjectPath.isEmpty()) {
        onSaveProjectAs();
        return;
    }

    // Check if project name has changed from the current filename
    CompositionSettings settings = scoreCanvasWindow->getSettings();
    QString projectName = settings.compositionName;
    QFileInfo fi(m_currentProjectPath);
    QString currentBaseName = fi.baseName();

    // If project name differs from filename, save to new file with new name
    if (!projectName.isEmpty() && projectName != "Untitled" && projectName != currentBaseName) {
        // Sanitize the new name for use as filename
        QString newName = projectName;
        newName.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");

        QString newPath = fi.absolutePath() + "/" + newName + ".kala";

        if (saveProject(newPath)) {
            m_currentProjectPath = newPath;
            m_projectDirty = false;
            updateWindowTitle();
        }
    } else {
        // Save to existing path
        if (saveProject(m_currentProjectPath)) {
            m_projectDirty = false;
            updateWindowTitle();
        }
    }
}

void KalaMain::onSaveProjectAs()
{
    // Get suggested filename from project name in settings
    CompositionSettings settings = scoreCanvasWindow->getSettings();
    QString suggestedName = settings.compositionName;
    if (suggestedName.isEmpty() || suggestedName == "Untitled") {
        suggestedName = "untitled";
    }
    // Sanitize filename
    suggestedName.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");

    QSettings appSettings;
    QString defaultPath;
    if (!m_currentProjectPath.isEmpty()) {
        // Use existing directory but with new name
        QFileInfo fi(m_currentProjectPath);
        defaultPath = fi.absolutePath() + "/" + suggestedName + ".kala";
    } else {
        // Use last used directory
        QString lastDir = appSettings.value("lastDirectory/project", QString()).toString();
        if (!lastDir.isEmpty()) {
            defaultPath = lastDir + "/" + suggestedName + ".kala";
        } else {
            defaultPath = suggestedName + ".kala";
        }
    }

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Project As",
        defaultPath,
        "Kala Project (*.kala);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Remember the directory for next time
    appSettings.setValue("lastDirectory/project", QFileInfo(filePath).absolutePath());

    // Ensure .kala extension
    if (!filePath.endsWith(".kala", Qt::CaseInsensitive)) {
        filePath += ".kala";
    }

    if (saveProject(filePath)) {
        m_currentProjectPath = filePath;
        m_projectDirty = false;
        updateWindowTitle();
    }
}

void KalaMain::onExportAudio()
{
    // Validate: Check if tracks exist
    if (trackManager->getTrackCount() == 0) {
        QMessageBox::warning(this, "Export Audio",
                             "No tracks to export. Please add a track first.");
        return;
    }

    // Get composition settings
    CompositionSettings settings = scoreCanvasWindow->getSettings();
    double lengthMs = settings.lengthMs;
    unsigned int sampleRate = static_cast<unsigned int>(settings.sampleRate);
    int bitDepth = settings.bitDepth;

    // Get project name for default filename
    QString projectName = settings.compositionName;
    if (projectName.isEmpty()) {
        projectName = "Untitled";
    }

    // Show export dialog
    ExportAudioDialog dialog(projectName, trackManager->getTrackCount(), sampleRate, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    ExportAudioDialog::ExportMode mode = dialog.getExportMode();
    QString exportPath = dialog.getExportPath();

    if (exportPath.isEmpty()) {
        QMessageBox::warning(this, "Export Audio", "Please specify an export path.");
        return;
    }

    // Pre-render all dirty notes on all tracks
    qDebug() << "Pre-rendering tracks for export...";
    for (int i = 0; i < trackManager->getTrackCount(); ++i) {
        Track *track = trackManager->getTrack(i);
        if (track) {
            track->prerenderDirtyNotes(sampleRate);
        }
    }

    // Calculate export end time: use the maximum of configured length and actual rendered content.
    // Playback already does this dynamically; export must do the same so notes beyond the
    // composition length setting are not silently truncated.
    double renderedEndMs = 0.0;
    for (int i = 0; i < trackManager->getTrackCount(); ++i) {
        Track *track = trackManager->getTrack(i);
        if (track) {
            double trackEnd = track->getRenderedEndTimeMs();
            if (trackEnd > renderedEndMs) renderedEndMs = trackEnd;
        }
    }
    double exportLengthMs = std::max(lengthMs, renderedEndMs);

    // Calculate total mono samples needed
    size_t monoSamples = static_cast<size_t>((exportLengthMs / 1000.0) * sampleRate);

    bool success = true;
    QStringList exportedFiles;

    // Helper lambda to convert mono buffer to interleaved stereo
    auto monoToStereo = [](const std::vector<float> &mono) -> std::vector<float> {
        std::vector<float> stereo(mono.size() * 2);
        for (size_t i = 0; i < mono.size(); ++i) {
            stereo[i * 2] = mono[i];      // Left channel
            stereo[i * 2 + 1] = mono[i];  // Right channel (same as left)
        }
        return stereo;
    };

    if (mode == ExportAudioDialog::SingleFile) {
        // Single file mode: Mix all unmuted tracks together (mono first)
        std::vector<float> masterBuffer(monoSamples, 0.0f);

        // Check for solo tracks
        bool hasSolo = false;
        for (int i = 0; i < trackManager->getTrackCount(); ++i) {
            Track *track = trackManager->getTrack(i);
            if (track && track->isSolo()) {
                hasSolo = true;
                break;
            }
        }

        // Mix tracks (mono)
        for (int i = 0; i < trackManager->getTrackCount(); ++i) {
            Track *track = trackManager->getTrack(i);
            if (!track) continue;

            // Skip muted tracks, or skip non-solo tracks if any track is soloed
            if (track->isMuted()) continue;
            if (hasSolo && !track->isSolo()) continue;

            std::vector<float> trackBuffer = track->getMixedBuffer(0, exportLengthMs);
            float trackVolume = track->getVolume() * track->getGain();

            // Mix into master buffer
            size_t samplesToMix = std::min(masterBuffer.size(), trackBuffer.size());
            for (size_t s = 0; s < samplesToMix; ++s) {
                masterBuffer[s] += trackBuffer[s] * trackVolume;
            }
        }

        // Clamp master buffer to [-1.0, 1.0]
        for (size_t s = 0; s < masterBuffer.size(); ++s) {
            masterBuffer[s] = std::clamp(masterBuffer[s], -1.0f, 1.0f);
        }

        // Convert mono to stereo for WAV output
        std::vector<float> stereoBuffer = monoToStereo(masterBuffer);

        // Ensure .wav extension
        QString filePath = exportPath;
        if (!filePath.endsWith(".wav", Qt::CaseInsensitive)) {
            filePath += ".wav";
        }

        // Write WAV file (stereo)
        if (writeWavFile(filePath, stereoBuffer, sampleRate, 2, bitDepth)) {
            exportedFiles << filePath;
        } else {
            success = false;
        }
    } else {
        // Separate files mode: Export each track individually
        for (int i = 0; i < trackManager->getTrackCount(); ++i) {
            Track *track = trackManager->getTrack(i);
            if (!track) continue;

            std::vector<float> trackBuffer = track->getMixedBuffer(0, exportLengthMs);
            float trackVolume = track->getVolume() * track->getGain();

            // Apply volume and clamp
            for (size_t s = 0; s < trackBuffer.size(); ++s) {
                trackBuffer[s] = std::clamp(trackBuffer[s] * trackVolume, -1.0f, 1.0f);
            }

            // Convert mono to stereo for WAV output
            std::vector<float> stereoBuffer = monoToStereo(trackBuffer);

            // Generate filename: ProjectName_TrackNumber_TrackName.wav
            QString trackName = track->getName();
            trackName.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");  // Sanitize filename
            QString filePath = QString("%1/%2_%3_%4.wav").arg(exportPath, projectName, QString::number(i + 1), trackName);

            if (writeWavFile(filePath, stereoBuffer, sampleRate, 2, bitDepth)) {
                exportedFiles << filePath;
            } else {
                success = false;
            }
        }
    }

    // Show result
    if (success && !exportedFiles.isEmpty()) {
        QString message = QString("Audio exported successfully!\n\nFiles created:\n%1")
                              .arg(exportedFiles.join("\n"));
        QMessageBox::information(this, "Export Audio", message);
    } else if (!exportedFiles.isEmpty()) {
        QString message = QString("Some files were exported:\n%1\n\nSome exports failed.")
                              .arg(exportedFiles.join("\n"));
        QMessageBox::warning(this, "Export Audio", message);
    } else {
        QMessageBox::critical(this, "Export Audio", "Export failed. Could not write audio files.");
    }
}

bool KalaMain::writeWavFile(const QString &filePath, const std::vector<float> &samples,
                                unsigned int sampleRate, int channels, int bitsPerSample)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    int bytesPerSample = bitsPerSample / 8;  // 3 for 24-bit
    uint32_t dataSize = static_cast<uint32_t>(samples.size()) * bytesPerSample;
    uint32_t fileSize = 36 + dataSize;

    // Helper lambda to write little-endian values
    auto writeLE16 = [&file](uint16_t value) {
        char bytes[2];
        bytes[0] = static_cast<char>(value & 0xFF);
        bytes[1] = static_cast<char>((value >> 8) & 0xFF);
        file.write(bytes, 2);
    };

    auto writeLE32 = [&file](uint32_t value) {
        char bytes[4];
        bytes[0] = static_cast<char>(value & 0xFF);
        bytes[1] = static_cast<char>((value >> 8) & 0xFF);
        bytes[2] = static_cast<char>((value >> 16) & 0xFF);
        bytes[3] = static_cast<char>((value >> 24) & 0xFF);
        file.write(bytes, 4);
    };

    // RIFF header
    file.write("RIFF", 4);
    writeLE32(fileSize);
    file.write("WAVE", 4);

    // fmt chunk
    file.write("fmt ", 4);
    writeLE32(16);  // Chunk size (16 for PCM)
    writeLE16(1);   // Audio format (1 = PCM)
    writeLE16(static_cast<uint16_t>(channels));
    writeLE32(sampleRate);
    writeLE32(sampleRate * channels * bytesPerSample);  // Byte rate
    writeLE16(static_cast<uint16_t>(channels * bytesPerSample));  // Block align
    writeLE16(static_cast<uint16_t>(bitsPerSample));

    // data chunk
    file.write("data", 4);
    writeLE32(dataSize);

    // Write sample data (convert float to 24-bit signed int)
    for (size_t i = 0; i < samples.size(); ++i) {
        // Clamp and scale to 24-bit range
        float sample = std::clamp(samples[i], -1.0f, 1.0f);
        int32_t intSample = static_cast<int32_t>(sample * 8388607.0f);  // 2^23 - 1

        // Write 24-bit value in little-endian (3 bytes)
        char bytes[3];
        bytes[0] = static_cast<char>(intSample & 0xFF);
        bytes[1] = static_cast<char>((intSample >> 8) & 0xFF);
        bytes[2] = static_cast<char>((intSample >> 16) & 0xFF);
        file.write(bytes, 3);
    }

    file.close();
    qDebug() << "WAV file written:" << filePath << "(" << samples.size() << "samples)";
    return true;
}

bool KalaMain::saveProject(const QString &filePath)
{
    qDebug() << "Saving project to:" << filePath;

    QJsonObject projectJson;

    // Save file format version
    projectJson["version"] = 1;
    projectJson["fileType"] = "kala-project";

    // Save composition settings from ScoreCanvasWindow
    CompositionSettings settings = scoreCanvasWindow->getSettings();
    projectJson["compositionSettings"] = settings.toJson();

    // Save container settings (preferences)
    projectJson["containerSettings"] = ContainerSettings::instance().toJson();

    // Save scale settings
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    QJsonObject scaleJson;
    scaleJson["defaultScale"] = scoreCanvas->getCurrentScale().toJson();
    scaleJson["baseFrequency"] = scoreCanvas->getBaseFrequency();

    // Save scale changes (modulations)
    QJsonArray scaleChangesArray;
    QMap<double, QPair<Scale, double>> scaleChanges = scoreCanvas->getScaleChanges();
    for (auto it = scaleChanges.constBegin(); it != scaleChanges.constEnd(); ++it) {
        QJsonObject changeJson;
        changeJson["timeMs"] = it.key();
        changeJson["scale"] = it.value().first.toJson();
        changeJson["baseFrequency"] = it.value().second;
        scaleChangesArray.append(changeJson);
    }
    scaleJson["scaleChanges"] = scaleChangesArray;
    projectJson["scaleSettings"] = scaleJson;

    // Save tempo settings
    QJsonObject tempoJson;
    tempoJson["defaultTempo"] = scoreCanvas->getDefaultTempo();
    tempoJson["defaultTimeSigNum"] = scoreCanvas->getDefaultTimeSigNum();
    tempoJson["defaultTimeSigDenom"] = scoreCanvas->getDefaultTimeSigDenom();

    // Save tempo changes
    QJsonArray tempoChangesArray;
    QMap<double, TempoTimeSignature> tempoChangesMap = scoreCanvas->getTempoChanges();
    for (auto it = tempoChangesMap.constBegin(); it != tempoChangesMap.constEnd(); ++it) {
        QJsonObject changeJson;
        changeJson["timeMs"] = it.key();
        changeJson["tempoTimeSignature"] = it.value().toJson();
        tempoChangesArray.append(changeJson);
    }
    tempoJson["tempoChanges"] = tempoChangesArray;
    projectJson["tempoSettings"] = tempoJson;

    // Save current canvas state before serializing
    saveCurrentCanvasState(currentEditingTrack);

    // Save canvas states for all tracks
    QJsonObject canvasStatesJson;
    for (auto it = trackCanvasStates.constBegin(); it != trackCanvasStates.constEnd(); ++it) {
        canvasStatesJson[QString::number(it.key())] = it.value();
    }
    projectJson["trackCanvasStates"] = canvasStatesJson;

    // Save tracks
    QJsonArray tracksArray;
    for (int i = 0; i < trackManager->getTrackCount(); ++i) {
        Track *track = trackManager->getTrack(i);
        if (track) {
            tracksArray.append(track->toJson());
        }
    }
    projectJson["tracks"] = tracksArray;

    // Save notes (from ScoreCanvas phrase)
    QJsonArray notesArray;
    const Phrase &phrase = scoreCanvas->getPhrase();
    for (const Note &note : phrase.getNotes()) {
        notesArray.append(note.toJson());
    }
    projectJson["notes"] = notesArray;

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Save Error",
                              "Could not save project to:\n" + filePath + "\n\n" + file.errorString());
        return false;
    }

    QJsonDocument doc(projectJson);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    // Save render cache alongside project file
    QString renderPath = filePath;
    renderPath.replace(".kala", ".kalarender");
    RenderCache::saveToFile(renderPath, trackManager->getTracks(), audioEngine->getSampleRate());

    qDebug() << "Project saved successfully:" << tracksArray.size() << "tracks," << notesArray.size() << "notes";
    return true;
}

bool KalaMain::loadProject(const QString &filePath)
{
    qDebug() << "Loading project from:" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Load Error",
                              "Could not open project file:\n" + filePath + "\n\n" + file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "Load Error",
                              "Invalid project file:\n" + parseError.errorString());
        return false;
    }

    QJsonObject projectJson = doc.object();

    // Verify file type
    if (projectJson["fileType"].toString() != "kala-project") {
        QMessageBox::critical(this, "Load Error", "Not a valid Kala project file.");
        return false;
    }

    // Clear existing state (but don't create a new default track yet)
    trackSounitFiles.clear();
    trackSounitNames.clear();
    trackCanvasStates.clear();
    trackContainers.clear();
    trackConnections.clear();
    currentEditingTrack = -1;

    // Clear notes from ScoreCanvas
    scoreCanvasWindow->getScoreCanvas()->clearNotes();

    // Reset scale to default (will be overwritten if file has scale settings)
    scoreCanvasWindow->getScoreCanvas()->clearScaleChanges();

    // Clear project state
    m_currentProjectPath.clear();
    m_projectDirty = false;

    // Load composition settings
    if (projectJson.contains("compositionSettings")) {
        CompositionSettings settings = CompositionSettings::fromJson(projectJson["compositionSettings"].toObject());
        scoreCanvasWindow->updateFromSettings(settings);
    }

    // Load container settings (preferences)
    if (projectJson.contains("containerSettings")) {
        ContainerSettings::instance().fromJson(projectJson["containerSettings"].toObject());
        // Refresh the preferences tab UI to show loaded values
        populateSettingsTab();
    }

    // Load scale settings
    if (projectJson.contains("scaleSettings")) {
        QJsonObject scaleJson = projectJson["scaleSettings"].toObject();

        // Load default scale
        Scale defaultScale = Scale::fromJson(scaleJson["defaultScale"].toObject());
        double baseFreq = scaleJson["baseFrequency"].toDouble(25.0);
        scoreCanvasWindow->getScoreCanvas()->setScale(defaultScale);
        scoreCanvasWindow->getScoreCanvas()->setBaseFrequency(baseFreq);

        // Load scale changes (modulations)
        QJsonArray scaleChangesArray = scaleJson["scaleChanges"].toArray();
        for (const QJsonValue &val : scaleChangesArray) {
            QJsonObject changeJson = val.toObject();
            double timeMs = changeJson["timeMs"].toDouble();
            Scale scale = Scale::fromJson(changeJson["scale"].toObject());
            double bf = changeJson["baseFrequency"].toDouble(25.0);
            scoreCanvasWindow->getScoreCanvas()->addScaleChange(timeMs, scale, bf);
        }

        // Update timeline scale markers
        Timeline *timeline = scoreCanvasWindow->getTimeline();
        QMap<double, QPair<Scale, double>> scaleChangesMap = scoreCanvasWindow->getScoreCanvas()->getScaleChanges();
        QMap<double, QString> scaleNamesForTimeline;
        for (auto it = scaleChangesMap.constBegin(); it != scaleChangesMap.constEnd(); ++it) {
            scaleNamesForTimeline[it.key()] = it.value().first.getName();
        }
        timeline->setScaleChanges(scaleNamesForTimeline);

        // Update scale display
        updateScaleDisplay();
    }

    // Load tempo settings
    scoreCanvasWindow->getScoreCanvas()->clearTempoChanges();  // Clear any existing tempo changes

    if (projectJson.contains("tempoSettings")) {
        QJsonObject tempoJson = projectJson["tempoSettings"].toObject();
        ScoreCanvas *sc = scoreCanvasWindow->getScoreCanvas();

        // Load default tempo/time signature
        double defaultTempo = tempoJson["defaultTempo"].toDouble(120.0);
        int defaultTimeSigNum = tempoJson["defaultTimeSigNum"].toInt(4);
        int defaultTimeSigDenom = tempoJson["defaultTimeSigDenom"].toInt(4);
        sc->setDefaultTempo(defaultTempo);
        sc->setDefaultTimeSignature(defaultTimeSigNum, defaultTimeSigDenom);

        // Load tempo changes
        QJsonArray tempoChangesArray = tempoJson["tempoChanges"].toArray();
        for (const QJsonValue &val : tempoChangesArray) {
            QJsonObject changeJson = val.toObject();
            double timeMs = changeJson["timeMs"].toDouble();
            TempoTimeSignature tts = TempoTimeSignature::fromJson(changeJson["tempoTimeSignature"].toObject());
            sc->addTempoChange(timeMs, tts);
        }

        // Update timeline tempo markers
        Timeline *timeline = scoreCanvasWindow->getTimeline();
        QMap<double, TempoTimeSignature> tempoChangesMap = sc->getTempoChanges();
        QMap<double, QString> tempoDescsForTimeline;
        for (auto it = tempoChangesMap.constBegin(); it != tempoChangesMap.constEnd(); ++it) {
            QString desc = QString("%1").arg(static_cast<int>(it.value().bpm));
            if (it.value().gradualTransition) {
                desc += "~";
            }
            tempoDescsForTimeline[it.key()] = desc;
        }
        timeline->setTempoChanges(tempoDescsForTimeline);

        // Update tempo display
        updateTempoDisplay();
    }

    // Load tracks
    QJsonArray tracksArray = projectJson["tracks"].toArray();

    // Detach SounitBuilder from current canvas before clearing tracks
    sounitBuilder->setTrackCanvas(nullptr);

    // Now safe to clear all tracks
    trackManager->clearAllTracks();
    scoreCanvasWindow->getTrackSelector()->clearTracks();

    for (int i = 0; i < tracksArray.size(); ++i) {
        QJsonObject trackJson = tracksArray[i].toObject();
        QString trackName = trackJson["name"].toString("Track " + QString::number(i + 1));
        QColor trackColor(trackJson["color"].toString("#3498db"));

        Track *track = trackManager->addTrack(trackName, trackColor);
        track->fromJson(trackJson);

        // Update track selector UI
        scoreCanvasWindow->getTrackSelector()->addTrack(
            track->getSounitName().isEmpty() ? trackName : track->getSounitName(),
            trackColor,
            25.0, 2000.0
        );

        // Track sounit file references
        QString sounitPath = trackJson["sounitFilePath"].toString();
        if (!sounitPath.isEmpty()) {
            trackSounitFiles[i] = sounitPath;
            trackSounitNames[i] = track->getSounitName();
        }
    }

    // Load canvas states for all tracks
    trackCanvasStates.clear();
    if (projectJson.contains("trackCanvasStates")) {
        QJsonObject canvasStatesJson = projectJson["trackCanvasStates"].toObject();
        for (auto it = canvasStatesJson.constBegin(); it != canvasStatesJson.constEnd(); ++it) {
            int trackIndex = it.key().toInt();
            trackCanvasStates[trackIndex] = it.value().toObject();
        }
        qDebug() << "Loaded canvas states for" << trackCanvasStates.size() << "tracks";
    }

    // If no tracks were loaded, add a default one
    if (trackManager->getTrackCount() == 0) {
        Track *defaultTrack = trackManager->addTrack("Track 1", QColor("#3498db"));
        scoreCanvasWindow->getTrackSelector()->addTrack("(No Sounit)", QColor("#999999"), 25.0, 2000.0);
    }

    // Update SounitBuilder to show first track's canvas
    Track *firstTrack = trackManager->getTrack(0);
    if (firstTrack) {
        sounitBuilder->setTrackCanvas(firstTrack);
        currentEditingTrack = 0;
        trackManager->setCurrentTrack(0);  // Sync TrackManager
        sounitBuilder->setCurrentEditingTrack(0);

        // Reconnect canvas signals
        Canvas *canvas = firstTrack->getCanvas();
        connect(canvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
        connect(canvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
        connect(canvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
        connect(canvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
        connect(canvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);

        // Restore canvas state with saved container parameters
        loadCanvasStateForTrack(0);

        // Process events to ensure deleteLater() completes for old containers
        // before rebuilding the graph (otherwise findChildren may find both old and new)
        QCoreApplication::processEvents();

        // CRITICAL: Rebuild the graph after loading canvas state
        // loadCanvasStateForTrack() deletes old containers and creates new ones,
        // which invalidates the graph's container pointers that were set during
        // fromJson()/loadSounit(). Without this rebuild, the graph has dangling
        // pointers that cause crashes when rendering new notes during playback.
        //
        // Preserve graphHash so the render cache remains valid - the canvas state
        // matches what was saved, so the cached renders are still correct.
        uint64_t savedGraphHash = firstTrack->getGraphHash();
        firstTrack->rebuildGraph(audioEngine->getSampleRate());
        firstTrack->setGraphHash(savedGraphHash);
    }

    // Load notes
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    scoreCanvas->clearNotes();

    QJsonArray notesArray = projectJson["notes"].toArray();
    Phrase &phrase = scoreCanvas->getPhrase();
    for (const QJsonValue &val : notesArray) {
        Note note = Note::fromJson(val.toObject());
        phrase.addNote(note);
    }

    // Load render cache if available
    QString renderPath = filePath;
    renderPath.replace(".kala", ".kalarender");
    int loadedRenders = RenderCache::loadFromFile(renderPath, trackManager->getTracks());
    if (loadedRenders >= 0) {
        qDebug() << "Loaded" << loadedRenders << "cached renders";
    }

    // Rebuild UI
    rebuildMixer();
    updateSounitSelector();
    updateVariationSelector();
    refreshVariationsList();
    updateWindowTitle();

    // CRITICAL: Reset sounit dirty flag for all tracks after loading
    // This is needed because loadCanvasStateForTrack can trigger canvas signals
    // after Track::fromJson has already cleared its loading flag
    for (int i = 0; i < trackManager->getTrackCount(); ++i) {
        Track *track = trackManager->getTrack(i);
        if (track) {
            track->clearSounitDirty();
        }
    }

    qDebug() << "Project loaded successfully:" << trackManager->getTrackCount() << "tracks,"
             << phrase.getNotes().size() << "notes";

    return true;
}

// ============================================================================
// Sounit Navigation
// ============================================================================

void KalaMain::updateSounitSelector()
{

    qDebug() << "=== DEBUG updateSounitSelector ===";
    qDebug() << "trackSounitNames map contents:";
    for (auto it = trackSounitNames.constBegin(); it != trackSounitNames.constEnd(); ++it) {
        qDebug() << "  Track" << it.key() << "->" << it.value();
    }

    // Update the combobox to show all loaded sounits
    ui->comboSounitSelector->blockSignals(true);

    // Clear and repopulate
    ui->comboSounitSelector->clear();

    // Add all tracks that have sounits loaded
    int indexToSelect = -1;
    for (auto it = trackSounitNames.constBegin(); it != trackSounitNames.constEnd(); ++it) {
        int trackIndex = it.key();
        QString sounitName = it.value();
        QString displayText = QString("Track %1: %2").arg(trackIndex).arg(sounitName);
        ui->comboSounitSelector->addItem(displayText, trackIndex);

        // Remember the index if this matches the current editing track
        if (trackIndex == currentEditingTrack) {
            indexToSelect = ui->comboSounitSelector->count() - 1;
        }
    }

    // Set the selection to match the current editing track
    if (indexToSelect >= 0) {
        ui->comboSounitSelector->setCurrentIndex(indexToSelect);
    }

    ui->comboSounitSelector->blockSignals(false);

    qDebug() << "SounitSelector: Updated with" << trackSounitNames.size() << "sounits (" << ui->comboSounitSelector->count() << "items)";
}

void KalaMain::saveCurrentCanvasState(int trackIndex)
{
    Canvas *canvas = sounitBuilder->getCanvas();
    if (!canvas) {
        qWarning() << "Cannot save canvas state: canvas is null";
        return;
    }

    QJsonObject state;

    // Save sounit metadata
    QJsonObject sounitMeta;
    sounitMeta["name"] = canvas->getSounitName();
    sounitMeta["comment"] = canvas->getSounitComment();
    state["sounit"] = sounitMeta;

    // Serialize containers
    QJsonArray containersArray;
    QList<Container*> containers = canvas->findChildren<Container*>();
    for (const Container *container : containers) {
        QJsonObject containerJson;
        containerJson["type"] = container->getName();
        containerJson["instanceName"] = container->getInstanceName();
        containerJson["color"] = container->getColor().name();

        QJsonObject posObj;
        posObj["x"] = container->x();
        posObj["y"] = container->y();
        containerJson["position"] = posObj;

        // Save parameters
        QJsonObject params;
        const QMap<QString, double>& containerParams = container->getParameters();
        for (auto it = containerParams.constBegin(); it != containerParams.constEnd(); ++it) {
            params[it.key()] = it.value();
        }
        containerJson["parameters"] = params;

        // Save custom DNA name (for Harmonic Generator containers)
        if (!container->getCustomDnaName().isEmpty()) {
            containerJson["customDnaName"] = container->getCustomDnaName();
        }

        // Save digit formula string (for Harmonic Generator in digit-formula mode)
        if (!container->getDigitString().isEmpty()) {
            containerJson["digitString"] = container->getDigitString();
        }

        // Save custom envelope data (for Envelope Engine containers)
        if (container->hasCustomEnvelopeData()) {
            EnvelopeData envData = container->getCustomEnvelopeData();
            QJsonObject customEnv;
            customEnv["name"] = envData.name;
            customEnv["loopMode"] = envData.loopMode;

            QJsonArray points;
            for (const EnvelopePoint &pt : envData.points) {
                QJsonObject ptObj;
                ptObj["time"] = pt.time;
                ptObj["value"] = pt.value;
                ptObj["curveType"] = pt.curveType;
                points.append(ptObj);
            }
            customEnv["points"] = points;
            containerJson["customEnvelope"] = customEnv;
        }

        // Save wavetable data (for Wavetable Synth containers)
        if (container->hasWavetableData()) {
            QJsonObject wtObj;
            wtObj["filePath"] = container->getWavetableFilePath();
            const std::vector<float> &wtData = container->getWavetableData();
            QByteArray rawBytes(reinterpret_cast<const char*>(wtData.data()),
                                static_cast<int>(wtData.size() * sizeof(float)));
            wtObj["samples"] = QString::fromLatin1(rawBytes.toBase64());
            wtObj["sampleCount"] = static_cast<int>(wtData.size());
            containerJson["wavetableData"] = wtObj;
        }

        // Save IR data (for IR Convolution containers)
        if (container->hasIRData()) {
            QJsonObject irObj;
            irObj["filePath"] = container->getIRFilePath();
            const std::vector<float> &irSamples = container->getIRData();
            QByteArray rawBytes(reinterpret_cast<const char*>(irSamples.data()),
                                static_cast<int>(irSamples.size() * sizeof(float)));
            irObj["samples"] = QString::fromLatin1(rawBytes.toBase64());
            irObj["sampleCount"] = static_cast<int>(irSamples.size());
            containerJson["irData"] = irObj;
        }

        containersArray.append(containerJson);
    }
    state["containers"] = containersArray;

    // Serialize connections
    QJsonArray connectionsArray;
    const QVector<Canvas::Connection>& connections = canvas->getConnections();
    for (const Canvas::Connection &conn : connections) {
        QJsonObject connJson;
        connJson["fromContainer"] = conn.fromContainer->getInstanceName();
        connJson["fromPort"] = conn.fromPort;
        connJson["toContainer"] = conn.toContainer->getInstanceName();
        connJson["toPort"] = conn.toPort;
        connJson["function"] = conn.function;
        connJson["weight"] = conn.weight;
        connectionsArray.append(connJson);
    }
    state["connections"] = connectionsArray;

    // Store in map
    trackCanvasStates[trackIndex] = state;
    qDebug() << "Saved canvas state for track" << trackIndex << "-" << containers.size() << "containers," << connections.size() << "connections";
}

void KalaMain::loadCanvasStateForTrack(int trackIndex)
{
    if (!trackCanvasStates.contains(trackIndex)) {
        qDebug() << "No saved canvas state for track" << trackIndex << "- canvas will be empty";
        return;
    }

    Canvas *canvas = sounitBuilder->getCanvas();
    if (!canvas) {
        qWarning() << "Cannot load canvas state: canvas is null";
        return;
    }

    QJsonObject state = trackCanvasStates[trackIndex];

    // Read sounit metadata
    QJsonObject sounitMeta = state["sounit"].toObject();
    QString sounitName = sounitMeta["name"].toString("Untitled Sounit");
    QString sounitComment = sounitMeta["comment"].toString("");
    canvas->setSounitName(sounitName);
    canvas->setSounitComment(sounitComment);

    // Delete existing containers - they were loaded from sounit file but we're
    // restoring from saved canvas state which has the correct container positions/params
    QList<Container*> existingContainers = canvas->findChildren<Container*>();
    for (Container *c : existingContainers) {
        c->setParent(nullptr);  // Detach from canvas so findChildren won't find them
        c->deleteLater();
    }
    canvas->getConnections().clear();
    canvas->clearSelection();

    // Deserialize containers (using Canvas's existing deserializeContainer method)
    // We need to use a temporary file approach since Canvas::loadFromJson is file-based
    // Actually, we'll manually reconstruct containers here
    QMap<QString, Container*> containerMap;
    QJsonArray containersArray = state["containers"].toArray();

    for (const QJsonValue &val : containersArray) {
        QJsonObject containerJson = val.toObject();
        QString type = containerJson["type"].toString();
        QString instanceName = containerJson["instanceName"].toString();
        QColor color(containerJson["color"].toString());

        QJsonObject posObj = containerJson["position"].toObject();
        QPoint position(posObj["x"].toInt(), posObj["y"].toInt());

        // Migrate legacy container names
        if (type == "Karplus Strong Attack") {
            type = "Karplus Strong";
        }

        // Get port lists from the single source of truth
        QStringList inputs, outputs;
        Canvas::getPortsForContainerType(type, inputs, outputs);
        if (inputs.isEmpty() && outputs.isEmpty()) {
            qWarning() << "Unknown container type:" << type;
            continue;
        }

        // Create container
        Container *container = new Container(canvas, type, color, inputs, outputs);
        container->setInstanceName(instanceName);
        container->move(position);

        // Restore parameters
        QJsonObject params = containerJson["parameters"].toObject();
        container->beginParameterUpdate();
        for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
            container->setParameter(it.key(), it.value().toDouble());
        }
        container->endParameterUpdate();

        // Restore custom DNA name if present
        if (containerJson.contains("customDnaName")) {
            container->setCustomDnaName(containerJson["customDnaName"].toString());
        }

        // Restore digit formula string if present
        if (containerJson.contains("digitString")) {
            container->setDigitString(containerJson["digitString"].toString());
        }

        // Restore custom envelope data if present
        if (containerJson.contains("customEnvelope")) {
            QJsonObject envObj = containerJson["customEnvelope"].toObject();
            EnvelopeData envData;
            envData.name = envObj["name"].toString();
            envData.loopMode = envObj["loopMode"].toInt();
            envData.isFactory = false;

            QJsonArray pointsArray = envObj["points"].toArray();
            for (const QJsonValue &ptVal : pointsArray) {
                QJsonObject ptObj = ptVal.toObject();
                EnvelopePoint pt;
                pt.time = ptObj["time"].toDouble();
                pt.value = ptObj["value"].toDouble();
                pt.curveType = ptObj["curveType"].toInt();
                envData.points.append(pt);
            }

            container->setCustomEnvelopeData(envData);
        }

        // Restore wavetable data if present
        if (containerJson.contains("wavetableData")) {
            QJsonObject wtObj = containerJson["wavetableData"].toObject();
            container->setWavetableFilePath(wtObj["filePath"].toString());
            int sampleCount = wtObj["sampleCount"].toInt();
            if (sampleCount > 0 && wtObj.contains("samples")) {
                QByteArray base64 = wtObj["samples"].toString().toLatin1();
                QByteArray rawBytes = QByteArray::fromBase64(base64);
                int expectedSize = sampleCount * static_cast<int>(sizeof(float));
                if (rawBytes.size() == expectedSize) {
                    std::vector<float> wtData(sampleCount);
                    std::memcpy(wtData.data(), rawBytes.constData(), rawBytes.size());
                    container->setWavetableData(wtData);
                }
            }
        }

        // Restore IR data if present
        if (containerJson.contains("irData")) {
            QJsonObject irObj = containerJson["irData"].toObject();
            container->setIRFilePath(irObj["filePath"].toString());
            int sampleCount = irObj["sampleCount"].toInt();
            if (sampleCount > 0 && irObj.contains("samples")) {
                QByteArray base64 = irObj["samples"].toString().toLatin1();
                QByteArray rawBytes = QByteArray::fromBase64(base64);
                int expectedSize = sampleCount * static_cast<int>(sizeof(float));
                if (rawBytes.size() == expectedSize) {
                    std::vector<float> irSamples(sampleCount);
                    std::memcpy(irSamples.data(), rawBytes.constData(), rawBytes.size());
                    container->setIRData(irSamples);
                }
            }
        }

        container->show();
        containerMap[instanceName] = container;

        // Connect signals
        sounitBuilder->connectContainerSignals(container);
    }

    // Deserialize connections
    // Guard with setLoading(true) so that each addConnection() call does not emit
    // graphChanged → SounitBuilder::rebuildGraph() → Track::rebuildGraph() →
    // m_graphHash++.  Without this guard every connection increments m_graphHash,
    // causing a hash mismatch against the value stored in .kalarender and forcing
    // a full re-render of all notes on the next play.
    canvas->setLoading(true);
    QJsonArray connectionsArray = state["connections"].toArray();
    for (const QJsonValue &val : connectionsArray) {
        QJsonObject connJson = val.toObject();

        QString fromName = connJson["fromContainer"].toString();
        QString toName = connJson["toContainer"].toString();

        if (!containerMap.contains(fromName) || !containerMap.contains(toName)) {
            qWarning() << "Connection references missing container:" << fromName << "->" << toName;
            continue;
        }

        Canvas::Connection conn;
        conn.fromContainer = containerMap[fromName];
        conn.fromPort = connJson["fromPort"].toString();
        conn.toContainer = containerMap[toName];
        conn.toPort = connJson["toPort"].toString();
        conn.function = connJson["function"].toString("passthrough");
        conn.weight = connJson["weight"].toDouble(1.0);

        canvas->addConnection(conn);
    }
    canvas->setLoading(false);

    canvas->update();
    qDebug() << "Loaded canvas state for track" << trackIndex << "-" << containerMap.size() << "containers," << connectionsArray.size() << "connections";
}

void KalaMain::switchToSounit(int trackIndex)
{
    // Get the target Track from TrackManager
    Track *track = trackManager->getTrack(trackIndex);
    if (!track) {
        qWarning() << "Invalid track index:" << trackIndex;
        return;
    }

    // Check if this track has a sounit loaded
    if (track->getSounitFilePath().isEmpty() && !trackSounitFiles.contains(trackIndex)) {
        qWarning() << "No sounit loaded for track" << trackIndex;
        return;
    }

    qDebug() << "=== Switching to Track" << trackIndex << "(" << track->getName() << ") ===";

    // Disconnect from old track's canvas before switching
    if (currentEditingTrack >= 0) {
        Track *oldTrack = trackManager->getTrack(currentEditingTrack);
        if (oldTrack && oldTrack->getCanvas()) {
            Canvas *oldCanvas = oldTrack->getCanvas();
            disconnect(oldCanvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
            disconnect(oldCanvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
            disconnect(oldCanvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
            disconnect(oldCanvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
            disconnect(oldCanvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);
        }
    }

    // Switch SounitBuilder to display this Track's canvas
    // Each Track has its own canvas - no need for show/hide logic
    sounitBuilder->setTrackCanvas(track);

    // Reconnect canvas signals to the new canvas
    Canvas *canvas = track->getCanvas();
    connect(canvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
    connect(canvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
    connect(canvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
    connect(canvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
    connect(canvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);

    // Update current editing track in both KalaMain and TrackManager
    currentEditingTrack = trackIndex;
    trackManager->setCurrentTrack(trackIndex);  // Critical: sync TrackManager's current track
    sounitBuilder->setCurrentEditingTrack(trackIndex);
    if (m_kalaTools) m_kalaTools->setCurrentTrackIndex(trackIndex);  // Sync AI tools

    // Update UI with this track's sounit info
    QString sounitName = track->getSounitName();
    if (sounitName.isEmpty()) {
        sounitName = trackSounitNames.value(trackIndex, "Unknown");
    }
    ui->editSounitName->setText(sounitName);

    QString sounitComment = track->getSounitComment();
    ui->editSounitComment->setPlainText(sounitComment);

    qDebug() << "Switched to editing sounit:" << sounitName;

    // Rebuild graph for this track (ensures AudioEngine graph is in sync).
    // Only rebuild the Track's compiled graph if it doesn't already have one,
    // so switching tracks doesn't invalidate note render caches unnecessarily.
    sounitBuilder->rebuildGraph(trackIndex, !track->hasValidGraph());

    // Update variation selector for new track and reset to base
    updateVariationSelector();
    ui->comboVariationSelector->setCurrentIndex(0);  // Reset to base sounit

    // Note: Pre-rendering is NOT triggered here - it only happens when play is pressed
    // This avoids expensive re-renders when just selecting notes or switching tracks

    qDebug() << "=== Switch complete ===";
}

void KalaMain::onSounitSelectorChanged(int index)
{
    if (index < 0) {
        qDebug() << "SounitSelector: Invalid index" << index;
        return;
    }

    // Get the track index from the item data
    QVariant data = ui->comboSounitSelector->itemData(index);
    if (!data.isValid()) {
        qWarning() << "SounitSelector: No data for index" << index;
        return;
    }

    int trackIndex = data.toInt();

    qDebug() << "SounitSelector: User selected index" << index << "-> track" << trackIndex
             << "(current editing track:" << currentEditingTrack << ")";

    // *** CRITICAL FIX: Update the label FIRST ***
    QString selectedName = trackSounitNames.value(trackIndex, "Unknown");
    ui->editSounitName->setText(selectedName);
    qDebug() << "Updated editSounitName to:" << selectedName;




    qDebug() << "SounitSelector: User selected index" << index << "-> track" << trackIndex
             << "(current editing track:" << currentEditingTrack << ")";

    // Only switch if it's a different track
    if (trackIndex != currentEditingTrack) {
        qDebug() << "SounitSelector: Switching from track" << currentEditingTrack << "to track" << trackIndex;
        switchToSounit(trackIndex);
    } else {
        qDebug() << "SounitSelector: Already editing track" << trackIndex << ", no switch needed";
    }
}

KalaMain::~KalaMain()
{
    // Clean up in reverse order
    delete scoreCanvasWindow;
    delete sounitBuilder;

    // Shutdown and delete audio engine last
    if (audioEngine) {
        audioEngine->stopDeviceNotifier();
        audioEngine->shutdown();
        delete audioEngine;
    }

    delete ui;
}

void KalaMain::onNewSounit()
{
    Canvas *canvas = sounitBuilder->getCanvas();

    // Clear all containers
    QList<Container*> containers = canvas->findChildren<Container*>();
    for (Container *c : containers) {
        c->deleteLater();
    }

    // Clear connections
    canvas->getConnections().clear();
    canvas->clearSelection();
    canvas->update();

    // Reset sounit name and comment
    canvas->setSounitName("Untitled Sounit");
    canvas->setSounitComment("");
    currentSounitFilePath.clear();

    // Rebuild empty graph for the current editing track (or track 0 if none)
    int trackIndex = (currentEditingTrack >= 0) ? currentEditingTrack : 0;
    sounitBuilder->rebuildGraph(trackIndex);

    // Mark project as dirty
    markProjectDirty();

    qDebug() << "New Sounit created for track" << trackIndex;
}

// *** NEW: Manual sounit name update method ***
void KalaMain::updateSounitNameDisplay(const QString &name)
{
    // Only update if we're not in the middle of loading
    if (!m_isLoadingSounit) {
        onSounitNameChanged(name);
    } else {
        qDebug() << "Skipping sounit name display update during load:" << name;
    }
}

void KalaMain::onLoadSounit()
{
    // *** SET LOADING FLAG ***
    m_isLoadingSounit = true;

    QSettings settings;
    QString lastDir = settings.value("lastDirectory/sounit", QString()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Sounit",
        lastDir,
        "Sounit Files (*.sounit);;JSON Files (*.json);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        m_isLoadingSounit = false;
        return;  // User cancelled
    }

    // Ask user which track to assign this sounit to FIRST
    // (We need to know the target track before loading)
    QStringList trackNames;
    int trackCount = trackManager->getTrackCount();

    if (trackCount == 0) {
        QMessageBox::warning(this, "No Tracks",
                             "No tracks available. Please create a track first using Track > Add Track.");
        m_isLoadingSounit = false;
        return;
    }

    for (int i = 0; i < trackCount; i++) {
        Track *t = trackManager->getTrack(i);
        trackNames << QString("Track %1: %2").arg(i).arg(t ? t->getName() : "Unknown");
    }

    bool ok;
    QString selectedTrack = QInputDialog::getItem(
        this,
        "Assign Sounit to Track",
        "Load sounit to which track?",
        trackNames,
        0,  // Default to first track
        false,  // Not editable
        &ok
        );

    if (!ok) {
        m_isLoadingSounit = false;
        return;  // User cancelled
    }

    // Extract track index from selection
    int trackIndex = trackNames.indexOf(selectedTrack);

    // Get the target Track from TrackManager
    Track *track = trackManager->getTrack(trackIndex);
    if (!track) {
        QMessageBox::critical(this, "Error", "Invalid track selected.");
        m_isLoadingSounit = false;
        return;
    }

    qDebug() << "=== Loading sounit to Track" << trackIndex << "(" << track->getName() << ") ===";

    // If the track already has a sounit, ask whether to replace or create a variation
    bool hasSounit = !track->getCanvas()->findChildren<Container*>().isEmpty();
    if (hasSounit) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Track Already Has a Sounit");
        msgBox.setText(QString("Track %1 (\"%2\") already has \"%3\" loaded.")
                       .arg(trackIndex)
                       .arg(track->getName())
                       .arg(track->getSounitName()));
        msgBox.setInformativeText("What would you like to do with the selected file?");
        QPushButton *replaceBtn   = msgBox.addButton("Replace base sounit", QMessageBox::DestructiveRole);
        QPushButton *variationBtn = msgBox.addButton("Load as variation",   QMessageBox::AcceptRole);
        QPushButton *cancelBtn    = msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(variationBtn);
        msgBox.exec();

        if (msgBox.clickedButton() == cancelBtn) {
            m_isLoadingSounit = false;
            return;
        }

        if (msgBox.clickedButton() == variationBtn) {
            // Parse the sounit file without touching the main canvas
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::critical(this, "Load Error", "Failed to open file:\n" + fileName);
                m_isLoadingSounit = false;
                return;
            }
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            if (doc.isNull() || !doc.isObject()) {
                QMessageBox::critical(this, "Load Error", "Invalid sounit file:\n" + fileName);
                m_isLoadingSounit = false;
                return;
            }

            QJsonObject root = doc.object();
            QString fileSounitName = root["sounit"].toObject()["name"].toString("Unnamed");
            QJsonObject graphData;
            graphData["containers"] = root["containers"];
            graphData["connections"] = root["connections"];

            // Suggest a name; avoid collisions
            QString defaultName = fileSounitName;
            if (track->findVariationByName(defaultName) > 0) {
                defaultName = QString("%1 %2").arg(fileSounitName).arg(track->getVariationCount() + 1);
            }

            bool ok;
            QString variationName = QInputDialog::getText(this, "Load as Variation",
                                                          "Variation name:", QLineEdit::Normal,
                                                          defaultName, &ok);
            if (!ok || variationName.isEmpty()) {
                m_isLoadingSounit = false;
                return;
            }

            // Handle name collision
            int existingIndex = track->findVariationByName(variationName);
            if (existingIndex > 0) {
                QMessageBox::StandardButton reply = QMessageBox::question(this,
                    "Replace Variation?",
                    QString("A variation named \"%1\" already exists.\n\nReplace it?").arg(variationName),
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::No) {
                    m_isLoadingSounit = false;
                    return;
                }
                track->deleteVariation(existingIndex);
            }

            int index = track->createVariationFromJson(graphData, variationName);
            if (index > 0) {
                settings.setValue("lastDirectory/sounit", QFileInfo(fileName).absolutePath());
                refreshVariationsList();
                updateVariationSelector();
                markProjectDirty();
                QMessageBox::information(this, "Variation Loaded",
                    QString("Variation \"%1\" created from \"%2\".")
                        .arg(variationName)
                        .arg(QFileInfo(fileName).fileName()));
            } else {
                QMessageBox::critical(this, "Error",
                    "Failed to build a valid audio graph from the selected file.\n"
                    "The sounit may be empty or have an invalid structure.");
            }

            m_isLoadingSounit = false;
            return;
        }
        // replaceBtn was clicked — fall through to normal load
    }

    // Load sounit into the Track's own canvas
    // This clears old containers and loads new ones
    if (!track->loadSounit(fileName)) {
        QMessageBox::critical(this, "Load Error",
                              "Failed to load sounit from:\n" + fileName);
        m_isLoadingSounit = false;
        return;
    }

    QString sounitName = track->getSounitName();
    qDebug() << "Sounit loaded:" << sounitName;

    // Remember the directory for next time
    settings.setValue("lastDirectory/sounit", QFileInfo(fileName).absolutePath());

    // Disconnect from old track's canvas before switching
    if (currentEditingTrack >= 0) {
        Track *oldTrack = trackManager->getTrack(currentEditingTrack);
        if (oldTrack && oldTrack->getCanvas()) {
            Canvas *oldCanvas = oldTrack->getCanvas();
            disconnect(oldCanvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
            disconnect(oldCanvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
            disconnect(oldCanvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
            disconnect(oldCanvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
            disconnect(oldCanvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);
        }
    }

    // Switch SounitBuilder to display this Track's canvas
    sounitBuilder->setTrackCanvas(track);
    qDebug() << "SounitBuilder switched to Track" << trackIndex << "canvas";

    // Connect container signals for the newly loaded containers
    Canvas *canvas = track->getCanvas();

    // Reconnect canvas signals to KalaMain for inspector updates
    connect(canvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
    connect(canvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
    connect(canvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
    connect(canvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
    connect(canvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);

    // NOTE: Container signals are already connected in setTrackCanvas()
    // Do NOT call connectContainerSignals() again here - it causes duplicate signals
    // which leads to port clicks being processed twice (bug: wire stuck on mouse)

    // Update current editing track
    currentEditingTrack = trackIndex;
    trackManager->setCurrentTrack(trackIndex);  // Sync TrackManager's current track
    sounitBuilder->setCurrentEditingTrack(trackIndex);

    // Rebuild the graph. loadSounit() already called Track::rebuildGraph() internally,
    // so only the AudioEngine's preview graph needs syncing (rebuildTrackGraph=false).
    sounitBuilder->rebuildGraph(trackIndex, !track->hasValidGraph());
    qDebug() << "Graph rebuilt for track" << trackIndex;

    // Update variation selector for new track
    updateVariationSelector();

    // Update track name and color in ScoreCanvasWindow
    QColor sounitColor = scoreCanvasWindow->getNextTrackColor();
    scoreCanvasWindow->updateSounitTrack(trackIndex, sounitName, sounitColor);

    // Pre-render all notes with the new graph
    scoreCanvasWindow->prerenderNotes();
    qDebug() << "Pre-rendered notes after loading sounit to track" << trackIndex;

    // Update UI
    ui->editSounitName->blockSignals(true);
    ui->editSounitName->setText(sounitName);
    ui->editSounitName->blockSignals(false);

    QString sounitComment = track->getSounitComment();
    ui->editSounitComment->blockSignals(true);
    ui->editSounitComment->setPlainText(sounitComment);
    ui->editSounitComment->blockSignals(false);

    // Track file path and name for navigation (still useful for selector dropdown)
    trackSounitFiles[trackIndex] = fileName;
    trackSounitNames[trackIndex] = sounitName;
    currentSounitFilePath = fileName;

    // Update the sounit selector dropdown
    updateSounitSelector();

    m_isLoadingSounit = false;

    // Mark project as dirty
    markProjectDirty();

    QMessageBox::information(this, "Loaded",
                             QString("Sounit '%1' loaded successfully to Track %2")
                                 .arg(sounitName).arg(trackIndex));

    qDebug() << "=== Sounit load complete ===" << fileName;
}




void KalaMain::onSaveSounitAs()
{
    // Prompt for sounit name first
    Canvas *canvas = sounitBuilder->getCanvas();
    QString currentName = canvas->getSounitName();

    bool ok;
    QString sounitName = QInputDialog::getText(
        this,
        "Save Sounit As",
        "Enter sounit name:",
        QLineEdit::Normal,
        currentName,
        &ok
    );

    if (!ok || sounitName.isEmpty()) {
        return;  // User cancelled
    }

    // Update canvas sounit name
    canvas->setSounitName(sounitName);

    // Open file dialog with remembered directory
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/sounit", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Sounit As",
        lastDir + "/" + sounitName + ".sounit",  // Suggest filename based on sounit name
        "Sounit Files (*.sounit);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    // Remember this directory for next time
    settings.setValue("lastDirectory/sounit", QFileInfo(fileName).absolutePath());

    // Ensure .sounit extension
    if (!fileName.endsWith(".sounit", Qt::CaseInsensitive) &&
        !fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".sounit";
    }

    if (!canvas->saveToJson(fileName, sounitName)) {
        QMessageBox::critical(this, "Save Error",
            "Failed to save sounit to:\n" + fileName);
        return;
    }

    currentSounitFilePath = fileName;

    QMessageBox::information(this, "Saved",
        QString("Sounit '%1' saved successfully").arg(sounitName));

    qDebug() << "Sounit saved to:" << fileName;
}

void KalaMain::onSounitNameChanged(const QString &name)
{
    // Update window title to show sounit name
    setWindowTitle(QString("Kala - %1").arg(name));

    // Update the current editing track's name in our map
    if (currentEditingTrack >= 0) {
        trackSounitNames[currentEditingTrack] = name;
    }

    // Update the name text field (block signals to avoid recursive updates)
    ui->editSounitName->blockSignals(true);
    ui->editSounitName->setText(name);
    ui->editSounitName->blockSignals(false);

    // Refresh the selector to show updated name in dropdown list
    updateSounitSelector();
}

void KalaMain::onSounitCommentChanged(const QString &comment)
{
    // Skip if text is already identical to avoid resetting the cursor position
    if (ui->editSounitComment->toPlainText() == comment)
        return;
    // Update the UI field (block signals to avoid recursive updates)
    ui->editSounitComment->blockSignals(true);
    ui->editSounitComment->setPlainText(comment);
    ui->editSounitComment->blockSignals(false);
}

void KalaMain::onSounitNameEdited()
{
    // *** CHECK LOADING FLAG ***
    if (m_isLoadingSounit) {
        qDebug() << "Ignoring sounit name edit during loading operation";

        // Restore the original name from canvas
        Canvas *canvas = sounitBuilder->getCanvas();
        QString currentName = canvas->getSounitName();

        ui->editSounitName->blockSignals(true);
        ui->editSounitName->setText(currentName);
        ui->editSounitName->blockSignals(false);

        return;
    }

    // Get the new name from the QLineEdit
    QString newName = ui->editSounitName->text().trimmed();

    // Validate the name
    if (newName.isEmpty()) {
        qWarning() << "Sounit name cannot be empty";

        // Restore previous name
        Canvas *canvas = sounitBuilder->getCanvas();
        QString currentName = canvas->getSounitName();

        ui->editSounitName->blockSignals(true);
        ui->editSounitName->setText(currentName);
        ui->editSounitName->blockSignals(false);

        return;
    }

    Canvas *canvas = sounitBuilder->getCanvas();
    QString currentName = canvas->getSounitName();

    // Only update if the name actually changed
    if (newName != currentName) {
        qDebug() << "User edited sounit name from '" << currentName << "' to '" << newName << "'";

        // *** UPDATE THE CANVAS (WILL EMIT SIGNAL SINCE WE'RE NOT LOADING) ***
        canvas->setSounitName(newName);

        // Update our tracking map for the current editing track
        if (currentEditingTrack >= 0) {
            QString oldTrackName = trackSounitNames.value(currentEditingTrack, "");
            if (oldTrackName != newName) {
                trackSounitNames[currentEditingTrack] = newName;
                qDebug() << "Updated trackSounitNames[" << currentEditingTrack
                         << "] from '" << oldTrackName << "' to '" << newName << "'";
            }
        }

        // Update the static label
        ui->editSounitName->setText(newName);

        // Update window title
        setWindowTitle(QString("Kala - %1").arg(newName));

        // Refresh the selector dropdown
        updateSounitSelector();

        // Mark the project as modified (if you have a dirty flag)
        // isProjectModified = true;

        // If you have undo/redo support, add a command here
        // undoStack->push(new ChangeSounitNameCommand(canvas, currentName, newName));
    } else {
        qDebug() << "Sounit name unchanged:" << newName;
    }
}

void KalaMain::onSounitCommentEdited()
{
    Canvas *canvas = sounitBuilder->getCanvas();
    canvas->setSounitComment(ui->editSounitComment->toPlainText());
}

void KalaMain::onAddTrack()
{
    qDebug() << "KalaMain: Add Track menu item clicked";

    // Get track name from user
    bool ok;
    QString trackName = QInputDialog::getText(this, "Add Track",
                                             "Track name:",
                                             QLineEdit::Normal,
                                             QString("Track %1").arg(trackManager->getTrackCount() + 1),
                                             &ok);
    if (!ok || trackName.isEmpty()) {
        return;  // User canceled
    }

    // Get next color from palette
    QColor trackColor = scoreCanvasWindow->getNextTrackColor();

    // Add track to TrackManager (data)
    Track *newTrack = trackManager->addTrack(trackName, trackColor);
    if (!newTrack) {
        QMessageBox::warning(this, "Error", "Failed to create track.");
        return;
    }
    qDebug() << "KalaMain: Added track to TrackManager, now has"
             << trackManager->getTrackCount() << "tracks";

    // Add track to TrackSelector (visual) in ScoreCanvasWindow
    const double LOWEST_NOTE = 27.5;    // A0
    const double HIGHEST_NOTE = 4186.0; // C8
    scoreCanvasWindow->getTrackSelector()->addTrack(trackName, trackColor, LOWEST_NOTE, HIGHEST_NOTE);

    // Update sounit selector dropdown
    updateSounitSelector();

    // Rebuild mixer to include new track
    rebuildMixer();

    // Mark project as dirty
    markProjectDirty();

    qDebug() << "KalaMain: Added track" << trackName << "with color" << trackColor.name();
}

void KalaMain::onDeleteTrack()
{
    qDebug() << "KalaMain: Delete Track menu item clicked";

    int trackCount = trackManager->getTrackCount();
    if (trackCount == 0) {
        QMessageBox::warning(this, "Delete Track", "There are no tracks to delete.");
        return;
    }
    if (trackCount == 1) {
        QMessageBox::warning(this, "Delete Track",
                            "Cannot delete the last track.\n\n"
                            "A composition must have at least one track.");
        return;
    }

    // Build list of track names for the selection dialog
    QStringList trackNames;
    for (int i = 0; i < trackCount; ++i) {
        Track *track = trackManager->getTrack(i);
        if (track) {
            trackNames << QString("Track %1: %2").arg(i + 1).arg(track->getName());
        }
    }

    bool ok;
    QString selectedTrack = QInputDialog::getItem(this, "Delete Track",
                                                   "Select track to delete:",
                                                   trackNames, 0, false, &ok);
    if (!ok || selectedTrack.isEmpty()) return;

    int trackIndex = trackNames.indexOf(selectedTrack);
    if (trackIndex < 0) {
        qWarning() << "KalaMain: Could not find selected track";
        return;
    }

    deleteTrackAtIndex(trackIndex);
}

void KalaMain::deleteTrackAtIndex(int trackIndex)
{
    qDebug() << "KalaMain: deleteTrackAtIndex" << trackIndex;

    int trackCount = trackManager->getTrackCount();
    if (trackCount <= 1) {
        QMessageBox::warning(this, "Delete Track",
                            "Cannot delete the last track.\n\n"
                            "A composition must have at least one track.");
        return;
    }
    if (trackIndex < 0 || trackIndex >= trackCount) {
        qWarning() << "KalaMain: deleteTrackAtIndex - invalid index" << trackIndex;
        return;
    }

    Track *trackToDelete = trackManager->getTrack(trackIndex);
    if (!trackToDelete) {
        qWarning() << "KalaMain: Track not found at index" << trackIndex;
        return;
    }

    // Count notes on this track for the confirmation dialog
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<Note>& allNotes = scoreCanvas->getPhrase().getNotes();
    int notesOnTrack = 0;
    for (const Note& note : allNotes) {
        if (note.getTrackIndex() == trackIndex) notesOnTrack++;
    }

    QString confirmMsg = QString("Are you sure you want to delete \"%1\"?").arg(trackToDelete->getName());
    if (notesOnTrack > 0) {
        confirmMsg += QString("\n\nThis track contains %1 note(s) which will also be deleted.").arg(notesOnTrack);
    }

    QMessageBox::StandardButton result = QMessageBox::question(this, "Confirm Delete Track",
                                                                confirmMsg,
                                                                QMessageBox::Yes | QMessageBox::No,
                                                                QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    // Stop all audio playback before touching the track — the audio thread
    // holds Track* pointers (playbackTracks list) that would dangle after deletion.
    stopAllPlayback();
    audioEngine->stopTrackPlayback();

    // Delete this track's notes from the ScoreCanvas phrase
    if (notesOnTrack > 0) {
        scoreCanvas->deleteNotesOnTrack(trackIndex);
        qDebug() << "KalaMain: Deleted" << notesOnTrack << "notes from track" << trackIndex;
    }
    scoreCanvas->updateTrackIndicesAfterDeletion(trackIndex);

    // Clear inspector if it references a container owned by the track being deleted
    if (currentContainer) {
        QList<Container*> trackContainerList = trackToDelete->getCanvas()->findChildren<Container*>();
        if (trackContainerList.contains(currentContainer)) {
            currentContainer = nullptr;
            clearConfigInspector();
        }
    }

    // CRITICAL FIX: Detach SounitBuilder's canvas pointer BEFORE deleting the track.
    // If the track being deleted is currently displayed in SounitBuilder, its canvas
    // is stored in SounitBuilder::canvas as a raw pointer.  Deleting the track calls
    // Track::~Track() → delete m_canvas, leaving SounitBuilder::canvas dangling.
    // The subsequent setTrackCanvas() call would then crash on canvas->cancelPendingConnection().
    if (sounitBuilder && trackToDelete->getCanvas() == sounitBuilder->getCanvas()) {
        sounitBuilder->setTrackCanvas(nullptr);  // Detach + sets canvas = nullptr safely
    }

    // Update currentEditingTrack index before the removal shifts indices
    if (currentEditingTrack == trackIndex) {
        currentEditingTrack = (trackIndex == 0) ? 0 : trackIndex - 1;
    } else if (currentEditingTrack > trackIndex) {
        currentEditingTrack--;
    }

    // Clean up legacy per-track state maps
    trackSounitFiles.remove(trackIndex);
    trackSounitNames.remove(trackIndex);
    trackCanvasStates.remove(trackIndex);
    trackContainers.remove(trackIndex);
    trackConnections.remove(trackIndex);

    // Remove from TrackManager — this deletes the Track object and its canvas.
    // Safe now because SounitBuilder's canvas pointer has already been nulled above.
    bool removed = trackManager->removeTrack(trackIndex);
    if (!removed) {
        qWarning() << "KalaMain: Failed to remove track from TrackManager";
        return;
    }

    // Remove from TrackSelector (visual)
    TrackSelector *trackSelector = scoreCanvasWindow->getTrackSelector();
    if (trackSelector) {
        trackSelector->removeTrack(trackIndex);
    }

    // Update sounit selector dropdown
    updateSounitSelector();

    // Switch SounitBuilder to the new current track
    if (trackManager->getTrackCount() > 0 && currentEditingTrack >= 0) {
        Track *newCurrentTrack = trackManager->getTrack(currentEditingTrack);
        if (newCurrentTrack && sounitBuilder) {
            trackManager->setCurrentTrack(currentEditingTrack);
            sounitBuilder->setTrackCanvas(newCurrentTrack);
            sounitBuilder->setCurrentEditingTrack(currentEditingTrack);

            Canvas *newCanvas = newCurrentTrack->getCanvas();
            connect(newCanvas, &Canvas::containerSelected, this, &KalaMain::onContainerSelected);
            connect(newCanvas, &Canvas::connectionSelected, this, &KalaMain::onConnectionSelected);
            connect(newCanvas, &Canvas::selectionCleared, this, &KalaMain::onSelectionCleared);
            connect(newCanvas, &Canvas::sounitNameChanged, this, &KalaMain::onSounitNameChanged);
            connect(newCanvas, &Canvas::sounitCommentChanged, this, &KalaMain::onSounitCommentChanged);

            updateVariationSelector();
        }
    }

    qDebug() << "KalaMain: Track" << trackIndex << "deleted. Remaining tracks:" << trackManager->getTrackCount();
}

void KalaMain::rebuildMixer()
{
    // Clear existing mixer widgets
    for (QWidget *widget : mixerTrackWidgets) {
        widget->deleteLater();
    }
    mixerTrackWidgets.clear();

    // Get the mixer layout from the UI (now horizontal)
    QHBoxLayout *mixerLayout = qobject_cast<QHBoxLayout*>(ui->layoutMixerTracks);
    if (!mixerLayout) {
        qWarning() << "KalaMain: Could not find layoutMixerTracks";
        return;
    }

    // Create a column for each track (side by side)
    int trackCount = trackManager->getTrackCount();
    for (int i = 0; i < trackCount; ++i) {
        Track *track = trackManager->getTrack(i);
        if (!track) continue;

        // Create container widget for this track's mixer column with colored background
        QWidget *columnWidget = new QWidget();
        columnWidget->setMinimumWidth(70);
        columnWidget->setMaximumWidth(80);

        // Set background color to track color (slightly transparent)
        QColor bgColor = track->getColor();
        bgColor.setAlpha(180);
        columnWidget->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4); border-radius: 4px;")
                                    .arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alpha()));

        QVBoxLayout *columnLayout = new QVBoxLayout(columnWidget);
        columnLayout->setContentsMargins(4, 4, 4, 4);
        columnLayout->setSpacing(2);

        // Track name label (centered, at top)
        QLabel *nameLabel = new QLabel(track->getName());
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setToolTip(track->getName());
        nameLabel->setStyleSheet("background: transparent; font-weight: bold; font-size: 9px;");
        nameLabel->setWordWrap(true);
        nameLabel->setMaximumHeight(24);
        columnLayout->addWidget(nameLabel);

        // Volume dial
        QDial *volumeDial = new QDial();
        volumeDial->setRange(0, 100);
        volumeDial->setValue(static_cast<int>(track->getVolume() * 100));
        volumeDial->setToolTip("Volume");
        volumeDial->setFixedSize(50, 50);
        volumeDial->setStyleSheet("background: transparent;");
        columnLayout->addWidget(volumeDial, 0, Qt::AlignCenter);

        // Volume label
        QLabel *volumeLabel = new QLabel(QString("V:%1%").arg(static_cast<int>(track->getVolume() * 100)));
        volumeLabel->setAlignment(Qt::AlignCenter);
        volumeLabel->setStyleSheet("background: transparent; font-size: 8px;");
        columnLayout->addWidget(volumeLabel);

        // Gain dial
        QDial *gainDial = new QDial();
        gainDial->setRange(0, 800);  // 0% to 800% (unity = 100, max = +18dB)
        gainDial->setValue(static_cast<int>(track->getGain() * 100));
        gainDial->setToolTip("Gain");
        gainDial->setFixedSize(50, 50);
        gainDial->setStyleSheet("background: transparent;");
        columnLayout->addWidget(gainDial, 0, Qt::AlignCenter);

        // Gain label
        QLabel *gainLabel = new QLabel(QString("G:%1%").arg(static_cast<int>(track->getGain() * 100)));
        gainLabel->setAlignment(Qt::AlignCenter);
        gainLabel->setStyleSheet("background: transparent; font-size: 8px;");
        columnLayout->addWidget(gainLabel);

        // Pan dial
        QDial *panDial = new QDial();
        panDial->setRange(-100, 100);  // -100 = full left, 0 = center, 100 = full right
        panDial->setValue(static_cast<int>(track->getPan() * 100));
        panDial->setToolTip("Pan");
        panDial->setFixedSize(50, 50);
        panDial->setStyleSheet("background: transparent;");
        columnLayout->addWidget(panDial, 0, Qt::AlignCenter);

        // Pan label
        QLabel *panLabel = new QLabel(QString("P:%1").arg(static_cast<int>(track->getPan() * 100)));
        panLabel->setAlignment(Qt::AlignCenter);
        panLabel->setStyleSheet("background: transparent; font-size: 8px;");
        columnLayout->addWidget(panLabel);

        // Add stretch at bottom to push everything up
        columnLayout->addStretch();

        // Connect volume dial to track
        int trackIndex = i;  // Capture by value for lambda
        connect(volumeDial, &QDial::valueChanged, this, [this, trackIndex, volumeLabel](int value) {
            Track *t = trackManager->getTrack(trackIndex);
            if (t) {
                float volume = value / 100.0f;
                t->setVolume(volume);
                volumeLabel->setText(QString("V:%1%").arg(value));
            }
        });

        // Connect gain dial to track
        connect(gainDial, &QDial::valueChanged, this, [this, trackIndex, gainLabel](int value) {
            Track *t = trackManager->getTrack(trackIndex);
            if (t) {
                float gain = value / 100.0f;
                t->setGain(gain);
                gainLabel->setText(QString("G:%1%").arg(value));
            }
        });

        // Connect pan dial to track
        connect(panDial, &QDial::valueChanged, this, [this, trackIndex, panLabel](int value) {
            Track *t = trackManager->getTrack(trackIndex);
            if (t) {
                float pan = value / 100.0f;
                t->setPan(pan);
                panLabel->setText(QString("P:%1").arg(value));
            }
        });

        // Connect track name changes to update the label
        connect(track, &Track::nameChanged, nameLabel, [nameLabel](const QString &name) {
            nameLabel->setText(name);
            nameLabel->setToolTip(name);
        });

        // Connect track color changes to update the background
        connect(track, &Track::colorChanged, columnWidget, [columnWidget](const QColor &color) {
            QColor bgColor = color;
            bgColor.setAlpha(180);
            columnWidget->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4); border-radius: 4px;")
                                        .arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alpha()));
        });

        // Add to layout and track list
        mixerLayout->addWidget(columnWidget);
        mixerTrackWidgets.append(columnWidget);
    }

    // Add stretch at the end to push tracks to the left
    mixerLayout->addStretch();

    qDebug() << "KalaMain: Mixer rebuilt with" << trackCount << "tracks";
}

void KalaMain::updateMixerSelection()
{
    for (int i = 0; i < mixerTrackWidgets.size(); ++i) {
        QWidget *col = mixerTrackWidgets[i];
        if (!col) continue;
        Track *track = trackManager->getTrack(i);
        if (!track) continue;

        bool selected = m_selectedTrackIndices.contains(i);
        QColor bgColor = track->getColor();
        bgColor.setAlpha(180);

        if (selected) {
            col->setStyleSheet(QString(
                "background-color: rgba(%1,%2,%3,%4); border-radius: 4px;"
                "border: 2px solid white;")
                .arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alpha()));
        } else {
            col->setStyleSheet(QString(
                "background-color: rgba(%1,%2,%3,%4); border-radius: 4px;")
                .arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alpha()));
        }
    }
}

void KalaMain::onScaleEditClicked()
{
    qDebug() << "KalaMain: Scale Edit button clicked";

    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();

    // Get the selected marker's time from the spinbox
    double selectedMarkerTime = getSelectedScaleMarkerTime();

    // Check if we're editing the default (marker 1)
    bool isEditingDefault = (selectedMarkerTime == 0.0);

    // Get the scale and base frequency for the selected marker
    Scale currentScale = scoreCanvas->getScaleAtTime(selectedMarkerTime);
    double currentBaseFreq = scoreCanvas->getBaseFrequencyAtTime(selectedMarkerTime);

    // Can only delete non-default markers
    bool canDelete = (selectedMarkerTime > 0.0);

    ScaleDialog dialog(currentScale, currentBaseFreq, !canDelete, this);
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.isDeleteRequested() && canDelete) {
            // Delete the marker that's selected in the spinbox
            scoreCanvas->getUndoStack()->push(
                new DeleteScaleChangeCommand(scoreCanvas, selectedMarkerTime,
                                             currentScale, currentBaseFreq));
            qDebug() << "KalaMain: Deleted scale modulation at" << selectedMarkerTime << "ms";
        } else {
            Scale newScale = dialog.getSelectedScale();
            double newBaseFreq = dialog.getBaseFrequency();

            if (isEditingDefault) {
                // At time 0, update the default scale via undo command
                scoreCanvas->getUndoStack()->push(
                    new EditScaleChangeCommand(scoreCanvas, 0.0,
                                               currentScale, currentBaseFreq,
                                               newScale, newBaseFreq, true));
                qDebug() << "KalaMain: Updated default scale to" << newScale.getName();
            } else {
                // Edit the marker selected in the spinbox
                scoreCanvas->getUndoStack()->push(
                    new EditScaleChangeCommand(scoreCanvas, selectedMarkerTime,
                                               currentScale, currentBaseFreq,
                                               newScale, newBaseFreq, false));
                qDebug() << "KalaMain: Updated scale modulation at" << selectedMarkerTime << "ms";
            }
        }

        // Mark project as dirty
        markProjectDirty();
    }
}

void KalaMain::onScaleAddClicked()
{
    qDebug() << "KalaMain: Scale Add button clicked";

    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    Timeline *timeline = scoreCanvasWindow->getTimeline();

    // Get current timeline position - this is where we create the new modulation
    double timelinePosition = timeline->getNowMarker();

    // Don't allow adding at time 0 (that's the default, use Edit instead)
    if (timelinePosition == 0.0) {
        qDebug() << "KalaMain: Cannot add modulation at time 0, use Edit to modify default";
        return;
    }

    // Check if there's already a modulation at this position
    if (scoreCanvas->getScaleChanges().contains(timelinePosition)) {
        qDebug() << "KalaMain: Modulation already exists at" << timelinePosition << "ms, use Edit to modify";
        return;
    }

    // Get the scale at this time (for pre-filling the dialog)
    Scale currentScale = scoreCanvas->getScaleAtTime(timelinePosition);
    double currentBaseFreq = scoreCanvas->getBaseFrequencyAtTime(timelinePosition);

    // Open the scale dialog (delete disabled since we're creating new)
    ScaleDialog dialog(currentScale, currentBaseFreq, true, this);  // true = isAtTimeZero (disables delete)
    if (dialog.exec() == QDialog::Accepted) {
        Scale newScale = dialog.getSelectedScale();
        double newBaseFreq = dialog.getBaseFrequency();

        // Create the new modulation at the timeline position via undo command
        scoreCanvas->getUndoStack()->push(
            new AddScaleChangeCommand(scoreCanvas, timelinePosition, newScale, newBaseFreq));
        qDebug() << "KalaMain: Created new scale modulation at" << timelinePosition << "ms:"
                 << newScale.getName() << newBaseFreq << "Hz";

        // Mark project as dirty
        markProjectDirty();
    }
}

void KalaMain::updateScaleDisplay()
{
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    Timeline *timeline = scoreCanvasWindow->getTimeline();
    double nowTime = timeline->getNowMarker();

    // Build list of all scale markers: default (time 0) + any changes
    QMap<double, QPair<Scale, double>> changes = scoreCanvas->getScaleChanges();
    int totalMarkers = 1 + changes.size();  // Default + changes

    // Update timeline visual indicators
    QMap<double, QString> scaleNamesForTimeline;
    for (auto it = changes.constBegin(); it != changes.constEnd(); ++it) {
        scaleNamesForTimeline[it.key()] = it.value().first.getName();
    }
    timeline->setScaleChanges(scaleNamesForTimeline);

    // Find which marker index corresponds to the current timeline position
    int currentMarkerIndex = 1;  // Start with default
    QList<double> sortedTimes = changes.keys();
    std::sort(sortedTimes.begin(), sortedTimes.end());
    for (int i = 0; i < sortedTimes.size(); ++i) {
        if (sortedTimes[i] <= nowTime) {
            currentMarkerIndex = i + 2;  // +2 because marker 1 is default
        } else {
            break;
        }
    }

    // Update spinbox range and selection (block signals to prevent double updates)
    ui->spinScaleMarker->blockSignals(true);
    ui->spinScaleMarker->setMaximum(totalMarkers);
    ui->spinScaleMarker->setValue(currentMarkerIndex);
    ui->spinScaleMarker->blockSignals(false);

    // Update the display for the selected marker
    onScaleMarkerChanged(currentMarkerIndex);
}

void KalaMain::onScaleMarkerChanged(int markerIndex)
{
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();

    // Get the time, scale, and base freq for this marker
    double markerTime = getSelectedScaleMarkerTime();
    Scale scale = scoreCanvas->getScaleAtTime(markerTime);
    double baseFreq = scoreCanvas->getBaseFrequencyAtTime(markerTime);

    // Format timestamp as Min:Sec:Ms
    int totalMs = static_cast<int>(markerTime);
    int ms = totalMs % 1000;
    int totalSeconds = totalMs / 1000;
    int seconds = totalSeconds % 60;
    int minutes = totalSeconds / 60;
    QString timestamp = QString("%1:%2:%3")
                            .arg(minutes)
                            .arg(seconds, 2, 10, QChar('0'))
                            .arg(ms, 3, 10, QChar('0'));

    // Build display string: timestamp | scale name | base frequency
    QString displayStr = QString("%1 | %2 | %3 Hz")
                             .arg(timestamp)
                             .arg(scale.getName())
                             .arg(baseFreq, 0, 'f', 2);

    ui->labelScaleDisplay->setText(displayStr);
}

double KalaMain::getSelectedScaleMarkerTime() const
{
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    int markerIndex = ui->spinScaleMarker->value();

    // Marker 1 is always the default at time 0
    if (markerIndex == 1) {
        return 0.0;
    }

    // For markers > 1, get from the sorted list of scale changes
    QMap<double, QPair<Scale, double>> changes = scoreCanvas->getScaleChanges();
    QList<double> sortedTimes = changes.keys();
    std::sort(sortedTimes.begin(), sortedTimes.end());

    int changeIndex = markerIndex - 2;  // Subtract 2 because marker 1 is default, marker 2 is first change
    if (changeIndex >= 0 && changeIndex < sortedTimes.size()) {
        return sortedTimes[changeIndex];
    }

    return 0.0;  // Fallback
}

// ========== Tempo Management ==========

void KalaMain::onTempoEditClicked()
{
    qDebug() << "KalaMain: Tempo Edit button clicked";

    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    Timeline *timeline = scoreCanvasWindow->getTimeline();

    // Get the selected marker's time from the spinbox - this is what we're editing
    double selectedMarkerTime = getSelectedTempoMarkerTime();

    // Get current timeline position - for reference
    double timelinePosition = timeline->getNowMarker();

    // Check if we're editing the default (spinbox shows marker 1)
    bool isEditingDefault = (selectedMarkerTime == 0.0);

    // Get the tempo/time signature for the selected marker (for pre-filling the dialog)
    TempoTimeSignature currentTts = scoreCanvas->getTempoTimeSignatureAtTime(selectedMarkerTime);

    // Can only delete non-default markers
    bool canDelete = (selectedMarkerTime > 0.0);

    TempoDialog dialog(currentTts, !canDelete, this);  // Disable delete if we can't delete
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.isDeleteRequested() && canDelete) {
            // Delete the marker that's selected in the spinbox
            scoreCanvas->removeTempoChange(selectedMarkerTime);
            qDebug() << "KalaMain: Deleted tempo marker at" << selectedMarkerTime << "ms";
        } else {
            TempoTimeSignature newTts = dialog.getTempoTimeSignature();

            if (isEditingDefault) {
                // At time 0, update the defaults
                double oldTempo = scoreCanvas->getDefaultTempo();
                double newTempo = newTts.bpm;

                scoreCanvas->setDefaultTempo(newTts.bpm);
                scoreCanvas->setDefaultTimeSignature(newTts.timeSigNumerator, newTts.timeSigDenominator);

                // Scale note times to maintain musical positions when tempo changes
                if (oldTempo != newTempo && oldTempo > 0) {
                    double scaleFactor = oldTempo / newTempo;
                    scoreCanvasWindow->scaleNoteTimes(scaleFactor);
                    qDebug() << "KalaMain: Scaled note times by factor" << scaleFactor
                             << "(tempo changed from" << oldTempo << "to" << newTempo << "BPM)";

                    // Re-render notes with new durations
                    scoreCanvasWindow->prerenderNotes();
                }

                qDebug() << "KalaMain: Updated default tempo to" << newTts.bpm << "BPM,"
                         << newTts.timeSigNumerator << "/" << newTts.timeSigDenominator;
            } else {
                // Edit the marker selected in the spinbox
                scoreCanvas->addTempoChange(selectedMarkerTime, newTts);
                qDebug() << "KalaMain: Updated tempo marker at" << selectedMarkerTime << "ms";
            }
        }

        // Update the tempo display
        updateTempoDisplay();

        // Update timeline visual indicators
        QMap<double, TempoTimeSignature> tempoChangesMap = scoreCanvas->getTempoChanges();
        QMap<double, QString> tempoDescsForTimeline;
        for (auto it = tempoChangesMap.constBegin(); it != tempoChangesMap.constEnd(); ++it) {
            QString desc = QString("%1").arg(static_cast<int>(it.value().bpm));
            if (it.value().gradualTransition) {
                desc += "~";  // Indicate gradual transition
            }
            tempoDescsForTimeline[it.key()] = desc;
        }
        timeline->setTempoChanges(tempoDescsForTimeline);

        // Mark project as dirty
        markProjectDirty();
    }
}

void KalaMain::onTempoAddClicked()
{
    qDebug() << "KalaMain: Tempo Add button clicked";

    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    Timeline *timeline = scoreCanvasWindow->getTimeline();

    // Get current timeline position - this is where we create the new marker
    double timelinePosition = timeline->getNowMarker();

    // Don't allow adding at time 0 (that's the default, use Edit instead)
    if (timelinePosition == 0.0) {
        qDebug() << "KalaMain: Cannot add marker at time 0, use Edit to modify default";
        return;
    }

    // Check if there's already a marker at this position
    if (scoreCanvas->getTempoChanges().contains(timelinePosition)) {
        qDebug() << "KalaMain: Marker already exists at" << timelinePosition << "ms, use Edit to modify";
        return;
    }

    // Get the tempo/time signature at this time (for pre-filling the dialog)
    TempoTimeSignature currentTts = scoreCanvas->getTempoTimeSignatureAtTime(timelinePosition);

    // Open the tempo dialog (delete disabled since we're creating new)
    TempoDialog dialog(currentTts, true, this);  // true = disable delete
    if (dialog.exec() == QDialog::Accepted) {
        TempoTimeSignature newTts = dialog.getTempoTimeSignature();

        // Create the new marker at the timeline position
        scoreCanvas->addTempoChange(timelinePosition, newTts);
        qDebug() << "KalaMain: Created new tempo marker at" << timelinePosition << "ms:"
                 << newTts.bpm << "BPM," << newTts.timeSigNumerator << "/" << newTts.timeSigDenominator;

        // Update the tempo display
        updateTempoDisplay();

        // Update timeline visual indicators
        QMap<double, TempoTimeSignature> tempoChangesMap = scoreCanvas->getTempoChanges();
        QMap<double, QString> tempoDescsForTimeline;
        for (auto it = tempoChangesMap.constBegin(); it != tempoChangesMap.constEnd(); ++it) {
            QString desc = QString("%1").arg(static_cast<int>(it.value().bpm));
            if (it.value().gradualTransition) {
                desc += "~";
            }
            tempoDescsForTimeline[it.key()] = desc;
        }
        timeline->setTempoChanges(tempoDescsForTimeline);

        // Mark project as dirty
        markProjectDirty();
    }
}

void KalaMain::updateTempoDisplay()
{
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    Timeline *timeline = scoreCanvasWindow->getTimeline();
    double nowTime = timeline->getNowMarker();

    // Build list of all tempo markers: default (time 0) + any changes
    QMap<double, TempoTimeSignature> changes = scoreCanvas->getTempoChanges();
    int totalMarkers = 1 + changes.size();  // Default + changes

    // Find which marker index corresponds to the current timeline position
    // Marker 1 is default (time 0), marker 2+ are tempo changes in time order
    int currentMarkerIndex = 1;  // Start with default
    QList<double> sortedTimes = changes.keys();
    std::sort(sortedTimes.begin(), sortedTimes.end());
    for (int i = 0; i < sortedTimes.size(); ++i) {
        if (sortedTimes[i] <= nowTime) {
            currentMarkerIndex = i + 2;  // +2 because marker 1 is default
        } else {
            break;
        }
    }

    // Update spinbox range and selection (block signals to prevent double updates)
    ui->spinTempoMarker->blockSignals(true);
    ui->spinTempoMarker->setMaximum(totalMarkers);
    ui->spinTempoMarker->setValue(currentMarkerIndex);
    ui->spinTempoMarker->blockSignals(false);

    // Update the display for the selected marker
    onTempoMarkerChanged(currentMarkerIndex);
}

void KalaMain::onTempoMarkerChanged(int markerIndex)
{
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();

    // Get the time and tempo for this marker
    double markerTime = getSelectedTempoMarkerTime();
    TempoTimeSignature tts = scoreCanvas->getTempoTimeSignatureAtTime(markerTime);

    // Format timestamp as Min:Sec:Ms
    int totalMs = static_cast<int>(markerTime);
    int ms = totalMs % 1000;
    int totalSeconds = totalMs / 1000;
    int seconds = totalSeconds % 60;
    int minutes = totalSeconds / 60;
    QString timestamp = QString("%1:%2:%3")
                            .arg(minutes)
                            .arg(seconds, 2, 10, QChar('0'))
                            .arg(ms, 3, 10, QChar('0'));

    // Build display string: timestamp | tempo | time signature
    QString displayStr;
    if (tts.timeSigDenominator == 0) {
        // Simple mode: show just numerator
        displayStr = QString("%1 | %2 BPM | %3")
                         .arg(timestamp)
                         .arg(static_cast<int>(tts.bpm))
                         .arg(tts.timeSigNumerator);
    } else {
        displayStr = QString("%1 | %2 BPM | %3/%4")
                         .arg(timestamp)
                         .arg(static_cast<int>(tts.bpm))
                         .arg(tts.timeSigNumerator)
                         .arg(tts.timeSigDenominator);
    }

    // Add gradual transition indicator if enabled
    if (tts.gradualTransition) {
        displayStr += " ~";
    }

    ui->labelTempoDisplay->setText(displayStr);
}

double KalaMain::getSelectedTempoMarkerTime() const
{
    ScoreCanvas *scoreCanvas = scoreCanvasWindow->getScoreCanvas();
    int markerIndex = ui->spinTempoMarker->value();

    // Marker 1 is always the default at time 0
    if (markerIndex == 1) {
        return 0.0;
    }

    // For markers > 1, get from the sorted list of tempo changes
    QMap<double, TempoTimeSignature> changes = scoreCanvas->getTempoChanges();
    QList<double> sortedTimes = changes.keys();
    std::sort(sortedTimes.begin(), sortedTimes.end());

    int changeIndex = markerIndex - 2;  // Subtract 2 because marker 1 is default, marker 2 is first change
    if (changeIndex >= 0 && changeIndex < sortedTimes.size()) {
        return sortedTimes[changeIndex];
    }

    return 0.0;  // Fallback
}

// ========== Variation Management ==========

void KalaMain::refreshVariationsList()
{
    ui->listVariations->clear();

    Track* track = trackManager->getCurrentTrack();
    if (!track) {
        ui->editVariationsTrackName->clear();
        ui->editVariationsTrackName->setEnabled(false);
        return;
    }

    ui->editVariationsTrackName->setText(track->getName());
    ui->editVariationsTrackName->setEnabled(true);

    // Add base sounit as item 0, labelled with the sounit name
    QString baseName = track->getSounitName();
    if (baseName.isEmpty()) baseName = track->getName();
    ui->listVariations->addItem(QString("0: %1").arg(baseName));

    // Add all variations (skip internal ones created by Note Mode)
    int varCount = track->getVariationCount();
    for (int i = 1; i <= varCount; ++i) {
        if (track->isInternalVariation(i)) continue;
        QString name = track->getVariationName(i);
        ui->listVariations->addItem(QString("%1: %2").arg(i).arg(name));
    }

    qDebug() << "KalaMain: Refreshed variations list -" << varCount << "variations for track" << track->getName();
}

void KalaMain::onVariationSelectionChanged()
{
    int row = ui->listVariations->currentRow();
    // Can only delete/rename variations (row > 0), not base sounit (row == 0)
    bool hasSelection = (row > 0);
    ui->btnDeleteVariation->setEnabled(hasSelection);
    ui->btnRenameVariation->setEnabled(hasSelection);
}

void KalaMain::onDeleteVariation()
{
    QListWidgetItem *item = ui->listVariations->currentItem();
    if (!item) return;

    // Extract the actual variation index from the item text ("N: name" format)
    int varIndex = item->text().section(':', 0, 0).trimmed().toInt();
    if (varIndex <= 0) return;  // Can't delete base sounit

    Track* track = trackManager->getCurrentTrack();
    if (!track) return;

    QString varName = track->getVariationName(varIndex);

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Variation",
        QString("Delete variation '%1'?\n\n"
                "Notes using this variation will be reset to the base sounit.").arg(varName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (track->deleteVariation(varIndex)) {
            refreshVariationsList();
            qDebug() << "KalaMain: Deleted variation" << varIndex << ":" << varName;
        }
    }
}

void KalaMain::onRenameVariation()
{
    QListWidgetItem *item = ui->listVariations->currentItem();
    if (!item) return;

    // Extract the actual variation index from the item text ("N: name" format)
    int varIndex = item->text().section(':', 0, 0).trimmed().toInt();
    if (varIndex <= 0) return;  // Can't rename base sounit

    Track* track = trackManager->getCurrentTrack();
    if (!track) return;

    QString currentName = track->getVariationName(varIndex);

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Variation",
                                           "New name:", QLineEdit::Normal,
                                           currentName, &ok);

    if (ok && !newName.isEmpty() && newName != currentName) {
        if (track->setVariationName(varIndex, newName)) {
            refreshVariationsList();
            qDebug() << "KalaMain: Renamed variation" << varIndex << "to:" << newName;
        }
    }
}

void KalaMain::onVariationsTrackNameEdited()
{
    Track *track = trackManager->getCurrentTrack();
    if (!track) return;

    QString newName = ui->editVariationsTrackName->text().trimmed();
    if (newName.isEmpty()) {
        ui->editVariationsTrackName->setText(track->getName());
        return;
    }

    // Determine which tracks to rename: all selected, or just the current one
    QVector<int> targets = m_selectedTrackIndices.isEmpty()
        ? QVector<int>{ trackManager->getCurrentTrackIndex() }
        : m_selectedTrackIndices;

    TrackSelector *trackSelector = scoreCanvasWindow->getTrackSelector();

    for (int idx : targets) {
        Track *t = trackManager->getTrack(idx);
        if (!t || t->getName() == newName) continue;
        t->setName(newName);
        trackSelector->updateTrack(idx, newName, t->getColor());
        qDebug() << "KalaMain: Renamed track" << idx << "to:" << newName;
    }

    trackSelector->update();
    markProjectDirty();
}

void KalaMain::updateVariationSelector()
{
    // Block signals while updating
    ui->comboVariationSelector->blockSignals(true);

    int previousIndex = ui->comboVariationSelector->currentIndex();
    ui->comboVariationSelector->clear();

    // Add base sounit as item 0, labelled with the sounit name
    Track *track = trackManager->getCurrentTrack();
    if (track) {
        QString baseName = track->getSounitName();
        if (baseName.isEmpty()) baseName = track->getName();
        ui->comboVariationSelector->addItem(baseName);

        int varCount = track->getVariationCount();
        for (int i = 1; i <= varCount; ++i) {
            if (track->isInternalVariation(i)) continue;
            QString name = track->getVariationName(i);
            ui->comboVariationSelector->addItem(QString("%1: %2").arg(i).arg(name));
        }
    } else {
        ui->comboVariationSelector->addItem("Base Sounit");
    }

    // Restore previous selection if valid, otherwise select base
    if (previousIndex >= 0 && previousIndex < ui->comboVariationSelector->count()) {
        ui->comboVariationSelector->setCurrentIndex(previousIndex);
    } else {
        ui->comboVariationSelector->setCurrentIndex(0);
    }

    ui->comboVariationSelector->blockSignals(false);

    qDebug() << "KalaMain: Updated variation selector -"
             << (ui->comboVariationSelector->count() - 1) << "variations";
}

void KalaMain::onVariationSelectorChanged(int index)
{
    if (index < 0) return;

    Track *track = trackManager->getCurrentTrack();
    if (!track) return;

    // Save base state before loading any variation (only if not already saved)
    if (index > 0 && !track->hasBaseCanvasState()) {
        track->saveBaseCanvasState();
    }

    // Load selected variation (or base sounit if index == 0)
    qDebug() << "KalaMain: Loading variation index" << index << "to canvas";

    if (track->loadVariationToCanvas(index)) {
        // Rebuild the graph with the new canvas state
        sounitBuilder->rebuildGraph(track->getTrackId());

        // Reset pending connection state (old containers were deleted)
        // and reconnect signals for the new containers
        Canvas *canvas = track->getCanvas();
        canvas->cancelPendingConnection();  // Clear any pending connection state
        for (Container *container : canvas->findChildren<Container*>()) {
            sounitBuilder->connectContainerSignals(container);
        }

        // Clear inspector since selection is now invalid
        onSelectionCleared();

        qDebug() << "KalaMain: Loaded variation index" << index << "successfully";
    } else {
        if (index == 0) {
            qDebug() << "KalaMain: Already showing base sounit";
        } else {
            qWarning() << "KalaMain: Failed to load variation" << index;
        }
    }
}

// ========== Note Inspector ==========

void KalaMain::onNoteSelectionChanged()
{
    updateNoteInspector();
}

void KalaMain::updateNoteInspector()
{
    qDebug() << "KalaMain::updateNoteInspector() CALLED";

    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();

    if (selectedIndices.isEmpty()) {
        // No selection - disable inspector widgets
        ui->spinNoteStart->setEnabled(false);
        ui->spinNoteDuration->setEnabled(false);
        ui->spinNotePitch->setEnabled(false);
        ui->noteVariationComboBox->setEnabled(false);
        ui->checkNoteVibrato->setEnabled(false);
        ui->btnNoteVibratoEdit->setEnabled(false);
        ui->comboExpressiveCurve->setEnabled(false);
        ui->btnExpressiveCurveAdd->setEnabled(false);
        ui->btnExpressiveCurveDelete->setEnabled(false);
        ui->btnExpressiveCurveApply->setEnabled(false);

        // Clear values
        m_updatingNoteInspector = true;
        ui->spinNoteStart->setValue(0.0);
        ui->spinNoteDuration->setValue(0.0);
        ui->spinNotePitch->setValue(0);
        ui->noteVariationComboBox->setCurrentIndex(-1);
        ui->checkNoteVibrato->setChecked(false);
        ui->comboExpressiveCurve->clear();
        m_updatingNoteInspector = false;
        return;
    }

    // Enable inspector widgets
    ui->spinNoteStart->setEnabled(true);
    ui->spinNoteDuration->setEnabled(true);
    ui->spinNotePitch->setEnabled(true);
    ui->noteVariationComboBox->setEnabled(true);
    ui->checkNoteVibrato->setEnabled(true);
    ui->btnNoteVibratoEdit->setEnabled(true);
    ui->comboExpressiveCurve->setEnabled(true);
    ui->btnExpressiveCurveAdd->setEnabled(true);

    // Get the first selected note (for single selection display)
    const QVector<Note> &notes = canvas->getPhrase().getNotes();
    if (selectedIndices.first() >= notes.size()) return;
    const Note &note = notes[selectedIndices.first()];

    // Update variation dropdown for current track
    updateNoteVariationComboBox();

    // Update expressive curves dropdown
    updateExpressiveCurvesDropdown();

    // Populate inspector with note values (prevent recursive signal handling)
    m_updatingNoteInspector = true;

    ui->spinNoteStart->setValue(note.getStartTime());
    ui->spinNoteDuration->setValue(note.getDuration());
    ui->spinNotePitch->setValue(static_cast<int>(note.getPitchHz()));

    // Set variation index (0 = base sounit, 1+ = variations)
    int varIndex = note.getVariationIndex();
    if (varIndex >= 0 && varIndex < ui->noteVariationComboBox->count()) {
        ui->noteVariationComboBox->setCurrentIndex(varIndex);
    } else {
        ui->noteVariationComboBox->setCurrentIndex(0);  // Default to base sounit
    }

    // Set vibrato checkbox
    qDebug() << "KalaMain::updateNoteInspector - Setting checkbox to vibrato.active ="
             << note.getVibrato().active;
    ui->checkNoteVibrato->setChecked(note.getVibrato().active);

    m_updatingNoteInspector = false;

    qDebug() << "KalaMain: Note inspector updated - start:" << note.getStartTime()
             << "duration:" << note.getDuration()
             << "pitch:" << note.getPitchHz()
             << "variation:" << note.getVariationIndex()
             << "vibrato.active:" << note.getVibrato().active;
}

void KalaMain::updateNoteVariationComboBox()
{
    // Block signals while updating
    ui->noteVariationComboBox->blockSignals(true);

    ui->noteVariationComboBox->clear();

    // Add base sounit as item 0, labelled with the sounit name
    Track *track = trackManager->getCurrentTrack();
    if (track) {
        QString baseName = track->getSounitName();
        if (baseName.isEmpty()) baseName = track->getName();
        ui->noteVariationComboBox->addItem(QString("0: %1").arg(baseName));

        int varCount = track->getVariationCount();
        for (int i = 1; i <= varCount; ++i) {
            if (track->isInternalVariation(i)) continue;
            QString name = track->getVariationName(i);
            ui->noteVariationComboBox->addItem(QString("%1: %2").arg(i).arg(name));
        }
    } else {
        ui->noteVariationComboBox->addItem("0: Base Sounit");
    }

    ui->noteVariationComboBox->blockSignals(false);
}

void KalaMain::onNoteStartChanged(double value)
{
    if (m_updatingNoteInspector) return;

    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) return;

    QVector<Note> &notes = canvas->getPhrase().getNotes();
    for (int index : selectedIndices) {
        if (index >= 0 && index < notes.size()) {
            notes[index].setStartTime(value);
            notes[index].setRenderDirty(true);
        }
    }

    canvas->update();
    markProjectDirty();
    qDebug() << "KalaMain: Note start changed to" << value << "ms";
}

void KalaMain::onNoteDurationChanged(double value)
{
    if (m_updatingNoteInspector) return;

    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) return;

    QVector<Note> &notes = canvas->getPhrase().getNotes();
    for (int index : selectedIndices) {
        if (index >= 0 && index < notes.size()) {
            notes[index].setDuration(value);
            notes[index].setRenderDirty(true);
        }
    }

    canvas->update();
    markProjectDirty();
    qDebug() << "KalaMain: Note duration changed to" << value << "ms";
}

void KalaMain::onNotePitchChanged(int value)
{
    if (m_updatingNoteInspector) return;

    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) return;

    QVector<Note> &notes = canvas->getPhrase().getNotes();
    for (int index : selectedIndices) {
        if (index >= 0 && index < notes.size()) {
            notes[index].setPitchHz(static_cast<double>(value));
            notes[index].setRenderDirty(true);
        }
    }

    canvas->update();
    markProjectDirty();
    qDebug() << "KalaMain: Note pitch changed to" << value << "Hz";
}

void KalaMain::onNoteVariationChanged(int index)
{
    if (m_updatingNoteInspector) return;

    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) return;

    QVector<Note> &notes = canvas->getPhrase().getNotes();
    for (int noteIndex : selectedIndices) {
        if (noteIndex >= 0 && noteIndex < notes.size()) {
            notes[noteIndex].setVariationIndex(index);
            notes[noteIndex].setRenderDirty(true);
        }
    }

    canvas->update();
    markProjectDirty();
    qDebug() << "KalaMain: Note variation changed to" << index;
}

void KalaMain::onNoteVibratoToggled(bool checked)
{
    qDebug() << "KalaMain::onNoteVibratoToggled ENTER - checked:" << checked
             << "m_updatingNoteInspector:" << m_updatingNoteInspector;

    if (m_updatingNoteInspector) {
        qDebug() << "KalaMain::onNoteVibratoToggled - SKIPPED (updating inspector)";
        return;
    }

    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) {
        qDebug() << "KalaMain::onNoteVibratoToggled - SKIPPED (no selection)";
        return;
    }

    QVector<Note> &notes = canvas->getPhrase().getNotes();
    for (int noteIndex : selectedIndices) {
        if (noteIndex >= 0 && noteIndex < notes.size()) {
            qDebug() << "KalaMain: Setting vibrato.active =" << checked
                     << "for note" << noteIndex
                     << "(was:" << notes[noteIndex].getVibrato().active << ")";
            notes[noteIndex].getVibrato().active = checked;
            notes[noteIndex].setRenderDirty(true);
            qDebug() << "KalaMain: After set, vibrato.active ="
                     << notes[noteIndex].getVibrato().active;
        }
    }

    canvas->update();
    markProjectDirty();

    qDebug() << "KalaMain: Note vibrato toggled to" << checked;
}

void KalaMain::onNoteVibratoEditClicked()
{
    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) return;

    QVector<Note> &notes = canvas->getPhrase().getNotes();
    if (selectedIndices.first() >= notes.size()) return;

    // Get vibrato from first selected note
    Vibrato vibrato = notes[selectedIndices.first()].getVibrato();

    // Open editor dialog
    VibratoEditorDialog dialog(this);
    dialog.setVibrato(vibrato);

    if (dialog.exec() == QDialog::Accepted) {
        Vibrato newVibrato = dialog.getVibrato();

        // Apply to all selected notes
        for (int noteIndex : selectedIndices) {
            if (noteIndex >= 0 && noteIndex < notes.size()) {
                notes[noteIndex].setVibrato(newVibrato);
                notes[noteIndex].setRenderDirty(true);
            }
        }

        // Update checkbox to reflect active state (guard to prevent retriggering toggle)
        m_updatingNoteInspector = true;
        ui->checkNoteVibrato->setChecked(newVibrato.active);
        m_updatingNoteInspector = false;

        canvas->update();
        markProjectDirty();

        qDebug() << "KalaMain: Vibrato edited for" << selectedIndices.size() << "note(s)";
    }
}

// ============================================================================
// Expressive Curves Inspector
// ============================================================================

void KalaMain::updateExpressiveCurvesDropdown()
{
    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QVector<int> &selectedIndices = canvas->getSelectedNoteIndices();
    if (selectedIndices.isEmpty()) return;

    const QVector<Note> &notes = canvas->getPhrase().getNotes();
    if (selectedIndices.first() >= notes.size()) return;
    const Note &firstNote = notes[selectedIndices.first()];

    ui->comboExpressiveCurve->blockSignals(true);
    ui->comboExpressiveCurve->clear();

    int count = firstNote.getExpressiveCurveCount();
    for (int i = 0; i < count; ++i) {
        ui->comboExpressiveCurve->addItem(firstNote.getExpressiveCurveName(i));
    }

    // If the active named curve is not present on the newly selected note, fall back to Dynamics.
    // This keeps the dropdown and canvas consistent: you never silently end up showing a curve
    // name in the dropdown that doesn't match what the canvas is actually drawing.
    int activeIdx = canvas->getActiveExpressiveCurveIndex();
    const QString activeName = canvas->getActiveExpressiveCurveName();
    if (activeIdx > 0 && activeName != QStringLiteral("Dynamics") && !activeName.isEmpty()) {
        bool noteHasActiveCurve = false;
        for (int i = 1; i < firstNote.getExpressiveCurveCount(); ++i) {
            if (firstNote.getExpressiveCurveName(i) == activeName) {
                noteHasActiveCurve = true;
                break;
            }
        }
        if (!noteHasActiveCurve) {
            activeIdx = 0;
            canvas->setActiveExpressiveCurveIndex(0, QStringLiteral("Dynamics"));
        }
    }
    if (activeIdx >= count) activeIdx = 0;
    ui->comboExpressiveCurve->setCurrentIndex(activeIdx);

    ui->comboExpressiveCurve->blockSignals(false);

    // Delete enabled whenever a non-Dynamics curve is selected (deletes from all notes)
    bool canDelete = (activeIdx > 0);
    ui->btnExpressiveCurveDelete->setEnabled(canDelete);

    // Apply enabled when there is at least one named curve anywhere in the phrase
    const QVector<Note> &allNotes = canvas->getPhrase().getNotes();
    bool hasNamedCurves = false;
    for (const Note &n : allNotes) {
        if (n.getExpressiveCurveCount() > 1) {
            hasNamedCurves = true;
            break;
        }
    }
    ui->btnExpressiveCurveApply->setEnabled(hasNamedCurves);
}

void KalaMain::onExpressiveCurveChanged(int index)
{
    if (m_updatingNoteInspector) return;
    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    const QString curveName = ui->comboExpressiveCurve->itemText(index);
    canvas->setActiveExpressiveCurveIndex(index, curveName);
    bool canDelete = (index > 0);
    ui->btnExpressiveCurveDelete->setEnabled(canDelete);
}

void KalaMain::onExpressiveCurveAdd()
{
    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    if (canvas->getSelectedNoteIndices().isEmpty()) return;

    // Collect existing named curve names across ALL notes in the phrase
    QSet<QString> existingNames;
    const QVector<Note> &allNotes = canvas->getPhrase().getNotes();
    for (const Note &n : allNotes) {
        for (int i = 1; i < n.getExpressiveCurveCount(); ++i)
            existingNames.insert(n.getExpressiveCurveName(i));
    }

    // Ask for a name, re-prompting on duplicate
    int curveN = ui->comboExpressiveCurve->count();
    QString defaultName = QString("Curve %1").arg(curveN);
    bool ok = false;
    QString name;
    while (true) {
        name = QInputDialog::getText(this, "Add Expressive Curve",
                                     "Curve name:", QLineEdit::Normal,
                                     defaultName, &ok);
        if (!ok || name.trimmed().isEmpty()) return;
        if (!existingNames.contains(name.trimmed())) break;
        QMessageBox::warning(this, "Duplicate Curve Name",
                             QString("A curve named \"%1\" already exists in this track.\n"
                                     "Please choose a unique name.").arg(name.trimmed()));
        defaultName = name;  // Keep what the user typed as the next default
    }

    // Show DynamicsCurveDialog to define the curve shape
    const QVector<int> &sel = canvas->getSelectedNoteIndices();
    const QVector<Note> &notes = canvas->getPhrase().getNotes();
    double startTime = std::numeric_limits<double>::max();
    double endTime   = std::numeric_limits<double>::lowest();
    for (int idx : sel) {
        if (idx >= 0 && idx < notes.size()) {
            startTime = qMin(startTime, notes[idx].getStartTime());
            endTime   = qMax(endTime,   notes[idx].getStartTime() + notes[idx].getDuration());
        }
    }
    double timeSpanMs = (endTime > startTime) ? (endTime - startTime) : 1000.0;

    DynamicsCurveDialog dialog(sel.size(), timeSpanMs, this);
    if (dialog.exec() != QDialog::Accepted) return;

    QVector<EnvelopePoint> curve = dialog.getCurve();
    double weight = dialog.getWeight();
    bool perNote = dialog.getPerNoteMode();

    canvas->addExpressiveCurveToSelection(name.trimmed(), curve, weight, perNote);

    // Select the newly added curve
    updateExpressiveCurvesDropdown();
    int newIdx = ui->comboExpressiveCurve->count() - 1;
    if (newIdx > 0) {
        ui->comboExpressiveCurve->setCurrentIndex(newIdx);
        canvas->setActiveExpressiveCurveIndex(newIdx, name.trimmed());
        ui->btnExpressiveCurveDelete->setEnabled(newIdx > 0);
    }

    markProjectDirty();
}

void KalaMain::onExpressiveCurveDelete()
{
    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    int currentIndex = ui->comboExpressiveCurve->currentIndex();
    if (currentIndex <= 0) return;

    const QString curveName = ui->comboExpressiveCurve->itemText(currentIndex);
    canvas->removeExpressiveCurveByName(curveName);

    updateExpressiveCurvesDropdown();
    ui->comboExpressiveCurve->setCurrentIndex(0);
    canvas->setActiveExpressiveCurveIndex(0, QStringLiteral("Dynamics"));
    ui->btnExpressiveCurveDelete->setEnabled(false);

    markProjectDirty();
}

void KalaMain::onExpressiveCurveApply()
{
    ScoreCanvas *canvas = scoreCanvasWindow->getScoreCanvas();
    if (canvas->getSelectedNoteIndices().isEmpty()) return;

    // Collect unique named curves from all notes in the phrase (name -> first Curve found)
    QMap<QString, Curve> namedCurves;
    const QVector<Note> &allNotes = canvas->getPhrase().getNotes();
    for (const Note &note : allNotes) {
        for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
            const QString &curveName = note.getExpressiveCurveName(i);
            if (!namedCurves.contains(curveName))
                namedCurves[curveName] = note.getExpressiveCurve(i);
        }
    }
    if (namedCurves.isEmpty()) return;

    // Build a simple dialog with a combo box
    QDialog dialog(this);
    dialog.setWindowTitle("Apply Named Curve");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel("Select curve to apply to selection:", &dialog);
    QComboBox *combo = new QComboBox(&dialog);
    for (const QString &curveName : namedCurves.keys())
        combo->addItem(curveName);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addWidget(label);
    layout->addWidget(combo);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) return;

    const QString selectedName = combo->currentText();
    canvas->applyNamedCurveToSelection(selectedName, namedCurves[selectedName]);

    // Refresh the dropdown and activate the applied curve so the canvas draws it
    updateExpressiveCurvesDropdown();
    const QVector<int> &sel = canvas->getSelectedNoteIndices();
    const QVector<Note> &updatedNotes = canvas->getPhrase().getNotes();
    if (!sel.isEmpty() && sel.first() < updatedNotes.size()) {
        const Note &firstNote = updatedNotes[sel.first()];
        for (int i = 1; i < firstNote.getExpressiveCurveCount(); ++i) {
            if (firstNote.getExpressiveCurveName(i) == selectedName) {
                ui->comboExpressiveCurve->setCurrentIndex(i);
                canvas->setActiveExpressiveCurveIndex(i, selectedName);
                ui->btnExpressiveCurveDelete->setEnabled(true);
                break;
            }
        }
    }

    markProjectDirty();
}

// ============================================================================
// Settings Tab Implementation
// ============================================================================

void KalaMain::populateSettingsTab()
{
    qDebug() << "populateSettingsTab() called";

    // Get the preferences tab widget
    QWidget *preferencesTab = ui->preferencesTab;
    if (!preferencesTab) {
        qWarning() << "Preferences tab not found";
        return;
    }

    // Hide the existing label_15 if it exists (from the UI file)
    QLabel *oldLabel = preferencesTab->findChild<QLabel*>("label_15");
    if (oldLabel) {
        oldLabel->hide();
        oldLabel->deleteLater();
    }

    // Get the existing layout or create a new one
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(preferencesTab->layout());
    if (!mainLayout) {
        mainLayout = new QVBoxLayout(preferencesTab);
    }

    // Clear any existing items from the layout (except the one we're deleting)
    while (mainLayout->count() > 0) {
        QLayoutItem *item = mainLayout->takeAt(0);
        if (item->widget() && item->widget() != oldLabel) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create scroll area for all settings
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollWidget = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(15);

    // ========== PROJECT SETTINGS SECTION ==========
    QLabel *projectTitle = new QLabel("Project Settings");
    projectTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 5px;");
    scrollLayout->addWidget(projectTitle);

    QLabel *projectDesc = new QLabel("Audio rendering and export settings for this project.");
    projectDesc->setWordWrap(true);
    projectDesc->setStyleSheet("color: gray; margin-bottom: 10px;");
    scrollLayout->addWidget(projectDesc);

    QGroupBox *projectGroup = new QGroupBox("Audio Settings");
    projectGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *projectLayout = new QFormLayout(projectGroup);

    // Project Name
    QLineEdit *projectNameEdit = new QLineEdit();
    CompositionSettings currentSettings = scoreCanvasWindow->getSettings();
    projectNameEdit->setText(currentSettings.compositionName);
    projectNameEdit->setPlaceholderText("Enter project name");
    connect(projectNameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        CompositionSettings settings = scoreCanvasWindow->getSettings();
        settings.compositionName = text;
        scoreCanvasWindow->updateFromSettings(settings);
        markProjectDirty();
        updateWindowTitle();  // Update title immediately when name changes
    });
    projectLayout->addRow("Project Name:", projectNameEdit);

    // Duration
    QTimeEdit *durationEdit = new QTimeEdit();
    durationEdit->setDisplayFormat("HH:mm:ss");
    durationEdit->setTimeRange(QTime(0, 0, 1), QTime(23, 59, 59));
    {
        int totalSec = static_cast<int>(currentSettings.lengthMs / 1000.0);
        durationEdit->setTime(QTime(totalSec / 3600, (totalSec % 3600) / 60, totalSec % 60));
    }
    connect(durationEdit, &QTimeEdit::timeChanged, this, [this](const QTime &t) {
        double newLengthMs = (t.hour() * 3600 + t.minute() * 60 + t.second()) * 1000.0;
        if (newLengthMs > 0) {
            CompositionSettings settings = scoreCanvasWindow->getSettings();
            settings.lengthMs = newLengthMs;
            settings.syncLengthFromMs();
            scoreCanvasWindow->updateFromSettings(settings);
            markProjectDirty();
        }
    });
    projectLayout->addRow("Duration:", durationEdit);

    // Sample Rate
    QComboBox *sampleRateCombo = new QComboBox();
    sampleRateCombo->addItem("44100 Hz", 44100);
    sampleRateCombo->addItem("48000 Hz", 48000);
    sampleRateCombo->addItem("96000 Hz", 96000);
    sampleRateCombo->addItem("192000 Hz", 192000);
    // Set current value
    int currentSampleRate = currentSettings.sampleRate;
    int srIndex = sampleRateCombo->findData(currentSampleRate);
    if (srIndex >= 0) sampleRateCombo->setCurrentIndex(srIndex);
    connect(sampleRateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, sampleRateCombo](int index) {
        CompositionSettings settings = scoreCanvasWindow->getSettings();
        settings.sampleRate = sampleRateCombo->itemData(index).toInt();
        scoreCanvasWindow->updateFromSettings(settings);
        markProjectDirty();
    });
    projectLayout->addRow("Sample Rate:", sampleRateCombo);

    // Bit Depth
    QComboBox *bitDepthCombo = new QComboBox();
    bitDepthCombo->addItem("16-bit", 16);
    bitDepthCombo->addItem("24-bit", 24);
    bitDepthCombo->addItem("32-bit", 32);
    // Set current value
    int currentBitDepth = currentSettings.bitDepth;
    int bdIndex = bitDepthCombo->findData(currentBitDepth);
    if (bdIndex >= 0) bitDepthCombo->setCurrentIndex(bdIndex);
    connect(bitDepthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, bitDepthCombo](int index) {
        CompositionSettings settings = scoreCanvasWindow->getSettings();
        settings.bitDepth = bitDepthCombo->itemData(index).toInt();
        scoreCanvasWindow->updateFromSettings(settings);
        markProjectDirty();
    });
    projectLayout->addRow("Bit Depth:", bitDepthCombo);

    scrollLayout->addWidget(projectGroup);

    // Separator
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    scrollLayout->addWidget(separator);

    // ========== CONTAINER PARAMETER SETTINGS SECTION ==========
    QLabel *containerTitle = new QLabel("Container Parameter Settings");
    containerTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 5px;");
    scrollLayout->addWidget(containerTitle);

    QLabel *containerDesc = new QLabel("Configure the min/max ranges for container parameter sliders. "
                                        "These settings affect the Sound Engine inspector controls.");
    containerDesc->setWordWrap(true);
    containerDesc->setStyleSheet("color: gray; margin-bottom: 10px;");
    scrollLayout->addWidget(containerDesc);

    // Harmonic Generator Settings
    QGroupBox *harmonicGroup = new QGroupBox("Harmonic Generator");
    harmonicGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *harmonicLayout = new QFormLayout(harmonicGroup);
    populateHarmonicGeneratorSettings(harmonicLayout);
    scrollLayout->addWidget(harmonicGroup);

    // Rolloff Processor Settings
    QGroupBox *rolloffGroup = new QGroupBox("Rolloff Processor");
    rolloffGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *rolloffLayout = new QFormLayout(rolloffGroup);
    populateRolloffProcessorSettings(rolloffLayout);
    scrollLayout->addWidget(rolloffGroup);

    // Formant Body Settings
    QGroupBox *formantGroup = new QGroupBox("Formant Body");
    formantGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *formantLayout = new QFormLayout(formantGroup);
    populateFormantBodySettings(formantLayout);
    scrollLayout->addWidget(formantGroup);

    // Noise Color Filter Settings
    QGroupBox *noiseGroup = new QGroupBox("Noise Color Filter");
    noiseGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *noiseLayout = new QFormLayout(noiseGroup);
    populateNoiseColorFilterSettings(noiseLayout);
    scrollLayout->addWidget(noiseGroup);

    // 10-Band EQ Settings
    QGroupBox *eqGroup = new QGroupBox("10-Band EQ");
    eqGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *eqLayout = new QFormLayout(eqGroup);
    populateBandpassEQSettings(eqLayout);
    scrollLayout->addWidget(eqGroup);

    // Comb Filter Settings
    QGroupBox *combGroup = new QGroupBox("Comb Filter");
    combGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *combLayout = new QFormLayout(combGroup);
    populateCombFilterSettings(combLayout);
    scrollLayout->addWidget(combGroup);

    // LP/HP Filter Settings
    QGroupBox *lpGroup = new QGroupBox("LP/HP Filter");
    lpGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *lpLayout = new QFormLayout(lpGroup);
    populateLowHighPassFilterSettings(lpLayout);
    scrollLayout->addWidget(lpGroup);

    // Physics System Settings
    QGroupBox *physicsGroup = new QGroupBox("Physics System");
    physicsGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *physicsLayout = new QFormLayout(physicsGroup);
    populatePhysicsSystemSettings(physicsLayout);
    scrollLayout->addWidget(physicsGroup);

    // Drift Engine Settings
    QGroupBox *driftGroup = new QGroupBox("Drift Engine");
    driftGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *driftLayout = new QFormLayout(driftGroup);
    populateDriftEngineSettings(driftLayout);
    scrollLayout->addWidget(driftGroup);

    // Gate Processor Settings - hidden from UI, code kept for potential future use
    // QGroupBox *gateGroup = new QGroupBox("Gate Processor");
    // gateGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    // QFormLayout *gateLayout = new QFormLayout(gateGroup);
    // populateGateProcessorSettings(gateLayout);
    // scrollLayout->addWidget(gateGroup);

    // Karplus Strong Settings
    QGroupBox *karplusGroup = new QGroupBox("Karplus Strong");
    karplusGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *karplusLayout = new QFormLayout(karplusGroup);
    populateKarplusStrongSettings(karplusLayout);
    scrollLayout->addWidget(karplusGroup);

    // Attack Settings
    QGroupBox *attackGroup = new QGroupBox("Attack");
    attackGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *attackLayout = new QFormLayout(attackGroup);
    populateAttackSettings(attackLayout);
    scrollLayout->addWidget(attackGroup);

    // LFO Settings
    QGroupBox *lfoGroup = new QGroupBox("LFO");
    lfoGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *lfoLayout = new QFormLayout(lfoGroup);
    populateLFOSettings(lfoLayout);
    scrollLayout->addWidget(lfoGroup);

    // Wavetable Synth Settings
    QGroupBox *wavetableGroup = new QGroupBox("Wavetable Synth");
    wavetableGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *wavetableLayout = new QFormLayout(wavetableGroup);
    populateWavetableSynthSettings(wavetableLayout);
    scrollLayout->addWidget(wavetableGroup);

    // Frequency Mapper Settings
    QGroupBox *freqMapGroup = new QGroupBox("Frequency Mapper");
    freqMapGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *freqMapLayout = new QFormLayout(freqMapGroup);
    populateFrequencyMapperSettings(freqMapLayout);
    scrollLayout->addWidget(freqMapGroup);

    // Easing Settings
    QGroupBox *easingGroup = new QGroupBox("Easing Functions");
    easingGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *easingLayout = new QFormLayout(easingGroup);
    populateEasingSettings(easingLayout);
    scrollLayout->addWidget(easingGroup);

    // ── AI Companion Settings ─────────────────────────────────────────────────
    QLabel *aiTitle = new QLabel("AI Companion");
    aiTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-top: 15px; margin-bottom: 5px;");
    scrollLayout->addWidget(aiTitle);

    QLabel *aiDesc = new QLabel(
        "Connect Kala Companion to any OpenAI-compatible LLM (local Ollama, OpenAI, Groq, OpenRouter) "
        "or to Anthropic Claude. Changes take effect the next time you send a message.");
    aiDesc->setWordWrap(true);
    aiDesc->setStyleSheet("color: gray; margin-bottom: 10px;");
    scrollLayout->addWidget(aiDesc);

    QGroupBox *aiGroup = new QGroupBox("LLM Connection");
    aiGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *aiLayout = new QFormLayout(aiGroup);
    aiLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Provider dropdown
    QComboBox *providerCombo = new QComboBox();
    providerCombo->addItem("Ollama (local)",         0);
    providerCombo->addItem("OpenAI",                 1);
    providerCombo->addItem("Groq",                   2);
    providerCombo->addItem("OpenRouter",             3);
    providerCombo->addItem("Anthropic Claude",       4);
    providerCombo->addItem("Custom",                 5);
    // Select current
    {
        QSettings s;
        QString savedUrl = s.value("llm/baseUrl", "http://localhost:11434/v1").toString();
        if (savedUrl.contains("localhost") || savedUrl.contains("127.0.0.1"))
            providerCombo->setCurrentIndex(0);
        else if (savedUrl.contains("openai.com"))
            providerCombo->setCurrentIndex(1);
        else if (savedUrl.contains("groq.com"))
            providerCombo->setCurrentIndex(2);
        else if (savedUrl.contains("openrouter.ai"))
            providerCombo->setCurrentIndex(3);
        else if (savedUrl.contains("anthropic.com"))
            providerCombo->setCurrentIndex(4);
        else
            providerCombo->setCurrentIndex(5);
    }
    aiLayout->addRow("Provider:", providerCombo);

    // Base URL
    QLineEdit *urlEdit = new QLineEdit();
    urlEdit->setText(m_llmConfig.baseUrl);
    urlEdit->setPlaceholderText("http://localhost:11434/v1");
    aiLayout->addRow("Base URL:", urlEdit);

    // API Key
    QLineEdit *keyEdit = new QLineEdit();
    keyEdit->setEchoMode(QLineEdit::Password);
    keyEdit->setText(m_llmConfig.apiKey);
    keyEdit->setPlaceholderText("API key (or 'ollama' for local)");
    aiLayout->addRow("API Key:", keyEdit);

    // Model
    QLineEdit *modelEdit = new QLineEdit();
    modelEdit->setText(m_llmConfig.model);
    modelEdit->setPlaceholderText("e.g. qwen3-coder:30b or claude-sonnet-4-6");
    aiLayout->addRow("Model:", modelEdit);

    // Provider presets — auto-fill URL when provider changes
    connect(providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [urlEdit, modelEdit](int idx) {
        switch (idx) {
            case 0: urlEdit->setText("http://localhost:11434/v1");         break;
            case 1: urlEdit->setText("https://api.openai.com/v1");
                    modelEdit->setPlaceholderText("gpt-4o");               break;
            case 2: urlEdit->setText("https://api.groq.com/openai/v1");
                    modelEdit->setPlaceholderText("llama-3.3-70b-versatile"); break;
            case 3: urlEdit->setText("https://openrouter.ai/api/v1");
                    modelEdit->setPlaceholderText("anthropic/claude-sonnet-4-6"); break;
            case 4: urlEdit->setText("https://api.anthropic.com");
                    modelEdit->setPlaceholderText("claude-sonnet-4-6");    break;
            default: break;
        }
    });

    // Apply button
    QPushButton *applyAiBtn = new QPushButton("Apply");
    applyAiBtn->setFixedWidth(80);
    connect(applyAiBtn, &QPushButton::clicked, this, [this, providerCombo, urlEdit, keyEdit, modelEdit]() {
        m_llmConfig.provider = (providerCombo->currentIndex() == 4)
                                   ? LLMProvider::Anthropic
                                   : LLMProvider::OpenAICompatible;
        m_llmConfig.baseUrl  = urlEdit->text().trimmed();
        m_llmConfig.apiKey   = keyEdit->text().trimmed();
        m_llmConfig.model    = modelEdit->text().trimmed();

        // Persist
        QSettings s;
        s.setValue("llm/provider",   (int)m_llmConfig.provider);
        s.setValue("llm/baseUrl",    m_llmConfig.baseUrl);
        s.setValue("llm/apiKey",     m_llmConfig.apiKey);
        s.setValue("llm/model",      m_llmConfig.model);

        // Push to live agent
        if (m_kalaAgent) m_kalaAgent->setConfig(m_llmConfig);

        // Also clear conversation history so the new model starts fresh
        if (m_kalaAgent) m_kalaAgent->clearHistory();
        if (m_companionPanel) m_companionPanel->clearLog();
    });
    aiLayout->addRow("", applyAiBtn);

    scrollLayout->addWidget(aiGroup);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);

    // Settings file buttons
    QHBoxLayout *settingsBtnLayout = new QHBoxLayout();
    settingsBtnLayout->setSpacing(6);

    QPushButton *saveSettingsBtn = new QPushButton("Save Settings...");
    saveSettingsBtn->setToolTip("Export these range settings to a .csettings file");
    connect(saveSettingsBtn, &QPushButton::clicked, this, &KalaMain::onSaveContainerSettings);
    settingsBtnLayout->addWidget(saveSettingsBtn);

    QPushButton *loadSettingsBtn = new QPushButton("Load Settings...");
    loadSettingsBtn->setToolTip("Import range settings from a .csettings file");
    connect(loadSettingsBtn, &QPushButton::clicked, this, &KalaMain::onLoadContainerSettings);
    settingsBtnLayout->addWidget(loadSettingsBtn);

    QPushButton *resetButton = new QPushButton("Reset to Defaults");
    resetButton->setToolTip("Reset all range settings to their built-in defaults");
    connect(resetButton, &QPushButton::clicked, this, &KalaMain::onResetSettingsToDefaults);
    settingsBtnLayout->addWidget(resetButton);

    QWidget *settingsBtnWidget = new QWidget();
    settingsBtnWidget->setLayout(settingsBtnLayout);
    settingsBtnWidget->setContentsMargins(0, 8, 0, 0);
    mainLayout->addWidget(settingsBtnWidget);

    qDebug() << "Settings tab populated";
}

void KalaMain::populateHarmonicGeneratorSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Max Harmonics
    QSpinBox *spinMaxHarmonics = new QSpinBox();
    spinMaxHarmonics->setRange(16, 512);
    spinMaxHarmonics->setValue(settings.harmonicGenerator.maxHarmonics);
    spinMaxHarmonics->setToolTip("Maximum number of harmonics (affects slider range)");
    connect(spinMaxHarmonics, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [&settings](int value) {
        settings.harmonicGenerator.maxHarmonics = value;
        settings.saveSettings();
    });
    layout->addRow("Max Harmonics:", spinMaxHarmonics);

    // Fundamental Hz Min
    QDoubleSpinBox *spinFundMin = new QDoubleSpinBox();
    spinFundMin->setRange(1.0, 100.0);
    spinFundMin->setDecimals(1);
    spinFundMin->setValue(settings.harmonicGenerator.fundamentalHzMin);
    spinFundMin->setSuffix(" Hz");
    connect(spinFundMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.fundamentalHzMin = value;
        settings.saveSettings();
    });
    layout->addRow("Fundamental Min:", spinFundMin);

    // Fundamental Hz Max
    QDoubleSpinBox *spinFundMax = new QDoubleSpinBox();
    spinFundMax->setRange(1000.0, 20000.0);
    spinFundMax->setDecimals(1);
    spinFundMax->setValue(settings.harmonicGenerator.fundamentalHzMax);
    spinFundMax->setSuffix(" Hz");
    connect(spinFundMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.fundamentalHzMax = value;
        settings.saveSettings();
    });
    layout->addRow("Fundamental Max:", spinFundMax);

    // Rolloff Power Min
    QDoubleSpinBox *spinRolloffMin = new QDoubleSpinBox();
    spinRolloffMin->setRange(0.01, 1.0);
    spinRolloffMin->setDecimals(2);
    spinRolloffMin->setValue(settings.harmonicGenerator.rolloffPowerMin);
    connect(spinRolloffMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.rolloffPowerMin = value;
        settings.saveSettings();
    });
    layout->addRow("Rolloff Power Min:", spinRolloffMin);

    // Rolloff Power Max
    QDoubleSpinBox *spinRolloffMax = new QDoubleSpinBox();
    spinRolloffMax->setRange(1.0, 20.0);
    spinRolloffMax->setDecimals(2);
    spinRolloffMax->setValue(settings.harmonicGenerator.rolloffPowerMax);
    connect(spinRolloffMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.rolloffPowerMax = value;
        settings.saveSettings();
    });
    layout->addRow("Rolloff Power Max:", spinRolloffMax);

    // Drift Max
    QDoubleSpinBox *spinDriftMax = new QDoubleSpinBox();
    spinDriftMax->setRange(0.01, 1.0);
    spinDriftMax->setDecimals(3);
    spinDriftMax->setValue(settings.harmonicGenerator.driftMax);
    connect(spinDriftMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.driftMax = value;
        settings.saveSettings();
    });
    layout->addRow("Drift Max:", spinDriftMax);

    // --- PADsynth ---
    QDoubleSpinBox *spinPadBwMin = new QDoubleSpinBox();
    spinPadBwMin->setRange(0.1, 100.0);
    spinPadBwMin->setDecimals(1);
    spinPadBwMin->setValue(settings.harmonicGenerator.padBandwidthMin);
    spinPadBwMin->setSuffix(" ct");
    connect(spinPadBwMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.padBandwidthMin = value;
        settings.saveSettings();
    });
    layout->addRow("PAD Bandwidth Min:", spinPadBwMin);

    QDoubleSpinBox *spinPadBwMax = new QDoubleSpinBox();
    spinPadBwMax->setRange(10.0, 1000.0);
    spinPadBwMax->setDecimals(1);
    spinPadBwMax->setValue(settings.harmonicGenerator.padBandwidthMax);
    spinPadBwMax->setSuffix(" ct");
    connect(spinPadBwMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.padBandwidthMax = value;
        settings.saveSettings();
    });
    layout->addRow("PAD Bandwidth Max:", spinPadBwMax);

    QDoubleSpinBox *spinPadBwScaleMin = new QDoubleSpinBox();
    spinPadBwScaleMin->setRange(0.0, 1.0);
    spinPadBwScaleMin->setDecimals(3);
    spinPadBwScaleMin->setValue(settings.harmonicGenerator.padBandwidthScaleMin);
    connect(spinPadBwScaleMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.padBandwidthScaleMin = value;
        settings.saveSettings();
    });
    layout->addRow("PAD BW Scale Min:", spinPadBwScaleMin);

    QDoubleSpinBox *spinPadBwScaleMax = new QDoubleSpinBox();
    spinPadBwScaleMax->setRange(0.5, 10.0);
    spinPadBwScaleMax->setDecimals(3);
    spinPadBwScaleMax->setValue(settings.harmonicGenerator.padBandwidthScaleMax);
    connect(spinPadBwScaleMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.harmonicGenerator.padBandwidthScaleMax = value;
        settings.saveSettings();
    });
    layout->addRow("PAD BW Scale Max:", spinPadBwScaleMax);
}

void KalaMain::populateRolloffProcessorSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Low Rolloff Max
    QDoubleSpinBox *spinLowMax = new QDoubleSpinBox();
    spinLowMax->setRange(0.5, 10.0);
    spinLowMax->setDecimals(2);
    spinLowMax->setValue(settings.rolloffProcessor.lowRolloffMax);
    connect(spinLowMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.rolloffProcessor.lowRolloffMax = value;
        settings.saveSettings();
    });
    layout->addRow("Low Rolloff Max:", spinLowMax);

    // High Rolloff Max
    QDoubleSpinBox *spinHighMax = new QDoubleSpinBox();
    spinHighMax->setRange(0.5, 10.0);
    spinHighMax->setDecimals(2);
    spinHighMax->setValue(settings.rolloffProcessor.highRolloffMax);
    connect(spinHighMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.rolloffProcessor.highRolloffMax = value;
        settings.saveSettings();
    });
    layout->addRow("High Rolloff Max:", spinHighMax);

    // Crossover Max
    QDoubleSpinBox *spinCrossoverMax = new QDoubleSpinBox();
    spinCrossoverMax->setRange(8.0, 128.0);
    spinCrossoverMax->setDecimals(0);
    spinCrossoverMax->setValue(settings.rolloffProcessor.crossoverMax);
    connect(spinCrossoverMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.rolloffProcessor.crossoverMax = value;
        settings.saveSettings();
    });
    layout->addRow("Crossover Harmonic Max:", spinCrossoverMax);

    // Transition Max
    QDoubleSpinBox *spinTransitionMax = new QDoubleSpinBox();
    spinTransitionMax->setRange(4.0, 64.0);
    spinTransitionMax->setDecimals(0);
    spinTransitionMax->setValue(settings.rolloffProcessor.transitionMax);
    connect(spinTransitionMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.rolloffProcessor.transitionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Transition Width Max:", spinTransitionMax);
}

void KalaMain::populateFormantBodySettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // F1 Frequency Range
    QDoubleSpinBox *spinF1Min = new QDoubleSpinBox();
    spinF1Min->setRange(50.0, 500.0);
    spinF1Min->setDecimals(0);
    spinF1Min->setValue(settings.formantBody.f1FreqMin);
    spinF1Min->setSuffix(" Hz");
    connect(spinF1Min, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.formantBody.f1FreqMin = value;
        settings.saveSettings();
    });
    layout->addRow("F1 Frequency Min:", spinF1Min);

    QDoubleSpinBox *spinF1Max = new QDoubleSpinBox();
    spinF1Max->setRange(500.0, 3000.0);
    spinF1Max->setDecimals(0);
    spinF1Max->setValue(settings.formantBody.f1FreqMax);
    spinF1Max->setSuffix(" Hz");
    connect(spinF1Max, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.formantBody.f1FreqMax = value;
        settings.saveSettings();
    });
    layout->addRow("F1 Frequency Max:", spinF1Max);

    // F2 Frequency Range
    QDoubleSpinBox *spinF2Min = new QDoubleSpinBox();
    spinF2Min->setRange(200.0, 1500.0);
    spinF2Min->setDecimals(0);
    spinF2Min->setValue(settings.formantBody.f2FreqMin);
    spinF2Min->setSuffix(" Hz");
    connect(spinF2Min, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.formantBody.f2FreqMin = value;
        settings.saveSettings();
    });
    layout->addRow("F2 Frequency Min:", spinF2Min);

    QDoubleSpinBox *spinF2Max = new QDoubleSpinBox();
    spinF2Max->setRange(1500.0, 8000.0);
    spinF2Max->setDecimals(0);
    spinF2Max->setValue(settings.formantBody.f2FreqMax);
    spinF2Max->setSuffix(" Hz");
    connect(spinF2Max, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.formantBody.f2FreqMax = value;
        settings.saveSettings();
    });
    layout->addRow("F2 Frequency Max:", spinF2Max);

    // Q Max
    QDoubleSpinBox *spinQMax = new QDoubleSpinBox();
    spinQMax->setRange(5.0, 100.0);
    spinQMax->setDecimals(1);
    spinQMax->setValue(settings.formantBody.qMax);
    connect(spinQMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.formantBody.qMax = value;
        settings.saveSettings();
    });
    layout->addRow("Q Factor Max:", spinQMax);
}

void KalaMain::populateNoiseColorFilterSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Color Min
    QDoubleSpinBox *spinColorMin = new QDoubleSpinBox();
    spinColorMin->setRange(20.0, 500.0);
    spinColorMin->setDecimals(0);
    spinColorMin->setValue(settings.noiseColorFilter.colorMin);
    spinColorMin->setSuffix(" Hz");
    connect(spinColorMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.noiseColorFilter.colorMin = value;
        settings.saveSettings();
    });
    layout->addRow("Color Min:", spinColorMin);

    // Color Max
    QDoubleSpinBox *spinColorMax = new QDoubleSpinBox();
    spinColorMax->setRange(2000.0, 20000.0);
    spinColorMax->setDecimals(0);
    spinColorMax->setValue(settings.noiseColorFilter.colorMax);
    spinColorMax->setSuffix(" Hz");
    connect(spinColorMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.noiseColorFilter.colorMax = value;
        settings.saveSettings();
    });
    layout->addRow("Color Max:", spinColorMax);

    // Filter Q Min
    QDoubleSpinBox *spinQMin = new QDoubleSpinBox();
    spinQMin->setRange(0.1, 2.0);
    spinQMin->setDecimals(2);
    spinQMin->setValue(settings.noiseColorFilter.filterQMin);
    connect(spinQMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.noiseColorFilter.filterQMin = value;
        settings.saveSettings();
    });
    layout->addRow("Filter Q Min:", spinQMin);

    // Filter Q Max
    QDoubleSpinBox *spinQMax = new QDoubleSpinBox();
    spinQMax->setRange(2.0, 50.0);
    spinQMax->setDecimals(1);
    spinQMax->setValue(settings.noiseColorFilter.filterQMax);
    connect(spinQMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.noiseColorFilter.filterQMax = value;
        settings.saveSettings();
    });
    layout->addRow("Filter Q Max:", spinQMax);
}

void KalaMain::populateBandpassEQSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Gain ---
    QDoubleSpinBox *spinGainMin = new QDoubleSpinBox();
    spinGainMin->setRange(0.0, 1.0);
    spinGainMin->setDecimals(3);
    spinGainMin->setValue(settings.bandpassEQ.gainMin);
    connect(spinGainMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.bandpassEQ.gainMin = value;
        settings.saveSettings();
    });
    layout->addRow("Gain Min:", spinGainMin);

    QDoubleSpinBox *spinGainMax = new QDoubleSpinBox();
    spinGainMax->setRange(1.0, 10.0);
    spinGainMax->setDecimals(3);
    spinGainMax->setValue(settings.bandpassEQ.gainMax);
    connect(spinGainMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.bandpassEQ.gainMax = value;
        settings.saveSettings();
    });
    layout->addRow("Gain Max:", spinGainMax);

    // --- Q ---
    QDoubleSpinBox *spinQMin = new QDoubleSpinBox();
    spinQMin->setRange(0.1, 1.0);
    spinQMin->setDecimals(2);
    spinQMin->setValue(settings.bandpassEQ.qMin);
    connect(spinQMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.bandpassEQ.qMin = value;
        settings.saveSettings();
    });
    layout->addRow("Q Min:", spinQMin);

    QDoubleSpinBox *spinQMax = new QDoubleSpinBox();
    spinQMax->setRange(5.0, 100.0);
    spinQMax->setDecimals(1);
    spinQMax->setValue(settings.bandpassEQ.qMax);
    connect(spinQMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.bandpassEQ.qMax = value;
        settings.saveSettings();
    });
    layout->addRow("Q Max:", spinQMax);
}

void KalaMain::populateCombFilterSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Delay Time ---
    QDoubleSpinBox *spinDelayMin = new QDoubleSpinBox();
    spinDelayMin->setRange(0.01, 10.0);
    spinDelayMin->setDecimals(2);
    spinDelayMin->setValue(settings.combFilter.delayTimeMin);
    spinDelayMin->setSuffix(" ms");
    connect(spinDelayMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.combFilter.delayTimeMin = value;
        settings.saveSettings();
    });
    layout->addRow("Delay Min:", spinDelayMin);

    QDoubleSpinBox *spinDelayMax = new QDoubleSpinBox();
    spinDelayMax->setRange(10.0, 2000.0);
    spinDelayMax->setDecimals(1);
    spinDelayMax->setValue(settings.combFilter.delayTimeMax);
    spinDelayMax->setSuffix(" ms");
    connect(spinDelayMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.combFilter.delayTimeMax = value;
        settings.saveSettings();
    });
    layout->addRow("Delay Max:", spinDelayMax);

    // --- Feedback ---
    QDoubleSpinBox *spinFbMin = new QDoubleSpinBox();
    spinFbMin->setRange(-2.0, 0.0);
    spinFbMin->setDecimals(3);
    spinFbMin->setValue(settings.combFilter.feedbackMin);
    connect(spinFbMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.combFilter.feedbackMin = value;
        settings.saveSettings();
    });
    layout->addRow("Feedback Min:", spinFbMin);

    QDoubleSpinBox *spinFbMax = new QDoubleSpinBox();
    spinFbMax->setRange(0.0, 2.0);
    spinFbMax->setDecimals(3);
    spinFbMax->setValue(settings.combFilter.feedbackMax);
    connect(spinFbMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.combFilter.feedbackMax = value;
        settings.saveSettings();
    });
    layout->addRow("Feedback Max:", spinFbMax);

    // --- Damping ---
    QDoubleSpinBox *spinDampMin = new QDoubleSpinBox();
    spinDampMin->setRange(0.0, 1.0);
    spinDampMin->setDecimals(3);
    spinDampMin->setValue(settings.combFilter.dampingMin);
    connect(spinDampMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.combFilter.dampingMin = value;
        settings.saveSettings();
    });
    layout->addRow("Damping Min:", spinDampMin);

    QDoubleSpinBox *spinDampMax = new QDoubleSpinBox();
    spinDampMax->setRange(0.0, 2.0);
    spinDampMax->setDecimals(3);
    spinDampMax->setValue(settings.combFilter.dampingMax);
    connect(spinDampMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.combFilter.dampingMax = value;
        settings.saveSettings();
    });
    layout->addRow("Damping Max:", spinDampMax);
}

void KalaMain::populateLowHighPassFilterSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Cutoff ---
    QDoubleSpinBox *spinCutoffMin = new QDoubleSpinBox();
    spinCutoffMin->setRange(1.0, 200.0);
    spinCutoffMin->setDecimals(1);
    spinCutoffMin->setValue(settings.lowHighPassFilter.cutoffMin);
    spinCutoffMin->setSuffix(" Hz");
    connect(spinCutoffMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.lowHighPassFilter.cutoffMin = value;
        settings.saveSettings();
    });
    layout->addRow("Cutoff Min:", spinCutoffMin);

    QDoubleSpinBox *spinCutoffMax = new QDoubleSpinBox();
    spinCutoffMax->setRange(1000.0, 96000.0);
    spinCutoffMax->setDecimals(1);
    spinCutoffMax->setValue(settings.lowHighPassFilter.cutoffMax);
    spinCutoffMax->setSuffix(" Hz");
    connect(spinCutoffMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.lowHighPassFilter.cutoffMax = value;
        settings.saveSettings();
    });
    layout->addRow("Cutoff Max:", spinCutoffMax);

    // --- Resonance ---
    QDoubleSpinBox *spinResMin = new QDoubleSpinBox();
    spinResMin->setRange(0.1, 2.0);
    spinResMin->setDecimals(3);
    spinResMin->setValue(settings.lowHighPassFilter.resonanceMin);
    connect(spinResMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.lowHighPassFilter.resonanceMin = value;
        settings.saveSettings();
    });
    layout->addRow("Resonance Min:", spinResMin);

    QDoubleSpinBox *spinResMax = new QDoubleSpinBox();
    spinResMax->setRange(5.0, 100.0);
    spinResMax->setDecimals(1);
    spinResMax->setValue(settings.lowHighPassFilter.resonanceMax);
    connect(spinResMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.lowHighPassFilter.resonanceMax = value;
        settings.saveSettings();
    });
    layout->addRow("Resonance Max:", spinResMax);
}

void KalaMain::populatePhysicsSystemSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Mass Max
    QDoubleSpinBox *spinMassMax = new QDoubleSpinBox();
    spinMassMax->setRange(1.0, 100.0);
    spinMassMax->setDecimals(1);
    spinMassMax->setValue(settings.physicsSystem.massMax);
    connect(spinMassMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.physicsSystem.massMax = value;
        settings.saveSettings();
    });
    layout->addRow("Mass Max:", spinMassMax);

    // Spring K Max
    QDoubleSpinBox *spinSpringMax = new QDoubleSpinBox();
    spinSpringMax->setRange(0.1, 10.0);
    spinSpringMax->setDecimals(3);
    spinSpringMax->setValue(settings.physicsSystem.springKMax);
    connect(spinSpringMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.physicsSystem.springKMax = value;
        settings.saveSettings();
    });
    layout->addRow("Spring K Max:", spinSpringMax);

    // Impulse Max
    QDoubleSpinBox *spinImpulseMax = new QDoubleSpinBox();
    spinImpulseMax->setRange(100.0, 10000.0);
    spinImpulseMax->setDecimals(0);
    spinImpulseMax->setValue(settings.physicsSystem.impulseMax);
    connect(spinImpulseMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.physicsSystem.impulseMax = value;
        settings.saveSettings();
    });
    layout->addRow("Impulse Max:", spinImpulseMax);
}

void KalaMain::populateDriftEngineSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Amount Max
    QDoubleSpinBox *spinAmountMax = new QDoubleSpinBox();
    spinAmountMax->setRange(0.01, 1.0);
    spinAmountMax->setDecimals(3);
    spinAmountMax->setValue(settings.driftEngine.amountMax);
    connect(spinAmountMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.driftEngine.amountMax = value;
        settings.saveSettings();
    });
    layout->addRow("Amount Max:", spinAmountMax);

    // Rate Max
    QDoubleSpinBox *spinRateMax = new QDoubleSpinBox();
    spinRateMax->setRange(1.0, 100.0);
    spinRateMax->setDecimals(2);
    spinRateMax->setValue(settings.driftEngine.rateMax);
    spinRateMax->setSuffix(" Hz");
    connect(spinRateMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.driftEngine.rateMax = value;
        settings.saveSettings();
    });
    layout->addRow("Rate Max:", spinRateMax);
}

void KalaMain::populateGateProcessorSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Attack Time Max
    QDoubleSpinBox *spinAttackMax = new QDoubleSpinBox();
    spinAttackMax->setRange(0.1, 10.0);
    spinAttackMax->setDecimals(3);
    spinAttackMax->setValue(settings.gateProcessor.attackTimeMax);
    spinAttackMax->setSuffix(" s");
    connect(spinAttackMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.gateProcessor.attackTimeMax = value;
        settings.saveSettings();
    });
    layout->addRow("Attack Time Max:", spinAttackMax);

    // Release Time Max
    QDoubleSpinBox *spinReleaseMax = new QDoubleSpinBox();
    spinReleaseMax->setRange(0.1, 20.0);
    spinReleaseMax->setDecimals(3);
    spinReleaseMax->setValue(settings.gateProcessor.releaseTimeMax);
    spinReleaseMax->setSuffix(" s");
    connect(spinReleaseMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.gateProcessor.releaseTimeMax = value;
        settings.saveSettings();
    });
    layout->addRow("Release Time Max:", spinReleaseMax);
}

void KalaMain::populateKarplusStrongSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Attack Portion ---
    QDoubleSpinBox *spinAttackMin = new QDoubleSpinBox();
    spinAttackMin->setRange(0.001, 0.5);
    spinAttackMin->setDecimals(3);
    spinAttackMin->setValue(settings.karplusStrong.attackPortionMin);
    connect(spinAttackMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.attackPortionMin = value;
        settings.saveSettings();
    });
    layout->addRow("Attack Portion Min:", spinAttackMin);

    QDoubleSpinBox *spinAttackMax = new QDoubleSpinBox();
    spinAttackMax->setRange(0.1, 2.0);
    spinAttackMax->setDecimals(3);
    spinAttackMax->setValue(settings.karplusStrong.attackPortionMax);
    connect(spinAttackMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.attackPortionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Attack Portion Max:", spinAttackMax);

    // --- Damping ---
    QDoubleSpinBox *spinDampingMin = new QDoubleSpinBox();
    spinDampingMin->setRange(0.0, 1.0);
    spinDampingMin->setDecimals(3);
    spinDampingMin->setValue(settings.karplusStrong.dampingMin);
    connect(spinDampingMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.dampingMin = value;
        settings.saveSettings();
    });
    layout->addRow("Damping Min:", spinDampingMin);

    QDoubleSpinBox *spinDampingMax = new QDoubleSpinBox();
    spinDampingMax->setRange(0.0, 2.0);
    spinDampingMax->setDecimals(3);
    spinDampingMax->setValue(settings.karplusStrong.dampingMax);
    connect(spinDampingMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.dampingMax = value;
        settings.saveSettings();
    });
    layout->addRow("Damping Max:", spinDampingMax);

    // --- Pluck Position ---
    QDoubleSpinBox *spinPluckPosMin = new QDoubleSpinBox();
    spinPluckPosMin->setRange(0.0, 1.0);
    spinPluckPosMin->setDecimals(3);
    spinPluckPosMin->setValue(settings.karplusStrong.pluckPositionMin);
    connect(spinPluckPosMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.pluckPositionMin = value;
        settings.saveSettings();
    });
    layout->addRow("Pluck Position Min:", spinPluckPosMin);

    QDoubleSpinBox *spinPluckPosMax = new QDoubleSpinBox();
    spinPluckPosMax->setRange(0.0, 2.0);
    spinPluckPosMax->setDecimals(3);
    spinPluckPosMax->setValue(settings.karplusStrong.pluckPositionMax);
    connect(spinPluckPosMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.pluckPositionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Pluck Position Max:", spinPluckPosMax);

    // --- Mix ---
    QDoubleSpinBox *spinMixMin = new QDoubleSpinBox();
    spinMixMin->setRange(0.0, 1.0);
    spinMixMin->setDecimals(3);
    spinMixMin->setValue(settings.karplusStrong.mixMin);
    connect(spinMixMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.mixMin = value;
        settings.saveSettings();
    });
    layout->addRow("Mix Min:", spinMixMin);

    QDoubleSpinBox *spinMixMax = new QDoubleSpinBox();
    spinMixMax->setRange(0.0, 2.0);
    spinMixMax->setDecimals(3);
    spinMixMax->setValue(settings.karplusStrong.mixMax);
    connect(spinMixMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.mixMax = value;
        settings.saveSettings();
    });
    layout->addRow("Mix Max:", spinMixMax);

    // --- Brightness ---
    QDoubleSpinBox *spinBrightnessMin = new QDoubleSpinBox();
    spinBrightnessMin->setRange(0.0, 1.0);
    spinBrightnessMin->setDecimals(3);
    spinBrightnessMin->setValue(settings.karplusStrong.brightnessMin);
    connect(spinBrightnessMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.brightnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Brightness Min:", spinBrightnessMin);

    QDoubleSpinBox *spinBrightnessMax = new QDoubleSpinBox();
    spinBrightnessMax->setRange(0.0, 2.0);
    spinBrightnessMax->setDecimals(3);
    spinBrightnessMax->setValue(settings.karplusStrong.brightnessMax);
    connect(spinBrightnessMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.brightnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Brightness Max:", spinBrightnessMax);

    // --- Excitation Softness ---
    QDoubleSpinBox *spinExcSoftnessMin = new QDoubleSpinBox();
    spinExcSoftnessMin->setRange(0.0, 1.0);
    spinExcSoftnessMin->setDecimals(3);
    spinExcSoftnessMin->setValue(settings.karplusStrong.excitationSoftnessMin);
    connect(spinExcSoftnessMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.excitationSoftnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Blend Ratio Min:", spinExcSoftnessMin);

    QDoubleSpinBox *spinExcSoftnessMax = new QDoubleSpinBox();
    spinExcSoftnessMax->setRange(0.0, 2.0);
    spinExcSoftnessMax->setDecimals(3);
    spinExcSoftnessMax->setValue(settings.karplusStrong.excitationSoftnessMax);
    connect(spinExcSoftnessMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.excitationSoftnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Blend Ratio Max:", spinExcSoftnessMax);

    // --- Pluck Hardness ---
    QDoubleSpinBox *spinPluckHardMin = new QDoubleSpinBox();
    spinPluckHardMin->setRange(0.0, 1.0);
    spinPluckHardMin->setDecimals(3);
    spinPluckHardMin->setValue(settings.karplusStrong.pluckHardnessMin);
    connect(spinPluckHardMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.pluckHardnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Pluck Hardness Min:", spinPluckHardMin);

    QDoubleSpinBox *spinPluckHardMax = new QDoubleSpinBox();
    spinPluckHardMax->setRange(0.0, 2.0);
    spinPluckHardMax->setDecimals(3);
    spinPluckHardMax->setValue(settings.karplusStrong.pluckHardnessMax);
    connect(spinPluckHardMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.pluckHardnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Pluck Hardness Max:", spinPluckHardMax);

    // --- Pick Direction ---
    QDoubleSpinBox *spinPickDirMin = new QDoubleSpinBox();
    spinPickDirMin->setRange(0.0, 1.0);
    spinPickDirMin->setDecimals(3);
    spinPickDirMin->setValue(settings.karplusStrong.pickDirectionMin);
    connect(spinPickDirMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.pickDirectionMin = value;
        settings.saveSettings();
    });
    layout->addRow("Pick Direction Min:", spinPickDirMin);

    QDoubleSpinBox *spinPickDirMax = new QDoubleSpinBox();
    spinPickDirMax->setRange(0.0, 2.0);
    spinPickDirMax->setDecimals(3);
    spinPickDirMax->setValue(settings.karplusStrong.pickDirectionMax);
    connect(spinPickDirMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.pickDirectionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Pick Direction Max:", spinPickDirMax);

    // --- Body Resonance ---
    QDoubleSpinBox *spinBodyResMin = new QDoubleSpinBox();
    spinBodyResMin->setRange(0.0, 1.0);
    spinBodyResMin->setDecimals(3);
    spinBodyResMin->setValue(settings.karplusStrong.bodyResonanceMin);
    connect(spinBodyResMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.bodyResonanceMin = value;
        settings.saveSettings();
    });
    layout->addRow("Body Resonance Min:", spinBodyResMin);

    QDoubleSpinBox *spinBodyResMax = new QDoubleSpinBox();
    spinBodyResMax->setRange(0.0, 2.0);
    spinBodyResMax->setDecimals(3);
    spinBodyResMax->setValue(settings.karplusStrong.bodyResonanceMax);
    connect(spinBodyResMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.bodyResonanceMax = value;
        settings.saveSettings();
    });
    layout->addRow("Body Resonance Max:", spinBodyResMax);

    // --- Body Frequency ---
    QDoubleSpinBox *spinBodyFreqMin = new QDoubleSpinBox();
    spinBodyFreqMin->setRange(20.0, 400.0);
    spinBodyFreqMin->setDecimals(1);
    spinBodyFreqMin->setValue(settings.karplusStrong.bodyFreqMin);
    spinBodyFreqMin->setSuffix(" Hz");
    connect(spinBodyFreqMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.bodyFreqMin = value;
        settings.saveSettings();
    });
    layout->addRow("Body Freq Min:", spinBodyFreqMin);

    QDoubleSpinBox *spinBodyFreqMax = new QDoubleSpinBox();
    spinBodyFreqMax->setRange(80.0, 4000.0);
    spinBodyFreqMax->setDecimals(1);
    spinBodyFreqMax->setValue(settings.karplusStrong.bodyFreqMax);
    spinBodyFreqMax->setSuffix(" Hz");
    connect(spinBodyFreqMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.karplusStrong.bodyFreqMax = value;
        settings.saveSettings();
    });
    layout->addRow("Body Freq Max:", spinBodyFreqMax);
}

void KalaMain::populateAttackSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Duration ---
    QDoubleSpinBox *spinDurationMin = new QDoubleSpinBox();
    spinDurationMin->setRange(0.001, 1.0);
    spinDurationMin->setDecimals(3);
    spinDurationMin->setValue(settings.attack.durationMin);
    spinDurationMin->setSuffix(" s");
    connect(spinDurationMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.durationMin = value;
        settings.saveSettings();
    });
    layout->addRow("Duration Min:", spinDurationMin);

    QDoubleSpinBox *spinDurationMax = new QDoubleSpinBox();
    spinDurationMax->setRange(0.5, 10.0);
    spinDurationMax->setDecimals(2);
    spinDurationMax->setValue(settings.attack.durationMax);
    spinDurationMax->setSuffix(" s");
    connect(spinDurationMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.durationMax = value;
        settings.saveSettings();
    });
    layout->addRow("Duration Max:", spinDurationMax);

    // --- Intensity ---
    QDoubleSpinBox *spinIntensityMin = new QDoubleSpinBox();
    spinIntensityMin->setRange(0.0, 1.0);
    spinIntensityMin->setDecimals(3);
    spinIntensityMin->setValue(settings.attack.intensityMin);
    connect(spinIntensityMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.intensityMin = value;
        settings.saveSettings();
    });
    layout->addRow("Intensity Min:", spinIntensityMin);

    QDoubleSpinBox *spinIntensityMax = new QDoubleSpinBox();
    spinIntensityMax->setRange(0.0, 2.0);
    spinIntensityMax->setDecimals(3);
    spinIntensityMax->setValue(settings.attack.intensityMax);
    connect(spinIntensityMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.intensityMax = value;
        settings.saveSettings();
    });
    layout->addRow("Intensity Max:", spinIntensityMax);

    // --- Mix ---
    QDoubleSpinBox *spinMixMin = new QDoubleSpinBox();
    spinMixMin->setRange(0.0, 1.0);
    spinMixMin->setDecimals(3);
    spinMixMin->setValue(settings.attack.mixMin);
    connect(spinMixMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.mixMin = value;
        settings.saveSettings();
    });
    layout->addRow("Mix Min:", spinMixMin);

    QDoubleSpinBox *spinMixMax = new QDoubleSpinBox();
    spinMixMax->setRange(0.0, 2.0);
    spinMixMax->setDecimals(3);
    spinMixMax->setValue(settings.attack.mixMax);
    connect(spinMixMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.mixMax = value;
        settings.saveSettings();
    });
    layout->addRow("Mix Max:", spinMixMax);

    // --- Air Jet ---
    QDoubleSpinBox *spinNoiseAmountMin = new QDoubleSpinBox();
    spinNoiseAmountMin->setRange(0.0, 1.0);
    spinNoiseAmountMin->setDecimals(3);
    spinNoiseAmountMin->setValue(settings.attack.noiseAmountMin);
    connect(spinNoiseAmountMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.noiseAmountMin = value;
        settings.saveSettings();
    });
    layout->addRow("Noise Amount Min:", spinNoiseAmountMin);

    QDoubleSpinBox *spinNoiseAmountMax = new QDoubleSpinBox();
    spinNoiseAmountMax->setRange(0.0, 2.0);
    spinNoiseAmountMax->setDecimals(3);
    spinNoiseAmountMax->setValue(settings.attack.noiseAmountMax);
    connect(spinNoiseAmountMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.noiseAmountMax = value;
        settings.saveSettings();
    });
    layout->addRow("Noise Amount Max:", spinNoiseAmountMax);

    QDoubleSpinBox *spinJetRatioMin = new QDoubleSpinBox();
    spinJetRatioMin->setRange(0.0, 1.0);
    spinJetRatioMin->setDecimals(3);
    spinJetRatioMin->setValue(settings.attack.jetRatioMin);
    connect(spinJetRatioMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.jetRatioMin = value;
        settings.saveSettings();
    });
    layout->addRow("Jet Ratio Min:", spinJetRatioMin);

    QDoubleSpinBox *spinJetRatioMax = new QDoubleSpinBox();
    spinJetRatioMax->setRange(0.0, 2.0);
    spinJetRatioMax->setDecimals(3);
    spinJetRatioMax->setValue(settings.attack.jetRatioMax);
    connect(spinJetRatioMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.jetRatioMax = value;
        settings.saveSettings();
    });
    layout->addRow("Jet Ratio Max:", spinJetRatioMax);

    // --- Reed (Clarinet/Saxophone) ---
    QDoubleSpinBox *spinReedStiffMin = new QDoubleSpinBox();
    spinReedStiffMin->setRange(0.0, 1.0);
    spinReedStiffMin->setDecimals(3);
    spinReedStiffMin->setValue(settings.attack.reedStiffnessMin);
    connect(spinReedStiffMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.reedStiffnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Reed Stiffness Min:", spinReedStiffMin);

    QDoubleSpinBox *spinReedStiffMax = new QDoubleSpinBox();
    spinReedStiffMax->setRange(0.0, 2.0);
    spinReedStiffMax->setDecimals(3);
    spinReedStiffMax->setValue(settings.attack.reedStiffnessMax);
    connect(spinReedStiffMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.reedStiffnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Reed Stiffness Max:", spinReedStiffMax);

    QDoubleSpinBox *spinReedApertureMin = new QDoubleSpinBox();
    spinReedApertureMin->setRange(0.0, 1.0);
    spinReedApertureMin->setDecimals(3);
    spinReedApertureMin->setValue(settings.attack.reedApertureMin);
    connect(spinReedApertureMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.reedApertureMin = value;
        settings.saveSettings();
    });
    layout->addRow("Reed Aperture Min:", spinReedApertureMin);

    QDoubleSpinBox *spinReedApertureMax = new QDoubleSpinBox();
    spinReedApertureMax->setRange(0.0, 2.0);
    spinReedApertureMax->setDecimals(3);
    spinReedApertureMax->setValue(settings.attack.reedApertureMax);
    connect(spinReedApertureMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.reedApertureMax = value;
        settings.saveSettings();
    });
    layout->addRow("Reed Aperture Max:", spinReedApertureMax);

    QDoubleSpinBox *spinBlowPosMin = new QDoubleSpinBox();
    spinBlowPosMin->setRange(0.0, 1.0);
    spinBlowPosMin->setDecimals(3);
    spinBlowPosMin->setValue(settings.attack.blowPositionMin);
    connect(spinBlowPosMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.blowPositionMin = value;
        settings.saveSettings();
    });
    layout->addRow("Blow Position Min:", spinBlowPosMin);

    QDoubleSpinBox *spinBlowPosMax = new QDoubleSpinBox();
    spinBlowPosMax->setRange(0.0, 2.0);
    spinBlowPosMax->setDecimals(3);
    spinBlowPosMax->setValue(settings.attack.blowPositionMax);
    connect(spinBlowPosMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.blowPositionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Blow Position Max:", spinBlowPosMax);

    // --- Brass ---
    QDoubleSpinBox *spinLipTensionMin = new QDoubleSpinBox();
    spinLipTensionMin->setRange(0.0, 1.0);
    spinLipTensionMin->setDecimals(3);
    spinLipTensionMin->setValue(settings.attack.lipTensionMin);
    connect(spinLipTensionMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.lipTensionMin = value;
        settings.saveSettings();
    });
    layout->addRow("Lip Tension Min:", spinLipTensionMin);

    QDoubleSpinBox *spinLipTensionMax = new QDoubleSpinBox();
    spinLipTensionMax->setRange(0.0, 2.0);
    spinLipTensionMax->setDecimals(3);
    spinLipTensionMax->setValue(settings.attack.lipTensionMax);
    connect(spinLipTensionMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.lipTensionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Lip Tension Max:", spinLipTensionMax);

    QDoubleSpinBox *spinBuzzQMin = new QDoubleSpinBox();
    spinBuzzQMin->setRange(0.1, 10.0);
    spinBuzzQMin->setDecimals(1);
    spinBuzzQMin->setValue(settings.attack.buzzQMin);
    connect(spinBuzzQMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.buzzQMin = value;
        settings.saveSettings();
    });
    layout->addRow("Buzz Q Min:", spinBuzzQMin);

    QDoubleSpinBox *spinBuzzQMax = new QDoubleSpinBox();
    spinBuzzQMax->setRange(5.0, 100.0);
    spinBuzzQMax->setDecimals(1);
    spinBuzzQMax->setValue(settings.attack.buzzQMax);
    connect(spinBuzzQMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.buzzQMax = value;
        settings.saveSettings();
    });
    layout->addRow("Buzz Q Max:", spinBuzzQMax);

    // --- FM Strike ---
    QDoubleSpinBox *spinHardnessMin = new QDoubleSpinBox();
    spinHardnessMin->setRange(0.0, 1.0);
    spinHardnessMin->setDecimals(3);
    spinHardnessMin->setValue(settings.attack.hardnessMin);
    connect(spinHardnessMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.hardnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Hardness Min:", spinHardnessMin);

    QDoubleSpinBox *spinHardnessMax = new QDoubleSpinBox();
    spinHardnessMax->setRange(0.0, 2.0);
    spinHardnessMax->setDecimals(3);
    spinHardnessMax->setValue(settings.attack.hardnessMax);
    connect(spinHardnessMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.hardnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Hardness Max:", spinHardnessMax);

    QDoubleSpinBox *spinBrightnessMin = new QDoubleSpinBox();
    spinBrightnessMin->setRange(0.0, 1.0);
    spinBrightnessMin->setDecimals(3);
    spinBrightnessMin->setValue(settings.attack.brightnessMin);
    connect(spinBrightnessMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.brightnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Brightness Min:", spinBrightnessMin);

    QDoubleSpinBox *spinBrightnessMax = new QDoubleSpinBox();
    spinBrightnessMax->setRange(0.0, 2.0);
    spinBrightnessMax->setDecimals(3);
    spinBrightnessMax->setValue(settings.attack.brightnessMax);
    connect(spinBrightnessMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.brightnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Brightness Max:", spinBrightnessMax);

    // --- Membrane Sweep ---
    QDoubleSpinBox *spinTightnessMin = new QDoubleSpinBox();
    spinTightnessMin->setRange(0.0, 1.0);
    spinTightnessMin->setDecimals(3);
    spinTightnessMin->setValue(settings.attack.tightnessMin);
    connect(spinTightnessMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.tightnessMin = value;
        settings.saveSettings();
    });
    layout->addRow("Tightness Min:", spinTightnessMin);

    QDoubleSpinBox *spinTightnessMax = new QDoubleSpinBox();
    spinTightnessMax->setRange(0.0, 2.0);
    spinTightnessMax->setDecimals(3);
    spinTightnessMax->setValue(settings.attack.tightnessMax);
    connect(spinTightnessMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.tightnessMax = value;
        settings.saveSettings();
    });
    layout->addRow("Tightness Max:", spinTightnessMax);

    QDoubleSpinBox *spinToneMin = new QDoubleSpinBox();
    spinToneMin->setRange(0.0, 1.0);
    spinToneMin->setDecimals(3);
    spinToneMin->setValue(settings.attack.toneMin);
    connect(spinToneMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.toneMin = value;
        settings.saveSettings();
    });
    layout->addRow("Tone Min:", spinToneMin);

    QDoubleSpinBox *spinToneMax = new QDoubleSpinBox();
    spinToneMax->setRange(0.0, 2.0);
    spinToneMax->setDecimals(3);
    spinToneMax->setValue(settings.attack.toneMax);
    connect(spinToneMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.attack.toneMax = value;
        settings.saveSettings();
    });
    layout->addRow("Tone Max:", spinToneMax);
}

void KalaMain::populateLFOSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Frequency Max
    QDoubleSpinBox *spinFreqMax = new QDoubleSpinBox();
    spinFreqMax->setRange(5.0, 100.0);
    spinFreqMax->setDecimals(2);
    spinFreqMax->setValue(settings.lfo.frequencyMax);
    spinFreqMax->setSuffix(" Hz");
    connect(spinFreqMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.lfo.frequencyMax = value;
        settings.saveSettings();
    });
    layout->addRow("Frequency Max:", spinFreqMax);
}

void KalaMain::populateWavetableSynthSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Position ---
    QDoubleSpinBox *spinPosMin = new QDoubleSpinBox();
    spinPosMin->setRange(0.0, 1.0);
    spinPosMin->setDecimals(3);
    spinPosMin->setValue(settings.wavetableSynth.positionMin);
    connect(spinPosMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.wavetableSynth.positionMin = value;
        settings.saveSettings();
    });
    layout->addRow("Position Min:", spinPosMin);

    QDoubleSpinBox *spinPosMax = new QDoubleSpinBox();
    spinPosMax->setRange(0.0, 2.0);
    spinPosMax->setDecimals(3);
    spinPosMax->setValue(settings.wavetableSynth.positionMax);
    connect(spinPosMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.wavetableSynth.positionMax = value;
        settings.saveSettings();
    });
    layout->addRow("Position Max:", spinPosMax);
}

void KalaMain::populateFrequencyMapperSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // --- Low Frequency ---
    QDoubleSpinBox *spinLowFreqMin = new QDoubleSpinBox();
    spinLowFreqMin->setRange(0.1, 500.0);
    spinLowFreqMin->setDecimals(1);
    spinLowFreqMin->setValue(settings.frequencyMapper.lowFreqMin);
    spinLowFreqMin->setSuffix(" Hz");
    connect(spinLowFreqMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.lowFreqMin = value;
        settings.saveSettings();
    });
    layout->addRow("Low Freq Min:", spinLowFreqMin);

    QDoubleSpinBox *spinLowFreqMax = new QDoubleSpinBox();
    spinLowFreqMax->setRange(10.0, 10000.0);
    spinLowFreqMax->setDecimals(1);
    spinLowFreqMax->setValue(settings.frequencyMapper.lowFreqMax);
    spinLowFreqMax->setSuffix(" Hz");
    connect(spinLowFreqMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.lowFreqMax = value;
        settings.saveSettings();
    });
    layout->addRow("Low Freq Max:", spinLowFreqMax);

    // --- High Frequency ---
    QDoubleSpinBox *spinHighFreqMin = new QDoubleSpinBox();
    spinHighFreqMin->setRange(10.0, 10000.0);
    spinHighFreqMin->setDecimals(1);
    spinHighFreqMin->setValue(settings.frequencyMapper.highFreqMin);
    spinHighFreqMin->setSuffix(" Hz");
    connect(spinHighFreqMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.highFreqMin = value;
        settings.saveSettings();
    });
    layout->addRow("High Freq Min:", spinHighFreqMin);

    QDoubleSpinBox *spinHighFreqMax = new QDoubleSpinBox();
    spinHighFreqMax->setRange(1000.0, 192000.0);
    spinHighFreqMax->setDecimals(1);
    spinHighFreqMax->setValue(settings.frequencyMapper.highFreqMax);
    spinHighFreqMax->setSuffix(" Hz");
    connect(spinHighFreqMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.highFreqMax = value;
        settings.saveSettings();
    });
    layout->addRow("High Freq Max:", spinHighFreqMax);

    // --- Output Floor (outputMin param) ---
    QDoubleSpinBox *spinOutMinMin = new QDoubleSpinBox();
    spinOutMinMin->setRange(-5.0, 0.0);
    spinOutMinMin->setDecimals(3);
    spinOutMinMin->setValue(settings.frequencyMapper.outputMinMin);
    connect(spinOutMinMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.outputMinMin = value;
        settings.saveSettings();
    });
    layout->addRow("Output Floor Min:", spinOutMinMin);

    QDoubleSpinBox *spinOutMinMax = new QDoubleSpinBox();
    spinOutMinMax->setRange(0.0, 2.0);
    spinOutMinMax->setDecimals(3);
    spinOutMinMax->setValue(settings.frequencyMapper.outputMinMax);
    connect(spinOutMinMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.outputMinMax = value;
        settings.saveSettings();
    });
    layout->addRow("Output Floor Max:", spinOutMinMax);

    // --- Output Ceiling (outputMax param) ---
    QDoubleSpinBox *spinOutMaxMin = new QDoubleSpinBox();
    spinOutMaxMin->setRange(0.0, 1.0);
    spinOutMaxMin->setDecimals(3);
    spinOutMaxMin->setValue(settings.frequencyMapper.outputMaxMin);
    connect(spinOutMaxMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.outputMaxMin = value;
        settings.saveSettings();
    });
    layout->addRow("Output Ceiling Min:", spinOutMaxMin);

    QDoubleSpinBox *spinOutMaxMax = new QDoubleSpinBox();
    spinOutMaxMax->setRange(1.0, 5.0);
    spinOutMaxMax->setDecimals(3);
    spinOutMaxMax->setValue(settings.frequencyMapper.outputMaxMax);
    connect(spinOutMaxMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.frequencyMapper.outputMaxMax = value;
        settings.saveSettings();
    });
    layout->addRow("Output Ceiling Max:", spinOutMaxMax);
}

void KalaMain::populateEasingSettings(QFormLayout *layout)
{
    ContainerSettings &settings = ContainerSettings::instance();

    // Add description label
    QLabel *descLabel = new QLabel("Default parameter values for easing functions.\n"
                                   "Used by both Sound Engine and Score rhythm easing.");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addRow(descLabel);

    // === Back Easing: Overshoot ===
    QLabel *backLabel = new QLabel("Back Easing");
    backLabel->setStyleSheet("font-weight: bold; margin-top: 8px;");
    layout->addRow(backLabel);

    QDoubleSpinBox *spinOvershoot = new QDoubleSpinBox();
    spinOvershoot->setRange(settings.easing.overshootMin, settings.easing.overshootMax);
    spinOvershoot->setDecimals(2);
    spinOvershoot->setSingleStep(0.1);
    spinOvershoot->setValue(settings.easing.overshootDefault);
    spinOvershoot->setToolTip("How far past target the easing overshoots (0 = no overshoot)");
    connect(spinOvershoot, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.easing.overshootDefault = value;
        settings.saveSettings();
    });
    layout->addRow("Overshoot Default:", spinOvershoot);

    // === Elastic Easing: Amplitude & Period ===
    QLabel *elasticLabel = new QLabel("Elastic Easing");
    elasticLabel->setStyleSheet("font-weight: bold; margin-top: 8px;");
    layout->addRow(elasticLabel);

    QDoubleSpinBox *spinAmplitude = new QDoubleSpinBox();
    spinAmplitude->setRange(settings.easing.amplitudeMin, settings.easing.amplitudeMax);
    spinAmplitude->setDecimals(2);
    spinAmplitude->setSingleStep(0.1);
    spinAmplitude->setValue(settings.easing.amplitudeDefault);
    spinAmplitude->setToolTip("Spring oscillation amplitude (higher = more bounce)");
    connect(spinAmplitude, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.easing.amplitudeDefault = value;
        settings.saveSettings();
    });
    layout->addRow("Amplitude Default:", spinAmplitude);

    QDoubleSpinBox *spinPeriod = new QDoubleSpinBox();
    spinPeriod->setRange(settings.easing.periodMin, settings.easing.periodMax);
    spinPeriod->setDecimals(2);
    spinPeriod->setSingleStep(0.05);
    spinPeriod->setValue(settings.easing.periodDefault);
    spinPeriod->setToolTip("Spring oscillation period (higher = slower oscillation)");
    connect(spinPeriod, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.easing.periodDefault = value;
        settings.saveSettings();
    });
    layout->addRow("Period Default:", spinPeriod);

    // === Wobble Easing: Frequency & Decay ===
    QLabel *wobbleLabel = new QLabel("Wobble Easing");
    wobbleLabel->setStyleSheet("font-weight: bold; margin-top: 8px;");
    layout->addRow(wobbleLabel);

    QDoubleSpinBox *spinFrequency = new QDoubleSpinBox();
    spinFrequency->setRange(settings.easing.frequencyMin, settings.easing.frequencyMax);
    spinFrequency->setDecimals(1);
    spinFrequency->setSingleStep(0.5);
    spinFrequency->setValue(settings.easing.frequencyDefault);
    spinFrequency->setToolTip("Wobble oscillation frequency (cycles per unit time)");
    connect(spinFrequency, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.easing.frequencyDefault = value;
        settings.saveSettings();
    });
    layout->addRow("Frequency Default:", spinFrequency);

    QDoubleSpinBox *spinDecay = new QDoubleSpinBox();
    spinDecay->setRange(settings.easing.decayMin, settings.easing.decayMax);
    spinDecay->setDecimals(2);
    spinDecay->setSingleStep(0.1);
    spinDecay->setValue(settings.easing.decayDefault);
    spinDecay->setToolTip("Wobble decay rate (higher = faster decay)");
    connect(spinDecay, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&settings](double value) {
        settings.easing.decayDefault = value;
        settings.saveSettings();
    });
    layout->addRow("Decay Default:", spinDecay);
}

void KalaMain::onResetSettingsToDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Reset Settings",
        "Are you sure you want to reset all container settings to their default values?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        ContainerSettings::instance().resetToDefaults();
        ContainerSettings::instance().saveSettings();
        populateSettingsTab();
        QMessageBox::information(this, "Settings Reset", "All settings have been reset to defaults.");
    }
}

void KalaMain::onSaveContainerSettings()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        "Save Container Settings",
        QString(),
        "Kala Settings (*.csettings);;JSON Files (*.json)"
    );
    if (path.isEmpty()) return;

    // Ensure correct extension when user omits it
    if (!path.endsWith(".csettings", Qt::CaseInsensitive) &&
        !path.endsWith(".json", Qt::CaseInsensitive)) {
        path += ".csettings";
    }

    QJsonObject json = ContainerSettings::instance().toJson();
    QJsonDocument doc(json);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not write to:\n" + path + "\n\n" + file.errorString());
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void KalaMain::onLoadContainerSettings()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Load Container Settings",
        QString(),
        "Kala Settings (*.csettings);;JSON Files (*.json);;All Files (*)"
    );
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Load Failed",
                             "Could not read:\n" + path + "\n\n" + file.errorString());
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, "Load Failed",
                             "File is not valid JSON:\n" + err.errorString());
        return;
    }

    ContainerSettings::instance().fromJson(doc.object());
    ContainerSettings::instance().saveSettings();  // Persist to QSettings too
    populateSettingsTab();
}

void KalaMain::onKeyboardShortcuts()
{
    KeyboardShortcutsDialog dialog(this);
    dialog.exec();
}

void KalaMain::onDocumentation()
{
    HelpDialog dialog(this);

    int tab = ui->MainTab->currentIndex();
    if (tab == 0 && currentContainer) {
        // Sound Engine tab with a container selected -> that container's doc page
        dialog.navigateToContainer(currentContainer->getName());
    } else if (tab == 0) {
        // Sound Engine tab, no selection -> overview
        dialog.navigateTo("sound-engine-overview.html");
    } else if (tab == 1) {
        // Composition tab -> Score Canvas
        dialog.navigateTo("score-canvas.html");
    } else if (tab == 2) {
        // Preferences tab -> Settings
        dialog.navigateTo("settings.html");
    } else {
        dialog.navigateTo("introduction.html");
    }

    dialog.exec();
}
