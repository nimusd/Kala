#pragma once
#include <QWidget>

class QTextBrowser;
class QPlainTextEdit;
class QPushButton;
class QLabel;

// CompanionPanel: the chat UI for the AI companion.
//
// Intentionally knows nothing about KalaAgent or LLMClient.
// KalaMain wires the signals/slots between this panel and KalaAgent.
//
// Layout:
//   ┌──────────────────────────────┐
//   │  chat log (QTextBrowser)     │  ← scrollable, HTML-formatted
//   ├──────────────────────────────┤
//   │  "Thinking..."  (status)     │  ← hidden when idle
//   ├──────────────────────────────┤
//   │  [text input]   [Send]       │  ← Enter=send, Shift+Enter=newline
//   └──────────────────────────────┘
class CompanionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CompanionPanel(QWidget *parent = nullptr);

public slots:
    // Append a message to the chat log.
    // role: "assistant" | "user" | "error" | "tool_info"
    void appendMessage(const QString &text, const QString &role);

    // Show/hide the "Thinking…" status label.
    void setThinking(bool thinking);

    // Clear the entire chat history from the display.
    void clearLog();

signals:
    // Emitted when the user presses Enter (or clicks Send) with non-empty input.
    void userMessageSubmitted(const QString &text);

    // Emitted when the user clicks the Stop button during generation.
    void cancelRequested();

    // Emitted when the user clicks Clear — KalaMain clears agent history.
    void clearRequested();

private slots:
    void onSendClicked();

private:
    // Install an event filter on the input box so we can intercept Enter/Shift+Enter.
    bool eventFilter(QObject *watched, QEvent *event) override;

    QTextBrowser  *m_log;
    QPlainTextEdit *m_input;
    QPushButton   *m_sendBtn;
    QPushButton   *m_stopBtn;
    QPushButton   *m_clearBtn;
    QLabel        *m_statusLabel;
};
