#include "llmclient.h"

#include <QRegularExpression>
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
    const LLMProvider prov = m_config.provider;

    QJsonObject body;
    QString     url;

    if (prov == LLMProvider::Anthropic) {
        body = adaptRequestForAnthropic(buildOpenAIRequest(messages, tools));
        url  = QStringLiteral("https://api.anthropic.com/v1/messages");
    } else if (prov == LLMProvider::Ollama) {
        body = adaptRequestForOllama(messages, tools);
        url  = ollamaChatUrl();
    } else {
        body = buildOpenAIRequest(messages, tools);
        url  = m_config.baseUrl + QStringLiteral("/chat/completions");
    }

    QUrl qurl(url);
    QNetworkRequest req(qurl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    if (prov == LLMProvider::Anthropic) {
        req.setRawHeader("x-api-key",         m_config.apiKey.toUtf8());
        req.setRawHeader("anthropic-version", "2023-06-01");
    } else if (prov == LLMProvider::OpenAICompatible) {
        req.setRawHeader("Authorization",
                         ("Bearer " + m_config.apiKey).toUtf8());
    }
    // Ollama local: no auth header needed.

    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_nam->post(req, payload);
    m_currentReply = reply;

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, prov, callback]() {
                m_currentReply = nullptr;
                handleReply(reply, prov, callback);
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
    body["messages"]    = withNoThinkTag(messages);
    if (!tools.isEmpty()) {
        body["tools"]        = tools;
        body["tool_choice"]  = QStringLiteral("auto");
    }
    return body;
}

// Qwen3 honours the `/no_think` soft switch at the end of the latest user
// turn (system-prompt placement is inconsistent across builds, and the
// Ollama /v1 endpoint doesn't forward the native `think: false` param).
// Appending it here keeps reasoning disabled for all outbound requests.
QJsonArray LLMClient::withNoThinkTag(const QJsonArray &messages) const
{
    QJsonArray out = messages;
    for (int i = out.size() - 1; i >= 0; --i) {
        QJsonObject msg = out[i].toObject();
        if (msg["role"].toString() != "user") continue;

        QString content = msg["content"].toString();
        if (!content.trimmed().endsWith(QStringLiteral("/no_think"))) {
            if (!content.isEmpty() && !content.endsWith('\n'))
                content += '\n';
            content += QStringLiteral("/no_think");
            msg["content"] = content;
            out[i] = msg;
        }
        break;
    }
    return out;
}

void LLMClient::abort()
{
    if (m_currentReply)
        m_currentReply->abort();
}

void LLMClient::handleReply(QNetworkReply *reply, LLMProvider provider,
                              ResponseCallback callback)
{
    reply->deleteLater();

    QByteArray data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        // Try to extract a meaningful message from the response body first
        QJsonParseError pe;
        QJsonDocument doc = QJsonDocument::fromJson(data, &pe);
        if (pe.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonValue errVal = doc.object()["error"];
            QString msg;
            if (errVal.isObject())
                msg = errVal.toObject()["message"].toString();
            // Ollama surfaces errors as a top-level "error" string, not an object.
            if (msg.isEmpty() && errVal.isString())
                msg = errVal.toString();
            if (!msg.isEmpty()) {
                callback({}, msg);
                return;
            }
        }
        // Fall back to the Qt network error string
        callback({}, reply->errorString());
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        callback({}, "JSON parse error: " + parseError.errorString());
        return;
    }

    QJsonObject response = doc.object();

    if (response.contains("error")) {
        const QJsonValue errVal = response["error"];
        QString msg;
        if (errVal.isObject())
            msg = errVal.toObject()["message"].toString();
        else if (errVal.isString())
            msg = errVal.toString();
        if (msg.isEmpty())
            msg = "API error (see response body)";
        callback({}, msg);
        return;
    }

    switch (provider) {
    case LLMProvider::Anthropic:
        callback(adaptResponseFromAnthropic(response), {});
        return;
    case LLMProvider::Ollama:
        callback(normalizeTextToolCalls(adaptResponseFromOllama(response)), {});
        return;
    case LLMProvider::OpenAICompatible:
    default:
        callback(normalizeTextToolCalls(response), {});
        return;
    }
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

