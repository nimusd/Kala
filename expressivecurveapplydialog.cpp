#include "expressivecurveapplydialog.h"
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

ExpressiveCurveApplyDialog::ExpressiveCurveApplyDialog(int noteCount, double timeSpanMs,
                                                       const QStringList &curveNames,
                                                       QWidget *parent)
    : QDialog(parent)
    , m_noteCount(noteCount)
    , m_timeSpanMs(timeSpanMs)
{
    setWindowTitle("Apply Expressive Curve");
    setMinimumSize(450, 420);

    setupPresets();
    setupUi(curveNames);

    loadPreset(0);
}

void ExpressiveCurveApplyDialog::setupUi(const QStringList &curveNames)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Target curve selector
    QHBoxLayout *targetLayout = new QHBoxLayout();
    QLabel *targetLabel = new QLabel("Target curve:", this);
    curveNameCombo = new QComboBox(this);
    for (const QString &name : curveNames) {
        curveNameCombo->addItem(name);
    }
    targetLayout->addWidget(targetLabel);
    targetLayout->addWidget(curveNameCombo, 1);
    mainLayout->addLayout(targetLayout);

    // Preset selector with Save/Load
    QHBoxLayout *presetLayout = new QHBoxLayout();
    QLabel *presetLabel = new QLabel("Shape preset:", this);
    presetCombo = new QComboBox(this);
    for (const Preset &preset : presets) {
        presetCombo->addItem(preset.name);
    }
    loadButton = new QPushButton("Load...", this);
    saveButton = new QPushButton("Save...", this);
    loadButton->setMaximumWidth(70);
    saveButton->setMaximumWidth(70);

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(presetCombo, 1);
    presetLayout->addWidget(loadButton);
    presetLayout->addWidget(saveButton);
    mainLayout->addLayout(presetLayout);

    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExpressiveCurveApplyDialog::onPresetChanged);
    connect(loadButton, &QPushButton::clicked, this, &ExpressiveCurveApplyDialog::onLoadClicked);
    connect(saveButton, &QPushButton::clicked, this, &ExpressiveCurveApplyDialog::onSaveClicked);

    // Curve editor group
    QGroupBox *curveGroup = new QGroupBox("Shape Curve", this);
    QVBoxLayout *curveLayout = new QVBoxLayout(curveGroup);
    curveLayout->setContentsMargins(8, 8, 8, 8);

    curveCanvas = new EnvelopeCurveCanvas(this);
    curveCanvas->setMinimumHeight(200);
    curveLayout->addWidget(curveCanvas);

    QLabel *helpLabel = new QLabel(
        "Click to add points, drag to move. Right-click or drag off to delete.",
        this);
    helpLabel->setStyleSheet("color: #666; font-size: 11px;");
    helpLabel->setWordWrap(true);
    curveLayout->addWidget(helpLabel);

    mainLayout->addWidget(curveGroup);

    // Weight slider
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

    QLabel *weightHelpLabel = new QLabel(
        "0-1: Subtle effect | 1: Normal | 1-2: Boosted effect",
        this);
    weightHelpLabel->setStyleSheet("color: #666; font-size: 10px;");
    weightHelpLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(weightHelpLabel);

    connect(weightSlider, &QSlider::valueChanged,
            this, &ExpressiveCurveApplyDialog::onWeightChanged);

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

    mainLayout->addStretch();

    // Button bar
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    cancelButton = new QPushButton("Cancel", this);
    applyButton  = new QPushButton("Apply", this);
    applyButton->setDefault(true);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(applyButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyButton,  &QPushButton::clicked, this, &QDialog::accept);
}

void ExpressiveCurveApplyDialog::setupPresets()
{
    { Preset p; p.name = "Crescendo";
      p.curve.append(EnvelopePoint(0.0, 0.3, 0));
      p.curve.append(EnvelopePoint(1.0, 1.0, 0));
      presets.append(p); }

    { Preset p; p.name = "Decrescendo";
      p.curve.append(EnvelopePoint(0.0, 1.0, 0));
      p.curve.append(EnvelopePoint(1.0, 0.3, 0));
      presets.append(p); }

    { Preset p; p.name = "Swell";
      p.curve.append(EnvelopePoint(0.0, 0.4, 0));
      p.curve.append(EnvelopePoint(0.5, 1.0, 0));
      p.curve.append(EnvelopePoint(1.0, 0.4, 0));
      presets.append(p); }

    { Preset p; p.name = "Accent First";
      p.curve.append(EnvelopePoint(0.0, 1.0, 0));
      p.curve.append(EnvelopePoint(0.15, 0.6, 0));
      p.curve.append(EnvelopePoint(1.0, 0.6, 0));
      presets.append(p); }

    { Preset p; p.name = "Fade Out";
      p.curve.append(EnvelopePoint(0.0, 1.0, 0));
      p.curve.append(EnvelopePoint(0.3, 0.7, 0));
      p.curve.append(EnvelopePoint(0.6, 0.4, 0));
      p.curve.append(EnvelopePoint(1.0, 0.15, 0));
      presets.append(p); }

    { Preset p; p.name = "Accent Last";
      p.curve.append(EnvelopePoint(0.0, 0.5, 0));
      p.curve.append(EnvelopePoint(0.85, 0.5, 0));
      p.curve.append(EnvelopePoint(1.0, 1.0, 0));
      presets.append(p); }

    { Preset p; p.name = "Flat";
      p.curve.append(EnvelopePoint(0.0, 0.8, 0));
      p.curve.append(EnvelopePoint(1.0, 0.8, 0));
      presets.append(p); }

    { Preset p; p.name = "Custom";
      p.curve.append(EnvelopePoint(0.0, 0.5, 0));
      p.curve.append(EnvelopePoint(1.0, 0.5, 0));
      presets.append(p); }
}

