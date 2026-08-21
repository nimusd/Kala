# Kala → VL70-m MIDI output: project brief

This is the context for a new Kala feature: sending MIDI out of Kala to control external hardware synthesizers, starting with the Yamaha VL70-m. It was worked out in a planning conversation with Nimus, Kala's developer, before any code was written. Read it in full before touching anything.

## Read first, in this order

1. `CLAUDE.md` — orientation, build steps, current working style.
2. `Claude-sounit-builder.md` — the container/port/connection vocabulary this feature extends. Confirm the terms used below against this file; some of what's described here was worked out without access to it and may not match the real vocabulary exactly.
3. `Claude-companion.md` — Anima's tool architecture, for later reference once this container needs its own agent tools.
4. `VL-Wizard Manual (draft).md` An app specially designed to create, modify, organize the voice patches of the VL70m giving deeper acces to parameters.
5. `VL70mE2.md` — the VL70-m's own manual. Its MIDI Data Format section (CC list, NRPN addresses, RPN, SysEx format) is the protocol reference for phases 3 onward.

## What this feature is

A new container type in Kala's sound-engine graph: one MIDI-out port, several CC/NRPN-mapped input ports, sitting where an internal "SignalOut" container currently sits. It's an adapter, not a special case — the same Modifier containers (Envelope, Physics, etc.) and the same connection combinators (through, add, subtract, weight) that drive internal synthesis now drive external hardware instead. The VL70-m is the first target; the container's definition should stay data, not hardcoded, so other synths can follow later.

## Why it exists

Not "add MIDI support" as a checkbox. Nimus has played the VL70-m for over twenty years — first with a WX7, then a WX5, and separately experimented with an FCB1010 plus two expression pedals. Even good wind-controller sensors couldn't drive more than a couple of the VL70-m's expressive dimensions continuously and simultaneously, because a body only has so many independently-controllable channels at once. The FCB1010 route added access at the cost of the playing no longer feeling like wind playing, and still left territory out.

The VL70-m's physical model has far more independently addressable continuous parameters per element than any performer could drive live: growl, throat formant, harmonic enhancer, damping, absorption, tonguing, scream, breath noise, embouchure (upper and lower depth), on top of pitch and breath. Kala was never trying to be performed live — a composer shapes one curve at a time, with unlimited editing time, and Kala plays them all back together afterward, in sync, at zero motor-coordination cost. So the actual target for this feature is several of those element-level curves running independently on one sustained note — expressive territory this setup has never reached in twenty years, not a MIDI-parity feature.

## Decisions already made

Treat these as settled unless something concrete contradicts them during implementation — don't relitigate from scratch.

- **Timing model — bake, then dispatch.** Kala's engine is offline everywhere else: notes render to buffers in a background thread, the audio thread just mixes cached PCM. Real hardware can't be pre-rendered — it only exists in real time. Resolve this by keeping the same precompute philosophy: a background pass walks a note's curves, resamples them for the MIDI pipeline speed, applies the connection combinators, and bakes a timestamped list of MIDI messages — the MIDI equivalent of a `noteRender`. A separate, lightweight real-time thread walks that list and fires messages in sync with the transport during playback. Two levels of playback: 1.- live for a small section that is under construction (you don't want to have to re-record the whole thing just because you change one setting on one note), 2.- playback with simultaneous recording or "baking" in DAW parlance. This is then just added to the rest of the pre-rendered audio for playback/save/load with the rest.

