#include "envelopelibraryDialog.h"
#include "envelopecurvecanvas.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QSplitter>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QSettings>
#include <QFileInfo>

EnvelopeLibraryDialog::EnvelopeLibraryDialog(QWidget *parent)
    : QDialog(parent)
    , selectedPresetIndex(-1)
    , editingUserPresetIndex(-1)
    , isModified(false)
{
    setWindowTitle("Envelope Library");
    resize(700, 450);

    setupUI();
    loadFactoryPresets();
    updatePresetList();

    // Select first preset by default
    if (presetList->count() > 0) {
        presetList->setCurrentRow(0);
        onPresetSelected(0);
    }
}

void EnvelopeLibraryDialog::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // Left side: Preset list with buttons
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

    QLabel *presetsLabel = new QLabel("Presets:");
    leftLayout->addWidget(presetsLabel);

    QLabel *noteLabel = new QLabel("<i><small>Note: User presets are session-only.<br>Use 'Save to File...' to keep them.</small></i>");
    noteLabel->setWordWrap(true);
    noteLabel->setStyleSheet("color: #999; padding: 2px;");
    leftLayout->addWidget(noteLabel);

    presetList = new QListWidget();
    presetList->setMaximumWidth(200);
    connect(presetList, &QListWidget::currentRowChanged, this, &EnvelopeLibraryDialog::onPresetSelected);
    leftLayout->addWidget(presetList);

    // Preset management buttons
    QHBoxLayout *presetButtonLayout = new QHBoxLayout();
    btnNew = new QPushButton("New");
    btnDelete = new QPushButton("Delete");
    connect(btnNew, &QPushButton::clicked, this, &EnvelopeLibraryDialog::onNewPreset);
    connect(btnDelete, &QPushButton::clicked, this, &EnvelopeLibraryDialog::onDeletePreset);
    presetButtonLayout->addWidget(btnNew);
    presetButtonLayout->addWidget(btnDelete);
    leftLayout->addLayout(presetButtonLayout);

    mainLayout->addWidget(leftPanel);

    // Right side: Envelope editor
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    // Envelope name
    QFormLayout *formLayout = new QFormLayout();
    nameEdit = new QLineEdit();
    connect(nameEdit, &QLineEdit::textChanged, this, &EnvelopeLibraryDialog::onNameChanged);
    formLayout->addRow("Name:", nameEdit);

    // Loop mode
    loopModeCombo = new QComboBox();
    loopModeCombo->addItem("None");
    loopModeCombo->addItem("Loop");
    loopModeCombo->addItem("Ping-pong");
    connect(loopModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnvelopeLibraryDialog::onLoopModeChanged);
    formLayout->addRow("Loop Mode:", loopModeCombo);

    rightLayout->addLayout(formLayout);

    // Curve canvas
    curveCanvas = new EnvelopeCurveCanvas();
    curveCanvas->setMinimumSize(400, 250);
    connect(curveCanvas, &EnvelopeCurveCanvas::curveChanged, this, &EnvelopeLibraryDialog::onCurveChanged);
    rightLayout->addWidget(curveCanvas);

    // Instructions
    QLabel *instructions = new QLabel(
        "<i>Click to add points • Drag to move • Right-click to delete • Shift+drag to snap to grid • Ctrl+click point to cycle curve type (blue=linear, green=smooth, orange=step)</i>");
    instructions->setWordWrap(true);
    rightLayout->addWidget(instructions);

    // File operation buttons
    QHBoxLayout *fileButtonLayout = new QHBoxLayout();
    QPushButton *btnSaveToFile = new QPushButton("Save to File...");
    QPushButton *btnLoadFromFile = new QPushButton("Load from File...");
    btnSaveToFile->setToolTip("Save custom envelope to .env.json file");
    btnLoadFromFile->setToolTip("Load custom envelope from .env.json file");
    fileButtonLayout->addWidget(btnSaveToFile);
    fileButtonLayout->addWidget(btnLoadFromFile);
    fileButtonLayout->addStretch();
    rightLayout->addLayout(fileButtonLayout);

    connect(btnSaveToFile, &QPushButton::clicked, this, &EnvelopeLibraryDialog::onSaveToFile);
    connect(btnLoadFromFile, &QPushButton::clicked, this, &EnvelopeLibraryDialog::onLoadFromFile);

    // Bottom buttons
    QHBoxLayout *bottomButtonLayout = new QHBoxLayout();
    bottomButtonLayout->addStretch();

    btnReset = new QPushButton("Reset");
    btnSave = new QPushButton("Save to Presets");
    QPushButton *btnOK = new QPushButton("OK");
    QPushButton *btnCancel = new QPushButton("Cancel");

    connect(btnReset, &QPushButton::clicked, this, &EnvelopeLibraryDialog::onReset);
    connect(btnSave, &QPushButton::clicked, this, &EnvelopeLibraryDialog::onSavePreset);
    connect(btnOK, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    bottomButtonLayout->addWidget(btnReset);
    bottomButtonLayout->addWidget(btnSave);
    bottomButtonLayout->addWidget(btnOK);
    bottomButtonLayout->addWidget(btnCancel);

    rightLayout->addLayout(bottomButtonLayout);

    mainLayout->addWidget(rightPanel, 1);
}

