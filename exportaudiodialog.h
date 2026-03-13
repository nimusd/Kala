#ifndef EXPORTAUDIODIALOG_H
#define EXPORTAUDIODIALOG_H

#include <QDialog>

class QRadioButton;
class QLineEdit;
class QPushButton;
class QLabel;
class QButtonGroup;

/**
 * ExportAudioDialog - Export composition as WAV file(s)
 *
 * Modal dialog accessible via File > Export Audio (Ctrl+E)
 * Options:
 *   - Single mixed file: All tracks mixed together
 *   - Separate files: One WAV per track (ProjectName_TrackName.wav)
 *
 * Audio settings are fixed:
 *   - Sample rate: From AudioEngine (typically 44100 Hz)
 *   - Bit depth: 24-bit
 *   - Channels: Stereo
 */
class ExportAudioDialog : public QDialog
{
    Q_OBJECT

public:
    enum ExportMode { SingleFile, SeparateFiles };

    /**
     * Create export dialog
     * @param projectName Current project name (used for default filename)
     * @param trackCount Number of tracks in the composition
     * @param sampleRate Audio sample rate from AudioEngine
     * @param parent Parent widget
     */
    explicit ExportAudioDialog(const QString &projectName,
                               int trackCount,
                               unsigned int sampleRate,
                               QWidget *parent = nullptr);

    /**
     * Get the selected export mode
     */
    ExportMode getExportMode() const;

    /**
     * Get the export path (directory for separate files, full path for single file)
     */
    QString getExportPath() const;

private slots:
    void onBrowseClicked();
    void onExportModeChanged(int buttonId);

private:
    void setupUI();

    // UI Components
    QRadioButton *radioSingleFile;
    QRadioButton *radioSeparateFiles;
    QButtonGroup *exportModeGroup;
    QLineEdit *editExportPath;
    QPushButton *btnBrowse;
    QLabel *labelTrackInfo;
    QLabel *labelSampleRate;
    QLabel *labelBitDepth;
    QLabel *labelChannels;

    // Data
    QString m_projectName;
    int m_trackCount;
    unsigned int m_sampleRate;
};

#endif // EXPORTAUDIODIALOG_H
