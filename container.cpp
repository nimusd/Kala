#include "container.h"
#include "ui_container.h"
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QShowEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

Container::Container(QWidget *parent, const QString &name, const QColor &color,
                     const QStringList &inputs, const QStringList &outputs)
    : QWidget(parent)
    , ui(new Ui::Container)
    , dragging(false)
{
    ui->setupUi(this);
    containerColor = color;
    // Set the container name
    ui->labelInstanceName->setText(name);

    // Set the header color
    QString style = QString("background-color: %1; color: white;").arg(color.name());
    ui->containerFrameHeader->setStyleSheet(style);

    // Set pale background for the body
    QColor paleColor = color.lighter(180);
    QString bodyStyle = QString("background-color: %1;").arg(paleColor.name());
    ui->frameInputs->setStyleSheet(bodyStyle);
    ui->frameOutputs->setStyleSheet(bodyStyle);

    // Clear placeholder labels from inputs frame
    QLayoutItem *item;
    while ((item = ui->frameInputs->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Clear placeholder labels from outputs frame
    while ((item = ui->frameOutputs->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add input port labels
    for (const QString &input : inputs) {
        QLabel *label = new QLabel(input, this);
        ui->frameInputs->layout()->addWidget(label);

        // Create circle on the container itself
        QLabel *circle = new QLabel("●", this);
        circle->setFixedSize(24, 24);
        circle->setStyleSheet("color: #666; font-size: 22px; background-color: #e8d0f0;");
        ports.append({input, false, circle});
    }

    // Add output port labels
    for (const QString &output : outputs) {
        QLabel *label = new QLabel(output, this);
        label->setAlignment(Qt::AlignRight);
        ui->frameOutputs->layout()->addWidget(label);

        // Create circle on the container itself
        QLabel *circle = new QLabel("●", this);
        circle->setFixedSize(24, 24);
        circle->setStyleSheet("color: #666; font-size: 22px; background-color: #e8d0f0;");
        ports.append({output, true, circle});
    }

    // Store container name
    containerName = name;
    instanceName = name;  // Default instance name is same as type name

    // Install event filter on all port circles
    for (const PortInfo &port : ports) {
        port.circle->installEventFilter(this);
    }

    // Let the layout compute the correct size for the number of ports
    adjustSize();
}

void Container::setInputPorts(const QStringList &inputs)
{
    // Remove existing input entries: delete their circles, then erase the
    // PortInfo entries (backwards so indices stay valid). Outputs stay put.
    for (int i = ports.size() - 1; i >= 0; --i) {
        if (ports[i].isOutput) continue;
        delete ports[i].circle;
        ports.removeAt(i);
    }

    // The inputs frame layout holds exactly one label per input port.
    QLayout *inputLayout = ui->frameInputs->layout();
    QLayoutItem *item;
    while ((item = inputLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Rebuild inputs exactly as the constructor does.
    for (const QString &input : inputs) {
        QLabel *label = new QLabel(input, this);
        inputLayout->addWidget(label);

        // Create circle on the container itself
        QLabel *circle = new QLabel("●", this);
        circle->setFixedSize(24, 24);
        circle->setStyleSheet("color: #666; font-size: 22px; background-color: #e8d0f0;");
        circle->installEventFilter(this);
        // The container is typically already visible here, and Qt does not
        // auto-show a child created under a visible parent (constructor
        // circles were shown with the container itself). Without this, the
        // rebuilt circles stay hidden and the ports "disappear".
        circle->show();
        ports.append({input, false, circle});
    }

    // Refresh the layout chain's cached size hints before resizing. On a
    // visible widget, invalidation propagates upward via updateGeometry();
    // on a hidden one (the creation-time trim) it does not, and sizeHint()
    // would return the stale all-rows height.
    ui->frameInputs->layout()->invalidate();
    ui->frameBody->layout()->invalidate();
    ui->verticalLayout->invalidate();

    // Grow/shrink the container to the new port count. resize(sizeHint())
    // rather than adjustSize(): adjustSize does not shrink a hidden widget.
    // The resize/activate order differs by visibility (both orders verified
    // empirically - Qt defers layout work differently in each state):
    if (isVisible()) {
        // Visible: invalidation already propagated up via updateGeometry(),
        // sizeHint() is fresh; resize first so the synchronous resizeEvent
        // repositions, then force the nested layouts to compute label
        // geometry now (otherwise deferred to a polish pass and the circles
        // below would read stale positions and pile at the top).
        resize(sizeHint());
        ui->frameBody->layout()->activate();
        ui->frameInputs->layout()->activate();
    } else {
        // Hidden (creation-time trim): updateGeometry() propagation is
        // deferred, and resize() does not stick while the freshly
        // invalidated layout chain is unactivated - activate first.
        ui->frameBody->layout()->activate();
        ui->frameInputs->layout()->activate();
        resize(sizeHint());
    }
    positionPortCircles();
    // Deferred safety net: once the event loop's polish pass has fully
    // settled the layout chain, re-position the circles so they always match
    // the labels, even if the synchronous activation above ran on stale
    // frame sizes. Context object `this` keeps it safe after destruction.
    QTimer::singleShot(0, this, [this]() { positionPortCircles(); });
}



Container::~Container()
{
    delete ui;
}


void Container::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    positionPortCircles();
}

bool Container::event(QEvent *event)
{
    // A LayoutRequest means the layout chain has (re)activated and the port
    // labels have fresh geometry - the manually-positioned port circles must
    // follow. Covers the cases resizeEvent/showEvent miss: the creation-time
    // port trim on a still-hidden container, and nested-layout activation
    // after setInputPorts (whose deferred polish would otherwise leave the
    // circles at stale positions).
    if (event->type() == QEvent::LayoutRequest) {
        positionPortCircles();
    }
    return QWidget::event(event);
}

void Container::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    positionPortCircles();
}

void Container::positionPortCircles()
{
    int inputIndex = 0;
    int outputIndex = 0;

    for (const PortInfo &port : ports) {
        QLayout *layout;
        int index;
        int xPos;

        if (port.isOutput) {
            layout = ui->frameOutputs->layout();
            index = outputIndex++;
            xPos = width() - 24;
        } else {
            layout = ui->frameInputs->layout();
            index = inputIndex++;
            xPos = 0;
        }

        if (QLayoutItem *item = layout->itemAt(index)) {
            QWidget *label = item->widget();
            if (label) {
                int y = label->mapTo(this, QPoint(0, label->height()/2)).y() - 12;
                port.circle->move(xPos, y);
            }
        }
    }
}

bool Container::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel *circle = qobject_cast<QLabel*>(watched);
        if (circle) {
            for (const PortInfo &port : ports) {
                if (port.circle == circle) {
                    emit portClicked(this, port.name, port.isOutput, circle->mapToGlobal(QPoint(12, 12)));
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Container::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragStartPosition = event->pos();
        m_posBeforeDrag = pos();  // Store widget position before drag for undo
        emit dragStarted(this);  // Fires before clicked() so SounitBuilder can set companions
    }
    emit clicked(this);
}

void Container::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        QPoint newPos = mapToParent(event->pos() - dragStartPosition);
        QPoint delta = newPos - pos();
        move(newPos);
        // Move multi-drag companions by the same delta
        for (Container *companion : m_multiDragCompanions) {
            companion->move(companion->pos() + delta);
        }
    }
}

void Container::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (dragging && pos() != m_posBeforeDrag) {
            emit moveCompleted(this, m_posBeforeDrag, pos());
        }
        dragging = false;
        m_multiDragCompanions.clear();
    }
}

void Container::setMultiDragCompanions(const QVector<Container*> &companions)
{
    m_multiDragCompanions = companions;
}

void Container::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    emit moved();
}


