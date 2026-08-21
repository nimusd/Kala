#pragma once
#include <QObject>
#include <QVector>
#include <QString>

class QPlainTextEdit;
class QWidget;
class QListWidget;
class QLabel;

//  nimaPalette: slash-triggered template palette for the companion input.
//
// Behaviour:
//   - Parses anima-templates.md from the application directory at startup.
//   - When the user types '/' with only whitespace before it on the line,
//     a popup list of templates appears. Typing filters the list.
//   - Enter inserts the selected template body and selects the first
//     {PLACEHOLDER}. Ctrl+Right / Ctrl+Left jump to next / previous slot.
//     (Tab is eaten app-wide by KalaMain for subwindow cycling, so the
//     palette uses Ctrl+arrows instead.)
//   - Escape / click-outside / backspacing past the '/' closes the popup.
//
// The palette installs an event filter on the provided QPlainTextEdit.
// The input must outlive the palette.
class AnimaPalette : public QObject
{
    Q_OBJECT
public:
    explicit AnimaPalette(QPlainTextEdit *input, QObject *parent = nullptr);
    ~AnimaPalette() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTextChanged();
    void onSelectionHintUpdate();

private:
    struct Template {
        QString title;       // "13. Crescendo/decrescendo across selection"
        QString body;        // fenced-block contents (TASK/STEPS/REPORT)
        QString exampleFill; // prose from **Example fill**:
    };

    void loadTemplates();
    QString templatesFilePath() const;

    bool isValidTriggerPosition(int slashPos) const;
    QString currentFilterString() const;

    void showPopup();
    void hidePopup();
    void refilter();
    void positionPopup();
    void moveSelection(int delta);
    void insertSelectedTemplate();
    bool advancePlaceholder();
    bool retreatPlaceholder();

    QPlainTextEdit *m_input;
    QWidget *m_popup = nullptr;
    QListWidget *m_list = nullptr;
    QLabel *m_hint = nullptr;

    QVector<Template> m_templates;
    QVector<int> m_filtered;     // indices into m_templates for current filter

    int  m_triggerPos = -1;      // document position of the active '/'
    bool m_popupVisible = false;
    bool m_insertingTemplate = false;
};
