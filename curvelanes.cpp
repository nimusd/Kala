#include "curvelanes.h"

#include "note.h"
#include "track.h"
#include "canvas.h"
#include "container.h"
#include "vl70mrows.h"

namespace CurveLanes {

namespace {

// Same band test refreshCurveSelector uses (scorecanvaswindow.cpp): case- and
// whitespace-insensitive "band 1".."band 10". Returns the band number, or 0.
int bandNumber(const QString &name)
{
    const QString n = name.simplified().toLower();
    if (!n.startsWith(QStringLiteral("band "))) return 0;
    bool ok = false;
    const int num = n.mid(5).toInt(&ok);
    return (ok && num >= 1 && num <= 10) ? num : 0;
}

bool hasAllBands(const QStringList &names)
{
    QVector<bool> seen(11, false);
    for (const QString &n : names) {
        const int b = bandNumber(n);
        if (b) seen[b] = true;
    }
    for (int b = 1; b <= 10; ++b)
        if (!seen[b]) return false;
    return true;
}

// The collapsed EQ lane shows the first band curve the note carries - all ten
// share one time-shape scaled by their own delta, so any of them reads as the
// EQ intensity over time. Mirrors ScoreCanvas::resolveActiveCurveIndex.
int firstBandCurveIndex(const Note &note)
{
    for (int i = 1; i < note.getExpressiveCurveCount(); ++i)
        if (bandNumber(note.getExpressiveCurveName(i))) return i;
    return -1;
}

// The VL70-m container of a variation's canvas, or nullptr.
//
// Deliberately found through the canvas rather than through
// Track::getGraphForVariation(): that function logs several qDebug lines per
// call, and this runs per note per repaint.
Container *midiContainerOf(Canvas *canvas)
{
    if (!canvas) return nullptr;
    const QList<Container *> containers = canvas->findChildren<Container *>();
    for (Container *c : containers)
        if (c && c->getName() == QStringLiteral("VL70-m")) return c;
    return nullptr;
}

} // namespace

QColor familyColor(Family f)
{
    switch (f) {
    case Family::Amplitude: return QColor(0x2a, 0x78, 0xd6);  // categorical slot 1, blue
    case Family::Element:   return QColor(0xeb, 0x68, 0x34);  // slot 2, orange
    case Family::Part:      return QColor(0x1b, 0xaf, 0x7a);  // slot 3, aqua
    case Family::Other:     break;
    }
    return QColor(0x52, 0x51, 0x4e);  // secondary ink - non-VL70-m curves stay neutral
}

Family familyFor(const QString &rowName)
{
    const Vl70mRows::Row *row = Vl70mRows::find(rowName);
    if (!row) return Family::Other;

    switch (row->kind) {
    case Vl70mRows::Kind::Volume:
        return Family::Amplitude;
    case Vl70mRows::Kind::Nrpn:
        return Family::Part;
    case Vl70mRows::Kind::Cc:
        // Breath (CC2) and expression (CC11) ride the cheap tier alongside
        // volume; everything else is a Table 9 element parameter.
        return (rowName == QStringLiteral("breath") ||
                rowName == QStringLiteral("expression"))
               ? Family::Amplitude : Family::Element;
    }
    return Family::Other;
}

Context makeContext(Track *track, int variationIndex)
{
    Context ctx;
    if (!track) return ctx;

    Canvas *canvas = track->getCanvasForVariation(variationIndex);
    if (!canvas) return ctx;

    ctx.liveNames = canvas->getExpressiveCurveNames();
    ctx.midiContainer = midiContainerOf(canvas);
    ctx.collapseEq = hasAllBands(ctx.liveNames);
    ctx.valid = true;
    return ctx;
}

QVector<Lane> resolve(const Note &note, const Context &ctx)
{
    QVector<Lane> lanes;
    if (!ctx.valid) return lanes;

    const QStringList &liveNames = ctx.liveNames;
    const bool collapseEq = ctx.collapseEq;
    Container *midiContainer = ctx.midiContainer;

    // ------------------------------------------------------------------
    // Ordering (D10). Catalog rows first, in Vl70mRows order - which is the
    // module's own Table 9 order and the same order the row inspector uses -
    // then anything else in the order the canvas declared it. This gives a
    // VL70-m sounit its expected order without needing to detect the
    // container type, and leaves generic sounits alone.
    // ------------------------------------------------------------------
    struct Entry { QString name; bool graphDriven; };
    QVector<Entry> ordered;

    for (const Vl70mRows::Row &row : Vl70mRows::catalog()) {
        if (row.kind == Vl70mRows::Kind::Volume) continue;  // volume is the note body, never a lane
        const bool live = liveNames.contains(row.name);
        if (live) {
            ordered.append({row.name, false});
            continue;
        }
        // Phase 6's syncVl70mCurveNames REMOVES a row from the canvas name
        // list when a Modifier connection drives its port. So "active on the
        // container but absent from the live names" is exactly the
        // graph-driven case - it still deserves a lane, marked inert, or the
        // row looks like it vanished.
        if (midiContainer && Vl70mRows::isActive(midiContainer, row))
            ordered.append({row.name, true});
    }

    bool eqInserted = false;
    for (const QString &name : liveNames) {
        bool already = false;
        for (const Entry &e : ordered)
            if (e.name == name) { already = true; break; }
        if (already) continue;

        if (collapseEq && bandNumber(name)) {
            // Ten band curves collapse to one lane, placed where the first
            // band the canvas declared would have gone.
            if (!eqInserted) {
                ordered.append({QStringLiteral("EQ Curve"), false});
                eqInserted = true;
            }
            continue;
        }
        ordered.append({name, false});
    }

    // ------------------------------------------------------------------
    // Per-lane state
    // ------------------------------------------------------------------
    for (const Entry &entry : ordered) {
        Lane lane;
        lane.name = entry.name;
        lane.eqCollapsed = (entry.name == QStringLiteral("EQ Curve"));
        lane.family = lane.eqCollapsed ? Family::Other : familyFor(entry.name);
        lane.curveIndex = lane.eqCollapsed
                        ? firstBandCurveIndex(note)
                        : note.findExpressiveCurveIndexByName(entry.name);

        if (midiContainer && !lane.eqCollapsed) {
            if (const Vl70mRows::Row *row = Vl70mRows::find(entry.name))
                lane.neutral = Vl70mRows::neutral(midiContainer, *row);
        }

        if (entry.graphDriven)
            lane.state = State::GraphDriven;
        else if (lane.curveIndex >= 0)
            lane.state = State::Curve;
        else
            lane.state = State::Empty;

        lanes.append(lane);
    }

    // ------------------------------------------------------------------
    // Orphans (R1). Row deactivation is non-destructive: the name leaves the
    // canvas list but the curve stays on the note - stored, not baked, and
    // invisible without a lane. Pinned last so they sit at the ribbon's
    // outer edge.
    // ------------------------------------------------------------------
    for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
        const QString name = note.getExpressiveCurveName(i);
        if (name.isEmpty() || liveNames.contains(name)) continue;
        if (collapseEq && bandNumber(name)) continue;

        bool alreadyLaned = false;
        for (const Lane &l : lanes)
            if (l.name == name) { alreadyLaned = true; break; }
        if (alreadyLaned) continue;   // e.g. an active graph-driven row

        Lane lane;
        lane.name = name;
        lane.state = State::Orphan;
        lane.family = familyFor(name);
        lane.curveIndex = i;
        lanes.append(lane);
    }

    return lanes;
}

} // namespace CurveLanes
