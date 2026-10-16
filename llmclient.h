#pragma once
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <functional>
#include "llmconfig.h"

class QNetworkAccessManager;
class QNetworkReply;

// LLMClient: stateless HTTP client for LLM API calls.
// Always speaks OpenAI format externally — the Anthropic adapter is internal.
// Thread model: everything runs on the main Qt thread (QNetworkReply::finished
// fires on the main thread; callers must not call from worker threads).
class LLMClient : public QObject
{
    Q_OBJECT

public:
    explicit LLMClient(QObject *parent = nullptr);

    void setConfig(const LLMConfig &config);
    const LLMConfig &config() const { return m_config; }

    // Send a chat request. messages and tools are in OpenAI format.
    // callback(response, errorString): response is OpenAI format; if errorString
    // is non-empty the request failed and response may be empty.
    using ResponseCallback = std::function<void(QJsonObject, QString)>;
    void sendChatRequest(const QJsonArray &messages,
                         const QJsonArray &tools,
                         ResponseCallback callback);

private:
    QJsonObject buildOpenAIRequest(const QJsonArray &messages,
                                   const QJsonArray &tools) const;

    // True when the configured model is a Qwen — the only family that honours
    // the `/no_think` soft switch. Other models get no tag (it would just be
    // prompt noise), and Ollama's native API uses `think: false` instead.
    bool isQwenModel() const;

    // True when the configured model is a DeepSeek. Their thinking mode is ON
    // by default; `reasoning_effort: "none"` turns it off, and sending that key
    // to any other provider is an error — hence the model check.
    bool isDeepSeekModel() const;

    // Append `/no_think` to the last user message so Qwen3 skips reasoning.
    QJsonArray  withNoThinkTag(const QJsonArray &messages) const;

    // Anthropic adapters — transform request/response so the rest of the
    // codebase stays completely unaware of the Anthropic wire format.
    QJsonObject adaptRequestForAnthropic(const QJsonObject &openAiRequest) const;
    QJsonObject adaptResponseFromAnthropic(const QJsonObject &anthropicResponse) const;

    // Ollama native-API adapters — uses /api/chat with `think: false` so
    // reasoning models (Qwen3) skip the <think> phase. Ollama's native
    // response wraps the reply in `message` instead of `choices[0]`, and
    // tool_call arguments are an object instead of a stringified JSON.
    QJsonObject adaptRequestForOllama(const QJsonArray &messages,
                                      const QJsonArray &tools) const;
    QJsonObject adaptResponseFromOllama(const QJsonObject &ollamaResponse) const;
    QString     ollamaChatUrl() const;

    // Text-format tool call parser — local models (Ollama, etc.) emit
    // <function=name><parameter=p>v</parameter></function> when they can't
    // produce structured JSON. This normalizes them to the standard tool_calls format.
    QJsonObject normalizeTextToolCalls(const QJsonObject &raw) const;

    void handleReply(QNetworkReply *reply, LLMProvider provider,
                     ResponseCallback callback);

    LLMConfig               m_config;
    QNetworkAccessManager  *m_nam;
    QNetworkReply          *m_currentReply = nullptr;

public:
    // Abort the in-flight request (if any). The pending callback will still fire
    // but KalaAgent checks m_aborted before acting on it.
    void abort();
};
