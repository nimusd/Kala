#include "vibratoeditordialog.h"
#include "envelopecurvecanvas.h"
#include "envelopelibraryDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>

VibratoEditorDialog::VibratoEditorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Vibrato Editor");
    setMinimumSize(500, 500);
    loadFactoryPresets();
    loadUserPresets();
    setupUI();
}

void VibratoEditorDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Preset selector at top
    QHBoxLayout *presetLayout = new QHBoxLayout();
    presetLayout->addWidget(new QLabel("Preset:"));
    comboVibratoPreset = new QComboBox();
    comboVibratoPreset->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    presetLayout->addWidget(comboVibratoPreset);
    btnSavePreset = new QPushButton("Save...");
    btnSavePreset->setToolTip("Save current settings as a new preset");
    presetLayout->addWidget(btnSavePreset);
    btnDeletePreset = new QPushButton("Delete");
    btnDeletePreset->setToolTip("Delete the selected user preset");
    presetLayout->addWidget(btnDeletePreset);
    btnSetDefault = new QPushButton("Set Default");
    btnSetDefault->setToolTip("Use this preset as the default when opening the vibrato editor");
    presetLayout->addWidget(btnSetDefault);
    mainLayout->addLayout(presetLayout);
    updatePresetCombo();

    connect(comboVibratoPreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VibratoEditorDialog::onVibratoPresetChanged);
    connect(btnSavePreset, &QPushButton::clicked,
            this, &VibratoEditorDialog::onSaveVibratoPresetClicked);
    connect(btnDeletePreset, &QPushButton::clicked,
            this, &VibratoEditorDialog::onDeleteVibratoPresetClicked);
    connect(btnSetDefault, &QPushButton::clicked,
            this, &VibratoEditorDialog::onSetDefaultPresetClicked);

    // Parameters group
    QGroupBox *paramsGroup = new QGroupBox("Parameters");
    QGridLayout *paramsLayout = new QGridLayout(paramsGroup);

    int row = 0;

    // Rate
    paramsLayout->addWidget(new QLabel("Rate (Hz):"), row, 0);
    spinRate = new QDoubleSpinBox();
    spinRate->setRange(0.5, 20.0);
    spinRate->setSingleStep(0.5);
    spinRate->setDecimals(1);
    spinRate->setValue(5.0);
    spinRate->setToolTip("Speed of vibrato oscillation (typical: 4-7 Hz)");
    paramsLayout->addWidget(spinRate, row, 1);
    connect(spinRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VibratoEditorDialog::onRateChanged);
    row++;

    // Pitch Depth
    paramsLayout->addWidget(new QLabel("Pitch Depth (%):"), row, 0);
    spinPitchDepth = new QDoubleSpinBox();
    spinPitchDepth->setRange(0.0, 20.0);
    spinPitchDepth->setSingleStep(0.5);
    spinPitchDepth->setDecimals(1);
    spinPitchDepth->setValue(2.0);
    spinPitchDepth->setToolTip("How much pitch varies (typically small, 1-3%)");
    paramsLayout->addWidget(spinPitchDepth, row, 1);
    connect(spinPitchDepth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VibratoEditorDialog::onPitchDepthChanged);
    row++;

    // Amplitude Depth
    paramsLayout->addWidget(new QLabel("Amplitude Depth (%):"), row, 0);
    spinAmplitudeDepth = new QDoubleSpinBox();
    spinAmplitudeDepth->setRange(0.0, 100.0);
    spinAmplitudeDepth->setSingleStep(5.0);
    spinAmplitudeDepth->setDecimals(0);
    spinAmplitudeDepth->setValue(30.0);
    spinAmplitudeDepth->setToolTip("How much loudness varies (typically larger than pitch)");
    paramsLayout->addWidget(spinAmplitudeDepth, row, 1);
    connect(spinAmplitudeDepth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VibratoEditorDialog::onAmplitudeDepthChanged);
    row++;

    // Onset
    paramsLayout->addWidget(new QLabel("Onset (%):"), row, 0);
    spinOnset = new QDoubleSpinBox();
    spinOnset->setRange(0.0, 90.0);
    spinOnset->setSingleStep(5.0);
    spinOnset->setDecimals(0);
    spinOnset->setValue(20.0);
    spinOnset->setToolTip("When vibrato starts (% into note duration)");
    paramsLayout->addWidget(spinOnset, row, 1);
    connect(spinOnset, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VibratoEditorDialog::onOnsetChanged);
    row++;

    // Regularity
    paramsLayout->addWidget(new QLabel("Regularity:"), row, 0);
    QHBoxLayout *regLayout = new QHBoxLayout();
    QLabel *labelMechanical = new QLabel("Mechanical");
    labelMechanical->setStyleSheet("color: gray; font-size: 10px;");
    sliderRegularity = new QSlider(Qt::Horizontal);
    sliderRegularity->setRange(0, 100);
    sliderRegularity->setValue(50);
    sliderRegularity->setToolTip("0 = perfectly regular, 100 = organic variation");
    QLabel *labelOrganic = new QLabel("Organic");
    labelOrganic->setStyleSheet("color: gray; font-size: 10px;");
    labelRegularityValue = new QLabel("50%");
    labelRegularityValue->setMinimumWidth(40);
    regLayout->addWidget(labelMechanical);
    regLayout->addWidget(sliderRegularity);
    regLayout->addWidget(labelOrganic);
    regLayout->addWidget(labelRegularityValue);
    paramsLayout->addLayout(regLayout, row, 1);
    connect(sliderRegularity, &QSlider::valueChanged,
            this, &VibratoEditorDialog::onRegularitySliderChanged);

    mainLayout->addWidget(paramsGroup);

    // Envelope group
    QGroupBox *envelopeGroup = new QGroupBox("Intensity Envelope");
    QVBoxLayout *envLayout = new QVBoxLayout(envelopeGroup);

    QHBoxLayout *envHeaderLayout = new QHBoxLayout();
    QLabel *envHint = new QLabel("Click to add points, drag to move, right-click to delete. "
                                  "Shift+drag to snap to grid.");
    envHint->setStyleSheet("color: gray; font-size: 10px;");
    envHint->setWordWrap(true);
    envHeaderLayout->addWidget(envHint, 1);
    btnLoadEnvelope = new QPushButton("Load from Library...");
    btnLoadEnvelope->setToolTip("Load envelope curve from the envelope library");
    envHeaderLayout->addWidget(btnLoadEnvelope);
    envLayout->addLayout(envHeaderLayout);

    connect(btnLoadEnvelope, &QPushButton::clicked,
            this, &VibratoEditorDialog::onLoadEnvelopeClicked);

    envelopeCanvas = new EnvelopeCurveCanvas();
    envelopeCanvas->setMinimumHeight(180);
    envLayout->addWidget(envelopeCanvas);
    connect(envelopeCanvas, &EnvelopeCurveCanvas::curveChanged,
            this, &VibratoEditorDialog::onEnvelopeChanged);

    mainLayout->addWidget(envelopeGroup);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void VibratoEditorDialog::setVibrato(const Vibrato &v)
{
    currentVibrato = v;

    // If vibrato is inactive (fresh note) and we have a default preset, apply it
    if (!v.active && !defaultPresetName.isEmpty()) {
        for (const auto &preset : userPresets) {
            if (preset.name.compare(defaultPresetName, Qt::CaseInsensitive) == 0) {
                currentVibrato = preset.vibrato;
                currentVibrato.active = false;
                break;
            }
        }
        for (const auto &preset : factoryPresets) {
            if (preset.name.compare(defaultPresetName, Qt::CaseInsensitive) == 0) {
                currentVibrato = preset.vibrato;
                currentVibrato.active = false;
                break;
            }
        }
    }

    // Block signals while updating UI
    spinRate->blockSignals(true);
    spinPitchDepth->blockSignals(true);
    spinAmplitudeDepth->blockSignals(true);
    spinOnset->blockSignals(true);
    sliderRegularity->blockSignals(true);

    spinRate->setValue(currentVibrato.rate);
    spinPitchDepth->setValue(currentVibrato.pitchDepth * 100.0);
    spinAmplitudeDepth->setValue(currentVibrato.amplitudeDepth * 100.0);
    spinOnset->setValue(currentVibrato.onset * 100.0);
    sliderRegularity->setValue(static_cast<int>(currentVibrato.regularity * 100.0));
    updateRegularityLabel();

    envelopeCanvas->setPoints(currentVibrato.envelope);

    spinRate->blockSignals(false);
    spinPitchDepth->blockSignals(false);
    spinAmplitudeDepth->blockSignals(false);
    spinOnset->blockSignals(false);
    sliderRegularity->blockSignals(false);

    // Try to match current vibrato against presets and select in combo
    int matchIndex = findMatchingPreset(currentVibrato);
    updatingFromPreset = true;
    comboVibratoPreset->setCurrentIndex(matchIndex);
    updatingFromPreset = false;
    updatePresetButtons();
}

