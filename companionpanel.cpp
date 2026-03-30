#include "companionpanel.h"

#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDateTime>
#include <QRegularExpression>

CompanionPanel::CompanionPanel(QWidget *parent)
    : QWidget(parent)
{
    // ── Chat log ─────────────────────────────────────────────────────────────
    m_log = new QTextBrowser(this);
    m_log->setReadOnly(true);
    m_log->setOpenExternalLinks(false);
    m_log->setFont(QFont("Segoe UI", 9));
    m_log->setStyleSheet(
        "QTextBrowser {"
        "  background: #ffffff;"
        "  color: #1a1a1a;"
        "  border: none;"
        "  padding: 6px;"
        "}"
    );

    // Welcome message
    m_log->setHtml(
        "<p style='color:#888888; font-style:italic;'>"
        "Anima is ready. Ask a question or describe the task you want to be done."
        "</p>"
    );

    // ── Status label ("Thinking…") ────────────────────────────────────────────
    m_statusLabel = new QLabel(QString("Thinking") + QChar(0x2026), this);
    m_statusLabel->setStyleSheet(
        "QLabel { color: #2563eb; font-style: italic; font-size: 9pt; padding: 2px 6px; }"
    );
    m_statusLabel->setVisible(false);

    m_stopBtn = new QPushButton("Stop", this);
    m_stopBtn->setFixedWidth(48);
    m_stopBtn->setFixedHeight(22);
    m_stopBtn->setStyleSheet(
        "QPushButton {"
        "  background: #dc2626;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "  font-size: 8pt;"
        "}"
        "QPushButton:hover   { background: #ef4444; }"
        "QPushButton:pressed { background: #b91c1c; }"
    );
    m_stopBtn->setVisible(false);
    connect(m_stopBtn, &QPushButton::clicked, this, &CompanionPanel::cancelRequested);

    // ── Input area ────────────────────────────────────────────────────────────
    m_input = new QPlainTextEdit(this);
    m_input->setMaximumHeight(72);
    m_input->setPlaceholderText("Ask something or describe a task…");
    m_input->setFont(QFont("Segoe UI", 9));
    m_input->setStyleSheet(
        "QPlainTextEdit {"
        "  background: #f5f5f5;"
        "  color: #1a1a1a;"
        "  border: 1px solid #cccccc;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
        "QPlainTextEdit:focus { border-color: #2563eb; }"
    );
    m_input->installEventFilter(this);  // Intercept Enter key

    m_sendBtn = new QPushButton("Send", this);
    m_sendBtn->setFixedWidth(56);
    m_sendBtn->setFixedHeight(28);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "  background: #2563eb;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover   { background: #3b82f6; }"
        "QPushButton:pressed { background: #1d4ed8; }"
        "QPushButton:disabled { background: #cccccc; color: #888888; }"
    );
    connect(m_sendBtn, &QPushButton::clicked, this, &CompanionPanel::onSendClicked);

    m_clearBtn = new QPushButton("Clear", this);
    m_clearBtn->setFixedWidth(48);
    m_clearBtn->setFixedHeight(28);
    m_clearBtn->setToolTip("Clear conversation history");
    m_clearBtn->setStyleSheet(
        "QPushButton {"
        "  background: #f5f5f5;"
        "  color: #888888;"
        "  border: 1px solid #cccccc;"
        "  border-radius: 4px;"
        "  font-size: 8pt;"
        "}"
        "QPushButton:hover   { background: #e5e5e5; color: #1a1a1a; }"
        "QPushButton:pressed { background: #d4d4d4; }"
    );
    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        clearLog();
        emit clearRequested();
    });

    auto *inputRow = new QHBoxLayout;
    inputRow->setContentsMargins(0, 0, 0, 0);
    inputRow->setSpacing(6);
    inputRow->addWidget(m_input, 1);
    inputRow->addWidget(m_clearBtn, 0, Qt::AlignBottom);
    inputRow->addWidget(m_sendBtn, 0, Qt::AlignBottom);

    m_historyLabel = new QLabel(this);
    m_historyLabel->setStyleSheet(
        "QLabel { color: #888888; font-size: 8pt; padding: 2px 4px; }"
    );
    m_historyLabel->setVisible(false);

    m_modeLabel = new QLabel(this);
    m_modeLabel->setStyleSheet(
        "QLabel { color: #0891b2; font-size: 8pt; padding: 2px 4px; }"
    );
    m_modeLabel->setToolTip("Current tool mode. Click to cycle: Sounit \342\206\222 Score \342\206\222 Full");
    m_modeLabel->setCursor(Qt::PointingHandCursor);
    m_modeLabel->setVisible(false);
    m_modeLabel->installEventFilter(this);

    auto *statusRow = new QHBoxLayout;
    statusRow->setContentsMargins(0, 0, 0, 0);
    statusRow->setSpacing(6);
    statusRow->addWidget(m_statusLabel, 1);
    statusRow->addWidget(m_historyLabel, 0, Qt::AlignVCenter);
    statusRow->addWidget(m_modeLabel, 0, Qt::AlignVCenter);
    statusRow->addWidget(m_stopBtn, 0, Qt::AlignVCenter);

    // ── Root layout ───────────────────────────────────────────────────────────
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(4, 4, 4, 4);
    root->setSpacing(4);
    root->addWidget(m_log, 1);
    root->addLayout(statusRow);
    root->addLayout(inputRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
// Markdown → HTML
// Handles: ### headings, **bold**, `inline code`, ``` code blocks,
//          - / * bullet lists, blank-line paragraphs.
// ─────────────────────────────────────────────────────────────────────────────

static QString markdownToHtml(const QString &md)
{
    QStringList lines = md.split('\n');
    QString html;
    bool inCode  = false;
    bool inList  = false;

    auto closeList = [&]() {
        if (inList) { html += "</ul>"; inList = false; }
    };

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];

        // ── Fenced code block ─────────────────────────────────────────────
        if (line.trimmed().startsWith("```")) {
            closeList();
            if (!inCode) {
                html += "<pre style='"
                        "background:#f3f4f6; color:#7c3aed;"
                        "padding:6px 10px; border-radius:4px;"
                        "font-family:Consolas,monospace; font-size:8.5pt;"
                        "margin:4px 0; white-space:pre-wrap;'>";
                inCode = true;
            } else {
                html += "</pre>";
                inCode = false;
            }
            continue;
        }
        if (inCode) {
            html += line.toHtmlEscaped() + "\n";
            continue;
        }

        // ── Headings: ###, ##, # ─────────────────────────────────────────
        if (line.startsWith("### ")) {
            closeList();
            html += QString("<p style='margin:8px 0 2px 0; color:#2563eb;"
                            "font-weight:bold; font-size:9.5pt;'>%1</p>")
                        .arg(line.mid(4).toHtmlEscaped());
            continue;
        }
        if (line.startsWith("## ")) {
            closeList();
            html += QString("<p style='margin:10px 0 2px 0; color:#7c3aed;"
                            "font-weight:bold; font-size:10pt;'>%1</p>")
                        .arg(line.mid(3).toHtmlEscaped());
            continue;
        }
        if (line.startsWith("# ")) {
            closeList();
            html += QString("<p style='margin:10px 0 4px 0; color:#be185d;"
                            "font-weight:bold; font-size:11pt;'>%1</p>")
                        .arg(line.mid(2).toHtmlEscaped());
            continue;
        }

        // ── Bullet list items: - or * ────────────────────────────────────
        if (line.startsWith("- ") || line.startsWith("* ")) {
            if (!inList) { html += "<ul style='margin:2px 0; padding-left:18px;'>"; inList = true; }
            QString item = line.mid(2);
            // inline formatting on the item text
            item = item.toHtmlEscaped();
            item.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
            item.replace(QRegularExpression("`([^`]+)`"),
                         "<code style='background:#f0f0f0; color:#16a34a;"
                         "padding:0 3px; border-radius:2px; font-size:8.5pt;'>\\1</code>");
            html += "<li style='margin:1px 0;'>" + item + "</li>";
            continue;
        }

        closeList();

        // ── Blank line → paragraph break ────────────────────────────────
        if (line.trimmed().isEmpty()) {
            html += "<br>";
            continue;
        }

        // ── Normal line: inline formatting ───────────────────────────────
        QString safe = line.toHtmlEscaped();
        // **bold**
        safe.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
        // *italic*
        safe.replace(QRegularExpression("\\*([^*]+)\\*"), "<i>\\1</i>");
        // `inline code`
        safe.replace(QRegularExpression("`([^`]+)`"),
                     "<code style='background:#f0f0f0; color:#16a34a;"
                     "padding:0 3px; border-radius:2px; font-size:8.5pt;'>\\1</code>");
        html += "<p style='margin:1px 0;'>" + safe + "</p>";
    }

    closeList();
    if (inCode) html += "</pre>";  // Unterminated code block safety

    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public slots
