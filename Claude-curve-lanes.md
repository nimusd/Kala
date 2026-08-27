# Kala → Curve lanes: requirements

Making a note's expressive curves visible and directly editable on the score
canvas, instead of hidden one-at-a-time behind a toolbar combo and a right-click
dialog. Worked out with Nimus 2026-08-26, after the VL70-m feature reached daily
use. Read `Claude-vl70m-midi.md` first — this is a direct consequence of it.

## The problem, in Nimus's words

> For most patches I don't use the dynamics curve at all, the amplitude is
> controlled either by the breath (CC#2) or the expression (CC#11). So visually I
> am left with notes that look like a straight line although there may be up to
> six or seven curves affecting the note, all from expressive curves. First it
> makes setting up a note quite cumbersome since for each parameter I have to
> first right click to access the expressive curves then choose one from a drop
> down before being able to choose or make a curve. Then I lose sight of that
> curve and all other curves also.

Two distinct costs, and the second is the larger one:

1. **Invisibility.** A note carrying six streams to the module looks identical to
   a bare one. There is no way to see what a note does without opening things.
2. **Serial access.** Every parameter is a right-click → menu → dropdown →
   dialog round trip, and seeing one curve means losing sight of the rest. You
   cannot shape embouchure *against* the breath curve you already drew, because
   you can't see both.

## Vocabulary

Fixed terms, used consistently below.

| Term | Meaning |
|---|---|
| **Lane** | One thin horizontal strip drawn outside the note's hull, carrying one parameter's curve for that one note. Same width as the note. |
| **Ribbon** | The whole stack of lanes belonging to one note (above + below). |
| **Lane set** | The ordered list of parameters that earn a lane on a given note. Derived, never stored. |
| **Note hull** | The existing selection rectangle with its drag dots (`drawSelectionRectangle`, `scorecanvas.cpp:2657`). Unchanged by this feature. |
| **Lane hull** | A *separate* hull drawn around a single lane when that lane is being edited. Top-edge dots only. |
| **Primary curve** | The one curve that shapes the note body's top edge — what the toolbar's "Show curve" combo selects today. |
| **Density tier** | Off / Overlay / Lanes — how much of the ribbon is drawn. |

## What already exists

This is mostly a **de-globalization of machinery that is already there**, not a
new subsystem. Establishing this first, because it sets the true size of the job.

- `drawNote` does not hardcode dynamics. It calls `resolveActiveCurveIndex`
  (`scorecanvas.cpp:5504`), which maps `"Dynamics"`/empty → the dynamics curve
  and **any other name → a named expressive curve**. Rendering a named curve as a
  note's top edge already works.
- Drag-editing a named curve through hull dots already works — `EditingTopCurve`
  (`scorecanvas.cpp:1906`) rebuilds whatever curve `resolveActiveCurveIndex`
  resolved, with undo via `ApplyExpressiveCurveToSelectionCommand`
  (`scorecanvascommands.cpp:2513`).
- The set of live parameters is already computed and already synced.
  `Canvas::getExpressiveCurveNames()` (`canvas.h:79`) is per-variation, follows
  active VL70-m rows live, and is connection-aware — Phase 6 built that
  (`sounitbuilder.cpp:1337`, `syncVl70mCurveNames`).
- Curves are already stored per name on the note:
  `QVector<NamedCurve> m_additionalExpressiveCurves` (`note.h:189`), looked up by
  `findExpressiveCurveIndexByName`. Original editor shapes are kept per name in
  `m_envelopeControlPoints` (`note.h:196`).

All three legs exist. They are wired to **one global selection** — the
`curveSelectorCombo` (`scorecanvaswindow.cpp:551`). This feature makes that
selection per-note and per-lane, and draws N instead of 1.

**Consequence: no schema change.** `Note::toJson`/`fromJson` (`note.cpp:368-499`)
are untouched. The lane set is derived, lane selection is transient UI state.
The only new persisted values are view toggles, which belong in QSettings.

### The ribbon is a literal picture of the bake

`audioengine.cpp:1534` — a row streams to the module **iff**
`note.findExpressiveCurveIndexByName(row.name) >= 0`. That is the identical test
a lane uses to decide whether it has a curve. "This lane has a shape" and "this
CC will be sent for this note" are therefore the same fact, not two things kept
in sync. Any divergence is a bug, and the ribbon is a correctness surface, not
just a convenience.

## Settled decisions

Treat as settled unless something concrete contradicts them. Each records what
would change it.

### D1 — Lanes sit outside the note hull, split above and below

Nimus's original instinct, adopted. Lanes never enter the hull's space, so there
is no contention with the vibrato edge (which carves the blob's bottom in
`drawNote`, `scorecanvas.cpp:1382`) or with the pitch curve's edit dots (which
live on the hull's bottom edge, `scorecanvas.cpp:2765`).

Splitting is not cosmetic. `frequencyToPixel` (`scorecanvas.cpp:406`) maps the
full 20–8000 Hz range across the widget height, so a high-register note sits
close to the widget's top edge with genuinely little room above it. A
top-only ribbon clips there. Splitting halves the exposure at both extremes.

- Lane order is **catalog order running outward from the hull** in both
  directions — lane 1 hugs the note, lane N is furthest away. Proximity encodes
  priority, and the catalog opens with volume / breath / expression, which are
  the rows most likely to matter.
- The split point is a single user setting ("lanes above: N"), default balanced.
- *Would change it:* if in practice the ribbon never approaches a widget edge at
  Nimus's working zoom, top-only is simpler and should win.

Confirmed by Nimus 2026-08-26, with one adjustment to expectations: the
volume/breath/expression grouping nearest the note is right, but the element
parameters are a **re-orientation, not just a new layout** — on the module those
parameters are always read horizontally, and here they become a vertical stack.
That is a learned reading, not an instantly legible one. Don't over-tune the
ordering before the habit has had a chance to form.

### D2 — The note body keeps a silhouette, driven by a primary curve

The toolbar's "Show curve" combo stops meaning "the only curve you can see" and
starts meaning "which curve shapes the note body." Everything else goes to lanes.

Default stays `Dynamics`, so existing behavior is unchanged for existing
projects. For Nimus's VL70-m patches the combo points at breath or expression —
the curve actually carrying loudness — so a note finally *looks* like what it
sounds like instead of a flat bar. That alone addresses the "notes look like a
straight line" complaint before a single lane is drawn.

- *Would change it:* if a uniform "every parameter is a lane, the body is a plain
  bar" reads better in practice. Cheap to switch — it is one branch in `drawNote`.

**The exclusion applies in Overlay too** — confirmed by Nimus 2026-08-26 against
a proposal to relax it. The argument for including the primary in the Overlay
band was that Overlay has no click targets, so "drawn twice" costs nothing and
leaving it out makes the band an incomplete picture of the note. Rejected on
sight, and the reason is the one that matters: the band is already visually busy
with the other five or six curves, and a seventh polyline duplicating the note's
own silhouette buys nothing for the clutter it adds. Density is the scarce
resource in Overlay, not completeness.

### D3 — Three density tiers, with auto-promote on selection

- **Off** — notes only. Today's canvas.
- **Overlay** — one shared band just outside the hull, all lane curves drawn into
  it together. One band tall regardless of curve count.
- **Lanes** — the full split ribbon, one strip per parameter, clickable.

Plus the rule that does the real work: **the selected note always renders in
Lanes, even when the global tier is Overlay.** This is what makes chords workable
without a dedicated mode — the note being worked on is always the detailed one,
everything around it is compact. Dim-others (D9) stacks on top.

### D4 — Overlay shows texture, not identity

This is a correction to the Overlay idea as originally pitched, forced by
measurement rather than taste.

In Overlay, curves cross arbitrarily, so any two can end up adjacent — the
all-pairs case. Validated against Kala's white canvas
(`node scripts/validate_palette.js … --pairs all --surface "#ffffff"`), **at most
three categorical colors survive all-pairs separation.** Six overlaid curves
cannot be told apart by color, in any ordering. That is a property of human
color vision, not of the palette.

So Overlay's job is explicitly *not* "which curve is which." Its job is "this
note is busy, and here is the shape of its activity." Identity lives in Lanes.

- **Non-goal, stated plainly:** you will not be able to distinguish embouchure
  from growl in Overlay. Promote the note to Lanes (or just select it) for that.

### D5 — Color encodes family; position and label encode identity

There are up to 22 VL70-m rows (`vl70mrows.h:49-72`) and up to 10 EQ band
curves. There are 8 categorical color slots, and the method's hard rule is that
hues are assigned in fixed order and **never cycled** — a 9th series is never a
generated hue. Twenty-two distinguishable colors do not exist.

Therefore color carries **family**, three slots, which is exactly the number that
passes all-pairs and so works identically in Overlay and Lanes:

| Family | Rows | Light slot |
|---|---|---|
| Amplitude / cheap CC tier | volume, breath, expression | `#2a78d6` blue |
| Element parameters | pressure, filter, amplitude, embouchure, tonguing, scream, breathNoise, growl, throatFormant, harmonicEnhancer, damping, absorption | `#eb6834` orange |
| NRPN part offsets | vibratoRate/Depth, filterCutoff/Resonance, filterEgDepth, egAttack/Decay/Release | `#1baf7a` aqua |

Identity comes from position (stable catalog order), the name label, and the
legend — never from color alone.

Two rules inherited from the method that bite here:

- **Color follows the entity, never its rank.** Family is looked up from the
  *catalog*, not from the lane's visible index. Deactivating a row in the
  inspector must not repaint the lanes below it.
- **Contrast relief is mandatory, not optional.** The validator returns a
  contrast WARN for aqua on white (2.82:1, below the 3:1 mark floor). That WARN
  is not dismissable — it obligates visible labels. The hover label, the
  on-select label, and the legend (D6) *are* that relief. They cannot be cut for
  tidiness.
- Text — lane names, values — wears ink (`#52514e` secondary, `#898781` muted),
  never the family color.

### D6 — Identity aids: label on hover, label on select, legend panel

- **Hover** a lane → its parameter name appears.
- **Select** a lane → the name is written under the lane hull (Nimus's original
  request).
- **Legend** — a small toggleable panel listing the selected note's lane set in
  order, with family swatch. Required by both the ≥2-series legend rule and the
  contrast relief above.

Position alone is not sufficient as a memory aid, because it is not globally
stable: two notes on the same track can use different variations
(`Track::getCanvasForVariation`, `track.cpp:551`), and different variations can
declare different lane sets. Lane 3 does not always mean the same parameter.

### D7 — Editing a lane opens its own lane hull

Nimus's spec: a lane gets *its own* hull, not the note's.

- Click a lane → lane hull appears around that lane: top-edge drag dots only, no
  pitch dots (a lane has no pitch), name label beneath.
- Dot count follows the existing rule, `max(4, width/50)`
  (`calculateCurveDotCount`, `scorecanvas.cpp:3039`).
- Dragging reuses the `EditingTopCurve` path (`scorecanvas.cpp:1906`), retargeted
  from `resolveActiveCurveIndex` to the lane's parameter name.
- The note hull and the lane hull are mutually exclusive — one active at a time.

### D8 — Never coarsen a curve; refining on widen must still work

**This decision exists because of a technique Nimus uses daily**, and an earlier
draft of this doc would have broken it:

> I stretch the note very wide, thus gaining access to more editing points so I
> can be more precise and when finished I compress it back to its short length.

That works because dot count is derived from pixel width, and `EditingTopCurve`
rebuilds the curve at `numDots + 2` uniform points. Widening genuinely buys edit
resolution; curve times are normalized 0–1, so compressing back preserves it.

The same mechanism is also a data-loss path. A curve drawn in the dialog is
stored at 30 sample points (`buildShapedCurve`, `scorecanvascommands.cpp:2544`),
but a single hull-dot drag rebuilds it at `max(4, width/50) + 2` — on a narrow
note, that is 6 points replacing 30. This happens today on the active curve; with
several clickable lanes it would happen several times as often.

The rule must therefore be **never coarsen, but still refine**:

> A drag rebuilds the curve at `max(existingPointCount, numDots + 2)` points.

Widening still adds resolution. A drag on a narrow note preserves the detail
already there instead of flattening it. A naive "always rebuild from
`m_envelopeControlPoints`" fix would have satisfied the second goal and destroyed
the first.

**Dropped 2026-08-26:** an earlier draft offered an explicit dot-density control
on the lane hull as an optional alternative to the stretch trick. Nimus's reason
for rejecting it is stronger than the reason for offering it — on a short note
there is no *physical room* for more dots, so even if they could be added they
would sit too close together to click. The stretch technique isn't a workaround
he tolerates, it's the only thing that can work at that width. Leave it alone.

### D9 — Chords: dim-others, orthogonal to the density tier

A toggle that drops every note except the one being edited to low alpha. The
mechanism exists — `drawNote` already runs notes at alpha 51 for
selected-but-inactive tracks and 255 for selected notes
(`scorecanvas.cpp:1295-1331`) — so this is a new alpha branch, not new
machinery.

### D10 — Generic, not VL70-m-specific

The lane set comes from `Canvas::getExpressiveCurveNames()`, which is a Canvas
method, not a VL70-m one. Internal-synthesis tracks get lanes from the same code
path at no extra cost. The VL70-m is simply where the need is sharpest.

- Ordering: VL70-m lane sets follow `Vl70mRows::catalog()`
  (`vl70mrows.h:49-72`) — which already runs in the module's own Table 9 order
  (pressure → filter → amplitude → embouchure → tonguing → scream → breathNoise →
  growl → throatFormant → harmonicEnhancer → damping → absorption) and is the
  same order `populateVl70mInspector` renders the row editors in
  (`kalamain.cpp:3591`). Nimus asked for "the same order as the vl70m screen";
  it is already that. Other sounits follow their canvas name-list order.
- **EQ collapse:** the toolbar combo already folds ten `band N` curves into one
  synthesized "EQ Curve" entry (`scorecanvaswindow.cpp:602-618`). Lanes must do
  the same, or an EQ note sprouts ten lanes.

## Requirements

### R1 — Lane set resolution

For a note, resolve an ordered lane list from its variation's canvas curve names,
in catalog order, with the EQ collapse applied. Each lane carries a state:

| State | Meaning | Treatment |
|---|---|---|
| **Curve** | Row is live and the note has a curve for it | Normal — draw the curve. This is exactly what the bake will send. |
| **Empty** | Row is live, note has no curve | Faint flat line at the row's `neutral`. Clickable to create (R6). |
| **Orphan** | Note has a curve whose row is no longer live | Distinct muted/hatched style, pinned to the outer edge of the ribbon. |
| **Graph-driven** | A Modifier connection drives the row port | Marked as not-yours-to-draw; not click-to-edit. |

Orphan and graph-driven are not edge cases — both are produced by normal use
today:

- Row deactivation is **non-destructive**: `syncVl70mCurveNames` removes the name
  from the canvas list, but `m_additionalExpressiveCurves` keeps the curve. The
  bake iterates active rows, so an orphan curve is stored, invisible, **and not
  sent**. It exists only to reappear if the row is reactivated. Without a lane
  state for it, that stays invisible data.
- When `graph->isMidiPortConnected(row.name)` is true (`sounitgraph.h:126`), the
  connection wins in the bake (`audioengine.cpp:1519`) and a drawn curve there is
  dead. The name already leaves the combo. As a lane it must be visibly *inert*
  rather than absent, or the row appears to have vanished.

### R2 — Geometry

- Lanes are drawn outside `getNoteRect()` (`scorecanvas.cpp:2983`), above its top
  and below its bottom. Never inside.
- Lane strip height and inter-lane gap are fixed in pixels — consistent with
  `blobHeight`, which is already `20 + maxDynamics*60` and independent of zoom.
  Starting values: 14 px strip, 2 px gap (the 2 px surface gap between adjacent
  fills is a mark-spec rule, not a taste choice). Tunable in one place.
- A lane's curve spans exactly the note's width, normalized 0–1 — the same
  contract the dynamics curve already has. No time-axis mapping, no zoom-dependent
  alignment.

### R3 — Density tiers and degradation

Off / Overlay / Lanes per D3, with selected-note auto-promote.

Vertical zoom changes note *spacing*, not lane height, so collisions are a zoom
function. Required behavior: when the pixel gap to the nearest neighbouring note
is smaller than the ribbon's extent, that note degrades one tier (Lanes →
Overlay → Off) automatically. Degradation is per note, not global.

