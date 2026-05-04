#include "eqcurvedialog.h"
#include "envelopecurvecanvas.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

EqCurveDialog::EqCurveDialog(int noteCount, double timeSpanMs, QWidget *parent)
    : QDialog(parent)
    , m_noteCount(noteCount)
    , m_timeSpanMs(timeSpanMs)
{
    setWindowTitle("Apply EQ Curve");
    setMinimumSize(480, 620);

    setupShapePresets();
    setupIntensityPresets();
    setupUi();

    // Defaults: Shape = Flat (0.5 everywhere), Intensity = Swell
    int flatIdx = 0;
    for (int i = 0; i < shapePresets.size(); ++i) {
        if (shapePresets[i].name == "Flat") { flatIdx = i; break; }
    }
    shapePresetCombo->setCurrentIndex(flatIdx);
    loadShapePreset(flatIdx);

    int swellIdx = 0;
    for (int i = 0; i < intensityPresets.size(); ++i) {
        if (intensityPresets[i].name == "Swell") { swellIdx = i; break; }
    }
    intensityPresetCombo->setCurrentIndex(swellIdx);
    loadIntensityPreset(swellIdx);
}

void EqCurveDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // --- Shape curve group ---
    QGroupBox *shapeGroup = new QGroupBox("EQ Shape  (X: Band 1 → Band 10, Y: gain, 0.5 = flat)", this);
    QVBoxLayout *shapeLayout = new QVBoxLayout(shapeGroup);
    shapeLayout->setContentsMargins(8, 8, 8, 8);
    shapeLayout->setSpacing(6);

    QHBoxLayout *shapePresetLayout = new QHBoxLayout();
    QLabel *shapePresetLabel = new QLabel("Preset:", this);
    shapePresetCombo = new QComboBox(this);
    for (const Preset &p : shapePresets) shapePresetCombo->addItem(p.name);
    shapeFlatButton = new QPushButton("Flat", this);
    shapeFlatButton->setToolTip("Reset the shape to a flat line at 0.5 (no EQ effect).");
    shapeFlatButton->setMaximumWidth(60);
    shapeLoadButton = new QPushButton("Load...", this);
    shapeSaveButton = new QPushButton("Save...", this);
    shapeLoadButton->setMaximumWidth(70);
    shapeSaveButton->setMaximumWidth(70);
    shapePresetLayout->addWidget(shapePresetLabel);
    shapePresetLayout->addWidget(shapePresetCombo, 1);
    shapePresetLayout->addWidget(shapeFlatButton);
    shapePresetLayout->addWidget(shapeLoadButton);
    shapePresetLayout->addWidget(shapeSaveButton);
    shapeLayout->addLayout(shapePresetLayout);

    shapeCanvas = new EnvelopeCurveCanvas(this);
    shapeCanvas->setMinimumHeight(160);
    shapeLayout->addWidget(shapeCanvas);

    // Band-label row under the shape canvas
    QHBoxLayout *bandLabelLayout = new QHBoxLayout();
    bandLabelLayout->setSpacing(0);
    bandLabelLayout->setContentsMargins(0, 0, 0, 0);
    for (int b = 1; b <= 10; ++b) {
        QLabel *lbl = new QLabel(QString::number(b), this);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color: #888; font-size: 10px;");
        bandLabelLayout->addWidget(lbl, 1);
    }
    shapeLayout->addLayout(bandLabelLayout);

    mainLayout->addWidget(shapeGroup);

    connect(shapePresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EqCurveDialog::onShapePresetChanged);
    connect(shapeFlatButton, &QPushButton::clicked, this, &EqCurveDialog::onShapeFlatResetClicked);
    connect(shapeLoadButton, &QPushButton::clicked, this, &EqCurveDialog::onShapeLoadClicked);
    connect(shapeSaveButton, &QPushButton::clicked, this, &EqCurveDialog::onShapeSaveClicked);

    // --- Intensity curve group ---
    QGroupBox *intensityGroup = new QGroupBox("Intensity over Time  (0 = flat EQ, 1 = full shape)", this);
    QVBoxLayout *intensityLayout = new QVBoxLayout(intensityGroup);
    intensityLayout->setContentsMargins(8, 8, 8, 8);
    intensityLayout->setSpacing(6);

    QHBoxLayout *intensityPresetLayout = new QHBoxLayout();
    QLabel *intensityPresetLabel = new QLabel("Preset:", this);
    intensityPresetCombo = new QComboBox(this);
    for (const Preset &p : intensityPresets) intensityPresetCombo->addItem(p.name);
    intensityLoadButton = new QPushButton("Load...", this);
    intensitySaveButton = new QPushButton("Save...", this);
    intensityLoadButton->setMaximumWidth(70);
    intensitySaveButton->setMaximumWidth(70);
    intensityPresetLayout->addWidget(intensityPresetLabel);
    intensityPresetLayout->addWidget(intensityPresetCombo, 1);
    intensityPresetLayout->addWidget(intensityLoadButton);
    intensityPresetLayout->addWidget(intensitySaveButton);
    intensityLayout->addLayout(intensityPresetLayout);

    intensityCanvas = new EnvelopeCurveCanvas(this);
    intensityCanvas->setMinimumHeight(160);
    intensityLayout->addWidget(intensityCanvas);

    QLabel *helpLabel = new QLabel(
        "Click to add points, drag to move. Right-click or drag off to delete.",
        this);
    helpLabel->setStyleSheet("color: #666; font-size: 11px;");
    helpLabel->setWordWrap(true);
    intensityLayout->addWidget(helpLabel);

    mainLayout->addWidget(intensityGroup);

    connect(intensityPresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EqCurveDialog::onIntensityPresetChanged);
    connect(intensityLoadButton, &QPushButton::clicked, this, &EqCurveDialog::onIntensityLoadClicked);
    connect(intensitySaveButton, &QPushButton::clicked, this, &EqCurveDialog::onIntensitySaveClicked);

    // --- Weight slider ---
    QHBoxLayout *weightLayout = new QHBoxLayout();
    QLabel *weightLabel = new QLabel("Weight:", this);
    weightSlider = new QSlider(Qt::Horizontal, this);
    weightSlider->setRange(0, 200);
    weightSlider->setValue(100);
    weightSlider->setTickPosition(QSlider::TicksBelow);
    weightSlider->setTickInterval(50);

    weightValueLabel = new QLabel("1.00", this);
    weightValueLabel->setMinimumWidth(40);
    weightValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    weightLayout->addWidget(weightLabel);
    weightLayout->addWidget(weightSlider, 1);
    weightLayout->addWidget(weightValueLabel);
    mainLayout->addLayout(weightLayout);

    connect(weightSlider, &QSlider::valueChanged, this, &EqCurveDialog::onWeightChanged);

    // Per-note toggle
    perNoteToggle = new QPushButton("Per note", this);
    perNoteToggle->setCheckable(true);
    perNoteToggle->setChecked(false);
    perNoteToggle->setToolTip(
        "On: the intensity curve spans each note independently — EQ morphs over\n"
        "the note's own duration (good for long notes).\n"
        "Off: the intensity curve spreads over the whole selection — each note\n"
        "gets a static EQ shape, scaled by its position on the intensity curve\n"
        "(morph over the phrase).");
    perNoteToggle->setStyleSheet(
        "QPushButton {"
        "  padding: 4px 12px;"
        "  border: 1px solid #aaa;"
        "  border-radius: 4px;"
        "  background-color: #e8e8e8;"
        "  color: #333;"
        "}"
        "QPushButton:checked {"
        "  background-color: #1976D2;"
        "  border-color: #1565C0;"
        "  color: white;"
        "}"
        "QPushButton:hover:!checked { background-color: #d8d8d8; }"
        "QPushButton:hover:checked  { background-color: #1565C0; }");
    mainLayout->addWidget(perNoteToggle);

    // Selection info
    QString timeStr;
    if (m_timeSpanMs >= 1000) {
        timeStr = QString::number(m_timeSpanMs / 1000.0, 'f', 1) + "s";
    } else {
        timeStr = QString::number(m_timeSpanMs, 'f', 0) + "ms";
    }
    QString selectionText = (m_noteCount == 1)
        ? QString("Selection: 1 note, %1 duration").arg(timeStr)
        : QString("Selection: %1 notes, %2 span").arg(m_noteCount).arg(timeStr);
    selectionLabel = new QLabel(selectionText, this);
    selectionLabel->setStyleSheet("color: #444; font-style: italic;");
    mainLayout->addWidget(selectionLabel);

    // Button bar
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    cancelButton = new QPushButton("Cancel", this);
    applyButton = new QPushButton("Apply", this);
    applyButton->setDefault(true);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(applyButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyButton, &QPushButton::clicked, this, &QDialog::accept);
}