Vibrato VibratoEditorDialog::getVibrato() const
{
    return currentVibrato;
}

bool VibratoEditorDialog::getDefaultPreset(Vibrato &out)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = appDataPath + "/vibrato_presets.json";

    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QString defaultName = root["defaultPreset"].toString();
    if (defaultName.isEmpty()) return false;

    // Search user presets
    QJsonArray arr = root["presets"].toArray();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        if (obj["name"].toString().compare(defaultName, Qt::CaseInsensitive) == 0) {
            out = Vibrato::fromJson(obj["vibrato"].toObject());
            return true;
        }
    }

    return false;
}

void VibratoEditorDialog::onRateChanged(double value)
{
    currentVibrato.rate = value;
}

void VibratoEditorDialog::onPitchDepthChanged(double value)
{
    currentVibrato.pitchDepth = value / 100.0;  // Convert from percentage
}

void VibratoEditorDialog::onAmplitudeDepthChanged(double value)
{
    currentVibrato.amplitudeDepth = value / 100.0;
}

void VibratoEditorDialog::onOnsetChanged(double value)
{
    currentVibrato.onset = value / 100.0;
}

void VibratoEditorDialog::onRegularitySliderChanged(int value)
{
    currentVibrato.regularity = value / 100.0;
    updateRegularityLabel();
}