void EnvelopeLibraryDialog::loadFactoryPresets()
{
    factoryPresets.clear();

    // 1. Linear Rise
    {
        EnvelopeData env;
        env.name = "Linear Rise";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 0.0, 0));
        env.points.append(EnvelopePoint(1.0, 1.0, 0));
        factoryPresets.append(env);
    }

    // 2. Linear Fall
    {
        EnvelopeData env;
        env.name = "Linear Fall";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 1.0, 0));
        env.points.append(EnvelopePoint(1.0, 0.0, 0));
        factoryPresets.append(env);
    }

    // 3. Attack-Decay
    {
        EnvelopeData env;
        env.name = "Attack-Decay";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 0.0, 0));
        env.points.append(EnvelopePoint(0.3, 1.0, 0));
        env.points.append(EnvelopePoint(1.0, 0.0, 0));
        factoryPresets.append(env);
    }

    // 4. Swell
    {
        EnvelopeData env;
        env.name = "Swell";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 0.0, 1));  // Smooth curve
        env.points.append(EnvelopePoint(0.5, 1.0, 1));
        env.points.append(EnvelopePoint(1.0, 0.0, 1));
        factoryPresets.append(env);
    }

    // 5. Fade Out
    {
        EnvelopeData env;
        env.name = "Fade Out";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 1.0, 1));  // Smooth curve
        env.points.append(EnvelopePoint(0.7, 1.0, 1));
        env.points.append(EnvelopePoint(1.0, 0.0, 1));
        factoryPresets.append(env);
    }

    // 6. Bell Curve
    {
        EnvelopeData env;
        env.name = "Bell Curve";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 0.0, 1));  // Smooth
        env.points.append(EnvelopePoint(0.25, 0.5, 1));
        env.points.append(EnvelopePoint(0.5, 1.0, 1));
        env.points.append(EnvelopePoint(0.75, 0.5, 1));
        env.points.append(EnvelopePoint(1.0, 0.0, 1));
        factoryPresets.append(env);
    }

    // 7. Step Up
    {
        EnvelopeData env;
        env.name = "Step Up";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 0.0, 2));  // Step curve type
        env.points.append(EnvelopePoint(0.25, 0.33, 2));
        env.points.append(EnvelopePoint(0.5, 0.66, 2));
        env.points.append(EnvelopePoint(0.75, 1.0, 2));
        env.points.append(EnvelopePoint(1.0, 1.0, 2));
        factoryPresets.append(env);
    }

    // 8. Step Down
    {
        EnvelopeData env;
        env.name = "Step Down";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0, 1.0, 2));  // Step curve type
        env.points.append(EnvelopePoint(0.25, 0.75, 2));
        env.points.append(EnvelopePoint(0.5, 0.5, 2));
        env.points.append(EnvelopePoint(0.75, 0.25, 2));
        env.points.append(EnvelopePoint(1.0, 0.0, 2));
        factoryPresets.append(env);
    }

    // 9. Log Fade In
    // Approximates y = log10(1 + 9x): rises quickly at the start, then gradually flattens
    {
        EnvelopeData env;
        env.name = "Log Fade In";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0,  0.0,  0));
        env.points.append(EnvelopePoint(0.2,  0.45, 0));
        env.points.append(EnvelopePoint(0.4,  0.66, 0));
        env.points.append(EnvelopePoint(0.6,  0.81, 0));
        env.points.append(EnvelopePoint(0.8,  0.91, 0));
        env.points.append(EnvelopePoint(1.0,  1.0,  0));
        factoryPresets.append(env);
    }

    // 10. Log Fade Out
    // Mirror of Log Fade In: holds near full, then drops off sharply at the end
    {
        EnvelopeData env;
        env.name = "Log Fade Out";
        env.isFactory = true;
        env.loopMode = 0;
        env.points.append(EnvelopePoint(0.0,  1.0,  0));
        env.points.append(EnvelopePoint(0.2,  0.55, 0));
        env.points.append(EnvelopePoint(0.4,  0.34, 0));
        env.points.append(EnvelopePoint(0.6,  0.19, 0));
        env.points.append(EnvelopePoint(0.8,  0.09, 0));
        env.points.append(EnvelopePoint(1.0,  0.0,  0));
        factoryPresets.append(env);
    }
}