void EqCurveDialog::setupShapePresets()
{
    // Flat - no EQ effect
    { Preset p; p.name = "Flat";
      p.curve.append(EnvelopePoint(0.0, 0.5, 0));
      p.curve.append(EnvelopePoint(1.0, 0.5, 0));
      shapePresets.append(p); }

    // Bright - tilt up (more highs)
    { Preset p; p.name = "Bright";
      p.curve.append(EnvelopePoint(0.0, 0.3, 0));
      p.curve.append(EnvelopePoint(1.0, 0.85, 0));
      shapePresets.append(p); }

    // Dark - tilt down (more lows, less highs)
    { Preset p; p.name = "Dark";
      p.curve.append(EnvelopePoint(0.0, 0.85, 0));
      p.curve.append(EnvelopePoint(1.0, 0.25, 0));
      shapePresets.append(p); }

    // Scooped - V shape (lows + highs boosted, mids cut)
    { Preset p; p.name = "Scooped";
      p.curve.append(EnvelopePoint(0.0, 0.8, 1));
      p.curve.append(EnvelopePoint(0.5, 0.25, 1));
      p.curve.append(EnvelopePoint(1.0, 0.8, 0));
      shapePresets.append(p); }

    // Mid Bump - mids boosted, lows and highs reduced
    { Preset p; p.name = "Mid Bump";
      p.curve.append(EnvelopePoint(0.0, 0.3, 1));
      p.curve.append(EnvelopePoint(0.5, 0.9, 1));
      p.curve.append(EnvelopePoint(1.0, 0.3, 0));
      shapePresets.append(p); }

    // High Shelf - flat lows, boosted highs
    { Preset p; p.name = "High Shelf";
      p.curve.append(EnvelopePoint(0.0, 0.5, 0));
      p.curve.append(EnvelopePoint(0.55, 0.5, 0));
      p.curve.append(EnvelopePoint(0.75, 0.85, 1));
      p.curve.append(EnvelopePoint(1.0, 0.85, 0));
      shapePresets.append(p); }

    // Low Shelf - boosted lows, flat highs
    { Preset p; p.name = "Low Shelf";
      p.curve.append(EnvelopePoint(0.0, 0.85, 0));
      p.curve.append(EnvelopePoint(0.25, 0.85, 1));
      p.curve.append(EnvelopePoint(0.45, 0.5, 0));
      p.curve.append(EnvelopePoint(1.0, 0.5, 0));
      shapePresets.append(p); }

    // Custom - starting point user can modify (flat)
    { Preset p; p.name = "Custom";
      p.curve.append(EnvelopePoint(0.0, 0.5, 0));
      p.curve.append(EnvelopePoint(1.0, 0.5, 0));
      shapePresets.append(p); }
}

