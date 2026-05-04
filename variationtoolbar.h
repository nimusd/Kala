#ifndef VARIATIONTOOLBAR_H
#define VARIATIONTOOLBAR_H

#include <QWidget>
#include <QPointer>
#include <QVector>

class QVBoxLayout;
class QToolButton;
class QButtonGroup;
class QLabel;
class Track;

/**
 * Floating vertical toolbar that exposes a track's variations as buttons.
 *
 *   - Left-click a button: route to the score canvas — if there is a selection,
 *     reassign the selected notes to that variation; otherwise set the
 *     active variation for newly-created notes.
 *   - Right-click a button: emit variationButtonRightClicked(...) so the
 *     host can show a small management menu (rename / delete / etc.).
 *
 * The toolbar is purely a control surface; it does not mutate notes itself.
 *
 * Owner is responsible for:
 *   - Calling setCurrentTrack() whenever the active track changes.
 *   - Connecting activeVariationChanged() to the score-canvas dispatcher.
 *   - Calling refreshFromSelection() on noteSelectionChanged.
 */
class VariationToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit VariationToolbar(QWidget *parent = nullptr);

    void setCurrentTrack(Track *track);

    // Update button check state from the currently-selected notes.
    // selectedVariations: variation indices of the selection (deduplicated by caller).
    // If the list is empty, the toolbar reflects the active-for-new-notes index.
    void refreshFromSelection(const QVector<int> &selectedVariations,
                              int activeForNewNotes);

signals:
    // Emitted when the user left-clicks a variation button.
    void activeVariationChanged(int index);

    // Emitted when the user right-clicks a variation button.
    void variationButtonRightClicked(int index, const QPoint &globalPos);

    // Emitted when the toolbar is shown/hidden (e.g. user clicks the X).
    // Lets the host menu action stay in sync.
    void visibilityChanged(bool visible);

protected:
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void rebuildButtons();

private:
    void persistGeometry() const;
    void restoreGeometry();

    QPointer<Track> m_track;
    QVBoxLayout    *m_layout = nullptr;
    QLabel         *m_header = nullptr;
    QButtonGroup   *m_group  = nullptr;
    QVector<QToolButton*> m_buttons;  // index 0 = Base, index i = variation i

    // Track parent-driven show/hide so the View-menu checkbox isn't toggled
    // when the score canvas window hides us.
    bool m_followingParent = false;
    bool m_wasVisibleBeforeParentHide = false;
};

#endif // VARIATIONTOOLBAR_H
