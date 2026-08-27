#ifndef CURVELANES_H
#define CURVELANES_H

#include <QString>
#include <QVector>
#include <QColor>

#include <QStringList>

class Note;
class Track;
class Container;

// Curve lanes: the ordered set of expressive-curve strips drawn outside a
// note's hull. See Claude-curve-lanes.md for the requirements this implements.
//
// The lane set is DERIVED, never stored. A note's variation canvas declares
// which curve names are live (Canvas::getExpressiveCurveNames(), already kept
// in sync with the active VL70-m rows and already connection-aware since
// Phase 6), and the note supplies the shapes by name. Nothing here touches
// serialization - no field is added to Note or to the project file.
namespace CurveLanes {

// R1 lane states.
enum class State {
    Curve,        // row live, note carries a curve -> draw it. This is exactly
                  // what the bake will send: audioengine.cpp streams a row iff
                  // findExpressiveCurveIndexByName(row) >= 0, the same test.
    Empty,        // row live, no curve on this note -> faint line at the row neutral
    Orphan,       // note carries a curve whose row is no longer live -> stored,
                  // NOT baked, invisible without a lane
    GraphDriven   // a Modifier connection drives the port -> connection wins in
                  // the bake, so a drawn curve here would be dead data
};

// D5: colour encodes family, never individual identity. There are up to 22
// VL70-m rows and only 8 categorical slots, and hues are never cycled - so
// 22 distinguishable colours simply do not exist. Identity comes from
// position, label and legend instead.
enum class Family {
    Amplitude,  // volume / breath / expression - the cheap CC tier
    Element,    // the twelve Table 9 element parameters
    Part,       // NRPN part offsets
    Other       // anything a non-VL70-m sounit declares
};

struct Lane {
    QString name;             // curve name (== row name); "EQ Curve" for the collapsed band lane
    State   state = State::Empty;
    Family  family = Family::Other;
    int     curveIndex = -1;  // index into the note's expressive curves, -1 if absent
    int     neutral = 64;     // 0..127 resting value; the Empty line's height
    bool    eqCollapsed = false;
};

// Validated against Kala's white canvas with the skill's palette validator.
// Only three slots are used because three is the most that survives all-pairs
// colour separation - which is what the Overlay tier needs, since curves there
// cross arbitrarily and any two can end up adjacent.
QColor familyColor(Family f);

Family familyFor(const QString &rowName);

// Everything about a lane set that depends on the VARIATION rather than on
// the note: the declared curve names, the VL70-m container, whether the ten
// EQ bands collapse. Building this walks the canvas's child widgets, so it is
// far too expensive to redo per note per repaint - the caller builds one per
// variation index and reuses it across every note on that variation.
struct Context {
    QStringList liveNames;
    Container  *midiContainer = nullptr;
    bool        collapseEq = false;
    bool        valid = false;
};

Context makeContext(Track *track, int variationIndex);

// Resolve the ordered lane set for one note.
//
// Every live parameter gets a lane - INCLUDING the primary curve that also
// shapes the note body. Reverted 2026-08-26: the original exclusion meant the
// curve you could see was exactly the one you could not click, and Nimus needs
// to reach it. The primary's lane is the same curve as the body, so editing it
// visibly moves the body too; the body itself never switches curves unless the
// "Show curve" combo says so.
QVector<Lane> resolve(const Note &note, const Context &ctx);

} // namespace CurveLanes

#endif // CURVELANES_H
