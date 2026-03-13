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
        "  background: #1e1e2e;"
        "  color: #cdd6f4;"
        "  border: none;"
        "  padding: 6px;"
        "}"
    );

    // Welcome message
    m_log->setHtml(
        "<p style='color:#6c7086; font-style:italic;'>"
        "Kala Companion is ready. Ask a question or describe the sound you want to build."
        "</p>"
    );

    // ── Status label ("Thinking…") ────────────────────────────────────────────
    m_statusLabel = new QLabel(QString("Thinking") + QChar(0x2026), this);
    m_statusLabel->setStyleSheet(
        "QLabel { color: #89b4fa; font-style: italic; font-size: 9pt; padding: 2px 6px; }"
    );
    m_statusLabel->setVisible(false);

    m_stopBtn = new QPushButton("Stop", this);
    m_stopBtn->setFixedWidth(48);
    m_stopBtn->setFixedHeight(22);
    m_stopBtn->setStyleSheet(
        "QPushButton {"
        "  background: #f38ba8;"
        "  color: #1e1e2e;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "  font-size: 8pt;"
        "}"
        "QPushButton:hover   { background: #eba0ac; }"
        "QPushButton:pressed { background: #e6647d; }"
    );
    m_stopBtn->setVisible(false);
    connect(m_stopBtn, &QPushButton::clicked, this, &CompanionPanel::cancelRequested);

    // ── Input area ────────────────────────────────────────────────────────────
    m_input = new QPlainTextEdit(this);
    m_input->setMaximumHeight(72);
    m_input->setPlaceholderText("Ask something or describe a sound…");
    m_input->setFont(QFont("Segoe UI", 9));
    m_input->setStyleSheet(
        "QPlainTextEdit {"
        "  background: #313244;"
        "  color: #cdd6f4;"
        "  border: 1px solid #45475a;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
        "QPlainTextEdit:focus { border-color: #89b4fa; }"
    );
    m_input->installEventFilter(this);  // Intercept Enter key

    m_sendBtn = new QPushButton("Send", this);
    m_sendBtn->setFixedWidth(56);
    m_sendBtn->setFixedHeight(28);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "  background: #89b4fa;"
        "  color: #1e1e2e;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover   { background: #b4befe; }"
        "QPushButton:pressed { background: #74c7ec; }"
        "QPushButton:disabled { background: #45475a; color: #6c7086; }"
    );
    connect(m_sendBtn, &QPushButton::clicked, this, &CompanionPanel::onSendClicked);

    m_clearBtn = new QPushButton("Clear", this);
    m_clearBtn->setFixedWidth(48);
    m_clearBtn->setFixedHeight(28);
    m_clearBtn->setToolTip("Clear conversation history");
    m_clearBtn->setStyleSheet(
        "QPushButton {"
        "  background: #313244;"
        "  color: #6c7086;"
        "  border: 1px solid #45475a;"
        "  border-radius: 4px;"
        "  font-size: 8pt;"
        "}"
        "QPushButton:hover   { background: #45475a; color: #cdd6f4; }"
        "QPushButton:pressed { background: #585b70; }"
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

    auto *statusRow = new QHBoxLayout;
    statusRow->setContentsMargins(0, 0, 0, 0);
    statusRow->setSpacing(6);
    statusRow->addWidget(m_statusLabel, 1);
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
                        "background:#181825; color:#cba6f7;"
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
            html += QString("<p style='margin:8px 0 2px 0; color:#89b4fa;"
                            "font-weight:bold; font-size:9.5pt;'>%1</p>")
                        .arg(line.mid(4).toHtmlEscaped());
            continue;
        }
        if (line.startsWith("## ")) {
            closeList();
            html += QString("<p style='margin:10px 0 2px 0; color:#cba6f7;"
                            "font-weight:bold; font-size:10pt;'>%1</p>")
                        .arg(line.mid(3).toHtmlEscaped());
            continue;
        }
        if (line.startsWith("# ")) {
            closeList();
            html += QString("<p style='margin:10px 0 4px 0; color:#f5c2e7;"
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
                         "<code style='background:#313244; color:#a6e3a1;"
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
                     "<code style='background:#313244; color:#a6e3a1;"
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
            "<span style='color:#a6e3a1; font-weight:bold;'>You</span>"
            "<span style='color:#6c7086; font-size:8pt;'> %1</span>"
            "</p>"
            "<p style='margin:0 0 8px 20px; color:#cdd6f4;'>%2</p>"
        ).arg(QDateTime::currentDateTime().toString("hh:mm"),
              text.toHtmlEscaped().replace("\n", "<br>"));

    } else if (role == "assistant") {
        html = QString(
            "<p style='margin:6px 0 4px 0;'>"
            "<span style='color:#89b4fa; font-weight:bold;'>Kala</span>"
            "</p>"
            "<div style='margin:0 0 8px 12px; color:#cdd6f4;'>%1</div>"
        ).arg(markdownToHtml(text));

    } else if (role == "tool_info") {
        // Compact single-line: tool activity feedback
        QString color = text.startsWith(QChar(0x26a0)) ? "#fab387" : "#a6e3a1";
        html = QString(
            "<p style='margin:1px 0 1px 20px; color:%1; font-size:8pt;'>%2</p>"
        ).arg(color, text.toHtmlEscaped());

    } else { // "error"
        html = QString(
            "<p style='margin:6px 0 8px 0; color:#f38ba8;'>%1</p>"
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
    return QWidget::eventFilter(watched, event);
}