void VibratoEditorDialog::onEnvelopeChanged()
{
    currentVibrato.envelope = envelopeCanvas->getPoints();
}

void VibratoEditorDialog::updateRegularityLabel()
{
    labelRegularityValue->setText(QString("%1%").arg(sliderRegularity->value()));
}

void VibratoEditorDialog::onLoadEnvelopeClicked()
{
    EnvelopeLibraryDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        EnvelopeData envData = dialog.getSelectedEnvelope();
        if (!envData.points.isEmpty()) {
            currentVibrato.envelope = envData.points;
            envelopeCanvas->setPoints(envData.points);
        }
    }
}

void VibratoEditorDialog::onVibratoPresetChanged(int index)
{
    if (updatingFromPreset || index < 0) return;

    // Index 0 is "Custom" - no action
    if (index == 0) return;

    updatePresetButtons();

    int adjustedIndex = index - 1;  // Account for "Custom" entry

    // User presets come first
    if (adjustedIndex < userPresets.size()) {
        applyPreset(userPresets[adjustedIndex].vibrato);
    } else {
        // Skip separator (1 item) then factory presets
        int factoryIndex = adjustedIndex - userPresets.size();
        if (!userPresets.isEmpty()) factoryIndex--;  // account for separator
        if (factoryIndex >= 0 && factoryIndex < factoryPresets.size()) {
            applyPreset(factoryPresets[factoryIndex].vibrato);
        }
    }
}

void VibratoEditorDialog::onSaveVibratoPresetClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Save Vibrato Preset",
                                          "Preset name:", QLineEdit::Normal,
                                          "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    name = name.trimmed();

    // Check for duplicate names
    for (const auto &preset : userPresets) {
        if (preset.name.compare(name, Qt::CaseInsensitive) == 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Preset Exists",
                QString("A preset named '%1' already exists. Overwrite?").arg(name),
                QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) return;

            // Remove the existing preset
            for (int i = 0; i < userPresets.size(); ++i) {
                if (userPresets[i].name.compare(name, Qt::CaseInsensitive) == 0) {
                    userPresets.removeAt(i);
                    break;
                }
            }
            break;
        }
    }

    VibratoPreset newPreset;
    newPreset.name = name;
    newPreset.vibrato = currentVibrato;
    newPreset.isFactory = false;
    userPresets.append(newPreset);

    saveUserPresets();
    updatePresetCombo();

    // Select the newly saved preset
    comboVibratoPreset->setCurrentText(name);
}

void VibratoEditorDialog::onDeleteVibratoPresetClicked()
{
    int userIndex;
    if (!isUserPresetSelected(&userIndex)) return;

    QString name = userPresets[userIndex].name;
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Preset",
        QString("Delete preset '%1'?").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    // If deleting the default, clear the default
    if (defaultPresetName.compare(name, Qt::CaseInsensitive) == 0) {
        defaultPresetName.clear();
    }

    userPresets.removeAt(userIndex);
    saveUserPresets();
    updatePresetCombo();
}

