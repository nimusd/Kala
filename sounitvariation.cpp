#include "sounitvariation.h"
#include "sounitgraph.h"
#include "canvas.h"
#include <QJsonArray>

SounitVariation::~SounitVariation() {
    delete compiledGraph;
    delete sourceCanvas;  // Also deletes child containers
}

SounitVariation& SounitVariation::operator=(SounitVariation&& other) noexcept {
    if (this != &other) {
        delete compiledGraph;  // Safe here - SounitGraph is fully defined via include
        delete sourceCanvas;
        name = std::move(other.name);
        graphData = std::move(other.graphData);
        compiledGraph = other.compiledGraph;
        sourceCanvas = other.sourceCanvas;
        graphHash = other.graphHash;
        isInternal = other.isInternal;
        other.compiledGraph = nullptr;
        other.sourceCanvas = nullptr;
    }
    return *this;
}

QJsonObject SounitVariation::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["graphData"] = graphData;
    json["graphHash"] = static_cast<qint64>(graphHash);
    if (isInternal) {
        json["isInternal"] = true;
    }
    return json;
}

SounitVariation SounitVariation::fromJson(const QJsonObject& json) {
    SounitVariation var;
    var.name = json["name"].toString();
    var.graphData = json["graphData"].toObject();
    var.graphHash = static_cast<uint64_t>(json["graphHash"].toInteger());
    var.isInternal = json["isInternal"].toBool(false);
    // Note: compiledGraph must be built separately after loading
    return var;
}