void EnvelopeLibraryDialog::updatePresetList()
{
    presetList->clear();

    // Add factory presets
    for (const auto &preset : factoryPresets) {
        QListWidgetItem *item = new QListWidgetItem(preset.name + " [Factory]");
        item->setForeground(QColor(150, 150, 150));
        presetList->addItem(item);
    }

    // Add user presets
    for (const auto &preset : userPresets) {
        QListWidgetItem *item = new QListWidgetItem(preset.name);
        item->setForeground(QColor(100, 200, 255));
        presetList->addItem(item);
    }
}

void EnvelopeLibraryDialog::loadEnvelopeToEditor(const EnvelopeData &env)
{
    currentEnvelope = env;

    // Block signals to prevent marking as modified
    nameEdit->blockSignals(true);
    loopModeCombo->blockSignals(true);
    curveCanvas->blockSignals(true);

    nameEdit->setText(env.name);
    loopModeCombo->setCurrentIndex(env.loopMode);
    curveCanvas->setPoints(env.points);

    nameEdit->blockSignals(false);
    loopModeCombo->blockSignals(false);
    curveCanvas->blockSignals(false);

    // Disable editing for factory presets
    bool isEditable = !env.isFactory;
    nameEdit->setEnabled(isEditable);
    loopModeCombo->setEnabled(isEditable);
    curveCanvas->setEnabled(isEditable);  // Disable curve editing for factory presets
    btnSave->setEnabled(isEditable);
    btnReset->setEnabled(isEditable);
    btnDelete->setEnabled(isEditable);

    isModified = false;
}

bool EnvelopeLibraryDialog::saveCurrentEnvelope()
{
    if (currentEnvelope.isFactory) {
        return false;  // Can't save factory presets
    }

    // Update current envelope from UI
    currentEnvelope.name = nameEdit->text();
    currentEnvelope.loopMode = loopModeCombo->currentIndex();
    currentEnvelope.points = curveCanvas->getPoints();

    // Check if we're editing an existing user preset
    if (editingUserPresetIndex >= 0 && editingUserPresetIndex < userPresets.size()) {
        // Update the existing preset
        userPresets[editingUserPresetIndex] = currentEnvelope;
    } else {
        // This is a new preset that hasn't been added yet
        // (shouldn't normally happen since "New" adds it, but handle it)
        userPresets.append(currentEnvelope);
        editingUserPresetIndex = userPresets.size() - 1;
        updatePresetList();

        // Select the newly added preset
        int newIndex = factoryPresets.size() + userPresets.size() - 1;
        presetList->setCurrentRow(newIndex);
    }

    isModified = false;
    return true;
}