void VibratoEditorDialog::onSetDefaultPresetClicked()
{
    int comboIndex = comboVibratoPreset->currentIndex();
    if (comboIndex <= 0) {
        // "Custom" selected — clear the default
        defaultPresetName.clear();
        saveUserPresets();
        updatePresetCombo();
        return;
    }

    int userIndex;
    if (isUserPresetSelected(&userIndex)) {
        defaultPresetName = userPresets[userIndex].name;
    } else {
        // Factory preset selected — allow setting factory as default too
        int adjustedIndex = comboIndex - 1;
        int factoryIndex = adjustedIndex - userPresets.size();
        if (!userPresets.isEmpty()) factoryIndex--;
        if (factoryIndex >= 0 && factoryIndex < factoryPresets.size()) {
            defaultPresetName = factoryPresets[factoryIndex].name;
        }
    }

    saveUserPresets();
    updatePresetCombo();
}

bool VibratoEditorDialog::isUserPresetSelected(int *userIndex) const
{
    int comboIndex = comboVibratoPreset->currentIndex();
    if (comboIndex <= 0) return false;

    int adjustedIndex = comboIndex - 1;
    if (adjustedIndex < userPresets.size()) {
        if (userIndex) *userIndex = adjustedIndex;
        return true;
    }
    return false;
}

int VibratoEditorDialog::findMatchingPreset(const Vibrato &v) const
{
    auto matches = [&](const Vibrato &p) {
        return qFuzzyCompare(v.rate, p.rate) &&
               qFuzzyCompare(v.pitchDepth, p.pitchDepth) &&
               qFuzzyCompare(v.amplitudeDepth, p.amplitudeDepth) &&
               qFuzzyCompare(v.onset, p.onset) &&
               qFuzzyCompare(v.regularity, p.regularity);
    };

    // Check user presets first (combo indices 1..userPresets.size())
    for (int i = 0; i < userPresets.size(); ++i) {
        if (matches(userPresets[i].vibrato))
            return i + 1;  // +1 for "Custom"
    }

    // Check factory presets (after user presets + separator)
    int factoryStart = 1 + userPresets.size() + (userPresets.isEmpty() ? 0 : 1);
    for (int i = 0; i < factoryPresets.size(); ++i) {
        if (matches(factoryPresets[i].vibrato))
            return factoryStart + i;
    }

    return 0;  // "Custom"
}

void VibratoEditorDialog::updatePresetButtons()
{
    bool isUser = isUserPresetSelected();
    btnDeletePreset->setEnabled(isUser);
    // Set Default works for any preset (user or factory), just not "Custom"
    btnSetDefault->setEnabled(comboVibratoPreset->currentIndex() > 0);
}

void VibratoEditorDialog::loadFactoryPresets()
{
    factoryPresets.clear();

    // Classic vocal vibrato
    {
        VibratoPreset p;
        p.name = "Classic Vocal";
        p.vibrato.rate = 5.5;
        p.vibrato.pitchDepth = 0.02;
        p.vibrato.amplitudeDepth = 0.15;
        p.vibrato.onset = 0.2;
        p.vibrato.regularity = 0.3;
        p.vibrato.envelope = {{0.0, 0.0}, {0.2, 1.0}, {0.8, 1.0}, {1.0, 0.5}};
        p.isFactory = true;
        factoryPresets.append(p);
    }

    // String instrument vibrato
    {
        VibratoPreset p;
        p.name = "String";
        p.vibrato.rate = 6.0;
        p.vibrato.pitchDepth = 0.03;
        p.vibrato.amplitudeDepth = 0.05;
        p.vibrato.onset = 0.1;
        p.vibrato.regularity = 0.4;
        p.vibrato.envelope = {{0.0, 0.0}, {0.15, 1.0}, {1.0, 1.0}};
        p.isFactory = true;
        factoryPresets.append(p);
    }

    // Slow wide vibrato
    {
        VibratoPreset p;
        p.name = "Slow Wide";
        p.vibrato.rate = 4.0;
        p.vibrato.pitchDepth = 0.05;
        p.vibrato.amplitudeDepth = 0.25;
        p.vibrato.onset = 0.3;
        p.vibrato.regularity = 0.2;
        p.vibrato.envelope = {{0.0, 0.0}, {0.3, 0.5}, {0.6, 1.0}, {1.0, 0.8}};
        p.isFactory = true;
        factoryPresets.append(p);
    }

    // Fast nervous vibrato
    {
        VibratoPreset p;
        p.name = "Fast Nervous";
        p.vibrato.rate = 8.0;
        p.vibrato.pitchDepth = 0.015;
        p.vibrato.amplitudeDepth = 0.4;
        p.vibrato.onset = 0.05;
        p.vibrato.regularity = 0.7;
        p.vibrato.envelope = {{0.0, 0.8}, {0.5, 1.0}, {1.0, 0.6}};
        p.isFactory = true;
        factoryPresets.append(p);
    }

    // Gentle fade-in
    {
        VibratoPreset p;
        p.name = "Gentle Fade-In";
        p.vibrato.rate = 5.0;
        p.vibrato.pitchDepth = 0.02;
        p.vibrato.amplitudeDepth = 0.1;
        p.vibrato.onset = 0.4;
        p.vibrato.regularity = 0.25;
        p.vibrato.envelope = {{0.0, 0.0}, {0.5, 0.3}, {0.8, 1.0}, {1.0, 1.0}};
        p.isFactory = true;
        factoryPresets.append(p);
    }

    // Tremolo (amplitude only)
    {
        VibratoPreset p;
        p.name = "Tremolo";
        p.vibrato.rate = 7.0;
        p.vibrato.pitchDepth = 0.0;
        p.vibrato.amplitudeDepth = 0.5;
        p.vibrato.onset = 0.0;
        p.vibrato.regularity = 0.1;
        p.vibrato.envelope = {{0.0, 1.0}, {1.0, 1.0}};
        p.isFactory = true;
        factoryPresets.append(p);
    }
}