QJsonObject LLMClient::normalizeTextToolCalls(const QJsonObject &raw) const
{
    QJsonArray choices = raw["choices"].toArray();
    if (choices.isEmpty()) return raw;

    QJsonObject choice = choices[0].toObject();
    QJsonObject message = choice["message"].toObject();
    if (message.isEmpty()) return raw;

    if (choice["finish_reason"].toString() == "tool_calls") return raw;

    QString content = message["content"].toString();
    if (content.isEmpty()) return raw;

    // Check for tool call markup
    if (!content.contains("<function=", Qt::CaseInsensitive) &&
        !content.contains("<tool_call", Qt::CaseInsensitive))
        return raw;

    // Strip the markup from content using simple string operations
    QString cleaned = content;
    cleaned.remove(QRegularExpression("<tool_calls?>.*?</tool_calls?>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption));
    cleaned.remove(QRegularExpression("<function[= ][^>]*>.*?</function>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption));
    cleaned = cleaned.trimmed();
    message["content"] = cleaned.isEmpty() ? QJsonValue() : QJsonValue(cleaned);

    // Parse text-format tool calls
    QJsonArray toolCalls;
    int callId = 1;
    int pos = 0;

    QRegularExpression nameAttrRe("name\\s*=\\s*[\"']([^\"']+)[\"']", QRegularExpression::CaseInsensitiveOption);

    while (pos < content.size()) {
        // Find <function= or <function name=
        int funcStart = content.indexOf("<function=", pos, Qt::CaseInsensitive);
        int funcStart2 = content.indexOf("<function ", pos, Qt::CaseInsensitive);

        if (funcStart < 0 && funcStart2 < 0) break;

        int tagOpen;
        QString funcName;

        if (funcStart >= 0 && (funcStart2 < 0 || funcStart <= funcStart2)) {
            // <function=NAME> format
            tagOpen = content.indexOf('>', funcStart);
            if (tagOpen < 0) break;
            funcName = content.mid(funcStart + 10, tagOpen - funcStart - 10).trimmed();
        } else {
            // <function name="..."> format
            tagOpen = content.indexOf('>', funcStart2);
            if (tagOpen < 0) break;
            QString tagBody = content.mid(funcStart2 + 10, tagOpen - funcStart2 - 10);
            QRegularExpressionMatch match = nameAttrRe.match(tagBody);
            if (!match.hasMatch()) {
                pos = funcStart2 + 1;
                continue;
            }
            funcName = match.captured(1);
        }

        int funcClose = content.indexOf("</function>", tagOpen, Qt::CaseInsensitive);
        if (funcClose < 0) break;

        QString inner = content.mid(tagOpen + 1, funcClose - tagOpen - 1);

        // Parse parameters
        QJsonObject args;
        int pPos = 0;
        while (pPos < inner.size()) {
            int paramStart = inner.indexOf("<parameter", pPos, Qt::CaseInsensitive);
            if (paramStart < 0) break;

            int tagClose = inner.indexOf('>', paramStart);
            if (tagClose < 0) break;

            QString tagBody = inner.mid(paramStart + 10, tagClose - paramStart - 10);
            QString paramName;

            if (tagBody.trimmed().startsWith("=")) {
                paramName = tagBody.trimmed().mid(1).trimmed();
            } else {
                QRegularExpressionMatch match = nameAttrRe.match(tagBody);
                if (!match.hasMatch()) {
                    pPos = tagClose + 1;
                    continue;
                }
                paramName = match.captured(1);
            }

            int paramClose = inner.indexOf("</parameter>", tagClose, Qt::CaseInsensitive);
            if (paramClose < 0) break;

            QString value = inner.mid(tagClose + 1, paramClose - tagClose - 1).trimmed();

            // Try to parse as JSON, fall back to string
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(value.toUtf8(), &err);
            args[paramName] = (err.error == QJsonParseError::NoError) ? doc.object() : QJsonValue(value);

            pPos = paramClose + 12;
        }

        QJsonObject toolCall;
        toolCall["id"] = QString("call_%1").arg(callId++);
        toolCall["type"] = "function";
        QJsonObject fn;
        fn["name"] = funcName;
        fn["arguments"] = QString::fromUtf8(QJsonDocument(args).toJson(QJsonDocument::Compact));
        toolCall["function"] = fn;
        toolCalls.append(toolCall);

        pos = funcClose + 11;
    }

    if (!toolCalls.isEmpty()) {
        message["tool_calls"] = toolCalls;
        choice["finish_reason"] = "tool_calls";
    }

    return raw;
}

// ─────────────────────────────────────────────────────────────
// Ollama native adapters
// ─────────────────────────────────────────────────────────────

// Convert a baseUrl like "http://localhost:11434/v1" (legacy config) or
// "http://localhost:11434" into the Ollama native chat endpoint.
QString LLMClient::ollamaChatUrl() const
{
    QString base = m_config.baseUrl;
    while (base.endsWith('/')) base.chop(1);
    if (base.endsWith(QStringLiteral("/v1")))
        base.chop(3);
    return base + QStringLiteral("/api/chat");
}

// Ollama's /api/chat accepts messages and tools in OpenAI shape, but
// reasoning models skip the <think> phase only if `think: false` is set
// at the top level of the request. Also moves temperature / num_predict
// into the `options` sub-object where Ollama expects them.
QJsonObject LLMClient::adaptRequestForOllama(const QJsonArray &messages,
                                              const QJsonArray &tools) const
{
    QJsonObject body;
    body["model"]    = m_config.model;
    body["messages"] = messages;
    body["stream"]   = false;
    body["think"]    = false;

    QJsonObject options;
    options["temperature"] = m_config.temperature;
    options["num_predict"] = m_config.maxTokens;
    body["options"] = options;

    if (!tools.isEmpty())
        body["tools"] = tools;

    return body;
}

// Ollama responds in the form
//   { "message": { "role":"assistant", "content":"...", "tool_calls":[
//        { "function": { "name":"...", "arguments": { ...object... } } } ] },
//     "done_reason": "stop", "done": true }
// The rest of the codebase expects OpenAI shape, so wrap the message in
// choices[0], synthesize an id for each tool call, stringify arguments,
// and surface a plausible finish_reason.
QJsonObject LLMClient::adaptResponseFromOllama(const QJsonObject &ollamaResponse) const
{
    QJsonObject message = ollamaResponse["message"].toObject();
    const QJsonArray rawCalls = message["tool_calls"].toArray();

    if (!rawCalls.isEmpty()) {
        QJsonArray outCalls;
        int idx = 0;
        for (const QJsonValue &cv : rawCalls) {
            QJsonObject c = cv.toObject();
            QJsonObject fn = c["function"].toObject();

            // Ollama returns arguments as an object; KalaAgent expects a
            // JSON-encoded string (matches OpenAI's choice to forward the
            // model's raw token stream).
            const QJsonValue argsVal = fn["arguments"];
            QString argsStr;
            if (argsVal.isObject() || argsVal.isArray()) {
                argsStr = QString::fromUtf8(
                    QJsonDocument::fromVariant(argsVal.toVariant())
                        .toJson(QJsonDocument::Compact));
            } else if (argsVal.isString()) {
                argsStr = argsVal.toString();
            }
            fn["arguments"] = argsStr;

            QJsonObject adapted;
            adapted["id"]       = QStringLiteral("call_%1").arg(idx++);
            adapted["type"]     = QStringLiteral("function");
            adapted["function"] = fn;
            outCalls.append(adapted);
        }
        message["tool_calls"] = outCalls;
    }

    QJsonObject choice;
    choice["index"]         = 0;
    choice["message"]       = message;
    choice["finish_reason"] = rawCalls.isEmpty()
        ? QStringLiteral("stop")
        : QStringLiteral("tool_calls");

    QJsonObject wrapped;
    QJsonArray choices;
    choices.append(choice);
    wrapped["choices"] = choices;
    return wrapped;
}
