#include "variationtoolbar.h"

#include "track.h"

#include <QButtonGroup>
#include <QCloseEvent>
#include <QLabel>
#include <QSettings>
#include <QToolButton>
#include <QVBoxLayout>

VariationToolbar::VariationToolbar(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowTitle(tr("Variations"));

    // Wide enough that the title bar text isn't truncated to "V..." on Windows.
    setMinimumWidth(120);

    // Explicit checked styling — colored background + white text — so the
    // active variation is unambiguous regardless of platform style.
    setStyleSheet(
        "QToolButton {"
        "  padding: 4px 6px;"
        "  background: palette(button);"
        "  color: palette(button-text);"
        "  border: 1px solid palette(mid);"
        "  border-radius: 3px;"
        "}"
        "QToolButton:hover {"
        "  background: palette(midlight);"
        "}"
        "QToolButton:checked {"
        "  background: #2d6cdf;"
        "  color: white;"
        "  border: 1px solid #1a4ea8;"
        "  font-weight: bold;"
        "}"
    );

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(6, 6, 6, 6);
    outer->setSpacing(4);

    m_header = new QLabel(tr("(no track)"), this);
    m_header->setAlignment(Qt::AlignHCenter);
    QFont f = m_header->font();
    f.setPointSizeF(qMax(7.0, f.pointSizeF() - 1.0));
    m_header->setFont(f);
    m_header->setStyleSheet("color: #888;");
    outer->addWidget(m_header);

    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);
    outer->addLayout(m_layout);
    outer->addStretch(1);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Follow parent (score canvas) visibility — Qt::Tool + WindowStaysOnTopHint
    // doesn't auto-hide with parent on Windows.
    if (parent)
        parent->installEventFilter(this);

    restoreGeometry();
}

void VariationToolbar::setCurrentTrack(Track *track)
{
    if (m_track == track) {
        rebuildButtons();
        return;
    }

    if (m_track) {
        disconnect(m_track, nullptr, this, nullptr);
    }

    m_track = track;

    if (m_track) {
        connect(m_track, &Track::variationCreated,
                this, [this](int, const QString &){ rebuildButtons(); });
        connect(m_track, &Track::variationDeleted,
                this, [this](int){ rebuildButtons(); });
        connect(m_track, &Track::variationRenamed,
                this, [this](int, const QString &){ rebuildButtons(); });
    }

    rebuildButtons();
}

void VariationToolbar::rebuildButtons()
{
    // Wipe existing buttons. QButtonGroup auto-removes deleted buttons.
    for (QToolButton *b : m_buttons) {
        if (b) b->deleteLater();
    }
    m_buttons.clear();

    if (!m_track) {
        m_header->setText(tr("(no track)"));
        return;
    }

    m_header->setText(m_track->getName().isEmpty() ? tr("(track)") : m_track->getName());

    // Index 0 = Base sounit
    auto makeButton = [this](int idx, const QString &label, const QString &tooltip) {
        auto *btn = new QToolButton(this);
        btn->setCheckable(true);
        btn->setAutoRaise(false);
        btn->setText(label);
        btn->setToolTip(tooltip);
        btn->setMinimumSize(36, 28);
        btn->setMaximumWidth(160);
        btn->setContextMenuPolicy(Qt::CustomContextMenu);
        btn->setProperty("variationIndex", idx);
        connect(btn, &QToolButton::clicked, this, [this, idx]() {
            emit activeVariationChanged(idx);
        });
        connect(btn, &QToolButton::customContextMenuRequested,
                this, [this, btn, idx](const QPoint &localPos) {
            emit variationButtonRightClicked(idx, btn->mapToGlobal(localPos));
        });
        m_group->addButton(btn);
        m_layout->addWidget(btn);
        m_buttons.append(btn);
    };

    makeButton(0, tr("Base"), tr("Base sounit"));

    const int count = m_track->getVariationCount();
    for (int i = 1; i <= count; ++i) {
        if (m_track->isInternalVariation(i))
            continue;  // hide note-mode auto variations
        const QString name = m_track->getVariationName(i);
        const QString label = name.isEmpty() ? QString::number(i) : name;
        const QString tooltip = name.isEmpty()
            ? tr("Variation %1").arg(i)
            : tr("%1: %2").arg(i).arg(name);
        makeButton(i, label, tooltip);
    }

    // Default check on Base after rebuild — host can update via refreshFromSelection.
    if (!m_buttons.isEmpty())
        m_buttons.first()->setChecked(true);

    adjustSize();
}

void VariationToolbar::refreshFromSelection(const QVector<int> &selectedVariations,
                                            int activeForNewNotes)
{
    // Decide which button to highlight
    int targetIndex = -1;
    if (selectedVariations.isEmpty()) {
        targetIndex = activeForNewNotes;
    } else {
        // All-equal? Highlight that one. Mixed? Uncheck all.
        const int first = selectedVariations.first();
        bool allEqual = true;
        for (int v : selectedVariations) {
            if (v != first) { allEqual = false; break; }
        }
        targetIndex = allEqual ? first : -1;
    }

    // Apply check state. Tear down exclusivity briefly so we can clear all.
    m_group->setExclusive(false);
    for (QToolButton *b : m_buttons) {
        if (!b) continue;
        const int idx = b->property("variationIndex").toInt();
        b->setChecked(targetIndex >= 0 && idx == targetIndex);
    }
    m_group->setExclusive(true);
}

void VariationToolbar::closeEvent(QCloseEvent *event)
{
    persistGeometry();
    QWidget::closeEvent(event);
}

void VariationToolbar::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (!m_followingParent)
        emit visibilityChanged(false);
}

void VariationToolbar::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_followingParent)
        emit visibilityChanged(true);
}

void VariationToolbar::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    persistGeometry();
}

bool VariationToolbar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parent()) {
        if (event->type() == QEvent::Hide && isVisible()) {
            m_wasVisibleBeforeParentHide = true;
            m_followingParent = true;
            hide();
            m_followingParent = false;
        } else if (event->type() == QEvent::Show && m_wasVisibleBeforeParentHide) {
            m_wasVisibleBeforeParentHide = false;
            m_followingParent = true;
            show();
            m_followingParent = false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void VariationToolbar::persistGeometry() const
{
    QSettings s;
    s.setValue("VariationToolbar/pos", pos());
}

void VariationToolbar::restoreGeometry()
{
    QSettings s;
    const QPoint p = s.value("VariationToolbar/pos", QPoint(40, 120)).toPoint();
    move(p);
}