EnvelopeData EnvelopeLibraryDialog::getSelectedEnvelope() const
{
    // Return current envelope with latest UI state
    EnvelopeData result = currentEnvelope;
    result.name = nameEdit->text();
    result.loopMode = loopModeCombo->currentIndex();
    result.points = curveCanvas->getPoints();
    return result;
}

void EnvelopeLibraryDialog::setEnvelope(const EnvelopeData &env)
{
    loadEnvelopeToEditor(env);
}

void EnvelopeLibraryDialog::onPresetSelected(int index)
{
    if (index < 0) return;

    selectedPresetIndex = index;

    // Determine if factory or user preset
    if (index < factoryPresets.size()) {
        editingUserPresetIndex = -1;  // Not editing a user preset
        loadEnvelopeToEditor(factoryPresets[index]);
    } else {
        int userIndex = index - factoryPresets.size();
        if (userIndex < userPresets.size()) {
            editingUserPresetIndex = userIndex;  // Track which user preset we're editing
            loadEnvelopeToEditor(userPresets[userIndex]);
        }
    }
}

void EnvelopeLibraryDialog::onNewPreset()
{
    // Generate unique name by finding the next available number
    int nextNumber = 1;
    QString baseName = "New Envelope";
    QString newName = baseName;

    // Check if name exists and increment
    bool nameExists = true;
    while (nameExists) {
        nameExists = false;
        for (const auto &preset : userPresets) {
            if (preset.name == newName) {
                nameExists = true;
                newName = baseName + " " + QString::number(++nextNumber);
                break;
            }
        }
    }

    EnvelopeData newEnv;
    newEnv.name = newName;
    newEnv.isFactory = false;
    newEnv.loopMode = 0;
    newEnv.points.append(EnvelopePoint(0.0, 0.0, 0));
    newEnv.points.append(EnvelopePoint(1.0, 1.0, 0));

    userPresets.append(newEnv);
    editingUserPresetIndex = userPresets.size() - 1;  // Track that we're editing this new preset
    updatePresetList();

    // Select the new preset
    int newIndex = factoryPresets.size() + userPresets.size() - 1;
    presetList->setCurrentRow(newIndex);
}

void EnvelopeLibraryDialog::onDeletePreset()
{
    if (currentEnvelope.isFactory) {
        QMessageBox::warning(this, "Cannot Delete", "Factory presets cannot be deleted.");
        return;
    }

    int userIndex = selectedPresetIndex - factoryPresets.size();
    if (userIndex >= 0 && userIndex < userPresets.size()) {
        userPresets.removeAt(userIndex);
        editingUserPresetIndex = -1;  // Clear editing index after delete
        updatePresetList();

        // Select first preset
        if (presetList->count() > 0) {
            presetList->setCurrentRow(0);
        }
    }
}

void EnvelopeLibraryDialog::onSavePreset()
{
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Please enter a name for the envelope.");
        return;
    }

    // If trying to save a factory preset, create a copy as user preset
    if (currentEnvelope.isFactory) {
        QMessageBox::information(this, "Factory Preset",
            "Factory presets cannot be modified.\n\n"
            "Click 'New' to create a new preset, or 'Load from File...' to load a custom envelope.");
        return;
    }

    if (saveCurrentEnvelope()) {
        QMessageBox::information(this, "Saved", "Envelope preset saved successfully.");
        updatePresetList(); // Refresh the list to show the saved preset
    } else {
        QMessageBox::warning(this, "Save Failed", "Failed to save envelope preset.");
    }
}

void EnvelopeLibraryDialog::onReset()
{
    if (currentEnvelope.isFactory) return;

    // Reset to default linear rise
    curveCanvas->reset();
    onCurveChanged();
}

void EnvelopeLibraryDialog::onNameChanged()
{
    if (!currentEnvelope.isFactory) {
        isModified = true;
    }
}