### R4 — Curve fidelity

Per D8: `max(existingPointCount, numDots + 2)`. Regression test: draw a 30-point
curve in the dialog, drag one dot on a narrow note, confirm 30 points survive;
then widen the note, drag, confirm the count rises.

### R5 — Lane hull editing

Per D7. Undo goes through the existing
`ApplyExpressiveCurveToSelectionCommand` (`scorecanvascommands.cpp:2513`),
which already snapshots `{hadCurve, oldCurve, oldControlPoints}`.

### R6 — Direct access (the ergonomic payoff)

The point of the feature. Each of these removes a step from the current
right-click → menu → dropdown → dialog round trip:

- **Click a lane with a curve** → its lane hull opens. No menu.
- **Click an empty lane** → creates the curve for that parameter and opens it. The
  dropdown disappears entirely for live rows.
- **Ghost context in the dialog.** When the modal editor is open on curve X, the
  note's other curves render inside it as dimmed, non-interactive lines. This is
  the direct answer to "I lose sight of that curve and all other curves" — you
  shape embouchure *against* a visible breath curve rather than from memory.
- **Switch curves without closing.** A strip of lane tabs in the dialog header, or
  clicking another lane while the hull is open. Cycling six parameters on one
  sustained note without reopening anything is where the time actually goes.

