#include "animapalette.h"

#include <QPlainTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QApplication>
#include <QScreen>
#include <QStringList>

// ─────────────────────────────────────────────────────────────────────────────
// Regex: a " user slot" is {UPPER_SNAKE}. Runtime expressions like
// {len(picks)}, {seq}, {RANGE_T/2} deliberately don't match.
static const QRegularExpression &placeholderRegex()
{
    static const QRegularExpression re(QStringLiteral("\\{[A-Z][A-Z_0-9]*\\}"));
    return re;
}

// ─────────────────────────────────────────────────────────────────────────────
AnimaPalette::AnimaPalette(QPlainTextEdit *input, QObject *parent)
    : QObject(parent), m_input(input)
{
    loadTemplates();

    // ── Popup widget ─────────────────────────────────────────────────────────
    m_popup = new QWidget(nullptr, Qt::Popup | Qt::FramelessWindowHint);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);
    m_popup->setFocusPolicy(Qt::NoFocus);

    m_list = new QListWidget(m_popup);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setUniformItemSizes(true);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_hint = new QLabel(m_popup);
    m_hint->setObjectName("hint");
    m_hint->setWordWrap(true);
    m_hint->setTextFormat(Qt::PlainText);
    m_hint->setMinimumHeight(36);

    auto *v = new QVBoxLayout(m_popup);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    v->addWidget(m_list, 1);
    v->addWidget(m_hint, 0);

    m_popup->setStyleSheet(
        "QWidget { background: #ffffff; }"
        "QListWidget {"
        "  border: 1px solid #2563eb;"
        "  border-top-left-radius: 4px;"
        "  border-top-right-radius: 4px;"
        "  border-bottom: none;"
        "  padding: 2px;"
        "  font-family: 'Segoe UI';"
        "  font-size: 9pt;"
        "  background: #ffffff;"
        "}"
        "QListWidget::item { padding: 3px 8px; color: #1a1a1a; }"
        "QListWidget::item:selected { background: #2563eb; color: #ffffff; }"
        "QLabel#hint {"
        "  color: #555555;"
        "  font-size: 8pt;"
        "  padding: 4px 8px;"
        "  background: #f5f7fb;"
        "  border: 1px solid #2563eb;"
        "  border-top: 1px solid #c8d4ef;"
        "  border-bottom-left-radius: 4px;"
        "  border-bottom-right-radius: 4px;"
        "}"
    );

    // ── Event wiring ─────────────────────────────────────────────────────────
    m_input->installEventFilter(this);
    connect(m_input, &QPlainTextEdit::textChanged,
            this, &AnimaPalette::onTextChanged);
    connect(m_list, &QListWidget::itemSelectionChanged,
            this, &AnimaPalette::onSelectionHintUpdate);
}

AnimaPalette::~AnimaPalette()
{
    delete m_popup;
}

// ─────────────────────────────────────────────────────────────────────────────
// Template parser
// ─────────────────────────────────────────────────────────────────────────────

QString AnimaPalette::templatesFilePath() const
{
    return QCoreApplication::applicationDirPath() + "/anima-templates.md";
}

