#include "kalaagent.h"
#include "kalatools.h"
#include "llmclient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

KalaAgent::KalaAgent(const LLMConfig &config,
                     KalaTools       *tools,
                     QObject         *parent)
    : QObject(parent)
    , m_client(new LLMClient(this))
    , m_tools(tools)
{
    m_client->setConfig(config);
    loadHistory();
}

void KalaAgent::setConfig(const LLMConfig &config)
{
    m_client->setConfig(config);
}

void KalaAgent::clearHistory()
{
    m_messages = QJsonArray{};
    QFile::remove(sessionFilePath());
    emit historyCountChanged(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────

void KalaAgent::sendUserMessage(const QString &text)
{
    m_aborted = false;
    m_messages.append(QJsonObject{
        {"role",    "user"},
        {"content", text}
    });
    saveHistory();
    emit thinkingStarted();
    continueConversation(kMaxToolRounds);
}

void KalaAgent::cancel()
{
    if (m_aborted) return;
    m_aborted = true;
    m_client->abort();
}

// ─────────────────────────────────────────────────────────────────────────────
// Conversation loop
// ─────────────────────────────────────────────────────────────────────────────

QJsonArray KalaAgent::trimForRequest(const QJsonArray &msgs) const
{
    // Identify "tool blocks": an assistant message with tool_calls followed by
    // one or more role:"tool" messages. We keep all non-tool-block messages
    // and only the LAST complete tool block.

    // First pass: find all tool block ranges [start, end) in the array
    struct Block { int start; int end; };
    QVector<Block> toolBlocks;

    int i = 0;
    while (i < msgs.size()) {
        const QJsonObject msg = msgs[i].toObject();
        const bool isToolCall = msg["role"].toString() == "assistant"
                                && !msg["tool_calls"].toArray().isEmpty();
        if (isToolCall) {
            int blockStart = i;
            ++i;
            // Consume following tool result messages
            while (i < msgs.size()
                   && msgs[i].toObject()["role"].toString() == "tool") {
                ++i;
            }
            toolBlocks.append({blockStart, i});
        } else {
            ++i;
        }
    }

    // If there are 0 or 1 tool blocks, nothing to trim
    if (toolBlocks.size() <= 1)
        return msgs;

    // Keep all messages that are NOT in an older (non-last) tool block
    const int lastBlockStart = toolBlocks.last().start;
    const int lastBlockEnd   = toolBlocks.last().end;

    // Build a set of indices to drop (all tool blocks except the last)
    QSet<int> drop;
    for (int b = 0; b < toolBlocks.size() - 1; ++b) {
        for (int j = toolBlocks[b].start; j < toolBlocks[b].end; ++j)
            drop.insert(j);
    }
    Q_UNUSED(lastBlockStart)
    Q_UNUSED(lastBlockEnd)

    QJsonArray result;
    for (int j = 0; j < msgs.size(); ++j) {
        if (!drop.contains(j))
            result.append(msgs[j]);
    }
    return result;
}

QJsonArray KalaAgent::buildRequestMessages() const
{
    QJsonArray msgs;
    msgs.append(QJsonObject{
        {"role",    "system"},
        {"content", buildSystemPrompt()}
    });
    QJsonArray trimmed = trimForRequest(m_messages);
    for (const QJsonValue &v : trimmed)
        msgs.append(v);
    return msgs;
}

void KalaAgent::continueConversation(int roundsLeft)
{
    m_client->sendChatRequest(
        buildRequestMessages(),
        m_tools->getToolSchemas(),
        [this, roundsLeft](const QJsonObject &response, const QString &errorStr) {
            onResponseReceived(response, errorStr, roundsLeft);
        }
    );
}

void KalaAgent::onResponseReceived(const QJsonObject &response,
                                    const QString     &errorStr,
                                    int                roundsLeft)
{
    if (m_aborted) {
        m_aborted = false;
        emit thinkingFinished();
        emit messageReady("*(Interrupted.)*", "assistant");
        return;
    }

    if (!errorStr.isEmpty()) {
        emit thinkingFinished();
        emit messageReady("Error: " + errorStr, "error");
        return;
    }

    const QJsonArray choices = response["choices"].toArray();
    if (choices.isEmpty()) {
        emit thinkingFinished();
        emit messageReady("Error: empty response from LLM.", "error");
        return;
    }

    const QJsonObject choice  = choices[0].toObject();
    const QJsonObject message = choice["message"].toObject();
    const QString finishReason = choice["finish_reason"].toString();

    if (finishReason == "tool_calls" && roundsLeft > 0) {
        // Append the assistant's tool-call message to history and execute
        m_messages.append(message);
        saveHistory();
        executeToolCalls(message, roundsLeft - 1);
    } else if (finishReason == "tool_calls") {
        // Rounds exhausted with pending tool calls.
        // Execute the tools so history ends with tool results (valid state),
        // but pass roundsLeft=0 so executeToolCalls won't recurse further.
        m_messages.append(message);
        saveHistory();
        emit messageReady("*(Reached tool call limit — executing final tools, then stopping. Say 'continue' to carry on.)*", "assistant");
        executeToolCalls(message, 0);
    } else {
        // Final response — extract text and surface it
        QString text = message["content"].toString().trimmed();
        m_messages.append(message);
        saveHistory();
        emit thinkingFinished();
        if (!text.isEmpty())
            emit messageReady(text, "assistant");
    }
}

void KalaAgent::executeToolCalls(const QJsonObject &assistantMessage, int roundsLeft)
{
    const QJsonArray toolCalls = assistantMessage["tool_calls"].toArray();

    for (const QJsonValue &tcVal : toolCalls) {
        const QJsonObject tc       = tcVal.toObject();
        const QString     callId   = tc["id"].toString();
        const QJsonObject fn       = tc["function"].toObject();
        const QString     name     = fn["name"].toString();

        // Parse the arguments JSON string into an object
        QJsonObject args;
        const QString argsStr = fn["arguments"].toString();
        if (!argsStr.isEmpty()) {
            QJsonParseError pe;
            const QJsonDocument argDoc = QJsonDocument::fromJson(argsStr.toUtf8(), &pe);
            if (pe.error == QJsonParseError::NoError && argDoc.isObject())
                args = argDoc.object();
        }

        // Execute the tool
        const QJsonObject result = m_tools->dispatchTool(name, args);

        // Notify UI with a brief tool activity summary (optional but useful)
        const QString toolSummary = result.contains("error")
            ? QString("⚠ %1: %2").arg(name, result["error"].toString())
            : QString("✓ %1").arg(name);
        emit messageReady(toolSummary, "tool_info");

        // Append tool result to history in OpenAI format
        const QString resultContent = QString::fromUtf8(
            QJsonDocument(result).toJson(QJsonDocument::Compact));

        m_messages.append(QJsonObject{
            {"role",         "tool"},
            {"tool_call_id", callId},
            {"content",      resultContent}
        });
        saveHistory();
    }

    // Continue the conversation with the tool results appended,
    // unless we just executed the final-round tools (limit reached).
    if (roundsLeft > 0) {
        continueConversation(roundsLeft);
    } else {
        emit thinkingFinished();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Session persistence
// ─────────────────────────────────────────────────────────────────────────────

QString KalaAgent::sessionFilePath()
{
    return QDir::tempPath() + "/kala_companion_session.json";
}

void KalaAgent::saveHistory()
{
    QFile f(sessionFilePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(m_messages).toJson(QJsonDocument::Compact));
    emit historyCountChanged(m_messages.size());
}

void KalaAgent::loadHistory()
{
    QFile f(sessionFilePath());
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    if (pe.error == QJsonParseError::NoError && doc.isArray())
        m_messages = doc.array();
    emit historyCountChanged(m_messages.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// System Prompt
// ─────────────────────────────────────────────────────────────────────────────

QString KalaAgent::buildSystemPrompt()
{
    return QStringLiteral(
R"(/no_think
You are Anima — AI assistant for Kala, a music composition app.
Kala uses pen-tablet input (no MIDI). Sound is built from "containers" in a graph called a "sounit".
Every graph must end with a signal-output container. Spectrum to Signal converts additive spectrum → audio.
Be concise. When building, narrate briefly as you work.

━━━ CONTAINER TYPES ━━━

ESSENTIAL (Blue):
Harmonic Generator — out: spectrum | in: purity, drift, digitWindowOffset | params: numHarmonics(64), dnaSelect(0–5,-1=custom), padEnabled, padBandwidth(cents), padBandwidthScale
Spectrum to Signal — in: spectrumIn, pitchMultiplier | out: signalOut | params: normalize(1), pitchMultiplier(1.0)
Signal Mixer — in: signalA, signalB | out: signalOut | params: gainA(1.0), gainB(1.0)
Note Tail — in: signalIn, length | out: signalOut | params: tailLength(ms, 1–9999)
Attack — out: signalOut | params: attackType(0=FluteChiff,1=ClarinetOnset,2=SaxHonk,3=BrassBuzz,4=PianoHammer,5=DrumHit)
Karplus Strong — in: signalIn, pitchMultiplier,… | out: signalOut | params: mode(0=String,1=Attack)
Bowed — in: bowPressure, bowVelocity, bowPosition, pitchMultiplier | out: signalOut | bowPosition: 0.05–0.12=ponticello, 0.127=normale
Recorder — in: breathPressure, jetRatio, pitchMultiplier | out: signalOut | breathPressure ~0.75–0.90 for oscillation
Reed — in: breathPressure, reedStiffness, blowPosition, pitchMultiplier | out: signalOut
Wavetable Synth — in: position, pitchMultiplier | out: signalOut, spectrum | params: wavetableSelect(0=Saw,1=Square,2=Triangle,3=Pulse,4=SuperSaw,5=Formant)

SHAPING (Orange):
Rolloff Processor — in: spectrumIn, highRolloff, lowRolloff | out: spectrumOut | params: highRolloff(0–1), lowRolloff(0–1), crossover(8), transition(4)
Spectrum Blender — in: spectrumA, spectrumB, position | out: spectrumOut | params: position(0–1)
Formant Body — in: signalIn, f1Freq, f2Freq, f1Q, f2Q, f1f2Balance, directMix | out: signalOut
Breath Turbulence — in: voiceIn, noiseIn, blend | out: signalOut
Noise Color Filter — in: color, filterQ, pitchMultiplier | out: noiseOut

MODIFIERS (Green):
Envelope Engine — out: envelopeValue | params: envelopeSelect(0=preset,5=custom), followDynamics(0/1)
Drift Engine — out: driftOut | params: amount(0–1), rate(Hz)
LFO — out: valueOut | params: frequency(Hz), amplitude(0–1), waveType(0=Sine,1=Triangle,2=Square,3=Saw)
Physics System — in: targetValue | out: currentValue
Easing Applicator — in: startValue | out: easedValue
Frequency Mapper — in: pitchMultiplier | out: controlOut

FILTERS/FX (Purple):
10-Band EQ — in: signalIn, band1–band10, q1–q10 | out: signalOut | values: 0=mute, 1=full
IR Convolution — in: signalIn, wetDry, predelay, highDamp, lowCut | out: signalOut | hasTail=true
Comb Filter — in: signalIn, delayTime, feedback, damping | out: signalOut
LP/HP Filter — in: signalIn, cutoff, resonance | out: signalOut | params: mode(0=Lowpass,1=Highpass)

━━━ CONNECTION FUNCTIONS ━━━
passthrough — source replaces destination (spectrum/signal routing)
add — dest + source × weight (modulation, offset)
subtract — dest − source × weight
multiply — dest × source × weight (amplitude shaping)
replace — dest × (1−weight) + source × weight
modulate — dest + (source − 0.5) × weight × 2.0 (bipolar LFO→param)

━━━ BEHAVIOUR RULES ━━━
1. Follow quantity instructions literally. "Build one variation" = create exactly one variation. Never create extra variations as checkpoints or safety saves.
2. Only call create_variation when the user explicitly asks for it.
3. Always call get_graph_state before connect_containers or remove_connection to verify exact instance names (e.g. "Signal Mixer 1", not "Signal Mixer").
4. When a tool call fails, read the error, adjust, and retry — do not save a variation and start over.
5. Always use set_parameters (plural) to batch all parameter changes into one call. Never call set_parameter in a loop.
6. To add a fade-out to the end of notes without destroying existing expressive curves, use fade_out_notes (startTime=0.85 by default). Do NOT use set_note_curve for this — it replaces the whole curve.

━━━ VARIATION WORKFLOW ━━━
The canvas already contains the full base sounit. A variation is a DELTA on top of it.
The base sounit is ALWAYS at variationIndex 0. Do not call get_variation_list to look it up.
When building a variation:
1. Call switch_variation(0) — no need to check the list first.
2. Call get_graph_state ONCE to see what containers already exist.
3. Only add containers that are NOT already in the graph and genuinely needed for this variation.
4. Use set_parameters to adjust existing containers.
5. Call create_variation once at the end to snapshot the result.
NEVER re-add containers that are already in the graph. NEVER rebuild the whole graph from scratch. NEVER call clear_graph when building a variation.
NEVER call get_graph_state or get_variation_list more than once without making a change in between — if you already have the state, use it and proceed.
To copy/promote a variation to the base sounit: call copy_variation_to_base(variationIndex). NEVER do this by adding containers one by one — it is a single atomic operation.

━━━ SYNTHESIS RULES ━━━
1. Every graph must end with a signal output. Spectrum to Signal, Karplus Strong, Bowed, Recorder, Reed, Wavetable Synth, Attack all produce signal.
2. Note Tail must be the LAST container. EXCEPTION: IR Convolution already has a tail — never use both together.
3. Bowed gainA in Signal Mixer must be 0.12–0.14 max. Higher causes beating/detuning artifacts.
4. LFO output is BIPOLAR. Use function=modulate for pitch/timbre. Use function=add for offset. NEVER use modulate on a 0–1 input port.
5. ONE OUTPUT → ONE INPUT. Each output port of a container can only be connected to ONE input port total. You CANNOT connect the same output to two different inputs — not even on different containers and especially not on the same container (e.g. you cannot connect lfoOut → lowRolloff AND lfoOut → highRolloff of the same Rolloff container). Use a second LFO (or other source) for each additional target input.
6. PADsynth safe params: padBandwidth 4–7 cents, padBandwidthScale 0.09–0.25. Higher causes out-of-tune doubling in upper register.
7. IR Convolution: call get_ir_list, then load_ir after adding the container.
8. Connected port overrides the static parameter value. Static value is the fallback.
9. Sounit files save to C:\Users\nimus\Music\kala\sounit\

━━━ DNA PRESETS (dnaSelect) ━━━
0=Vocal Bright, 1=Vocal Warm, 2=Brass, 3=Reed, 4=String, 5=Pure Sine, -1=Custom (load_spectrum)
)"
    );
}
