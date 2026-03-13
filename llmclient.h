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

    // Anthropic adapters — transform request/response so the rest of the
    // codebase stays completely unaware of the Anthropic wire format.
    QJsonObject adaptRequestForAnthropic(const QJsonObject &openAiRequest) const;
    QJsonObject adaptResponseFromAnthropic(const QJsonObject &anthropicResponse) const;

    void handleReply(QNetworkReply *reply, bool isAnthropic,
                     ResponseCallback callback);

    LLMConfig               m_config;
    QNetworkAccessManager  *m_nam;
    QNetworkReply          *m_currentReply = nullptr;

public:
    // Abort the in-flight request (if any). The pending callback will still fire
    // but KalaAgent checks m_aborted before acting on it.
    void abort();
};