void Container::addFollowDynamicsButton()
{
    if (followDynamicsBtn) return;  // Already created (safe to call multiple times)

    followDynamicsBtn = new QPushButton("~", this);
    followDynamicsBtn->setCheckable(true);
    followDynamicsBtn->setChecked(getParameter("followDynamics", 0.0) > 0.5);
    followDynamicsBtn->setFixedSize(22, 20);
    followDynamicsBtn->setFocusPolicy(Qt::NoFocus);
    followDynamicsBtn->setToolTip("Use score curve: bypass the envelope shape and output a named score curve value (select which curve in the inspector)");
    followDynamicsBtn->setStyleSheet(
        "QPushButton {"
        "    color: white;"
        "    background-color: rgba(0,0,0,0.25);"
        "    border: 1px solid rgba(255,255,255,0.4);"
        "    border-radius: 3px;"
        "    font-weight: bold;"
        "    font-size: 11px;"
        "    padding: 0px;"
        "}"
        "QPushButton:checked {"
        "    background-color: #2ecc71;"
        "    border-color: #27ae60;"
        "}"
    );

    // Insert into the header's QHBoxLayout, right of the instance name label
    ui->containerFrameHeader->layout()->addWidget(followDynamicsBtn);

    connect(followDynamicsBtn, &QPushButton::toggled, this, [this](bool checked) {
        setParameter("followDynamics", checked ? 1.0 : 0.0);
    });

    // Keep button visual in sync when the param is changed from outside the UI
    // (load, paste, AI tool calls). Block signals to avoid the toggled handler
    // re-writing the param.
    connect(this, &Container::parameterChanged, this, [this]() {
        if (!followDynamicsBtn) return;
        const bool desired = getParameter("followDynamics", 0.0) > 0.5;
        if (followDynamicsBtn->isChecked() != desired) {
            QSignalBlocker block(followDynamicsBtn);
            followDynamicsBtn->setChecked(desired);
        }
    });
}

void Container::setSelected(bool selected)
{
    isSelected = selected;
    update();  // Trigger repaint to show/hide selection border
}

void Container::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (isSelected) {
        QPainter painter(this);
        painter.setPen(QPen(QColor("#0066CC"), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5, 5);
    }
}

QStringList Container::getInputPorts() const
{
    QStringList inputs;
    for (const PortInfo &port : ports) {
        if (!port.isOutput) {
            inputs.append(port.name);
        }
    }
    return inputs;
}

QStringList Container::getOutputPorts() const
{
    QStringList outputs;
    for (const PortInfo &port : ports) {
        if (port.isOutput) {
            outputs.append(port.name);
        }
    }
    return outputs;
}

void Container::setParameter(const QString &name, double value)
{
    parameters[name] = value;
    // Only emit signal if not in batch update mode
    if (!batchUpdateInProgress) {
        emit parameterChanged();
    }
}

double Container::getParameter(const QString &name, double defaultValue) const
{
    return parameters.value(name, defaultValue);
}

void Container::setStringParameter(const QString &name, const QString &value)
{
    stringParameters[name] = value;
    if (!batchUpdateInProgress) {
        emit parameterChanged();
    }
}

QString Container::getStringParameter(const QString &name, const QString &defaultValue) const
{
    return stringParameters.value(name, defaultValue);
}

void Container::beginParameterUpdate()
{
    batchUpdateInProgress = true;
}

void Container::endParameterUpdate()
{
    batchUpdateInProgress = false;
    // Emit single parameterChanged signal after all updates
    emit parameterChanged();
}

void Container::setCustomEnvelopeData(const EnvelopeData &data)
{
    customEnvelopeData = data;
}
