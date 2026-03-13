#ifndef ENVELOPELIBRARYDIALOG_H
#define ENVELOPELIBRARYDIALOG_H

#include <QDialog>
#include <QVector>

class QListWidget;
class QLineEdit;
class QPushButton;
class QComboBox;
class EnvelopeCurveCanvas;

/**
 * EnvelopePoint - Single control point in an envelope
 */
struct EnvelopePoint {
    double time;        // 0.0 to 1.0
    double value;       // 0.0 to 1.0
    int curveType;      // 0=Linear, 1=Smooth, 2=Step
    
    EnvelopePoint(double t = 0.0, double v = 0.0, int c = 0)
        : time(t), value(v), curveType(c) {}
};

/**
 * EnvelopeData - Complete envelope curve definition
 */
struct EnvelopeData {
    QString name;
    QVector<EnvelopePoint> points;
    int loopMode;       // 0=None, 1=Loop, 2=Ping-pong
    bool isFactory;
    
    EnvelopeData() : loopMode(0), isFactory(false) {}
};

/**
 * EnvelopeLibraryDialog - Create and manage envelope curves
 * 
 * Interactive multi-point envelope editor with draggable control points
 */
class EnvelopeLibraryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnvelopeLibraryDialog(QWidget *parent = nullptr);

    // Get the selected envelope data (captures current UI state)
    EnvelopeData getSelectedEnvelope() const;

    // Set an envelope to edit
    void setEnvelope(const EnvelopeData &env);

private slots:
    void onPresetSelected(int index);
    void onNewPreset();
    void onDeletePreset();
    void onSavePreset();
    void onReset();
    void onNameChanged();
    void onLoopModeChanged(int index);
    void onCurveChanged();
    void onSaveToFile();
    void onLoadFromFile();

private:
    void setupUI();
    void loadFactoryPresets();
    void updatePresetList();
    void loadEnvelopeToEditor(const EnvelopeData &env);
    bool saveCurrentEnvelope();
    
    // UI Components
    QListWidget *presetList;
    EnvelopeCurveCanvas *curveCanvas;
    QLineEdit *nameEdit;
    QComboBox *loopModeCombo;
    QPushButton *btnNew;
    QPushButton *btnDelete;
    QPushButton *btnSave;
    QPushButton *btnReset;
    
    // Data
    QVector<EnvelopeData> factoryPresets;
    QVector<EnvelopeData> userPresets;
    EnvelopeData currentEnvelope;
    int selectedPresetIndex;
    int editingUserPresetIndex;  // Index in userPresets being edited, -1 if new
    bool isModified;
};

#endif // ENVELOPELIBRARYDIALOG_H