void ExpressiveCurveApplyDialog::onPresetChanged(int index)
{
    loadPreset(index);
}

void ExpressiveCurveApplyDialog::loadPreset(int index)
{
    if (index >= 0 && index < presets.size()) {
        curveCanvas->setPoints(presets[index].curve);
    }
}

QVector<EnvelopePoint> ExpressiveCurveApplyDialog::getCurve() const
{
    return curveCanvas->getPoints();
}

double ExpressiveCurveApplyDialog::getWeight() const
{
    return weightSlider->value() / 100.0;
}

QString ExpressiveCurveApplyDialog::getSelectedCurveName() const
{
    return curveNameCombo->currentText();
}

void ExpressiveCurveApplyDialog::onWeightChanged(int value)
{
    double weight = value / 100.0;
    weightValueLabel->setText(QString::number(weight, 'f', 2));
}

void ExpressiveCurveApplyDialog::onSaveClicked()
{
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/envelope", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Expressive Curve Shape",
        lastDir + "/expressive_shape.env.json",
        "Envelope Files (*.env.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) return;

    settings.setValue("lastDirectory/envelope", QFileInfo(fileName).absolutePath());

    if (!fileName.endsWith(".env.json") && !fileName.endsWith(".json")) {
        fileName += ".env.json";
    }

    QJsonObject jsonObj;
    jsonObj["name"] = "Expressive Shape";
    jsonObj["description"] = "Expressive curve shape created in Kala";
    jsonObj["version"] = "1.0";
    jsonObj["loopMode"] = 0;

    QJsonArray pointsArray;
    for (const EnvelopePoint &point : curveCanvas->getPoints()) {
        QJsonObject pointObj;
        pointObj["time"]      = point.time;
        pointObj["value"]     = point.value;
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
        "Expressive curve shape saved to:\n" + fileName);
}

void ExpressiveCurveApplyDialog::onLoadClicked()
{
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/envelope", QDir::homePath()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Expressive Curve Shape",
        lastDir,
        "Envelope Files (*.env.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) return;

    settings.setValue("lastDirectory/envelope", QFileInfo(fileName).absolutePath());

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Load Error",
            "Could not open file for reading:\n" + fileName);
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "Load Error",
            "Invalid JSON format in file:\n" + fileName);
        return;
    }

    QJsonObject jsonObj = doc.object();

    if (!jsonObj.contains("points") || !jsonObj["points"].isArray()) {
        QMessageBox::critical(this, "Load Error",
            "Missing or invalid 'points' field in JSON file.");
        return;
    }

    QVector<EnvelopePoint> loadedPoints;
    QJsonArray pointsArray = jsonObj["points"].toArray();
    for (const QJsonValue &val : pointsArray) {
        if (val.isObject()) {
            QJsonObject pointObj = val.toObject();
            EnvelopePoint point;
            point.time      = pointObj.value("time").toDouble(0.0);
            point.value     = pointObj.value("value").toDouble(0.0);
            point.curveType = pointObj.value("curveType").toInt(0);
            loadedPoints.append(point);
        }
    }

    if (loadedPoints.isEmpty()) {
        QMessageBox::critical(this, "Load Error",
            "No valid point data found in file.");
        return;
    }

    curveCanvas->setPoints(loadedPoints);

    int customIndex = presetCombo->findText("Custom");
    if (customIndex >= 0) {
        presetCombo->blockSignals(true);
        presetCombo->setCurrentIndex(customIndex);
        presetCombo->blockSignals(false);
    }

    QString name = jsonObj.value("name").toString("Loaded Curve");
    QMessageBox::information(this, "Loaded",
        QString("Successfully loaded: %1\n%2 points")
            .arg(name)
            .arg(loadedPoints.size()));
}