### R7 — Performance

Lane polylines are cached per note and invalidated on curve edit, zoom, or
resize. `Note::computeHash` (`note.cpp:236`) already hashes the named expressive
curves and is a reasonable cache key. Notes exist twice at runtime — the
`ScoreCanvas` `Phrase` and `Track::m_notes` (`track.h:764`, what the engine
bakes) — so any cache must key off the canvas copy and respect the existing sync.

### R8 — Persistence

Density tier, split point, dim-others, and legend visibility go to QSettings.
Nothing enters the project file. Nothing enters `Note::toJson`.

### Non-goals

- Identifying individual curves by color in Overlay (D4).
- Timeline-wide automation lanes under the score. Kala's curves are note-owned;
  a timeline lane would show disjoint per-note segments. Possibly a useful
  *companion* view later for comparing one parameter across a phrase — out of
  scope here.
- Rendering the baked value of a graph-driven row as a read-only trace. Lovely,
  but it needs a graph walk per note per repaint. Later, if ever.
- Any change to bake, dispatch, or the MIDI path. This is a canvas feature.

## Implementation plan

Six phases, house style: one at a time, build → test → celebrate → repeat. Each
is independently useful; the first three are pure rendering and cannot break
existing editing.

**A — Lane model and geometry.** `LaneSet` resolver (note → variation → canvas
curve names → ordered lanes, EQ collapse, four states from R1) plus
`laneRect(note, laneIndex)`. No visible feature; a debug overlay draws empty lane
boxes.
*Test:* boxes land outside the hull, split correctly, follow catalog order, and
track row activation live. High-register notes keep their below-lanes on screen.

