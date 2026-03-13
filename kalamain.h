#ifndef KALAMAIN_H
#define KALAMAIN_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "sounitbuilder.h"
#include "scorecanvaswindow.h"
#include "audioengine.h"
#include "trackmanager.h"
#include "containersettings.h"
#include "keyboardshortcutsdialog.h"
#include "llmconfig.h"

class KalaTools;
class KalaAgent;
class CompanionPanel;

// Forward declarations for Qt widgets
class QDockWidget;
class QScrollArea;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QSlider;
class QSpinBox;
class QDoubleSpinBox;
class QFormLayout;
class SpectrumVisualizer;
class EnvelopeVisualizer;

QT_BEGIN_NAMESPACE
namespace Ui {
class KalaMain;
}
QT_END_NAMESPACE

class KalaMain : public QMainWindow
{
    Q_OBJECT

public:
    KalaMain(QWidget *parent = nullptr);
    ~KalaMain();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;  // Catch Tab from all child widgets

private:
    // Window state tracking for single-screen toggle
    bool m_sounitBuilderWasMaximized = false;
    bool m_scoreCanvasWasMaximized = false;
    bool m_mainWindowWasMaximized = false;

private:
    Ui::KalaMain *ui;

private:
    SounitBuilder *sounitBuilder;
    ScoreCanvasWindow *scoreCanvasWindow;
    AudioEngine *audioEngine;  // Shared audio engine for both windows
    TrackManager *trackManager;  // NEW: Manages all Track objects

    // AI Companion
    LLMConfig       m_llmConfig;
    KalaTools      *m_kalaTools     = nullptr;
    KalaAgent      *m_kalaAgent     = nullptr;
    CompanionPanel *m_companionPanel = nullptr;
    QDockWidget    *m_companionDock  = nullptr;

    // Inspector state
    Container *currentContainer = nullptr;

    // Sounit file management
    QString currentSounitFilePath;
    QMap<int, QString> trackSounitFiles;  // Map track index -> sounit file path
    QMap<int, QString> trackSounitNames;  // Map track index -> sounit name
    QMap<int, QJsonObject> trackCanvasStates;  // Map track index -> canvas JSON (containers + connections)
    QMap<int, QList<Container*>> trackContainers;  // Map track index -> list of Container objects
    QMap<int, QVector<Canvas::Connection>> trackConnections;  // Map track index -> connections
    int currentEditingTrack;  // Which track's sounit is currently loaded in Sound Engine tab

    // Dynamic inspector controls (created based on container type)
    QComboBox *comboDnaSelect = nullptr;
    QSlider *sliderNumHarmonics = nullptr;
    QSpinBox *spinNumHarmonics = nullptr;
    SpectrumVisualizer *spectrumViz = nullptr;
    QLabel *dnaNameLabel = nullptr;

    // PADsynth inspector controls
    QCheckBox *checkPadEnabled = nullptr;
    QWidget *padParamsWidget = nullptr;

    // Envelope Engine inspector controls
    QComboBox *comboEnvelopeSelect = nullptr;
    QComboBox *comboScoreCurve = nullptr;     // Score curve selector (for followDynamics mode)
    QWidget *envelopeParamsWidget = nullptr;  // Container for contextual parameters
    EnvelopeVisualizer *envelopeViz = nullptr;
    QLabel *envelopeNameLabel = nullptr;
    QMetaObject::Connection m_envelopePreviewConnection;  // Tracks container→preview connection

    // Mixer widgets (dynamically created per track)
    QVector<QWidget*> mixerTrackWidgets;  // Container widgets for each track's mixer row
    void rebuildMixer();  // Rebuild all mixer track sliders
    void updateMixerSelection();  // Highlight selected mixer columns without full rebuild

    // Multi-track selection (synced from TrackSelector)
    QVector<int> m_selectedTrackIndices;

    // Helper methods
    void clearConfigInspector();
    void populateHarmonicGeneratorInspector();
    void populateRolloffProcessorInspector();
    void populateSpectrumToSignalInspector();
    void populateFormantBodyInspector();
    void populateBreathTurbulenceInspector();
    void populateNoiseColorFilterInspector();
    void populatePhysicsSystemInspector();
    void populateEnvelopeEngineInspector();
    void populateDriftEngineInspector();
    void populateGateProcessorInspector();
    void populateEasingApplicatorInspector();
    void populateKarplusStrongInspector();
    void populateAttackInspector();
    void populateLFOInspector();
    void populateFrequencyMapperInspector();
    void populateSignalMixerInspector();
    void populateSpectrumBlenderInspector();
    void populateWavetableSynthInspector();
    void populateBandpassEQInspector();
    void populateCombFilterInspector();
    void populateLowHighPassFilterInspector();
    void populateIRConvolutionInspector();
    void populateNoteTailInspector();
    void populateRecorderInspector();
    void populateBowedInspector();
    void populateReedInspector();
    void updateSpectrumVisualization();
    void updateEnvelopeParameters(int envelopeType);  // Update contextual envelope params
    void updateEnvelopePreview();  // Update envelope preview visualization
    QString getContainerDescription(const QString &containerType);

    // Helper to add a simple parameter slider
    void addParameterSlider(QFormLayout *layout, const QString &label,
                           const QString &paramName, double minVal, double maxVal,
                           double defaultVal, double step = 0.01, int decimals = 2);

    // WAV file export helper
    bool writeWavFile(const QString &filePath, const std::vector<float> &samples,
                      unsigned int sampleRate, int channels, int bitsPerSample);