void AnimaPalette::loadTemplates()
{
    QFile f(templatesFilePath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const QStringList lines = QString::fromUtf8(f.readAll()).split('\n');
    const QRegularExpression titleRe(QStringLiteral(R"(^## (\d+)\.\s+(.+)$)"));
    const QString exampleTag = QStringLiteral("**Example fill**:");

    int i = 0;
    while (i < lines.size()) {
        const auto tm = titleRe.match(lines[i]);
        if (!tm.hasMatch()) { ++i; continue; }

        Template t;
        t.title = QString("%1. %2").arg(tm.captured(1), tm.captured(2).trimmed());
        ++i;

        // Skip prose until we hit the opening ``` fence, or a new title.
        while (i < lines.size()
               && lines[i].trimmed() != QStringLiteral("```")
               && !titleRe.match(lines[i]).hasMatch()) {
            ++i;
        }
        if (i >= lines.size() || lines[i].trimmed() != QStringLiteral("```"))
            continue;
        ++i; // past opening fence

        // Collect body until closing fence
        QStringList bodyLines;
        while (i < lines.size() && lines[i].trimmed() != QStringLiteral("```")) {
            bodyLines.append(lines[i]);
            ++i;
        }
        if (i < lines.size()) ++i; // past closing fence
        t.body = bodyLines.join('\n');

        // Collect example fill (one paragraph after **Example fill**:)
        QStringList exampleLines;
        while (i < lines.size()) {
            const QString &ln = lines[i];
            if (titleRe.match(ln).hasMatch()) break;
            if (ln.startsWith("---")) break;

            if (ln.startsWith(exampleTag)) {
                const QString head = ln.mid(exampleTag.length()).trimmed();
                if (!head.isEmpty()) exampleLines.append(head);
                ++i;
                while (i < lines.size()
                       && !lines[i].trimmed().isEmpty()
                       && !titleRe.match(lines[i]).hasMatch()
                       && !lines[i].startsWith("---")) {
                    exampleLines.append(lines[i].trimmed());
                    ++i;
                }
                break;
            }
            ++i;
        }
        t.exampleFill = exampleLines.join(' ');

        if (!t.body.isEmpty())
            m_templates.push_back(t);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Trigger detection
// ─────────────────────────────────────────────────────────────────────────────

bool AnimaPalette::isValidTriggerPosition(int slashPos) const
{
    QTextBlock block = m_input->document()->findBlock(slashPos);
    if (!block.isValid()) return false;
    const int lineStart = block.position();
    const QString before = block.text().left(slashPos - lineStart);
    return before.trimmed().isEmpty();
}

QString AnimaPalette::currentFilterString() const
{
    if (m_triggerPos < 0) return QString();
    const QString text = m_input->toPlainText();
    if (m_triggerPos >= text.length() || text[m_triggerPos] != QLatin1Char('/'))
        return QString();
    const int cursorPos = m_input->textCursor().position();
    if (cursorPos <= m_triggerPos) return QString();
    return text.mid(m_triggerPos + 1, cursorPos - m_triggerPos - 1);
}

void AnimaPalette::onTextChanged()
{
    if (m_insertingTemplate || m_templates.isEmpty()) return;

    if (!m_popupVisible) {
        // Did the user just type '/' at a valid position?
        QTextCursor c = m_input->textCursor();
        const int pos = c.position();
        if (pos < 1) return;
        const QString text = m_input->toPlainText();
        if (pos > text.length() || text[pos - 1] != QLatin1Char('/')) return;
        if (!isValidTriggerPosition(pos - 1)) return;

        m_triggerPos = pos - 1;
        showPopup();
        refilter();
        return;
    }

    // Popup visible: update filter or close if the user moved on.
    const QString filter = currentFilterString();
    if (filter.isNull()) { hidePopup(); return; }
    if (filter.contains(QLatin1Char(' ')) || filter.contains(QLatin1Char('\n'))
        || filter.contains(QLatin1Char('\t'))) {
        hidePopup();
        return;
    }
    refilter();
}

// ─────────────────────────────────────────────────────────────────────────────
// Popup lifecycle & filtering
// ─────────────────────────────────────────────────────────────────────────────

void AnimaPalette::showPopup()
{
    if (m_popupVisible) return;
    m_popupVisible = true;
    positionPopup();
    m_popup->show();
    m_input->setFocus(Qt::OtherFocusReason);
}

void AnimaPalette::hidePopup()
{
    m_popup->hide();
    m_popupVisible = false;
    m_triggerPos = -1;
}

void AnimaPalette::positionPopup()
{
    const int popupW = qMax(320, m_input->width() - 20);
    const int popupH = 260;
    m_popup->resize(popupW, popupH);

    const QRect cr = m_input->cursorRect();
    const QPoint globalAtCursor = m_input->viewport()->mapToGlobal(cr.topLeft());

    // Prefer showing ABOVE the cursor line so the input stays visible.
    QPoint target(globalAtCursor.x(), globalAtCursor.y() - popupH - 4);

    // Clamp to screen
    QScreen *scr = QApplication::screenAt(globalAtCursor);
    if (!scr) scr = QApplication::primaryScreen();
    const QRect avail = scr ? scr->availableGeometry() : QRect(0, 0, 1920, 1080);

    if (target.y() < avail.top() + 8) {
        // Not enough space above — show below.
        target.setY(globalAtCursor.y() + cr.height() + 4);
    }
    if (target.x() + popupW > avail.right())
        target.setX(avail.right() - popupW - 4);
    if (target.x() < avail.left() + 4)
        target.setX(avail.left() + 4);

    m_popup->move(target);
}

void AnimaPalette::refilter()
{
    const QString filter = currentFilterString().toLower();
    m_filtered.clear();
    for (int i = 0; i < m_templates.size(); ++i) {
        if (filter.isEmpty() || m_templates[i].title.toLower().contains(filter))
            m_filtered.append(i);
    }

    m_list->clear();
    for (int idx : m_filtered)
        m_list->addItem(m_templates[idx].title);

    if (m_list->count() > 0) {
        m_list->setCurrentRow(0);
    } else {
        m_hint->setText(QStringLiteral("no matches"));
    }
}

void AnimaPalette::onSelectionHintUpdate()
{
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_filtered.size()) {
        m_hint->clear();
        return;
    }
    const Template &t = m_templates[m_filtered[row]];
    m_hint->setText(t.exampleFill.isEmpty() ? QStringLiteral("(no example)") : t.exampleFill);
}

void AnimaPalette::moveSelection(int delta)
{
    const int n = m_list->count();
    if (n == 0) return;
    int row = m_list->currentRow() + delta;
    if (row < 0) row = 0;
    if (row >= n) row = n - 1;
    m_list->setCurrentRow(row);
}

// ─────────────────────────────────────────────────────────────────────────────
// Template insertion + placeholder navigation
// ─────────────────────────────────────────────────────────────────────────────

void AnimaPalette::insertSelectedTemplate()
{
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_filtered.size()) { hidePopup(); return; }
    const Template t = m_templates[m_filtered[row]];

    const int trigger = m_triggerPos;
    const int cursorPos = m_input->textCursor().position();

    hidePopup();

    m_insertingTemplate = true;
    QTextCursor c = m_input->textCursor();
    c.beginEditBlock();
    c.setPosition(trigger);
    c.setPosition(cursorPos, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    const int insertPos = c.position();
    c.insertText(t.body);
    c.endEditBlock();
    // Position cursor at the start of the inserted block, so advancePlaceholder
    // finds the first {PLACEHOLDER} in it.
    c.setPosition(insertPos);
    m_input->setTextCursor(c);
    m_insertingTemplate = false;

    advancePlaceholder();
}

bool AnimaPalette::advancePlaceholder()
{
    QTextCursor c = m_input->textCursor();
    const int startPos = c.selectionEnd();
    const QString text = m_input->toPlainText();

    const auto match = placeholderRegex().match(text, startPos);
    if (!match.hasMatch()) return false;

    QTextCursor sel(m_input->document());
    sel.setPosition(match.capturedStart());
    sel.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
    m_input->setTextCursor(sel);
    return true;
}

bool AnimaPalette::retreatPlaceholder()
{
    QTextCursor c = m_input->textCursor();
    const int cutoff = c.selectionStart();
    const QString text = m_input->toPlainText();

    auto it = placeholderRegex().globalMatch(text);
    int bestStart = -1, bestEnd = -1;
    while (it.hasNext()) {
        const auto m = it.next();
        if (m.capturedEnd() > cutoff) break;
        bestStart = m.capturedStart();
        bestEnd   = m.capturedEnd();
    }
    if (bestStart < 0) return false;

    QTextCursor sel(m_input->document());
    sel.setPosition(bestStart);
    sel.setPosition(bestEnd, QTextCursor::KeepAnchor);
    m_input->setTextCursor(sel);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Event  filter
// ─────────────────────────────────────────────────────────────────────────────

bool AnimaPalette::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_input) return QObject::eventFilter(watched, event);
    if (event->type() != QEvent::KeyPress) return QObject::eventFilter(watched, event);

    auto *ke = static_cast<QKeyEvent*>(event);
    const int key = ke->key();

    if (m_popupVisible) {
        switch (key) {
        case Qt::Key_Escape:
            hidePopup();
            return true;
        case Qt::Key_Up:
            moveSelection(-1);
            return true;
        case Qt::Key_Down:
            moveSelection(+1);
            return true;
        case Qt::Key_PageUp:
            moveSelection(-5);
            return true;
        case Qt::Key_PageDown:
            moveSelection(+5);
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Tab:
            insertSelectedTemplate();
            return true;
        case Qt::Key_Backspace: {
            // If cursor is right after the trigger '/', closing is cleanest.
            QTextCursor c = m_input->textCursor();
            if (c.position() <= m_triggerPos + 1)
                hidePopup();
            return false; // let the backspace actually delete a char
        }
        default:
            return false; // let the key edit the text; onTextChanged refilters
        }
    }

    // Popup not visible: Ctrl+Right / Ctrl+Left navigate placeholders in
    // already-inserted template text. If no slot is found in that direction,
    // fall through so QPlainTextEdit's built-in word-jump still works.
    if (ke->modifiers() == Qt::ControlModifier) {
        if (key == Qt::Key_Right && advancePlaceholder()) return true;
        if (key == Qt::Key_Left  && retreatPlaceholder()) return true;
    }
    return QObject::eventFilter(watched, event);
}