**B — Overlay tier.** One shared band, family colors, contrast relief via hover
labels.
*Test:* a note carrying six streams visibly differs at a glance from a bare one.
First moment the invisibility problem is gone.

**C — Lanes tier.** Split ribbon, four lane states styled distinctly, per-note
degradation, split-point control.
*Test:* every state is visually distinguishable — in particular an orphan curve
and a graph-driven row are both *visible* and obviously not normal.

**D — Lane hull and editing.** Selection, lane hull with top dots and name label,
`EditingTopCurve` retargeted, and the R4 never-coarsen rule.
*Test:* the R4 regression test passes. Then the real one — edit a note's six
curves without touching a context menu.

**E — Direct access.** Click-empty-to-create, ghost curves in the modal dialog,
lane switching without closing.
*Test:* shape embouchure against a visible breath curve. The second complaint —
losing sight of the other curves — is answered.

**F — View controls and polish.** Toolbar tier control, dim-others, legend panel,
QSettings persistence, keyboard shortcut for the tier toggle (it will be pressed
constantly).
*Test:* a chord is workable.

## Risks

- **Vertical space is genuinely tight.** Split helps but does not eliminate it. If
  Phase A shows the ribbon crowding at Nimus's normal zoom, the honest answer may
  be smaller lanes, fewer default-active rows, or leaning harder on Overlay —
  not more phases.