- **This container is a solo voice, not a polyphonic target.** The VL70-m is confirmed monophonic — one note, full stop. Build the container that way rather than adding voice-stealing logic. Two overlapping notes routed to it is an authoring conflict Kala should flag, not something to silently resolve. Give the container a declared max-polyphony property now, even fixed at 1, so this doesn't need rework when a genuinely polyphonic synth is added later.
- **Controller bandwidth is two-tier, not uniform.** Plain CC, pitch bend, and aftertouch (breath CC2, volume CC7, expression CC11, foot CC4, modulation CC1, pitch bend) are 2–3 byte messages and cheap — twenty years of continuous breath and pitch-bend streaming from WX7/WX5 into this exact module confirms that tier handles sustained continuous traffic fine. NRPN-addressed parameters (filter, vibrato, EG times, and the element-level dimensions above) cost 4–6 bytes per update and are unproven at high rates, especially several at once — no wind controller ever drove more than two or three of these continuously and simultaneously. Test this deliberately (see roadmap) rather than assume it. Resampling/rate control for MIDI output should be a per-port budget, not one global slider — tight for the cheap tier, looser or delta-thresholded for the NRPN tier.
- **Dynamics maps to Volume (CC7), not Breath (CC2).** Reversed 2026-08-19 on Nimus's Reaper test against the module: a bare note plays a uniform sound; CC7, CC11 and CC2 lanes each affect the patch differently and can be sent simultaneously, and CC7 is the one compulsory controller. Cost of the reversal: CC7 dynamics is amplitude-only — the tone stays uniform at every level, dropping the breathier-tone-at-low-breath behavior Phase 2 verified from CC2. Gain: breath CC2 becomes a free, independent row (pressure/timbre curve), decoupling loudness from tone — territory no WX5 could reach with a single breath stream. Related module facts: system parameter 00 00 0B BREATH CONTROL NUMBER (BC/EXPRESSION) is the module-side counterpart of the WX5's CC2/CC11 choice, and per-voice 00 0A EXPRESSION MODE (BC/VOLUME) declares what each voice's expression follows — even these CCs' meaning is patch-level routing, matching the rows architecture below. Amended 2026-08-20 after a full day of hardware testing: CC7 dynamics is not always the right fit, so the volume row is now **toggleable** — off means no dynamics stream at all, and loudness is expected to come from a breath (CC2) or expression (CC11) curve row instead (both already stream any curve named after them), or from the module's own volume setting. The toggle is per container, default on.
- **Pitch bend is not simple transposition on this instrument.** It's channel-wide (irrelevant for a monophonic VL70-m, relevant once polyphonic synths are added — that will need an MPE-style per-note-channel approach). On many VL70-m voices it's programmed to also drive embouchure, not just pitch — check each target voice's manual comment rather than assume a flat mapping. Pitch Bend Sensitivity (RPN 0,0) needs to be set to match whatever range the curves expect; default is ±2 semitones, and the 14-bit bend value divided across that range is the real precision ceiling.
- **Full controller-to-parameter routing only exists on Custom (6 slots) and Internal (64 slots) voices.** The 256 factory presets have their routing fixed at whatever's documented in each voice's manual comment. Prototype against one well-documented, breath-oriented factory preset before building anything that assumes arbitrary curve-to-parameter routing.
- **CC assignments belong to the patch, not to Kala (rows architecture).** Settled 2026-08-19. The VL70-m stores per-voice CONTROL NO. assignments (Table 9's twelve element parameters: pressure, filter, amplitude, embouchure, tonguing, scream, breath noise, growl, throat formant, harmonic enhancer, damping, absorption — each off/CC 0–95/AT/VELOCITY/PB, plus per-element DEPTH and CURVE), so Nimus designs patches, assignments included, in VL-Wizard and on the module; Kala never writes the edit buffer at play start (this kills the Phase 5 restore-by-re-selecting-voice wart). The container becomes a declaration of the contract instead of an assigner: a fixed core (pitch via bend, channel, bend range, note on/off) plus rows — volume CC7 (dynamics, toggleable since 2026-08-20, default on), breath CC2 and expression CC11 (optional), the twelve element parameters (CC-assigned in the patch), and NRPN part offsets (filter cutoff/res, vibrato, EG times — no CC# needed). Each row: active toggle, CC#, resolution ms (element rows default 25 ms, part rows 12 ms — the Phase 4 measurements), and no-curve semantics (decided 2026-08-19): neutral sent once per active row at play start; a curve-carrying row streams its curve and sends its neutral at note end; a no-curve row sends nothing — the bake knows the whole piece, so end-resets are delta-skipped when the module is already resting. The neutral is itself per-row data (row.<name>.neutral, inspector-editable; defaults 64 offset-type, 127 expression) — the patch's DEPTH + MODE decide where "off" lives: symmetric ±DEPTH CENTER BASE rests at 64, one-directional patches at 0. Found on hardware 2026-08-19: throat formant (CC15) stayed at the curve's end value because the patch was not centered on 64. Per-row resetAtNoteEnd flag, default on for offset rows and off for expression, since the note-off reset makes the release tail play at neutral — audible for multiplier rows. Volume CC7 is exempt (dynamics always has data). Stop-mid-note keeps the data-driven close reset. Saved as a sounit named after the patch it matches; the name match is a human contract Kala cannot verify (possible later mitigation: Program Change + bank select field on the container). This delivers Phase 8's map-as-data early. Also noted for the contract: each part has Rcv toggles (Rcv EXPRESSION etc.) — a row only works if the part receives that CC.

## Roadmap

Eight phases, one at a time, each with its own test and its own stopping point. Don't start a phase without confirmation the previous one is done.

1. **Hello note** — hardcoded Note On/Off through an opened MIDI port, no score integration. Proves the wire works.
Phase 1 — Hello note. Prove the wire works before anything else touches the score. A hardcoded Note On at a fixed pitch, held a beat, then Note Off — triggered by a button, nothing wired to Kala's timeline yet. This is also where you make the one real implementation call this early: talking to Windows MIDI directly, or through a small library like RtMidi, since that choice shapes the dispatcher thread you'll build in Phase 3. Test: you hear the VL70-m answer. Celebrate: first sound Kala has ever put into real hardware.

2. **Static container** — minimal VL70-m container (MIDI-out port, pitch + breath input ports), driven by a real Note's static `pitchHz`, no curves. Proves the plumbing and Note On/Off timing against the timeline.
Phase 2 — A container that plays one static note. Build the actual VL70-m container type, minimal: one MIDI-out port, two input ports — pitch and breath — each fed a single static value pulled from a real Kala Note's pitchHz, no curves yet. This is where Note On/Off gets tied to actual note start and duration on the timeline, and where Pitch Bend Sensitivity gets set up, since turning Hz into a bend value requires knowing the configured range. (The exact container/port vocabulary here is where your sounit-builder spec would matter most — still worth sending when you get the chance.) Test: place a note on this container's variation in the score, hit play, correct pitch comes out. Celebrate: first note played from the actual score, even flat and static.

3. **Continuous baseline** — real pitch and dynamics curves, bake-then-dispatch built for real. Proves Kala can reproduce an actual gesture (glide, vibrato, ornament) faithfully.
Phase 3 — The real baseline: continuous pitch and breath. Now the curves, and where the bake-then-dispatch machinery gets built for real — a background pass that walks a note's pitch and dynamics curves, resamples at a chosen interval, and produces a timestamped event list, plus a lightweight dispatcher that walks that list in sync with the transport during playback. Given twenty years of proof this specific hardware handles sustained continuous breath and bend without strain, the risk here is entirely on Kala's side — getting the resampling and timing right — not the VL70-m's. Test: draw an actual gesture, a glide or a vibrato or a raga-style ornament, and hear it translate faithfully. Celebrate: the first moment it sounds like Kala playing the VL70-m, not a MIDI test file.

4. **Bandwidth ceiling test** — deliberately push several simultaneous NRPN streams at once, find the real number before building more ports around an assumption.
Phase 4 — Find the real ceiling, on purpose. Before adding more ports, deliberately push several simultaneous fast NRPN streams at once — not pitch and breath, which are already proven, but the expensive tier: filter, vibrato, a couple of element parameters, all moving quickly together. This is genuinely untested ground, since even a WX5 only ever drives two or three things continuously, never eight. The output is a real number — how many simultaneous streams, at what interval, before anything audibly lags — replacing a guess with an actual budget for everything that follows.

5. **Independent curves, stacked** — the actual goal. Two or three element-level parameters (ask Nimus which ones they've most wanted extra hands for) as ports, each with its own curve, all evolving independently on one sustained note.
Phase 5 — The actual target: independent curves, stacked. The phase the whole project is really for. Pick two or three element-level parameters — growl, throat formant, harmonic enhancer, damping, whichever you've most wanted extra hands for — and add them as ports, each driven by its own hand-drawn curve to start, no Modifiers needed yet. Test: one sustained note where several of these evolve independently and simultaneously in ways no live performance, WX5 or FCB1010, could ever produce. Celebrate: the actual thesis proven — not "Kala can talk to a synth" but "here's expressive territory this hardware has never reached before."

6. **Modifier hookup** — Envelope, Physics, and the rest of the Modifiers menu driving these ports through the existing connection combinators, instead of hand-drawn curves only.
Phase 6 — Hook up the Modifiers. Let Envelope, Physics/drift, and the rest of the Modifiers menu drive these ports through the same connection combinators you already use elsewhere, instead of hand-drawn curves only. Test: a physics or drift modifier driving something like damping or absorption in a way that feels organic rather than manually drawn. Celebrate: the MIDI container is now a first-class citizen of the sound-engine graph, not a special case bolted to the side.

7. **Tab, canvas, inspector** — the dedicated MIDI authoring UI, built last, once the underlying graph behaves correctly headless.
Phase 7 — The tab, canvas, and inspector. Only now build the dedicated MIDI authoring surface, deliberately last — it's much easier to debug a UI against a graph you already know behaves correctly than to debug plumbing and interface at once. Test: build a VL70-m instrument entirely through the new UI, no hardcoded fallback underneath, and it behaves identically to what came before. Celebrate: authoring this now feels like authoring anything else in Kala.

8. **Beyond the VL70-m** — CC/NRPN map and polyphony ceiling become data the container reads, not hardcoded facts, so other synths can be defined the same way.
Phase 8 — Beyond the VL70-m. Turn the CC/NRPN map, port names, and the monophonic ceiling into data the container reads, rather than facts baked into code. Test: define a second, simpler synth's map and get a note out to different hardware without touching the container's core logic. Celebrate: "opens the door to other synths" — what you said in your very first message — actually delivered.



## How to work

- Wait for explicit go-ahead before writing any code, including for the very first phase.
- One phase at a time. Build → test → celebrate → repeat. Don't start the next phase in the same breath as finishing one.
- If something in "Decisions already made" turns out to be wrong once you're actually working against the hardware or the real sounit-builder spec, say so and explain why, rather than quietly working around it.

## Right now

Phase 1 (hello note) — done, tested 2026-08-18. "MIDI test" button in the score
window toolbar plus a "MIDI Setup" category in the Settings tab (Ctrl+3) with output
device selection, saved to QSettings. Hello note = breath CC2 + Note On A4 (500 ms)
through the configured port. First real test was via a Steinberg UR22mkII USB
interface into the VL70-m; note heard from the module's own output. (Returning the
VL70-m's audio into Kala's audio path is a separate item, not in this phase plan.)

Phase 2 (static container) — done, tested 2026-08-18. "VL70-m Out" container in the
Essential menu (blue): ports pitch/breath in, midiOut out (declarative); inspector
params midiChannel (1–16) and pitchBendRange (default ±2 st). Notes whose variation
graph contains it are baked at play start (breath CC2 = average dynamics, pitch bend
= pitchHz cents remainder vs nearest MIDI note, note on/off at note boundaries) and
dispatched by the audio callback in sync with the transport. RPN 0,0 bend sensitivity
set per channel at play start; All Notes Off + port close on stop. Verified: a Just
Intonation perfect fifth sounds true (bend working), dynamics map to loudness, and
the module's breathier tone at low breath comes through. Monophonic overlap logs a
conflict warning. Core code: audioengine.cpp bakeMidiEvents + callback dispatch,
midioutput.h/.cpp port helpers, track.cpp MIDI-only graph gating.

Phase 3 (continuous baseline) — done, tested 2026-08-18. Pitch and dynamics
curves resample into breath CC2 + pitch bend streams at the container param
midiUpdateIntervalMs (default 10 ms, inspector spinbox; bake-time only). Shared
applyVibrato() helper (vibrato.h) so the bake and the audio renderer produce the
same vibrato, baked into bend + breath. Per-note RPN 0,0 bend sensitivity: a
two-pass bake measures each note's real pitch excursion over its emission grid and
widens the range to cover it (max 24 st), replacing Phase 2's play-start
channel-wide RPN — wide glides stay continuous with no re-trigger. The MIDI note
number is fixed per note from the start pitch; semitone crossings ride the bend
wheel. Delta skip: unchanged bend/breath values are not re-sent; note start always
sends fresh values. Verified: saxophone voice responds, glides/vibrato translate
faithfully. Known artifact: huge pitch glides shift timbre because many VL70-m
voices route pitch bend to embouchure (voice data — check the voice's comment in
VL70mE2.md); mitigation is voice editing (VL-Wizard) or, later, counter-driving
embouchure via SysEx. Core code: audioengine.cpp bakeMidiEvents, vibrato.h
applyVibrato, kalamain.cpp populateVl70mInspector.

Phase 4 (bandwidth ceiling test) — done, tested 2026-08-18. Harness:
BandwidthTestDialog (Settings → MIDI Setup → "Bandwidth test…"; owns its
RtMidiOut, stops playback first, sustains A3 with a breath+bend baseline at
10 ms +600 B/s). Three routes, each with a ladder 1@50ms → 2@25 → 4@12 → 8@6 →
8@3 (1 ms step opt-in), manual mode with streams×interval for boundary probes,
audibility check per parameter, verdict buttons, measured B/s + overrun
counters per step. Message constructors in midioutput.h/.cpp (cc/nrpn/
parameterChange/vlDepthBytes) — reused by the Phase 5 bake. NRPN ships as three
3-byte CCs (RtMidi rejects >3-byte non-SysEx).

MEASURED CEILING (sax patch, custom voices, Patchman Turbo; UR22mkII; DIN leg
caps at 3125 B/s — steps above the cap measure the interface FIFO, not the
module, so only at/below-cap verdicts count):

- CC-assigned element modulation (SysEx assigns CONTROL NO. = CC13–20 + DEPTH
  127 once, then plain 3-byte CCs — the Phase 5 route): 2 streams @ 25 ms
  clean; 4 @ 25 ms very jagged; 4 @ 12 ms audible lag with 0 overruns
  (module-bound, reproduced twice); 8 @ 6 ms lag.
- NRPN part offsets (vibrato rate/depth, filter cutoff/res, EG times): clean
  through 4 streams @ 12 ms (~3.0 kB/s payload). 8 @ 6 and beyond drown in
  transport overruns.
- Direct SysEx DEPTH streaming: audibly verified (growl swells), ladder clean
  through 4 @ 12 ms; deeper steps unjudgeable (FIFO smoothing above the cap).

CONCLUSION: the module's scarce resource is element-parameter updates (physical
model recalculation), not MIDI bytes — part-level NRPN offsets are cheap,
element updates are expensive regardless of message encoding (CC lag at
1.6 kB/s < the DIN cap). Phase 5 budget: stack ~2–3 element curves at
≥ 25 ms per stream (2 @ 25 proven, 3 @ 25 untested, 4 @ 25 jagged), NRPN
part-curves free at up to 4 @ 12 ms concurrently. Per-port intervals are
justified by measurement, not guesswork. Architecture: assign-to-CC once per
voice (SysEx), stream cheap CCs; direct SysEx DEPTH as fallback if CC slots
run out. Voice edit buffer gets dirty from CC-assign/DEPTH runs — restore by
re-selecting the voice or power-cycling.