void EqCurveDialog::setupIntensityPresets()
{
    // Swell - smooth symmetric (user's go-to default)
    { Preset p; p.name = "Swell";
      p.curve.append(EnvelopePoint(0.0, 0.0, 1));
      p.curve.append(EnvelopePoint(0.5, 1.0, 1));
      p.curve.append(EnvelopePoint(1.0, 0.0, 0));
      intensityPresets.append(p); }

    // Ramp Up - fade in
    { Preset p; p.name = "Ramp Up";
      p.curve.append(EnvelopePoint(0.0, 0.0, 0));
      p.curve.append(EnvelopePoint(1.0, 1.0, 0));
      intensityPresets.append(p); }

    // Ramp Down - fade out
    { Preset p; p.name = "Ramp Down";
      p.curve.append(EnvelopePoint(0.0, 1.0, 0));
      p.curve.append(EnvelopePoint(1.0, 0.0, 0));
      intensityPresets.append(p); }

    // Hold - full intensity throughout (static EQ)
    { Preset p; p.name = "Hold";
      p.curve.append(EnvelopePoint(0.0, 1.0, 0));
      p.curve.append(EnvelopePoint(1.0, 1.0, 0));
      intensityPresets.append(p); }

    // Attack Then Hold - quick rise then sustain
    { Preset p; p.name = "Attack + Hold";
      p.curve.append(EnvelopePoint(0.0, 0.0, 1));
      p.curve.append(EnvelopePoint(0.15, 1.0, 1));
      p.curve.append(EnvelopePoint(1.0, 1.0, 0));
      intensityPresets.append(p); }

    // Hold Then Release - sustain then tail off
    { Preset p; p.name = "Hold + Release";
      p.curve.append(EnvelopePoint(0.0, 1.0, 0));
      p.curve.append(EnvelopePoint(0.7, 1.0, 1));
      p.curve.append(EnvelopePoint(1.0, 0.0, 0));
      intensityPresets.append(p); }

    // Custom
    { Preset p; p.name = "Custom";
      p.curve.append(EnvelopePoint(0.0, 0.0, 0));
      p.curve.append(EnvelopePoint(1.0, 1.0, 0));
      intensityPresets.append(p); }
}

