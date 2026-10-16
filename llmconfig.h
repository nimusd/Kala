#pragma once
#include <QString>

enum class LLMProvider {
    OpenAICompatible,   // OpenAI, Groq, OpenRouter, DeepSeek — standard /chat/completions
    Anthropic,          // api.anthropic.com — different headers and body shape
    Ollama              // localhost Ollama native /api/chat with think:false support
};

struct LLMConfig {
    LLMProvider provider = LLMProvider::OpenAICompatible;
    QString baseUrl      = "http://localhost:11434/v1";   // Ollama default
    QString apiKey       = "ollama";                       // placeholder for local
    QString model        = "qwen3:14b";
    int     maxTokens    = 16384;   // output cap per reply, NOT the context window
    double  temperature  = 0.7;
    // Reasoning/thinking mode for models that have one. true = the provider's
    // default (for DeepSeek that is thinking ON). Only DeepSeek reacts to this;
    // see LLMClient::buildOpenAIRequest().
    bool    thinking     = true;
};
