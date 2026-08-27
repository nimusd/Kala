#ifndef LANESLEGEND_H
#define LANESLEGEND_H

#include <QWidget>
#include <QPointer>

class QVBoxLayout;
class QLabel;
class ScoreCanvas;

/**
 * Floating legend panel for the curve-lanes feature (D6, Phase F).
 *
 * Lists the selected note's lane set in catalog order - family swatch, lane
 * name, state hint. Position encodes identity on the canvas; the legend is the
 * required contrast relief / memory aid, since two notes can use different
 * variations and therefore different lane sets (lane 3 is not always the same
 * parameter).
 *
 * Purely a read-out; the owner (ScoreCanvasWindow) calls refresh() on
 * selection changes, curve applications and undo, and toggles visibility with
 * the toolbar button.
 */
class LanesLegend : public QWidget
{
    Q_OBJECT

public:
    explicit LanesLegend(QWidget *parent = nullptr);

    void setScoreCanvas(ScoreCanvas *sc);

    // Rebuild the rows from the selected note's lane set. Hides the panel
    // when nothing is selected (the toolbar button stays checked = "armed").
    void refresh();

signals:
    // Emitted on show/hide so the toolbar toggle stays in sync
    // (same contract as VariationToolbar).
    void visibilityChanged(bool visible);

protected:
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void persistPos() const;
    void clearRows();

    QPointer<ScoreCanvas> m_canvas;
    QLabel      *m_header = nullptr;
    QVBoxLayout *m_rowsLayout = nullptr;

    // Follow parent visibility (Qt::Tool doesn't auto-hide with the parent on
    // Windows), VariationToolbar pattern.
    bool m_followingParent = false;
    bool m_wasVisibleBeforeParentHide = false;
    bool m_hasShown = false;   // first show may need a default position
    // refresh() hides the panel when nothing is selected, but that is an
    // internal state change, not the user closing it - the toolbar button
    // stays checked ("armed") so the legend reappears on the next selection.
    bool m_suppressVisibilitySignal = false;
};

#endif // LANESLEGEND_H