void EqCurveDialog::onShapePresetChanged(int index)     { loadShapePreset(index); }
void EqCurveDialog::onIntensityPresetChanged(int index) { loadIntensityPreset(index); }

void EqCurveDialog::loadShapePreset(int index)
{
    if (index >= 0 && index < shapePresets.size())
        shapeCanvas->setPoints(shapePresets[index].curve);
}

void EqCurveDialog::loadIntensityPreset(int index)
{
    if (index >= 0 && index < intensityPresets.size())
        intensityCanvas->setPoints(intensityPresets[index].curve);
}

void EqCurveDialog::onShapeFlatResetClicked()
{
    int flatIdx = shapePresetCombo->findText("Flat");
    if (flatIdx >= 0) {
        shapePresetCombo->setCurrentIndex(flatIdx);
        loadShapePreset(flatIdx);
    }
}

QVector<EnvelopePoint> EqCurveDialog::getShapeCurve() const     { return shapeCanvas->getPoints(); }
QVector<EnvelopePoint> EqCurveDialog::getIntensityCurve() const { return intensityCanvas->getPoints(); }
double EqCurveDialog::getWeight() const                         { return weightSlider->value() / 100.0; }
bool   EqCurveDialog::getPerNoteMode() const                    { return perNoteToggle->isChecked(); }

void EqCurveDialog::setInitialShapeCurve(const QVector<EnvelopePoint> &points)
{
    shapeCanvas->setPoints(points);
    int customIdx = shapePresetCombo->findText("Custom");
    if (customIdx >= 0) {
        shapePresetCombo->blockSignals(true);
        shapePresetCombo->setCurrentIndex(customIdx);
        shapePresetCombo->blockSignals(false);
    }
}

void EqCurveDialog::setInitialIntensityCurve(const QVector<EnvelopePoint> &points)
{
    intensityCanvas->setPoints(points);
    int customIdx = intensityPresetCombo->findText("Custom");
    if (customIdx >= 0) {
        intensityPresetCombo->blockSignals(true);
        intensityPresetCombo->setCurrentIndex(customIdx);
        intensityPresetCombo->blockSignals(false);
    }
}