// ─────────────────────────────────────────────────────────────────────────────

void CompanionPanel::appendMessage(const QString &text, const QString &role)
{
    QString html;

    if (role == "user") {
        html = QString(
            "<p style='margin:6px 0 2px 0;'>"
            "<span style='color:#16a34a; font-weight:bold;'>You</span>"
            "<span style='color:#888888; font-size:8pt;'> %1</span>"
            "</p>"
            "<p style='margin:0 0 8px 20px; color:#1a1a1a;'>%2</p>"
        ).arg(QDateTime::currentDateTime().toString("hh:mm"),
              text.toHtmlEscaped().replace("\n", "<br>"));

    } else if (role == "assistant") {
        html = QString(
            "<p style='margin:6px 0 4px 0;'>"
            "<span style='color:#2563eb; font-weight:bold;'>Anima</span>"
            "</p>"
            "<div style='margin:0 0 8px 12px; color:#1a1a1a;'>%1</div>"
        ).arg(markdownToHtml(text));

    } else if (role == "tool_info") {
        // Compact single-line: tool activity feedback
        QString color = text.startsWith(QChar(0x26a0)) ? "#ea580c" : "#16a34a";
        html = QString(
            "<p style='margin:1px 0 1px 20px; color:%1; font-size:8pt;'>%2</p>"
        ).arg(color, text.toHtmlEscaped());

    } else { // "error"
        html = QString(
            "<p style='margin:6px 0 8px 0; color:#dc2626;'>%1</p>"
        ).arg(text.toHtmlEscaped().replace("\n", "<br>"));
    }

    m_log->append(html);

    // Auto-scroll to bottom
    QScrollBar *sb = m_log->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void CompanionPanel::setThinking(bool thinking)
{
    m_statusLabel->setVisible(thinking);
    m_stopBtn->setVisible(thinking);
    m_sendBtn->setEnabled(!thinking);
    m_input->setEnabled(!thinking);
}

void CompanionPanel::clearLog()
{
    m_log->clear();
}

void CompanionPanel::setMessageCount(int count)
{
    if (count <= 0) {
        m_historyLabel->setVisible(false);
    } else {
        m_historyLabel->setText(QString("%1 msgs").arg(count));
        m_historyLabel->setVisible(true);
    }
}

void CompanionPanel::setToolModeLabel(const QString &label)
{
    if (label.isEmpty()) {
        m_modeLabel->setVisible(false);
    } else {
        m_modeLabel->setText(label);
        m_modeLabel->setVisible(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Private slots
// ─────────────────────────────────────────────────────────────────────────────

void CompanionPanel::onSendClicked()
{
    const QString text = m_input->toPlainText().trimmed();
    if (text.isEmpty()) return;

    m_input->clear();
    appendMessage(text, "user");
    emit userMessageSubmitted(text);
}

// ─────────────────────────────────────────────────────────────────────────────
// Event filter — Enter sends, Shift+Enter inserts newline
// ─────────────────────────────────────────────────────────────────────────────

bool CompanionPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_input && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            if (ke->modifiers() & Qt::ShiftModifier) {
                // Shift+Enter: insert a real newline
                m_input->insertPlainText("\n");
            } else {
                // Plain Enter: send
                onSendClicked();
            }
            return true;  // Event consumed
        }
    }
    if (watched == m_modeLabel && event->type() == QEvent::MouseButtonPress) {
        emit toolModeToggleRequested();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}
