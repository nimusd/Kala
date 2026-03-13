#pragma once
#include <QString>

enum class LLMProvider {
    OpenAICompatible,   // Ollama, OpenAI, Groq, OpenRouter — standard /chat/completions
    Anthropic           // api.anthropic.com — different headers and body shape
};

struct LLMConfig {
    LLMProvider provider = LLMProvider::OpenAICompatible;
    QString baseUrl      = "http://localhost:11434/v1";   // Ollama default
    QString apiKey       = "ollama";                       // placeholder for local
    QString model        = "qwen3-coder:30b";
    int     maxTokens    = 4096;
    double  temperature  = 0.7;
};