- **Lane positions shift when the lane set changes.** Positions are stable for a
  given lane set, but toggling a row in the inspector reflows the lanes outside
  it. Mitigated by the legend, not eliminated. Watch whether it actually bites.
- **`drawNote` is already a large function** (`scorecanvas.cpp:1266-1471`) with
  branches for segments, legato, variation badges, and pending notes. Lane
  rendering should be a separate function called from `paintEvent`, not more
  branches inside `drawNote`.
- **Phase D touches the one path that mutates curve data.** Phases A–C are
  additive rendering and safe to ship alone; D is where regressions would live.

## Open questions for Nimus

1. **Lane height.** 14 px strip + 2 px gap is a starting guess. A 6-lane note is
   then ~48 px above and ~48 px below the hull, against a note body of 20–80 px.
   Does that read as reasonable, or is it already too much at your working zoom?
   Nimus: Let's try it that way. First sight will tell me.

2. **Which rows deserve to sit closest to the note?** D1 puts catalog order
   outward from the hull, so volume/breath/expression hug the note. If in practice
   the element parameters are the ones you watch, the order should invert.
   Nimus:Your're right about volume/breath/expression. As for the others it might
   just be a case of getting used to it. In the vl70m the parameters are always
   given in horizontal view and now they will appear in vertical. Practice makes perfect.

