#ifndef CONTAINER_H
#define CONTAINER_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QColor>
#include <QStringList>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QMoveEvent>
#include <QMap>
#include <vector>
#include "envelopelibraryDialog.h"

namespace Ui {
class Container;
}

class Container : public QWidget
{
    Q_OBJECT

public:
    explicit Container(QWidget *parent = nullptr,
                       const QString &name = "Container",
                       const QColor &color = Qt::blue,
                       const QStringList &inputs = {},
                       const QStringList &outputs = {});
    struct PortInfo {
        QString name;
        bool isOutput;
        QLabel *circle;
    };

    QVector<PortInfo> getPorts() const { return ports; }
    QString getName() const { return containerName; }
    QString getInstanceName() const { return instanceName; }
    void setInstanceName(const QString &name) { instanceName = name; }
    QColor getColor() const { return containerColor; }
    void setSelected(bool selected);

    // Get input and output port lists separately
    QStringList getInputPorts() const;
    QStringList getOutputPorts() const;

    // Parameter management
    void setParameter(const QString &name, double value);
    double getParameter(const QString &name, double defaultValue = 0.0) const;
    QMap<QString, double> getParameters() const { return parameters; }

    // Batch parameter updates (to avoid triggering rebuild for each parameter)
    void beginParameterUpdate();
    void endParameterUpdate();

    // Custom envelope data storage
    void setCustomEnvelopeData(const EnvelopeData &data);
    EnvelopeData getCustomEnvelopeData() const { return customEnvelopeData; }
    bool hasCustomEnvelopeData() const { return customEnvelopeData.points.size() > 0; }

    // Custom DNA name storage
    void setCustomDnaName(const QString &name) { customDnaName = name; }
    QString getCustomDnaName() const { return customDnaName; }

    // Digit formula string storage (precomputed fractional digits for window modulation)
    void setDigitString(const QString &digits) { digitString = digits; }
    QString getDigitString() const { return digitString; }

    // Wavetable data storage (for Wavetable Synth)
    void setWavetableData(const std::vector<float> &data) { wavetableData = data; }
    const std::vector<float>& getWavetableData() const { return wavetableData; }
    bool hasWavetableData() const { return !wavetableData.empty(); }
    void setWavetableFilePath(const QString &path) { wavetableFilePath = path; }
    QString getWavetableFilePath() const { return wavetableFilePath; }

    // IR data storage (for IR Convolution)
    void setIRData(const std::vector<float> &data) { irData = data; }
    const std::vector<float>& getIRData() const { return irData; }
    bool hasIRData() const { return !irData.empty(); }
    void setIRFilePath(const QString &path) { irFilePath = path; }
    QString getIRFilePath() const { return irFilePath; }

    // Follow-Dynamics toggle button (Envelope Engine only)
    void addFollowDynamicsButton();


protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
     void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
     void moveEvent(QMoveEvent *event) override;
     void paintEvent(QPaintEvent *event) override;
 signals:
     void portClicked(Container *container, const QString &portName, bool isOutput, QPoint globalPos);
      void moved();
      void clicked(Container *container);
      void parameterChanged();
      void moveCompleted(Container *container, const QPoint &oldPos, const QPoint &newPos);
      void dragStarted(Container *container);  // Fires at start of drag, before clicked()


private:
    Ui::Container *ui;
    bool dragging;
    QPoint dragStartPosition;
    QPoint m_posBeforeDrag;  // Position before drag started (for undo)
    QColor containerColor;
    void positionPortCircles();
    QVector<PortInfo> ports;
    QString containerName;
    QString instanceName;  // User-editable instance name
    bool isSelected = false;
    QMap<QString, double> parameters;  // Internal parameters (config)
    EnvelopeData customEnvelopeData;  // Custom envelope data (for Envelope Engine)
    QString customDnaName;  // Name of custom DNA pattern (for Harmonic Generator)
    QString digitString;    // Precomputed fractional digit string (digit formula mode)
    std::vector<float> wavetableData;  // Loaded WAV samples (for Wavetable Synth)
    QString wavetableFilePath;  // Path to loaded WAV file
    std::vector<float> irData;  // Loaded IR samples (for IR Convolution)
    QString irFilePath;  // Path to loaded IR WAV file
    bool batchUpdateInProgress = false;  // True when batching parameter updates
    QVector<Container*> m_multiDragCompanions;  // Containers to move together during drag
    QPushButton *followDynamicsBtn = nullptr;    // "Follow Dynamics" toggle (Envelope Engine only)

public:
 ~Container();
 void setMultiDragCompanions(const QVector<Container*> &companions);
};

#endif // CONTAINER_H
