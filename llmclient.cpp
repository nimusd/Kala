#include "llmclient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QUrl>

// NOTE (Windows / MinGW): HTTPS calls to api.anthropic.com, api.openai.com,
// api.groq.com etc. require OpenSSL DLLs in the build output directory:
//   libssl-3-x64.dll, libcrypto-3-x64.dll
// Ollama on localhost uses plain HTTP and does NOT need OpenSSL.

LLMClient::LLMClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void LLMClient::setConfig(const LLMConfig &config)
{
    m_config = config;
}

// ─────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────

void LLMClient::sendChatRequest(const QJsonArray &messages,
                                 const QJsonArray &tools,
                                 ResponseCallback callback)
{
    const bool isAnthropic = (m_config.provider == LLMProvider::Anthropic);

    QJsonObject body = buildOpenAIRequest(messages, tools);
    if (isAnthropic)
        body = adaptRequestForAnthropic(body);

    const QString url = isAnthropic
        ? QStringLiteral("https://api.anthropic.com/v1/messages")
        : m_config.baseUrl + QStringLiteral("/chat/completions");

    QUrl qurl(url);
    QNetworkRequest req(qurl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    if (isAnthropic) {
        req.setRawHeader("x-api-key",         m_config.apiKey.toUtf8());
        req.setRawHeader("anthropic-version", "2023-06-01");
    } else {
        req.setRawHeader("Authorization",
                         ("Bearer " + m_config.apiKey).toUtf8());
    }

    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_nam->post(req, payload);
    m_currentReply = reply;

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, isAnthropic, callback]() {
                m_currentReply = nullptr;
                handleReply(reply, isAnthropic, callback);
            });
}

// ─────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────

QJsonObject LLMClient::buildOpenAIRequest(const QJsonArray &messages,
                                           const QJsonArray &tools) const
{
    QJsonObject body;
    body["model"]       = m_config.model;
    body["max_tokens"]  = m_config.maxTokens;
    body["temperature"] = m_config.temperature;
    body["messages"]    = messages;
    if (!tools.isEmpty()) {
        body["tools"]        = tools;
        body["tool_choice"]  = QStringLiteral("auto");
    }
    return body;
}

void LLMClient::abort()
{
    if (m_currentReply)
        m_currentReply->abort();
}

void LLMClient::handleReply(QNetworkReply *reply, bool isAnthropic,
                              ResponseCallback callback)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        callback({}, reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        callback({}, "JSON parse error: " + parseError.errorString());
        return;
    }

    QJsonObject response = doc.object();

    // Both APIs return an "error" object on failure (auth, rate limit, etc.)
    if (response.contains("error")) {
        const QJsonValue errVal = response["error"];
        QString msg;
        if (errVal.isObject())
            msg = errVal.toObject()["message"].toString();
        if (msg.isEmpty())
            msg = "API error (see response body)";
        callback({}, msg);
        return;
    }

    if (isAnthropic)
        response = adaptResponseFromAnthropic(response);

    callback(response, {});
}

// ─────────────────────────────────────────────────────────────
// Anthropic adapters
// ─────────────────────────────────────────────────────────────

QJsonObject LLMClient::adaptRequestForAnthropic(const QJsonObject &openAiRequest) const
{
    QJsonObject body;
    body["model"]      = m_config.model;
    body["max_tokens"] = m_config.maxTokens;
    // Note: temperature intentionally omitted — Anthropic allows it but
    // defaults are fine; add back here if needed.

    // Split system message out of the messages array (Anthropic wants it top-level)
    QJsonArray inMessages  = openAiRequest["messages"].toArray();
    QJsonArray outMessages;
    QString    systemText;

    for (const QJsonValue &val : inMessages) {
        QJsonObject msg = val.toObject();
        if (msg["role"].toString() == "system") {
            systemText += msg["content"].toString();
        } else {
            outMessages.append(msg);
        }
    }

    if (!systemText.isEmpty())
        body["system"] = systemText;
    body["messages"] = outMessages;

    // Convert tool schemas: OpenAI uses "parameters", Anthropic uses "input_schema"
    // The JSON Schema structure inside is identical — only the key name differs.
    QJsonArray inTools  = openAiRequest["tools"].toArray();
    QJsonArray outTools;
    for (const QJsonValue &toolVal : inTools) {
        QJsonObject fn = toolVal.toObject()["function"].toObject();
        QJsonObject t;
        t["name"]         = fn["name"];
        t["description"]  = fn["description"];
        t["input_schema"] = fn["parameters"];
        outTools.append(t);
    }
    if (!outTools.isEmpty())
        body["tools"] = outTools;

    return body;
}

QJsonObject LLMClient::adaptResponseFromAnthropic(const QJsonObject &anthropicResponse) const
{
    // Anthropic response shape:
    //   { content: [ {type:"text", text:"..."}, {type:"tool_use", id, name, input:{}} ],
    //     stop_reason: "end_turn" | "tool_use" }
    //
    // OpenAI response shape we produce:
    //   { choices: [ { message: { role:"assistant", content, tool_calls:[] },
    //                  finish_reason: "stop" | "tool_calls" } ] }

    QJsonArray content    = anthropicResponse["content"].toArray();
    QString    stopReason = anthropicResponse["stop_reason"].toString();

    QString    textContent;
    QJsonArray toolCalls;

    for (const QJsonValue &val : content) {
        QJsonObject block = val.toObject();
        const QString type = block["type"].toString();

        if (type == "text") {
            textContent += block["text"].toString();

        } else if (type == "tool_use") {
            // Anthropic "input" is a JSON object; OpenAI "arguments" is a JSON string.
            QJsonObject fn;
            fn["name"] = block["name"];
            fn["arguments"] = QString::fromUtf8(
                QJsonDocument(block["input"].toObject())
                    .toJson(QJsonDocument::Compact));

            QJsonObject toolCall;
            toolCall["id"]       = block["id"];
            toolCall["type"]     = QStringLiteral("function");
            toolCall["function"] = fn;
            toolCalls.append(toolCall);
        }
    }

    QJsonObject message;
    message["role"]    = QStringLiteral("assistant");
    message["content"] = textContent;
    if (!toolCalls.isEmpty())
        message["tool_calls"] = toolCalls;

    QJsonObject choice;
    choice["message"]       = message;
    choice["finish_reason"] = (stopReason == "tool_use")
                                  ? QStringLiteral("tool_calls")
                                  : QStringLiteral("stop");

    QJsonObject result;
    result["choices"] = QJsonArray{ choice };
    return result;
}