3. **Overlay's honest limit** (D4) — Overlay can show that a note is busy and
   what its activity looks like, but not which curve is which. Is that still worth
   having as a tier, or would you rather it were simply Off/Lanes with
   dim-others?
   Nimus: can only answer this one after testing, really. maybe it is a useless feature
   and maybe it is something you get used to and it tells a lot. Non save. As I undersatand it
   it could help declutter the score at the expense of cluttering the exprissive curves
   read out. So sometimes you may want one, sometimes the other. So we try it.

4. **The dot-density control** (D8, optional follow-on). Would an explicit
   "more points" control on the lane hull be welcome, or is stretch-and-compress
   a technique you actually like and want left alone?
   Nimus: actually when a note is short (in duration) there is no space to add dots
   and even if you could add dots they would be impossible to click on being too 
   near to one another. So the stretch technique is the best and only one so far.
   Maybe one of those days i will wake up "seeing" another solution but for now
   we keep things as they are.
************************************************
Nimus:
   Otherwise it looks like this will unlock some latent potential hidden
   in Kala's interface since we are not touching the structure but just reshuffle/reuse the UI so to speak.
   This will also be a true productive addition to the regular sounit since I had the same 
   problem when using expressive curves on the original sounits (non vl70m sounds).
   Your example of the EQ is spot on. I do remember setting this up for the first time
   and scratching my head as to how to remember all that wiring without any visual clue.
   This should bring new dimensions to the Kala's score visual experience.
   True going forward. let's do it!

## Right now

Requirements agreed 2026-08-26; all four open questions answered above; go-ahead
given. Resolutions folded into D1 (ordering confirmed, with the horizontal →
vertical re-orientation caveat) and D8 (dot-density control dropped for good).
Two items deliberately left to first sight rather than decided on paper: the
14 px + 2 px lane metrics (R2) and whether the Overlay tier earns its place (D4)
— both are trial values, and the doc should be amended from what the canvas
actually looks like, not from more argument.