void EnvelopeLibraryDialog::onLoopModeChanged(int index)
{
    Q_UNUSED(index);
    if (!currentEnvelope.isFactory) {
        isModified = true;
    }
}

void EnvelopeLibraryDialog::onCurveChanged()
{
    if (!currentEnvelope.isFactory) {
        isModified = true;
    }
}

void EnvelopeLibraryDialog::onSaveToFile()
{
    // Get current envelope data from UI
    EnvelopeData envelopeToSave = getSelectedEnvelope();

    if (envelopeToSave.points.isEmpty()) {
        QMessageBox::warning(this, "No Envelope", "No envelope data to save.");
        return;
    }

    // Open file dialog for save location
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/envelope", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Custom Envelope",
        lastDir,
        "Custom Envelope Files (*.env.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    settings.setValue("lastDirectory/envelope", QFileInfo(fileName).absolutePath());

    // Ensure .env.json extension
    if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".env.json";
    }

    // Create JSON object
    QJsonObject jsonObj;
    jsonObj["name"] = envelopeToSave.name;
    jsonObj["description"] = "Custom envelope created in Kala Envelope Library";
    jsonObj["version"] = "1.0";
    jsonObj["loopMode"] = envelopeToSave.loopMode;

    // Store points array
    QJsonArray pointsArray;
    for (const EnvelopePoint &point : envelopeToSave.points) {
        QJsonObject pointObj;
        pointObj["time"] = point.time;
        pointObj["value"] = point.value;
        pointObj["curveType"] = point.curveType;
        pointsArray.append(pointObj);
    }
    jsonObj["points"] = pointsArray;

    // Write to file
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
        "Custom envelope saved successfully to:\n" + fileName);
}

void EnvelopeLibraryDialog::onLoadFromFile()
{
    // Open file dialog for load location
    QSettings settings;
    QString lastDir = settings.value("lastDirectory/envelope", QDir::homePath()).toString();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Custom Envelope",
        lastDir,
        "Custom Envelope Files (*.env.json);;JSON Files (*.json);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    settings.setValue("lastDirectory/envelope", QFileInfo(fileName).absolutePath());

    // Read file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Load Error",
            "Could not open file for reading:\n" + fileName);
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "Load Error",
            "Invalid JSON format in file:\n" + fileName);
        return;
    }

    QJsonObject jsonObj = doc.object();

    // Validate required fields
    if (!jsonObj.contains("points") || !jsonObj["points"].isArray()) {
        QMessageBox::critical(this, "Load Error",
            "Missing or invalid 'points' field in JSON file.");
        return;
    }

    // Extract envelope data
    EnvelopeData loadedEnvelope;
    loadedEnvelope.name = jsonObj.value("name").toString("Loaded Envelope");
    loadedEnvelope.loopMode = jsonObj.value("loopMode").toInt(0);
    loadedEnvelope.isFactory = false;

    // Extract points
    QJsonArray pointsArray = jsonObj["points"].toArray();
    for (const QJsonValue &val : pointsArray) {
        if (val.isObject()) {
            QJsonObject pointObj = val.toObject();
            EnvelopePoint point;
            point.time = pointObj.value("time").toDouble(0.0);
            point.value = pointObj.value("value").toDouble(0.0);
            point.curveType = pointObj.value("curveType").toInt(0);
            loadedEnvelope.points.append(point);
        }
    }

    if (loadedEnvelope.points.isEmpty()) {
        QMessageBox::critical(this, "Load Error",
            "No valid point data found in file.");
        return;
    }

    // Load the envelope into the editor
    setEnvelope(loadedEnvelope);
    editingUserPresetIndex = -1;  // Loaded envelope is not yet in user presets

    QMessageBox::information(this, "Loaded",
        "Successfully loaded: " + loadedEnvelope.name + "\n" +
        QString::number(loadedEnvelope.points.size()) + " points\n\n" +
        "You can now edit it and click 'Save to Presets' to add it to your preset list.");
}