void EqCurveDialog::onWeightChanged(int value)
{
    double weight = value / 100.0;
    weightValueLabel->setText(QString::number(weight, 'f', 2));
}

void EqCurveDialog::saveCurveToFile(const QVector<EnvelopePoint> &points,
                                    const QString &defaultBaseName,
                                    const QString &dialogTitle,
                                    const QString &nameTag)
{
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/envelope", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        dialogTitle,
        lastDir + "/" + defaultBaseName,
        "Envelope Files (*.env.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) return;

    settings.setValue("lastDirectory/envelope", QFileInfo(fileName).absolutePath());

    if (!fileName.endsWith(".env.json") && !fileName.endsWith(".json"))
        fileName += ".env.json";

    QJsonObject jsonObj;
    jsonObj["name"] = nameTag;
    jsonObj["description"] = nameTag + " created in Kala";
    jsonObj["version"] = "1.0";
    jsonObj["loopMode"] = 0;

    QJsonArray pointsArray;
    for (const EnvelopePoint &point : points) {
        QJsonObject pointObj;
        pointObj["time"] = point.time;
        pointObj["value"] = point.value;
        pointObj["curveType"] = point.curveType;
        pointsArray.append(pointObj);
    }
    jsonObj["points"] = pointsArray;

    QJsonDocument doc(jsonObj);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Save Error",
            "Could not open file for writing:\n" + fileName);
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Saved",
        nameTag + " saved successfully to:\n" + fileName);
}

QVector<EnvelopePoint> EqCurveDialog::loadCurveFromFile(const QString &dialogTitle, bool *ok)
{
    if (ok) *ok = false;

    QSettings settings;
    QString lastDir = settings.value("lastDirectory/envelope", QDir::homePath()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        dialogTitle,
        lastDir,
        "Envelope Files (*.env.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) return {};

    settings.setValue("lastDirectory/envelope", QFileInfo(fileName).absolutePath());

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Load Error",
            "Could not open file for reading:\n" + fileName);
        return {};
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "Load Error",
            "Invalid JSON format in file:\n" + fileName);
        return {};
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("points") || !jsonObj["points"].isArray()) {
        QMessageBox::critical(this, "Load Error",
            "Missing or invalid 'points' field in JSON file.");
        return {};
    }

    QVector<EnvelopePoint> loadedPoints;
    QJsonArray pointsArray = jsonObj["points"].toArray();
    for (const QJsonValue &val : pointsArray) {
        if (val.isObject()) {
            QJsonObject pointObj = val.toObject();
            EnvelopePoint point;
            point.time = pointObj.value("time").toDouble(0.0);
            point.value = pointObj.value("value").toDouble(0.0);
            point.curveType = pointObj.value("curveType").toInt(0);
            loadedPoints.append(point);
        }
    }

    if (loadedPoints.isEmpty()) {
        QMessageBox::critical(this, "Load Error",
            "No valid point data found in file.");
        return {};
    }

    if (ok) *ok = true;
    return loadedPoints;
}

void EqCurveDialog::onShapeSaveClicked()
{
    saveCurveToFile(shapeCanvas->getPoints(), "eq_shape.env.json",
                    "Save EQ Shape", "EQ Shape");
}

void EqCurveDialog::onShapeLoadClicked()
{
    bool ok = false;
    QVector<EnvelopePoint> loaded = loadCurveFromFile("Load EQ Shape", &ok);
    if (!ok) return;
    setInitialShapeCurve(loaded);
}

void EqCurveDialog::onIntensitySaveClicked()
{
    saveCurveToFile(intensityCanvas->getPoints(), "eq_intensity.env.json",
                    "Save EQ Intensity", "EQ Intensity");
}

void EqCurveDialog::onIntensityLoadClicked()
{
    bool ok = false;
    QVector<EnvelopePoint> loaded = loadCurveFromFile("Load EQ Intensity", &ok);
    if (!ok) return;
    setInitialIntensityCurve(loaded);
}