Phase 5 (independent curves, stacked) — built 2026-08-18, hardware test
pending (the rows rework below supersedes its mechanism before that test
matters much). Nimus picked embouchure + breath noise as the element pair,
plus filter cutoff/resonance as part offsets. The bake assigns
embouchure→CC13 and breathNoise→CC14 by writing SysEx CONTROL NO. + DEPTH
into the edit buffer at play start (restore by re-selecting the voice), then
streams them on the element tier (default 25 ms); filter cutoff/res go out
as NRPN part offsets (01 20 / 01 21, 12 ms); expression CC11 rides the cheap
tier.

Next: the rows rework (patch-owned CC assignments — see Decisions above).
Turn the container into fixed core + parameter rows with the per-row
inspector (active toggle / CC# / resolution ms), replace the hardcoded
CC13/CC14/NRPN bake blocks with data-driven emission, and save
patch-contract sounits named after their patches. Both blockers cleared 2026-08-19: the no-curve semantics are decided (see
Decisions above), and Nimus has given the go-ahead.

Rows rework: built and hardware-tested (see Decisions). 2026-08-20 additions
from a full day of hardware testing: (1) the volume row became toggleable
(default on; off = no dynamics stream, loudness from a breath/expression
curve row or the module's own volume — see Decisions); (2) MIDI dispatch
moved out of the audio callback — playback stuttered because the callback
flushed every due event as blocking midiOutShortMsg bursts (20-40 messages
at note starts). The bake is now serialized into WinMM MIDIEVENT buffers
(midistream.cpp buildMidiEventStream) and played by Windows midiStreamOut
through the configured port, started at the same instant as the transport.
The callback no longer touches MIDI at all; natural-end teardown waits a
100 ms grace (kMidiStreamEndGraceMs) then a queued trackPlaybackEnded →
stopTrackPlayback() runs the stop resets on the main thread. Device
resolved by RtMidi port index == WinMM device ID
(MidiOutput::configuredDeviceIndex). Hardware-tested 2026-08-20 — see the
test result below the correction.

Correction 2026-08-20 (before that hardware test ran): the first cut of
this serialized a format-0 **Standard MIDI File** and handed it to
midiStreamOut. That cannot work and would have read as a dead cable or a
wrong device ID — midiStreamOut does not parse MThd/MTrk, its buffer "contains
one or more MIDI events, each of which is defined by a MIDIEVENT structure"
{dwDeltaTime, dwStreamID, dwEvent}. Five defects fixed at once, all in
midistream.cpp: (1) SMF → MIDIEVENT records, MEVT_SHORTMSG for 1-3 byte
messages, MEVT_LONGMSG + DWORD padding for SysEx; (2) the missing
midiStreamRestart — midiStreamOpen leaves the device **paused**, so nothing
plays without it; (3) dwBytesRecorded was never set (output reads it for how
much to send); (4) the 1 ms tick rate now goes through midiStreamProperty
(MIDIPROP_TIMEDIV 1000 + MIDIPROP_TEMPO 1,000,000) — an embedded FF 51 tempo
meta event means nothing to the stream API, which otherwise defaults to
96 PPQN / 500,000 µs; (5) the device was opened twice (midiOutOpen +
midiStreamOpen on the same ID), which risks MMSYSERR_ALLOCATED on
exclusive-access drivers — now only the stream is opened and HMIDISTRM is
cast to HMIDIOUT for the midiOut* calls. Also new: midiStreamOut caps one
buffer at 64K (~5400 short events, ~27 s of a 2-stream 10 ms bake), so the
stream is split at record boundaries into ≤48 KB chunks, all prepared and
queued before the restart; deltas stay correct across splits because the
stream clock runs continuously through queued buffers. Teardown is
midiStreamStop → midiOutReset (hands back queued buffers) → stop messages →
unprepare each header → midiStreamClose.

HARDWARE TEST 2026-08-20 — passed on all three counts, the stream rework is
done:

- **1 min 35 s piece, no stutter, no drift.** That length crosses several
  48 KB chunk boundaries, so the one part of the design that was reasoning
  from the docs rather than from measurement — deltas carrying across queued
  buffers on a continuous stream clock — is now confirmed empirically. The
  callback-dispatch stutter that motivated the whole rework is gone.
- **Hardware and internal audio in perfect sync.** The same track content
  copied to a second track with a regular Kala sounit, both played together:
  no audible offset over the full length. Confirms the stream clock and the
  transport really do start together (session opened microseconds before
  useTrackPlayback=true) and that the two clocks don't diverge — the risk
  the bake-then-dispatch model was designed around.
- **Maqam Hijaz, intonation perfect.** The pitch-bend path holds for
  microtonal writing, not just 12-TET — the cents-remainder-vs-nearest-MIDI-
  note bend plus per-note RPN range reproduces a non-equal-tempered scale
  accurately on real hardware. Notable because pitch bend was the part of
  this feature with the most hedging in the decisions above (14-bit
  precision ceiling, bend-driving-embouchure on some voices).

Phase 6 (modifier hookup) — done, hardware-tested 2026-08-21. Nimus:
"the vl70m is alive and breathing (literally)".
Modifier containers now drive the VL70-m container's row input ports through
the same connection combinators as the internal graph. Precedence:
connection wins — a connected row streams the graph value; an unconnected
active row keeps the named-curve path (Phase 5 output byte-identical).
SounitGraph grew isMidiPortConnected()/getMidiPortValue(): the value reader
folds the row port's cached connections through isSourceActive +
applyConnectionFunction with base 0.5 (row ports have no static parameter;
0.5 pivots modulate at CC 64 = the row neutral; passthrough maps an Envelope
0-1 to CC 0-127 exactly like the curve path). generateSample's validity
early-return now also lets MIDI-only graphs execute their modifier subgraph.
The bake walks each connected note at audio rate on a cloned graph
(clone + reset(isLegato) + per-sample generateSample mirroring
Track::renderNoteImpl, including the scoreCurveValues assembly), row grid
ticks as sampling points, post-loop flush for the endMs tick, delta-skip and
lastSentBake/end-resets shared with the curve path. The pitch port stays
note-curve + vibrato driven (a connection to it logs one warning per bake).
Known behavior: deactivating a row does not prune its canvas connections
(revive-on-reactivate is silent; sounit reload drops them); mixed
audio+MIDI graphs walk the whole cloned chain (cost parity with the audio
pre-render, timing logged per note). Test sounit per plan: Drift Engine →
damping (modulate, weight ~0.5), Envelope Engine → Physics targetValue →
Physics currentValue → absorption (passthrough), plus a hand-drawn
embouchure curve on the same note for the curve+walk coexistence check.

Test result: the modifier-driven rows breathe organically on the module
(the literal Phase 6 test). Two bugs found and fixed along the way
(2026-08-21, both in the rows rework, first exercised by Phase 6):
(1) `Container::setInputPorts` rebuilt port circles that were never
shown under an already-visible container and positioned them from stale
label geometry — row toggles in the inspector made the ports disappear;
fixed with circle->show(), explicit layout invalidation/activation with
a visibility-dependent order, a deferred reposition, and a
QEvent::LayoutRequest hook in Container::event. (2) The score canvas
"Show curves" dropdown listed all 22 row curves regardless of active
rows; it now follows the active rows live (canvas removeExpressiveCurveName
+ sync in the parameterChanged hook + variation canvases wired into the
score canvas refresh). Removal is non-destructive — drawn curves stay on
the notes and reappear on re-activation. Extended 2026-08-21 to be
connection-aware: rows whose port has an incoming Modifier connection also
leave the selector (connection wins in the bake, so a drawn curve there is
dead), with one exception — an Envelope Engine with followDynamics reading
the row's curve name keeps it listed. Sync fires on parameterChanged and
graphChanged (live on connect/disconnect/undo, correct after sounit load). (3) Crash on play with a Modifier
connection: getMidiPortValue used QMap's CONST operator[] which returns a
ProcessorData BY VALUE — the temporary's destructor deleted the real
processor objects (shallow-copied raw owning pointers), corrupting the
walk. Fixed with constFind (never index a QMap of owning raw pointers in a
const method). Reproduced + verified with a headless repro that drives
bakeMidiEvents directly; confirmed by Nimus 2026-08-21 — no crash, walk
verified from both the sound-engine canvas and the score canvas ("tigedou"). Authoring note confirmed on the way: Drift's
output is a ~1.0 detuning multiplier, so a Drift→row connection saturates
at CC 127 under passthrough or heavy modulate weight — use small modulate
weights (~0.05-0.15) or an Envelope/LFO/Physics source for full-range row
motion.