    // Sounit navigation helpers
    void updateSounitSelector();  // Refresh the combobox with all loaded sounits
    void switchToSounit(int trackIndex);  // Load a different sounit into the canvas for editing
    void saveCurrentCanvasState(int trackIndex);  // Save current canvas to trackCanvasStates
    void loadCanvasStateForTrack(int trackIndex);  // Load canvas state from trackCanvasStates

    //  NEW: Loading state flag ***
    bool m_isLoadingSounit = false;

    // Note inspector state
    bool m_updatingNoteInspector = false;  // Prevent recursive updates
    void updateNoteInspector();             // Update inspector from current note selection
    void updateNoteVariationComboBox();     // Populate variation dropdown for current track

    // Project file management
    QString m_currentProjectPath;  // Path to current .kala file (empty if new/unsaved)
    bool m_projectDirty = false;   // True if project has unsaved changes

    // Project helper methods
    bool saveProject(const QString &filePath);  // Save project to file
    bool loadProject(const QString &filePath);  // Load project from file
    void newProject();                          // Create new empty project
    void updateWindowTitle();                   // Update title bar with project name and dirty state
    bool checkUnsavedChanges();                 // Prompt to save if dirty, returns false if cancelled
    void markProjectDirty();                    // Mark project as having unsaved changes

public slots:
    void stopAllPlayback();  // Stop playback in both windows
    void toggleCanvasFocus();  // Toggle focus between canvas window and main window
    bool loadProjectFile(const QString &filePath);  // AI companion: open a project
    bool saveProjectFile(const QString &filePath);  // AI companion: save to path
    QString getProjectFilePath() const { return m_currentProjectPath; }

private slots:
    void onTabChanged(int index);

    // Project file menu slots
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveProjectAs();
    void onExportAudio();

    // Sounit file menu slots
    void onNewSounit();
    void onLoadSounit();
    void onSaveSounitAs();
    void onSounitNameChanged(const QString &name);
    void onSounitCommentChanged(const QString &comment);
    void onSounitSelectorChanged(int index);  // Handle sounit selection from dropdown
    void onSounitNameEdited();  // Handle inline editing of sounit name
    void onSounitCommentEdited();  // Handle editing of sounit comment

    // Track management slots
    void onAddTrack();
    void onDeleteTrack();

public slots:
    void deleteTrackAtIndex(int trackIndex);  // Delete a specific track (called from context menu)

private slots:

    // Variation management slots
    void refreshVariationsList();
    void onVariationSelectionChanged();
    void onDeleteVariation();
    void onRenameVariation();
    void onVariationsTrackNameEdited();
    void onVariationSelectorChanged(int index);  // Sound Engine tab variation selector
    void updateVariationSelector();               // Refresh Sound Engine variation dropdown

    // Scale management slots
    void onScaleEditClicked();
    void onScaleAddClicked();
    void onScaleMarkerChanged(int markerIndex);
    void updateScaleDisplay();
    double getSelectedScaleMarkerTime() const;

    // Tempo management slots
    void onTempoEditClicked();
    void onTempoAddClicked();
    void onTempoMarkerChanged(int markerIndex);
    void updateTempoDisplay();
    double getSelectedTempoMarkerTime() const;  // Returns time in ms for selected marker

    // Inspector update slots
    void onContainerSelected(Container *container);
    void onConnectionSelected(int connectionIndex);
    void onSelectionCleared();

    // Inspector edit slots
    void onInstanceNameChanged();
    void onConnectionFunctionChanged(int index);
    void onConnectionWeightChanged(double value);
    void onDisconnectClicked();
    void onConfigParameterChanged(QListWidgetItem *item);

    // Harmonic Generator inspector slots
    void onDnaSelectChanged(int index);
    void onNumHarmonicsChanged(int value);

    // Note inspector slots
    void onNoteSelectionChanged();
    void onNoteStartChanged(double value);
    void onNoteDurationChanged(double value);
    void onNotePitchChanged(int value);
    void onNoteVariationChanged(int index);
    void onNoteVibratoToggled(bool checked);
    void onNoteVibratoEditClicked();

    // Expressive curves inspector slots
    void onExpressiveCurveChanged(int index);
    void onExpressiveCurveAdd();
    void onExpressiveCurveDelete();
    void onExpressiveCurveApply();
    void updateExpressiveCurvesDropdown();

    // *** Manual update method ***
    void updateSounitNameDisplay(const QString &name);

    // Settings tab methods
    void populateSettingsTab();
    void populateHarmonicGeneratorSettings(QFormLayout *layout);
    void populateRolloffProcessorSettings(QFormLayout *layout);
    void populateFormantBodySettings(QFormLayout *layout);
    void populateNoiseColorFilterSettings(QFormLayout *layout);
    void populateBandpassEQSettings(QFormLayout *layout);
    void populateCombFilterSettings(QFormLayout *layout);
    void populateLowHighPassFilterSettings(QFormLayout *layout);
    void populatePhysicsSystemSettings(QFormLayout *layout);
    void populateDriftEngineSettings(QFormLayout *layout);
    void populateGateProcessorSettings(QFormLayout *layout);
    void populateKarplusStrongSettings(QFormLayout *layout);
    void populateAttackSettings(QFormLayout *layout);
    void populateLFOSettings(QFormLayout *layout);
    void populateWavetableSynthSettings(QFormLayout *layout);
    void populateFrequencyMapperSettings(QFormLayout *layout);
    void populateEasingSettings(QFormLayout *layout);
    void onResetSettingsToDefaults();
    void onSaveContainerSettings();
    void onLoadContainerSettings();

    // Help menu slots
    void onKeyboardShortcuts();
    void onDocumentation();
};

#endif // KALAMAIN_H
