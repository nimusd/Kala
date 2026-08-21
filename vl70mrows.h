#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include "container.h"

// VL70-m parameter-row catalog (rows architecture, settled 2026-08-19).
//
// The patch on the module owns the CC assignments (Table 9 CONTROL NO. per
// voice, built in VL-Wizard / on the module); a Kala "VL70-m" container is
// the contract declaring which parameters Kala drives and on which CCs.
// Per-instance row state lives as flat container parameters
// ("row.<name>.active|cc|ms|reset") so sounit serialization and
// SetParameterCommand undo work unchanged. The catalog below is the
// authoritative list of touchable parameters — order = inspector order =
// bake emission order.
//
// Future direct-SysEx DEPTH fallback (if CC slots run out) will use the
// Table 9 element CONTROL NO. addresses (base 20 00):
//   pressure 0x0B, filter 0x0F, amplitude 0x13, embouchure 0x17,
//   tonguing 0x1D, scream 0x21, breathNoise 0x25, growl 0x29,
//   throatFormant 0x2D, harmonicEnhancer 0x31, damping 0x35,
//   absorption 0x39   (DEPTH lives at address + 1; embouchure has upper
//   and lower DEPTH at +1 and +3).

namespace Vl70mRows {

enum class Kind {
    Volume,  // dynamics -> CC7; toggleable (off = no dynamics stream), no end-reset, no own grid
    Cc,      // element parameters, CC assigned in the patch
    Nrpn     // part-level NRPN offsets (fixed msb/lsb, no CC)
};

struct Row {
    QString name;                  // row key / expressive curve name / port name
    Kind kind = Kind::Cc;
    int defaultCc = -1;            // CC rows only
    int nrpnMsb = -1, nrpnLsb = -1; // NRPN rows only
    int neutral = 64;              // 127 for expression; unused for volume
    int defaultMs = 25;            // resolution default (unused for volume)
    bool defaultReset = true;      // resetAtNoteEnd default (unused for volume)
    bool defaultActive = false;    // Phase 5 rows default on (see brief)
};

inline const QVector<Row> &catalog()
{
    static const QVector<Row> rows = {
        // name             kind          cc  msb  lsb  neutral  ms  reset active
        {"volume",          Kind::Volume, 7,  -1,  -1,  64,      -1, false, true},
        {"breath",          Kind::Cc,     2,  -1,  -1,  64,      10, true,  true},
        {"expression",      Kind::Cc,     11, -1,  -1,  127,     10, true,  true},
        {"pressure",        Kind::Cc,     15, -1,  -1,  64,      25, true,  false},
        {"filter",          Kind::Cc,     16, -1,  -1,  64,      25, true,  false},
        {"amplitude",       Kind::Cc,     17, -1,  -1,  64,      25, true,  false},
        {"embouchure",      Kind::Cc,     13, -1,  -1,  64,      25, true,  true},
        {"tonguing",        Kind::Cc,     18, -1,  -1,  64,      25, true,  false},
        {"scream",          Kind::Cc,     19, -1,  -1,  64,      25, true,  false},
        {"breathNoise",     Kind::Cc,     14, -1,  -1,  64,      25, true,  true},
        {"growl",           Kind::Cc,     20, -1,  -1,  64,      25, true,  false},
        {"throatFormant",   Kind::Cc,     21, -1,  -1,  64,      25, true,  false},
        {"harmonicEnhancer",Kind::Cc,     22, -1,  -1,  64,      25, true,  false},
        {"damping",         Kind::Cc,     23, -1,  -1,  64,      25, true,  false},
        {"absorption",      Kind::Cc,     24, -1,  -1,  64,      25, true,  false},
        {"vibratoRate",     Kind::Nrpn,   -1, 0x01, 0x08, 64,     12, true,  false},
        {"vibratoDepth",    Kind::Nrpn,   -1, 0x01, 0x09, 64,     12, true,  false},
        {"filterCutoff",    Kind::Nrpn,   -1, 0x01, 0x20, 64,     12, true,  true},
        {"filterResonance", Kind::Nrpn,   -1, 0x01, 0x21, 64,     12, true,  true},
        {"filterEgDepth",   Kind::Nrpn,   -1, 0x01, 0x22, 64,     12, true,  false},
        {"egAttack",        Kind::Nrpn,   -1, 0x01, 0x63, 64,     12, true,  false},
        {"egDecay",         Kind::Nrpn,   -1, 0x01, 0x64, 64,     12, true,  false},
        {"egRelease",       Kind::Nrpn,   -1, 0x01, 0x66, 64,     12, true,  false},
    };
    return rows;
}

inline const Row *find(const QString &name)
{
    for (const Row &r : catalog())
        if (r.name == name)
            return &r;
    return nullptr;
}

// Flat per-instance parameter keys ("row.<name>.<field>").
inline QString pActive(const QString &name) { return QStringLiteral("row.") + name + QStringLiteral(".active"); }
inline QString pCc(const QString &name)     { return QStringLiteral("row.") + name + QStringLiteral(".cc"); }
inline QString pMs(const QString &name)     { return QStringLiteral("row.") + name + QStringLiteral(".ms"); }
inline QString pReset(const QString &name)  { return QStringLiteral("row.") + name + QStringLiteral(".reset"); }
inline QString pNeutral(const QString &name){ return QStringLiteral("row.") + name + QStringLiteral(".neutral"); }

// Instance readers with catalog fallbacks.
inline bool isActive(const Container *c, const Row &r)
{
    // Volume toggles like any other row (default on). "Always on" was
    // reversed 2026-08-20: with CC7 off, no dynamics stream is sent and
    // loudness is expected to come from a breath/expression curve row
    // (CC2/CC11) or the module's own volume setting.
    return c->getParameter(pActive(r.name), r.defaultActive ? 1.0 : 0.0) > 0.5;
}

inline int ccNum(const Container *c, const Row &r)
{
    if (r.kind == Kind::Volume) return 7;
    return qBound(1, static_cast<int>(c->getParameter(pCc(r.name), r.defaultCc)), 95);
}

inline int resolutionMs(const Container *c, const Row &r)
{
    if (r.kind == Kind::Volume) return -1;  // streams on the cheap-tier grid
    return qBound(1, static_cast<int>(c->getParameter(pMs(r.name), r.defaultMs)), 100);
}

inline bool resetAtNoteEnd(const Container *c, const Row &r)
{
    if (r.kind == Kind::Volume) return false;
    return c->getParameter(pReset(r.name), r.defaultReset ? 1.0 : 0.0) > 0.5;
}

// Resting value sent at play start and note end. Default 64 (center/off for
// symmetric ±DEPTH CENTER BASE patches); one-directional patches rest at 0
// (or 127) - set per row to match the patch's DEPTH + MODE.
inline int neutral(const Container *c, const Row &r)
{
    if (r.kind == Kind::Volume) return 0;  // unused
    return qBound(0, static_cast<int>(c->getParameter(pNeutral(r.name), r.neutral)), 127);
}

// Every row except volume doubles as an expressive curve name.
inline QStringList curveNames()
{
    QStringList names;
    for (const Row &r : catalog())
        if (r.kind != Kind::Volume)
            names.append(r.name);
    return names;
}

// Static superset of input ports (deserialization needs a superset of any
// active-row port list; instance trimming happens via setInputPorts).
inline QStringList allInputPorts()
{
    QStringList ports{"pitch"};
    ports.append(curveNames());
    return ports;
}

inline QStringList activeInputPorts(const Container *c)
{
    QStringList ports{"pitch"};
    for (const Row &r : catalog())
        if (r.kind != Kind::Volume && isActive(c, r))
            ports.append(r.name);
    return ports;
}

} // namespace Vl70mRows
