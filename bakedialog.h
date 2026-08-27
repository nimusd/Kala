#ifndef BAKEDIALOG_H
#define BAKEDIALOG_H

#include "audiocapture.h"
#include <QDialog>
#include <QList>
#include <QPair>

class Track;
class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;

/**
 * BakeDialog - Phase 9: asks where to record the VL70-m's audio and which
 * track the resulting clip attaches to. The caller starts the capture and
 * the transport together on accept. Also shows the selected track's clip
 * state and can clear an existing bake (to return a track to live MIDI).
 */
class BakeDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param tracks       All tracks (combo index = track index)
     * @param defaultTrack Suggested target (the VL70-m track)
     * @param inputDevices Input-capable devices from AudioCapture::listInputDevices()
     * @param defaultPath  Initial WAV path
     */
    explicit BakeDialog(const QList<Track *> &tracks, int defaultTrack,
                        const QList<AudioCapture::CaptureDevice> &inputDevices,
                        const QString &defaultPath, QWidget *parent = nullptr);

    int selectedTrack() const;
    QString selectedDeviceName() const;
    AudioCapture::ChannelMode channelMode() const;
    QString filePath() const;

private slots:
    void onBrowseClicked();
    void onTrackChanged();
    void onClearClipClicked();

private:
    void updateClipStatus();

    QList<Track *> m_tracks;
    QComboBox *m_trackCombo = nullptr;
    QComboBox *m_deviceCombo = nullptr;
    QComboBox *m_channelCombo = nullptr;
    QLineEdit *m_pathEdit = nullptr;
    QLabel *m_clipStatusLabel = nullptr;
    QPushButton *m_clearClipBtn = nullptr;
};

#endif // BAKEDIALOG_H