void VibratoEditorDialog::loadUserPresets()
{
    userPresets.clear();
    defaultPresetName.clear();

    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = appDataPath + "/vibrato_presets.json";

    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    QJsonArray arr;
    if (doc.isObject()) {
        // New format: { "defaultPreset": "name", "presets": [...] }
        QJsonObject root = doc.object();
        defaultPresetName = root["defaultPreset"].toString();
        arr = root["presets"].toArray();
    } else if (doc.isArray()) {
        // Old format: plain array — migrate on next save
        arr = doc.array();
    } else {
        return;
    }

    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        VibratoPreset p;
        p.name = obj["name"].toString();
        p.isFactory = false;

        if (obj.contains("vibrato")) {
            p.vibrato = Vibrato::fromJson(obj["vibrato"].toObject());
        }

        if (!p.name.isEmpty()) {
            userPresets.append(p);
        }
    }
}

void VibratoEditorDialog::saveUserPresets()
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = appDataPath + "/vibrato_presets.json";

    QJsonArray arr;
    for (const auto &preset : userPresets) {
        QJsonObject obj;
        obj["name"] = preset.name;
        obj["vibrato"] = preset.vibrato.toJson();
        arr.append(obj);
    }

    QJsonObject root;
    root["defaultPreset"] = defaultPresetName;
    root["presets"] = arr;

    QJsonDocument doc(root);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void VibratoEditorDialog::updatePresetCombo()
{
    updatingFromPreset = true;
    int previousIndex = comboVibratoPreset->currentIndex();
    comboVibratoPreset->clear();

    comboVibratoPreset->addItem("Custom");

    // User presets first
    for (const auto &preset : userPresets) {
        QString label = preset.name;
        if (!defaultPresetName.isEmpty() &&
            preset.name.compare(defaultPresetName, Qt::CaseInsensitive) == 0) {
            label += "  \u2605";  // star to mark default
        }
        comboVibratoPreset->addItem(label);
    }

    // Separator between user and factory presets
    if (!userPresets.isEmpty()) {
        comboVibratoPreset->insertSeparator(comboVibratoPreset->count());
    }

    // Factory presets after
    for (const auto &preset : factoryPresets) {
        QString label = preset.name;
        if (!defaultPresetName.isEmpty() &&
            preset.name.compare(defaultPresetName, Qt::CaseInsensitive) == 0) {
            label += "  \u2605";
        }
        comboVibratoPreset->addItem(label);
    }

    // Restore selection or go to Custom
    if (previousIndex >= 0 && previousIndex < comboVibratoPreset->count()) {
        comboVibratoPreset->setCurrentIndex(previousIndex);
    } else {
        comboVibratoPreset->setCurrentIndex(0);
    }

    updatingFromPreset = false;
    updatePresetButtons();
}

void VibratoEditorDialog::applyPreset(const Vibrato &preset)
{
    updatingFromPreset = true;

    // Preserve the current 'active' state - it's controlled by the main checkbox,
    // not by presets. Presets only affect the vibrato parameters.
    bool wasActive = currentVibrato.active;
    setVibrato(preset);
    currentVibrato.active = wasActive;

    updatingFromPreset = false;
}