Persistence (Q3's "Non save", settled for R8): view settings persist to
QSettings — split point, dim-others, legend visibility — but the density tier
deliberately does NOT. Every session starts at Off.

Phase A (lane model and geometry) — done, tested 2026-08-26. New
`curvelanes.h/.cpp` (lane-set resolver: ordering, four R1 states, family
colours) plus `laneRect`/`lanesAboveCount`/`drawLaneDebug` in ScoreCanvas and a
Ctrl+L debug overlay. Nimus: size and position good, and rows added/deleted in
the inspector move the lanes live ("Very cool").

Two findings from building it:

- **Graph-driven rows would have been invisible, not inert.** Phase 6's
  `syncVl70mCurveNames` already removes a connection-driven row from the canvas
  name list, so reading that list alone would have made such a row simply
  absent from the ribbon — the exact "where did my row go?" confusion R1 exists
  to prevent. Detected instead as "active on the container but missing from the
  live names", the inverse of that sync rule.
- **`Track::getGraphForVariation()` is unusable in a paint path.** It emits
  several qDebug lines per call; per note per repaint that is hundreds of lines
  a frame. The VL70-m container is found through the canvas's child widgets
  instead. Anything else added to the paint path should avoid it too.

Phase A simplification to revisit in Phase C: lanes resolve for the **active
track only**, because `m_currentTrack` is the sole real `Track*` ScoreCanvas
holds (TrackSelector keeps its own lightweight strip struct that happens to
share the name). This may be the right behaviour anyway — it is not yet a
decision.

Phase B (Overlay tier) — done, tested 2026-08-26. One shared band above each
note's hull carrying every curve that will actually be sent, in the three family
colours, plus a hover readout naming them (the contrast relief the palette
validator requires, not decoration). Ctrl+L now cycles Off → Overlay → Lanes.
Vertical range checked 30 Hz–5500 Hz with no clipping at either extreme, which
retires the high-register worry behind D1's split.

One real bug found by Nimus's eye and fixed: overlay curves did not match the
shape of the curve on the note. Cause — the band was built from `getNoteRect()`,
which pads **5 px on each side** for the hull's resize handles
(`scorecanvas.cpp:3016`). Normalizing a curve across that rect stretches it by
10 px and shifts it 5 px left, obvious on short notes and subtle on long ones.
Lanes had the identical defect. Both now span the note's own time extent via
`noteSpanX()` — which is what R2 said all along ("exactly the note's width");
the wrong rectangle was simply convenient. Anything else drawn per note should
use `noteSpanX()`, never `getNoteRect()`, for its horizontal extent.

Two mismatches that are NOT bugs and were checked at the same time: the note
body draws the primary curve while the band draws the others, so there is no
shared reference between them by design; and on a gliding note the blob's
centreline follows the pitch curve, so its top edge is pitch motion plus curve
value combined, while a band shows the curve alone on a flat baseline.

Phase C (Lanes tier) — done, tested 2026-08-26. The real split ribbon: four lane
states styled distinctly (curve = family colour + light fill; empty = a faint
line at the row's neutral; orphan = dashed grey in a dotted box, deliberately
NOT wearing its family colour since wearing it would imply it is being sent;
graph-driven = dotted midline + link glyph), hover naming the lane and
explaining its state, D3 auto-promote on selection, and per-note degradation
when neighbours crowd. All tested and passed.

**The split nudge is the finding.** Ctrl+Shift+Up/Down moving one lane between
above and below was added as interim scaffolding for a Phase F widget, and
turned out to be the thing that resolves the whole vertical-space problem —
Nimus: "a game changer, specially for those edge cases when super high note. No
limit now anymore." The auto-balanced default was never the answer on its own;
the manual override is. Phase F promoted it: the toolbar spin + Auto checkbox
drive the same `setLanesAbove` path as the keyboard, so both stay in sync in
both directions. The keyboard path was kept intact.

This also closes out the D1 argument properly. The original instinct was
top-only, with splitting treated as a concession to clipping. Splitting plus a
manual split point is strictly better: the clipping case stops being an edge
case at all.

Performance note: the resolver's per-variation work (canvas curve names, the
VL70-m container lookup, the EQ-collapse test) is now a `CurveLanes::Context`
built once per paint per variation, because Phase A's version called
`findChildren<Container*>()` per note — harmless behind a debug overlay,
badly wrong once every note resolves lanes every repaint. Still uncached:
`getNoteRect()` inside the degradation check, which samples each curve 42 times
and runs against every time-overlapping note. Not yet a problem in practice;
cache note rects if scrolling ever feels sluggish on a dense piece.

