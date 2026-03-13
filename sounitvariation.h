#ifndef SOUNITVARIATION_H
#define SOUNITVARIATION_H

#include <QString>
#include <QJsonObject>

// Forward declarations to avoid circular includes
class SounitGraph;
class Canvas;

/**
 * @brief Represents an immutable variation of a sounit
 *
 * A variation is a snapshot of the sounit graph state at the time of creation.
 * Once created, variations cannot be re-edited - users create new variations instead.
 * Each variation stores the serialized graph data and a pre-compiled SounitGraph
 * for efficient rendering.
 *
 * The variation owns both the compiledGraph AND the sourceCanvas (with its containers).
 * The graph needs the canvas and containers to remain alive during audio generation
 * because it accesses container parameters and connections dynamically.
 */
struct SounitVariation {
    QString name;                    // User-assigned name (e.g., "Bright", "Dark")
    QJsonObject graphData;           // Serialized containers + connections
    SounitGraph* compiledGraph;      // Owned, pre-built for rendering
    Canvas* sourceCanvas;            // Owned, keeps containers alive for graph execution
    uint64_t graphHash;              // Hash of graph state when created
    bool isInternal = false;         // True for note-mode auto-created variations (hidden from UI)

    SounitVariation()
        : compiledGraph(nullptr)
        , sourceCanvas(nullptr)
        , graphHash(0)
    {}

    ~SounitVariation();

    // Disable copy (owns pointer)
    SounitVariation(const SounitVariation&) = delete;
    SounitVariation& operator=(const SounitVariation&) = delete;

    // Enable move
    SounitVariation(SounitVariation&& other) noexcept
        : name(std::move(other.name))
        , graphData(std::move(other.graphData))
        , compiledGraph(other.compiledGraph)
        , sourceCanvas(other.sourceCanvas)
        , graphHash(other.graphHash)
        , isInternal(other.isInternal)
    {
        other.compiledGraph = nullptr;
        other.sourceCanvas = nullptr;
    }

    SounitVariation& operator=(SounitVariation&& other) noexcept;  // Defined in .cpp

    // Serialization for file persistence
    QJsonObject toJson() const;
    static SounitVariation fromJson(const QJsonObject& json);
};

#endif // SOUNITVARIATION_H
