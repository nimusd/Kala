#pragma once
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include "llmclient.h"
#include "llmconfig.h"

class KalaTools;

// KalaAgent: orchestrates the LLM conversation loop.
//
// Responsibilities:
//   - Maintains conversation history (OpenAI-format messages array)
//   - Builds the API request with system prompt + history + tool schemas
//   - Handles multi-turn tool-call loops (LLM calls a tool → we execute it →
//     feed result back → LLM continues) with a round cap to prevent loops
//   - Emits signals so the UI can show responses and thinking state
//
// Threading: all methods must be called on the main (GUI) thread.
class KalaAgent : public QObject
{
    Q_OBJECT

public:
    explicit KalaAgent(const LLMConfig &config,
                       KalaTools       *tools,
                       QObject         *parent = nullptr);

    void setConfig(const LLMConfig &config);
    void clearHistory();   // Start a fresh conversation

    // Export the full session trace as a JSON object for fine-tuning data.
    // Includes system prompt, messages, model info, timestamp, and rating.
    QJsonObject exportSession(const QString &rating) const;

public slots:
    void sendUserMessage(const QString &text);

    // Abort the current generation mid-flight. Safe to call at any time.
    void cancel();

signals:
    // Emitted for each complete message ready to display.
    // role: "assistant" | "error" | "tool_info"
    void messageReady(const QString &text, const QString &role);

    void thinkingStarted();   // Show spinner / "thinking..." in UI
    void thinkingFinished();  // Hide spinner

    // Emitted whenever the history size changes — for the message count indicator.
    void historyCountChanged(int messageCount);

private:
    // Build the full messages array (system message prepended) for the API request
    QJsonArray buildRequestMessages() const;

    // Trim old tool-call blocks from history before sending to API.
    // Keeps all non-tool-block messages and only the LAST complete tool block.
    QJsonArray trimForRequest(const QJsonArray &msgs) const;

    // Send current conversation to the LLM; roundsLeft caps the tool-call loop
    void continueConversation(int roundsLeft);

    // Handle a completed API response
    void onResponseReceived(const QJsonObject &response,
                            const QString     &errorStr,
                            int                roundsLeft);

    // Execute all tool calls in an assistant message and append results to history
    void executeToolCalls(const QJsonObject &assistantMessage, int roundsLeft);

    // The expert system prompt — encodes Kala's synthesis model and design rules.
    // Defined as a static function so it can be updated independently.
    static QString buildSystemPrompt();

    // Session persistence — survives companion resets within one app session,
    // deleted on app close (or manual Clear).
    void saveHistory();
    void loadHistory();
    static QString sessionFilePath();

    static constexpr int kMaxToolRounds = 250;

    LLMClient  *m_client;
    LLMConfig   m_config;
    KalaTools  *m_tools;
    QJsonArray  m_messages;  // Conversation history (system message NOT stored here)
    bool        m_aborted = false;
};
