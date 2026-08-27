#include "bakedialog.h"
#include "track.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>

BakeDialog::BakeDialog(const QList<Track *> &tracks, int defaultTrack,
                       const QList<AudioCapture::CaptureDevice> &inputDevices,
                       const QString &defaultPath, QWidget *parent)
    : QDialog(parent)
    , m_tracks(tracks)
{
    setWindowTitle("Bake VL70-m Track");
    resize(520, 280);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *explain = new QLabel(
        "Plays the composition once while recording the audio interface input "
        "carrying the VL70-m. The recording is attached to the selected track "
        "and auto-aligned to its first note.", this);
    explain->setWordWrap(true);
    explain->setStyleSheet("color: gray;");
    mainLayout->addWidget(explain);

    QFormLayout *form = new QFormLayout();

    m_trackCombo = new QComboBox(this);
    for (Track *t : tracks) {
        m_trackCombo->addItem(t ? t->getName() : QString());
    }
    if (defaultTrack >= 0 && defaultTrack < tracks.size()) {
        m_trackCombo->setCurrentIndex(defaultTrack);
    }
    form->addRow("Attach to track:", m_trackCombo);

    // Clip state of the selected track: a baked track plays its recording
    // instead of MIDI, so expose both the status and a way back to live.
    m_clipStatusLabel = new QLabel(this);
    m_clipStatusLabel->setStyleSheet("color: gray;");
    m_clearClipBtn = new QPushButton("Clear clip", this);
    m_clearClipBtn->setFixedWidth(90);
    QHBoxLayout *clipRow = new QHBoxLayout();
    clipRow->addWidget(m_clipStatusLabel, 1);
    clipRow->addWidget(m_clearClipBtn);
    form->addRow("Clip:", clipRow);
    connect(m_trackCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BakeDialog::onTrackChanged);
    connect(m_clearClipBtn, &QPushButton::clicked, this, &BakeDialog::onClearClipClicked);
    updateClipStatus();

    m_deviceCombo = new QComboBox(this);
    for (const auto &dev : inputDevices) {
        // Data = raw device name (stable across RtAudio instances); the
        // display name may carry the " (default)" suffix.
        m_deviceCombo->addItem(dev.displayName, dev.name);
    }
    form->addRow("Input device:", m_deviceCombo);

    m_channelCombo = new QComboBox(this);
    m_channelCombo->addItem("Stereo", AudioCapture::Stereo);
    m_channelCombo->addItem("Input 1 (mono)", AudioCapture::Input1);
    m_channelCombo->addItem("Input 2 (mono)", AudioCapture::Input2);
    form->addRow("Input channel:", m_channelCombo);

    m_pathEdit = new QLineEdit(defaultPath, this);
    QHBoxLayout *pathRow = new QHBoxLayout();
    pathRow->addWidget(m_pathEdit);
    QPushButton *browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, this, &BakeDialog::onBrowseClicked);
    pathRow->addWidget(browseBtn);
    form->addRow("Recording file:", pathRow);

    mainLayout->addLayout(form);
    mainLayout->addStretch();

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText("Bake");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

int BakeDialog::selectedTrack() const
{
    return m_trackCombo->currentIndex();
}

QString BakeDialog::selectedDeviceName() const
{
    return m_deviceCombo->currentData().toString();
}

AudioCapture::ChannelMode BakeDialog::channelMode() const
{
    return static_cast<AudioCapture::ChannelMode>(m_channelCombo->currentData().toInt());
}

QString BakeDialog::filePath() const
{
    return m_pathEdit->text();
}

void BakeDialog::onTrackChanged()
{
    updateClipStatus();
}

void BakeDialog::updateClipStatus()
{
    const int idx = m_trackCombo->currentIndex();
    Track *t = (idx >= 0 && idx < m_tracks.size()) ? m_tracks[idx] : nullptr;
    const bool has = t && t->hasClip();
    m_clearClipBtn->setEnabled(has);
    if (has) {
        // Amber, not grey: this pass overwrites an existing take, and the
        // track is exempt from the freeze while it records - worth noticing
        // before pressing Bake.
        m_clipStatusLabel->setStyleSheet("color: #e08000; font-weight: bold;");
        m_clipStatusLabel->setText(
            QString("Already baked (offset %1 ms) - this pass REPLACES that recording")
                .arg(t->clipOffsetMs(), 0, 'f', 1));
    } else {
        m_clipStatusLabel->setStyleSheet("color: gray;");
        m_clipStatusLabel->setText("No bake - MIDI plays live");
    }
}

void BakeDialog::onClearClipClicked()
{
    const int idx = m_trackCombo->currentIndex();
    Track *t = (idx >= 0 && idx < m_tracks.size()) ? m_tracks[idx] : nullptr;
    if (t && t->hasClip()) {
        t->clearClip();
        updateClipStatus();
    }
}

void BakeDialog::onBrowseClicked()
{
    QString startDir = m_pathEdit->text();
    if (startDir.isEmpty()) {
        startDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
    QString path = QFileDialog::getSaveFileName(
        this, "Bake Recording", startDir, "WAV Files (*.wav)");
    if (!path.isEmpty()) {
        if (!path.endsWith(".wav", Qt::CaseInsensitive)) path += ".wav";
        m_pathEdit->setText(path);
        QSettings().setValue("lastDirectory/bake", QFileInfo(path).absolutePath());
    }
}
