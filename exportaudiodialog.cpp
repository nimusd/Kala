#include "exportaudiodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>

ExportAudioDialog::ExportAudioDialog(const QString &projectName,
                                     int trackCount,
                                     unsigned int sampleRate,
                                     QWidget *parent)
    : QDialog(parent)
    , m_projectName(projectName)
    , m_trackCount(trackCount)
    , m_sampleRate(sampleRate)
{
    setupUI();
    setWindowTitle("Export Audio");
    resize(500, 300);
}

void ExportAudioDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Export Mode group
    QGroupBox *modeBox = new QGroupBox("Export Mode", this);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeBox);

    radioSingleFile = new QRadioButton("All audio to single file", this);
    radioSeparateFiles = new QRadioButton("Each track to separate file", this);
    radioSingleFile->setChecked(true);

    exportModeGroup = new QButtonGroup(this);
    exportModeGroup->addButton(radioSingleFile, 0);
    exportModeGroup->addButton(radioSeparateFiles, 1);

    modeLayout->addWidget(radioSingleFile);
    modeLayout->addWidget(radioSeparateFiles);

    // Track info label (shown when separate files selected)
    labelTrackInfo = new QLabel(QString("(%1 tracks will be exported)").arg(m_trackCount), this);
    labelTrackInfo->setStyleSheet("color: gray; margin-left: 20px;");
    labelTrackInfo->setVisible(false);
    modeLayout->addWidget(labelTrackInfo);

    mainLayout->addWidget(modeBox);

    // Output Location group
    QGroupBox *locationBox = new QGroupBox("Output Location", this);
    QHBoxLayout *locationLayout = new QHBoxLayout(locationBox);

    editExportPath = new QLineEdit(this);
    // Default to user's music folder with project name
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (!defaultPath.isEmpty()) {
        defaultPath += "/" + m_projectName + ".wav";
    }
    editExportPath->setText(defaultPath);

    btnBrowse = new QPushButton("Browse...", this);

    locationLayout->addWidget(editExportPath);
    locationLayout->addWidget(btnBrowse);

    mainLayout->addWidget(locationBox);

    // Settings group (read-only info)
    QGroupBox *settingsBox = new QGroupBox("Settings", this);
    QFormLayout *settingsLayout = new QFormLayout(settingsBox);

    labelSampleRate = new QLabel(QString("%1 Hz").arg(m_sampleRate), this);
    labelBitDepth = new QLabel("24-bit", this);
    labelChannels = new QLabel("Stereo", this);

    settingsLayout->addRow("Sample Rate:", labelSampleRate);
    settingsLayout->addRow("Bit Depth:", labelBitDepth);
    settingsLayout->addRow("Channels:", labelChannels);

    mainLayout->addWidget(settingsBox);

    // Spacer
    mainLayout->addStretch();

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    mainLayout->addWidget(buttonBox);

    // Connect signals
    connect(exportModeGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ExportAudioDialog::onExportModeChanged);
    connect(btnBrowse, &QPushButton::clicked,
            this, &ExportAudioDialog::onBrowseClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ExportAudioDialog::onBrowseClicked()
{
    QString path;
    QSettings settings;

    // Use current path, or fall back to remembered directory
    QString startDir = editExportPath->text();
    if (startDir.isEmpty()) {
        startDir = settings.value("lastDirectory/export", QStandardPaths::writableLocation(QStandardPaths::MusicLocation)).toString();
    }

    if (radioSingleFile->isChecked()) {
        // Single file: select save file path
        path = QFileDialog::getSaveFileName(
            this,
            "Export Audio",
            startDir,
            "WAV Files (*.wav)");
    } else {
        // Separate files: select directory
        path = QFileDialog::getExistingDirectory(
            this,
            "Select Export Directory",
            startDir);
    }

    if (!path.isEmpty()) {
        editExportPath->setText(path);
        // Remember the directory for next time
        settings.setValue("lastDirectory/export", QFileInfo(path).absolutePath());
    }
}

void ExportAudioDialog::onExportModeChanged(int buttonId)
{
    // Show/hide track info label
    labelTrackInfo->setVisible(buttonId == 1);

    // Update default path based on mode
    QString currentPath = editExportPath->text();
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);

    if (buttonId == 0) {
        // Single file mode - suggest .wav file path
        if (!currentPath.endsWith(".wav", Qt::CaseInsensitive)) {
            editExportPath->setText(basePath + "/" + m_projectName + ".wav");
        }
    } else {
        // Separate files mode - suggest directory
        if (currentPath.endsWith(".wav", Qt::CaseInsensitive)) {
            // Strip filename, keep directory
            QFileInfo fi(currentPath);
            editExportPath->setText(fi.absolutePath());
        }
    }
}

ExportAudioDialog::ExportMode ExportAudioDialog::getExportMode() const
{
    return radioSeparateFiles->isChecked() ? SeparateFiles : SingleFile;
}

QString ExportAudioDialog::getExportPath() const
{
    return editExportPath->text();
}