Phase D (lane hull and editing) — done, tested 2026-08-26. Nimus: "that feels
like pure power." Clicking a lane opens it: the strip grows 14 px → 56 px
(`EDIT_LANE_HEIGHT`) and gets its own hull in blue rather than the note hull's
grey, dots on the curve, parameter name written beneath. Escape or a click
elsewhere closes it. Undo rides the existing `EditCurveCommand`. Orphan lanes
open too (it is still the user's data); empty and graph-driven do not.

**The D8 rule fixed a pre-existing bug in daily use, not just a lane risk.**
The doc predicted lanes would multiply exposure to curve coarsening. In fact
`EditingTopCurve` was already destroying data on every single drag of the note
body: it cleared the curve and re-sampled at `numDots + 2`, so a 30-point
dialog-drawn curve became 6 points on a narrow note. That had been happening
silently for as long as expressive curves have existed. Both paths now go
through one helper.

`curveAfterDotDrag()` lives in **curve.cpp**, not in the canvas, specifically so
`test_curve_dotdrag.cpp` links the real function instead of a copy that could
drift. The first cut of the test did duplicate it — worth avoiding for the one
rule that protects a technique in daily use. Covered: 30 points survive a
narrow-note drag; a flat curve still refines to 22 on a wide note (stretch
technique intact); the dragged dot reaches its target value; detail outside the
dot's two neighbours is untouched; endpoints stay pinned; 25 successive drags do
not erode the curve. Not in the CMake build — compile line is in its header.

**Architectural note for anything else added to this canvas:** pen and mouse
have *entirely separate* press, move and release paths in scorecanvas.cpp
(`mousePressEvent` / `mouseMoveEvent` / `mouseReleaseEvent` versus the
`TabletPress` / `TabletMove` / `TabletRelease` cases in `tabletEvent`), each
with its own copy of the drag-mode switch. Nimus works with the tablet, so
anything wired only to the mouse path tests fine for a developer and is invisible
in real use. Lane press handling is shared via `handleLanePress()`; the drag
cases had to be added to both switches.

Deliberate inconsistency to watch: lane dot drags are positional (1:1 with the
cursor, since an opened lane is tall enough for that to read naturally), while
the note body's top-curve drag stays delta-based at 50 px per full range.

Phase E (direct access) — done, tested 2026-08-26. Click an empty lane → its
curve is created (flat at the row's neutral, then opened as a hull); the
dropdown round trip is gone for live rows. Ghost curves render behind the
editable curve in the modal editors, in family colours with a legend, and
re-filter when the target dropdown changes. Lane switching without closing
turned out to be free: Phase D's press handling already opens whichever lane is
clicked.

**Three decisions Nimus corrected while testing, all settled:**

- **Double-click = the dialog.** Single click gives the hull and dots;
  double-click opens the full curve dialog with that lane's curve pre-selected
  in the dropdown and pre-loaded when it has a shape. Nimus's original ask
  ("How could we have both?") and the answer to the dot-by-dot editing
  limitation. Works on the pen path too (the tablet double-tap routes to the
  same dialog with the existing deferred-open). Double-clicking over a dot is
  safe — the OS-level double-click detection wins.
- **The "Show curve" combo is now a pure manual picker.** Reverted two
  auto-switch behaviours that together made "editing a curve kidnaps the note
  body": the dialog handlers' direct activation calls AND the toolbar sync
  connection (`scorecanvaswindow.cpp`, `expressiveCurveApplied`) that pushed
  the combo to the last applied curve. The combo now only refreshes its ITEM
  LIST when a new name appears; the selection stays wherever the user put it.
  It is the one control that chooses the note body's silhouette, and nothing
  else writes it.
- **The primary curve's lane stays in the ribbon.** D2 originally excluded it
  ("don't draw a curve twice"); Nimus's editing reality reversed it: the curve
  you can see must be the one you can click. Every live parameter gets a lane,
  full stop. The primary's lane editing visibly moves the body — same curve,
  drawn twice — and that is correct, not a duplication bug.

One implementation note for whoever works on the dialogs next: a curve applied
by the undo command is re-sampled to **30 points** (`buildShapedCurve`,
`SAMPLE_COUNT`), so any "is this curve unshaped" test that keys on point count
is wrong. The dialog preload now skips flat curves by value range (hi - lo <
1e-9) so a freshly-created neutral line — which on one-directional patches with
neutral 0 is a line glued to the bottom of the editor — does not replace the
dialog's default preset.

Phase F (view controls and polish) — done, tested 2026-08-26, all tests passed
(Nimus: "Beautiful!"). Toolbar tier combo (Off/Overlay/Lanes, no focus-stealing
focus policy), application-wide Ctrl+L cycle that now works even while a combo
has focus (the canvas-local keyPressEvent branch was removed), lanes-above spin
+ Auto checkbox mirroring the keyboard nudge in both directions, dim-others
(alpha 51, works in every tier including Off, dims ribbons too), and a floating
legend panel (LanesLegend, VariationToolbar-style) listing the selected note's
lane set in catalog order with family swatches and state hints — hidden when
nothing is selected, but stays "armed" and reappears on the next selection.

Persistence settled exactly as written above: split, dim-others and legend
visibility restore from QSettings; the tier resets to Off every session.

Curve lanes are complete. Both trial values survived first sight: the 14 px +
2 px lane metrics stayed as specified, and the Overlay tier earned its place.

Unreproduced, not curve-lanes-caused, recorded as a lead: on first opening an
older two-track VL70-m project, "Apply Expressive Curve…" was greyed out; a new
project worked, and reopening the old one worked. `selectionCommonExpressive
CurveNames()` returns empty immediately when `m_currentTrack` is null
(`scorecanvas.cpp:5210`), which greys that action while everything else looks
normal — a plausible load-ordering window between project open and track
binding. Chase only if it recurs.
