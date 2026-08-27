#include "laneslegend.h"

#include "scorecanvas.h"
#include "track.h"
#include "curvelanes.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>

LanesLegend::LanesLegend(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowTitle(tr("Curve lanes"));

    setMinimumWidth(200);

    // Ink colours for text come from the palette method (D5): muted for state
    // hints, secondary for names. Panel keeps the neutral chrome look.
    setStyleSheet(
        "LanesLegend { background: palette(window); border: 1px solid #c9c7c1; }"
    );

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 6, 8, 6);
    outer->setSpacing(4);

    m_header = new QLabel(tr("(no note selected)"), this);
    QFont f = m_header->font();
    f.setPointSizeF(qMax(7.0, f.pointSizeF() - 1.0));
    m_header->setFont(f);
    m_header->setStyleSheet("color: #898781;");
    outer->addWidget(m_header);

    m_rowsLayout = new QVBoxLayout();
    m_rowsLayout->setContentsMargins(0, 0, 0, 0);
    m_rowsLayout->setSpacing(2);
    outer->addLayout(m_rowsLayout);
    outer->addStretch(1);

    // Follow parent (score canvas window) visibility - Qt::Tool +
    // WindowStaysOnTopHint doesn't auto-hide with parent on Windows.
    if (parent)
        parent->installEventFilter(this);

    // Restore a saved position; the first-show default (top-right of the
    // parent) is applied in showEvent when nothing was saved.
    QSettings s;
    if (s.contains("lanesLegend/pos"))
        move(s.value("lanesLegend/pos").toPoint());
}

void LanesLegend::setScoreCanvas(ScoreCanvas *sc)
{
    m_canvas = sc;
    refresh();
}

void LanesLegend::clearRows()
{
    while (QLayoutItem *item = m_rowsLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
}

void LanesLegend::refresh()
{
    clearRows();

    if (!m_canvas || m_canvas->getSelectedNoteIndices().isEmpty()) {
        // Internal hide - the toolbar button stays checked ("armed") so the
        // panel comes back the moment a note is selected again.
        m_suppressVisibilitySignal = true;
        hide();
        m_suppressVisibilitySignal = false;
        return;
    }

    const QVector<Note> &notes = m_canvas->getPhrase().getNotes();
    const int noteIndex = m_canvas->getSelectedNoteIndices().first();
    if (noteIndex < 0 || noteIndex >= notes.size()) {
        m_suppressVisibilitySignal = true;
        hide();
        m_suppressVisibilitySignal = false;
        return;
    }
    const Note &note = notes[noteIndex];

    // Header: which note this legend describes. The variation matters because
    // the lane set is per-variation - "lane 3" does not mean the same
    // parameter on every note (D6).
    QString header = tr("Note %1").arg(noteIndex + 1);
    if (Track *t = m_canvas->getCurrentTrack()) {
        const QString varName = t->getVariationName(note.getVariationIndex());
        if (!varName.isEmpty())
            header += tr(" · %1").arg(varName);
    }
    m_header->setText(header);

    const QVector<CurveLanes::Lane> lanes = m_canvas->resolveLanesForNote(noteIndex);
    for (const CurveLanes::Lane &lane : lanes) {
        auto *row = new QWidget(this);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 1, 0, 1);
        rowLayout->setSpacing(6);

        // Family swatch - same colour the lane strip wears on the canvas.
        auto *swatch = new QLabel(row);
        swatch->setFixedSize(12, 12);
        swatch->setStyleSheet(
            QString("background-color: %1; border: 1px solid #b0ada5;")
                .arg(CurveLanes::familyColor(lane.family).name()));

        auto *name = new QLabel(lane.name, row);
        name->setStyleSheet("color: #52514e;");

        QString stateText;
        switch (lane.state) {
        case CurveLanes::State::Curve:       stateText = tr("curve"); break;
        case CurveLanes::State::Empty:       stateText = tr("empty"); break;
        case CurveLanes::State::Orphan:      stateText = tr("orphan (stored)"); break;
        case CurveLanes::State::GraphDriven: stateText = tr("graph-driven"); break;
        }
        if (lane.eqCollapsed)
            stateText += tr(" · EQ collapsed");
        auto *state = new QLabel(stateText, row);
        state->setStyleSheet("color: #898781;");

        rowLayout->addWidget(swatch);
        rowLayout->addWidget(name);
        rowLayout->addStretch(1);
        rowLayout->addWidget(state);
        m_rowsLayout->addWidget(row);

        row->setToolTip(tr("neutral %1").arg(lane.neutral));
    }

    adjustSize();
}

void LanesLegend::closeEvent(QCloseEvent *event)
{
    persistPos();
    QWidget::closeEvent(event);
}

void LanesLegend::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (!m_followingParent && !m_suppressVisibilitySignal)
        emit visibilityChanged(false);
}

void LanesLegend::showEvent(QShowEvent *event)
{
    if (!m_hasShown) {
        m_hasShown = true;
        // Nothing saved: drop in at the parent's top-right, clear of its
        // toolbar, like the plan's "over scoreScrollArea" default.
        QSettings s;
        if (!s.contains("lanesLegend/pos")) {
            if (QWidget *p = parentWidget()) {
                const QPoint topRight = p->mapToGlobal(QPoint(p->width(), 0));
                move(topRight.x() - width() - 12, topRight.y() + 90);
            }
        }
    }
    QWidget::showEvent(event);
    if (!m_followingParent)
        emit visibilityChanged(true);
}

void LanesLegend::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    persistPos();
}

bool LanesLegend::eventFilter(QObject *obj, QEvent *event)
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

void LanesLegend::persistPos() const
{
    QSettings s;
    s.setValue("lanesLegend/pos", pos());
}
