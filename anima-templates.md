# Anima Prompt Templates — v0 (paper test)

Copy-paste into the CompanionPanel. Replace `{placeholder}` values.
Written in tool-call lingo so Anima executes in one round, no reasoning.

Goal of this batch: validate whether templates actually shortcut the
composing flow before investing in a slash-palette UI.

---

## 1. Selection filter — pitch range

```
TASK: select notes within a pitch range.
STEPS:
  1. call select_notes with:
       mode="range",
       pitchMinHz={MIN_HZ},
       pitchMaxHz={MAX_HZ}.
REPORT: one line, "selected {matchedCount} notes in {MIN_HZ}–{MAX_HZ} Hz".
```

**Example fill**: `MIN_HZ=300`, `MAX_HZ=1200` (middle register).
For "everything above 300 Hz" use `MAX_HZ=20000`; for "everything below
1200 Hz" use `MIN_HZ=0`.

---

## 2. Selection filter — flat dynamics in bar range

```
TASK: select flat-dynamics notes in a bar range, for expressive shaping.
STEPS:
  1. call select_flat_dynamics_notes (trackIndex = current).
  2. from returned ids, keep only notes whose startTime falls in bars
     {BAR_START}..{BAR_END} (use get_composition_state to resolve bar→ms
     if needed; tempo is on the project).
  3. hold the filtered ids as the working selection.
REPORT: matched count and id list. Do not modify.
```

**Example fill**: `BAR_START=5`, `BAR_END=12`.

---

## 3. Bulk transform — scale dynamics on current selection

```
TASK: scale dynamics of the user's current canvas selection.
STEPS:
  1. call get_selected_notes to fetch the selection.
  2. call scale_dynamics with factor={FACTOR} and those noteIds.
  3. do not verify afterwards. no get_composition_state. no summary of
     what you changed — the undo stack is the source of truth.
REPORT: one line, "scaled {n} notes by {FACTOR}".
```

**Example fill**: `FACTOR=0.7` (30% quieter) or `FACTOR=1.4`.

---

## 4. Bulk transform — bowed-string vibrato on selection

```
TASK: apply bowed-string vibrato to the user's current canvas selection.
STEPS:
  1. call get_selected_notes.
  2. call set_note_vibrato with:
       active=true,
       rate={RATE_HZ},
       pitchDepth={PITCH_DEPTH},
       amplitudeDepth=0.2,
       onset=0.25,
       regularity=0.7,
       envelope=[{"time":0,"value":0},{"time":0.3,"value":0},{"time":0.6,"value":1},{"time":1,"value":1}],
       noteIds=<from step 1>.
REPORT: one line, count only. No verification.
```

**Example fill**: `RATE_HZ=5.5`, `PITCH_DEPTH=0.025`. Envelope is the
late-swell shape typical of bowed strings — hardcoded, not a slot, because
changing it defeats the point of this template.

---

## 5. Multi-step — duplicate, shift, transpose, quieter

Phrase-building recipe: "take this phrase and echo it N beats later, up a
fifth, softer".

```
TASK: echo the current selection as a softer transposed copy.
STEPS:
  1. call get_selected_notes. call this SRC.
  2. compute offsetMs from {BEATS} using current tempo
     (get_composition_state → bpm → ms per beat).
  3. call duplicate_notes(offsetMs=<computed>, pitchRatio={RATIO},
     noteIds=SRC). capture returned noteIds as COPY.
  4. call scale_dynamics(factor={DYN_FACTOR}, noteIds=COPY).
  5. leave COPY as the canvas selection so I can keep editing it.
REPORT: one line, "echoed {n} notes: +{BEATS} beats, ×{RATIO} pitch, ×{DYN_FACTOR} dynamics".
```

**Example fill**: `BEATS=4`, `RATIO=1.5` (fifth up), `DYN_FACTOR=0.75`.

---

## 6. Pattern select — gap mode, preset sequence

Pick notes whose positions are separated by a named number sequence.
"Separated by N notes" means `nextIndex = currentIndex + N + 1` (N notes
between picks). Sequence repeats from the start once exhausted, so short
sequences become repeating patterns.

```
TASK: select notes by gap pattern using preset sequence "{PRESET}".
STEPS:
  1. resolve scope: call get_selected_notes. if selectedCount > 0, scope =
     those ids in their current ordering by startTime. else call
     get_composition_state and scope = all notes of the current track,
     ordered by startTime. let N = len(scope).
  2. sequence table (use verbatim, do not recompute):
       fibonacci   = [1,2,3,5,8,13,21,34,55,89,144,233,377,610,987]
       pi-digits   = [1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9]
       e-digits    = [7,1,8,2,8,1,8,2,8,4,5,9,0,4,5,2,3,5,3,6,0,2,8,7,4,7,1,3,5,2]
       primes      = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71]
       lucas       = [2,1,3,4,7,11,18,29,47,76,123,199,322,521,843]
       tribonacci  = [1,1,2,4,7,13,24,44,81,149,274,504,927]
       triangular  = [1,3,6,10,15,21,28,36,45,55,66,78,91,105,120]
       squares     = [1,4,9,16,25,36,49,64,81,100,121,144,169,196]
     let seq = table[{PRESET}].
  3. build picked indices: start with i = {ANCHOR_INDEX}. append scope[i]
     to picks. for k = 0, 1, 2, …: i = i + seq[k mod len(seq)] + 1. stop
     when i >= N. collect picks[] as noteIds[] (in order).
  4. hold picks as the working selection. 
REPORT: "{PRESET}: picked {len(picks)}/{N} notes from {anchor}" and the
id list. no verification.
```

**Example fill**: `PRESET=fibonacci`, `ANCHOR_INDEX=0`.

---

## 7. Pattern select — index mode, preset sequence

Pick notes at direct positions `seq[0], seq[1], …` (no cumulative walk).
Anchor offset is added to every index. Indices ≥ scope length are
skipped (not wrapped), since wrapping would just repeat the first few.

```
TASK: select notes by index pattern using preset sequence "{PRESET}".
STEPS:
  1.  resolve scope: call get_selected_notes. if selectedCount > 0, scope =
     those ids in their current ordering by startTime. else call
     get_composition_state and scope = all notes of the current track,
     ordered by startTime. let N = len(scope).
    2. sequence table (use verbatim, do not recompute):
       fibonacci   = [1,2,3,5,8,13,21,34,55,89,144,233,377,610,987]
       pi-digits   = [1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9]
       e-digits    = [7,1,8,2,8,1,8,2,8,4,5,9,0,4,5,2,3,5,3,6,0,2,8,7,4,7,1,3,5,2]
       primes      = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71]
       lucas       = [2,1,3,4,7,11,18,29,47,76,123,199,322,521,843]
       tribonacci  = [1,1,2,4,7,13,24,44,81,149,274,504,927]
       triangular  = [1,3,6,10,15,21,28,36,45,55,66,78,91,105,120]
       squares     = [1,4,9,16,25,36,49,64,81,100,121,144,169,196]
     let seq = table[{PRESET}].
  3. picks = [ scope[{ANCHOR_INDEX} + s] for s in seq if
     ({ANCHOR_INDEX} + s) < N ]. duplicates collapsed.
  4. hold picks as the working selection.
REPORT: "{PRESET} (index mode): picked {len(picks)}/{N} notes" and id list.
```

**Example fill**: `PRESET=primes`, `ANCHOR_INDEX=0`.

---

## 8. Pattern select — free-form sequence

For trying sequences that aren't in the preset table, or for pasting a
specific rhythm of skips (e.g. `[1,1,2,1,1,2,1,1,2]` = 3-against-2 feel).
MODE picks gap vs index semantics from templates 6/7.

```
TASK: select notes using MODE={MODE} and SEQUENCE={SEQUENCE}.
STEPS:
  1.  resolve scope: call get_selected_notes. if selectedCount > 0, scope =
     those ids in their current ordering by startTime. else call
     get_composition_state and scope = all notes of the current track,
     ordered by startTime. let N = len(scope).
  2. seq = {SEQUENCE}. reject and stop if seq is empty or contains
     negatives.
  3. if MODE=="gap",  build picked indices: start with i = {ANCHOR_INDEX}. append scope[i]
     to picks. for k = 0, 1, 2, …: i = i + seq[k mod len(seq)] + 1. stop
     when i >= N. collect picks[] as noteIds[] (in order).
     if MODE=="index",picks = [ scope[{ANCHOR_INDEX} + s] for s in seq if
     ({ANCHOR_INDEX} + s) < N ]. duplicates collapsed.
  4. hold picks as the working selection.
REPORT: "MODE={MODE}, SEQUENCE={SEQUENCE}: picked {len(picks)}/{N}" and
id list.
```

**Example fill**: `MODE=gap`, `SEQUENCE=[0,0,2,0,0,2,0,0,3]`.
Zero means "pick the immediately next note" in gap mode.

---

## 9. Pattern select — digits of a number

Treat any integer (birthday, phone number, catalogue number) as its
digit sequence in gap mode. Personal / symbolic selection.

```
TASK: select notes by gap pattern using digits of {NUMBER}.
STEPS:
  1.  resolve scope: call get_selected_notes. if selectedCount > 0, scope =
     those ids in their current ordering by startTime. else call
     get_composition_state and scope = all notes of the current track,
     ordered by startTime. let N = len(scope).
  2. seq = [int(d) for d in str({NUMBER}) if d.isdigit()]. strip sign,
     decimal point, spaces.
  3. apply template 6 step 3 with this seq.
  4. hold picks as the working selection.
REPORT: "digits of {NUMBER} ({seq}): picked {len(picks)}/{N}" and id list.
```

**Example fill**: `NUMBER=19820317` (a date). Leading zeros inside the
number are preserved as gap=0 (pick next note).

---

## 10. Pattern select — modular

Classic "every Nth note", generalised. Select note i if
`(i - OFFSET) mod PERIOD` is in RESIDUES. PERIOD=4, RESIDUES=[0] = every
4th starting at OFFSET. PERIOD=3, RESIDUES=[0,1] = two out of every three.

```
TASK: modular selection. PERIOD={PERIOD}, RESIDUES={RESIDUES}, OFFSET={OFFSET}.
STEPS:
  1.  resolve scope: call get_selected_notes. if selectedCount > 0, scope =
     those ids in their current ordering by startTime. else call
     get_composition_state and scope = all notes of the current track,
     ordered by startTime. let N = len(scope).
  2. picks = [ scope[i] for i in 0..N-1 if
     ((i - {OFFSET}) mod {PERIOD}) in {RESIDUES} ].
  3. hold picks as the working selection.
REPORT: "mod {PERIOD} residues {RESIDUES} off {OFFSET}: picked {len(picks)}/{N}"
and id list.
```

**Example fill**: `PERIOD=5`, `RESIDUES=[0,2]`, `OFFSET=0`.

---

## 11. Invert working selection

Takes whatever ids you're currently holding from a previous template and
returns the complement within the track (or an explicit scope). Useful
after a pattern select to shape "everything except the pattern notes".

```
TASK: invert the current working selection.
STEPS:
  1. take HELD_IDS = the id list from the previous template in this
     conversation. if none, stop and ask.
  2. resolve universe: if {SCOPE}=="track", call get_composition_state
     and take all note ids on the current track. if {SCOPE}=="selection",
     use the selection that was active before the held ids were derived
     (if known — else stop and ask).
  3. picks = universe \ HELD_IDS, preserving order.
  4. hold picks as the new working selection.
REPORT: "inverted: {len(picks)} notes (from universe of {len(universe)})"
and id list.
```

**Example fill**: `SCOPE=track`.

---

## 12. Per-note envelope variation — digit-driven middle point

Take the held selection, apply a named expressive curve to each note,
but perturb the curve's interior point per-note using two consecutive
digits from a sequence. First digit = time perturbation, next = value
perturbation. Deterministic, non-random, "subtle" via `RANGE_*` knobs.

Scope for v0: the curve must be "start + one interior + end" shape (like
"Swell"). Other interior points are left untouched.

```
TASK: apply {CURVE_NAME} to held selection with per-note perturbation
from {PRESET} digits.
STEPS:
  1. let HELD_IDS = id list from the previous template. if none, call
     get_selected_notes and use that. if still empty, stop.
  2. parse {CURVE_JSON}. extract points[] sorted by time. find the
     unique interior point (0 < time < 1). call it (baseT, baseV).
     if more than one interior point, stop and ask.
  3. build digit sequence from {PRESET}:
      
       sequence table (use verbatim, do not recompute):
       fibonacci   = [1,2,3,5,8,13,21,34,55,89,144,233,377,610,987]
       pi-digits   = [1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9]
       e-digits    = [7,1,8,2,8,1,8,2,8,4,5,9,0,4,5,2,3,5,3,6,0,2,8,7,4,7,1,3,5,2]
       primes      = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71]
       lucas       = [2,1,3,4,7,11,18,29,47,76,123,199,322,521,843]
       tribonacci  = [1,1,2,4,7,13,24,44,81,149,274,504,927]
       triangular  = [1,3,6,10,15,21,28,36,45,55,66,78,91,105,120]
       squares     = [1,4,9,16,25,36,49,64,81,100,121,144,169,196]
       let seq = table[{PRESET}].
       or
       seq = [int(d) for d in str({NUMBER}) if d.isdigit()]. strip sign,
       decimal point, spaces.
       - any preset  with values > 9 → flatten each term
         to its decimal digits (e.g. fibonacci → 1,2,3,5,8,1,3,2,1,3,4,…).
       - free-form: accept a digit string "31415…" or [0..9] array.
     each element must end up in 0..9. let seq = result.
  4. build pointsPerNote[]:
       for i, noteId in enumerate(HELD_IDS):
         dT = seq[(2*i)     mod len(seq)]
         dV = seq[(2*i + 1) mod len(seq)]
         newT = clamp(baseT + (dT - 4.5)/9 * {RANGE_T}, 0.05, 0.95)
         newV = clamp(baseV + (dV - 4.5)/9 * {RANGE_V}, 0.0, 1.0)
         points_i = curve points with the interior point replaced by
                    {time: newT, value: newV}; keep the others.
         append points_i to pointsPerNote.
     ONE call: set_dynamics(mode="expressive_batch", name={CURVE_NAME},
                            noteIds=HELD_IDS, pointsPerNote=pointsPerNote).
     → single undo entry reverts every note.
  5. done. no verification, no get_composition_state.
REPORT: "{CURVE_NAME}: varied {len(HELD_IDS)} notes, {PRESET} digits,
Δt ±{RANGE_T/2}, Δv ±{RANGE_V/2}".
```

**Example fill**: `CURVE_NAME="Brightness"`, `PRESET=pi-digits`,
`RANGE_T=0.3`, `RANGE_V=0.4`,
`CURVE_JSON={"points":[{"time":0,"value":0},{"time":0.5,"value":1},{"time":1,"value":0}]}`.

With those: note 0 gets (t=0.5+(1-4.5)/9·0.3, v=1+(4-4.5)/9·0.4) =
(0.383, 0.978). Note 1 gets (0.45, 0.778). Etc. Swell-recognisable,
each note slightly different, pattern reproducible.

---

## 13. Bulk transform — crescendo/decrescendo across selection

Scales dynamics of a selection with a factor that interpolates linearly
from START to END across the notes in startTime order. START=1.0,
END=1.5 = crescendo. START=1.2, END=0.6 = decrescendo. Curve shape on
each note is preserved since scale_dynamics multiplies level.

```
TASK: crescendo/decrescendo across the user's current canvas selection.
STEPS:
  1. call get_selected_notes. order ids by startTime → IDS.
     let N = len(IDS). if N < 2, stop and ask.
  2. for i, id in enumerate(IDS):
       t         = i / (N - 1)
       factor_i  = {FACTOR_START} + ({FACTOR_END} - {FACTOR_START}) * t
       call scale_dynamics(factor=factor_i, noteIds=[id]).
  3. do not verify. no get_composition_state afterwards.
REPORT: one line, "ramped {N} notes: ×{FACTOR_START} → ×{FACTOR_END}".
```

**Example fill**: `FACTOR_START=0.6`, `FACTOR_END=1.4` (classic
crescendo). Produces N undo entries — one per note. A batch
`scale_dynamics` with `factorsPerNote[]` would collapse this to a single
undo; flag as roadmap if the template sees real use.

---

## 14. Per-note vibrato variation — digit-driven rate/depth

Apply vibrato to the held selection with per-note perturbation of rate
and pitchDepth, driven by two consecutive digits from a sequence. Same
structure as template 12 but for vibrato. Deterministic, non-random,
"subtle" via `RANGE_*` knobs.

```
TASK: apply vibrato to held selection with per-note rate/depth
      perturbation from {PRESET} digits.
STEPS:
  1. let HELD_IDS = id list from the previous template. if none, call
     get_selected_notes and use that. if still empty, stop.
  2. build digit sequence from {PRESET}:

       sequence table (use verbatim, do not recompute):
       fibonacci   = [1,2,3,5,8,13,21,34,55,89,144,233,377,610,987]
       pi-digits   = [1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9]
       e-digits    = [7,1,8,2,8,1,8,2,8,4,5,9,0,4,5,2,3,5,3,6,0,2,8,7,4,7,1,3,5,2]
       primes      = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71]
       lucas       = [2,1,3,4,7,11,18,29,47,76,123,199,322,521,843]
       tribonacci  = [1,1,2,4,7,13,24,44,81,149,274,504,927]
       triangular  = [1,3,6,10,15,21,28,36,45,55,66,78,91,105,120]
       squares     = [1,4,9,16,25,36,49,64,81,100,121,144,169,196]
       let seq = table[{PRESET}].
       or
       seq = [int(d) for d in str({NUMBER}) if d.isdigit()]. strip sign,
       decimal point, spaces.
       - any preset with values > 9 → flatten each term to its decimal
         digits (e.g. fibonacci → 1,2,3,5,8,1,3,2,1,3,4,…).
       - free-form: accept a digit string "31415…" or [0..9] array.
     each element must end up in 0..9. let seq = result.
  3. for i, noteId in enumerate(HELD_IDS):
       dR = seq[(2*i)     mod len(seq)]
       dD = seq[(2*i + 1) mod len(seq)]
       rate_i  = clamp({BASE_RATE}        + (dR - 4.5)/9 * {RANGE_RATE},        0.5, 12.0)
       depth_i = clamp({BASE_PITCH_DEPTH} + (dD - 4.5)/9 * {RANGE_PITCH_DEPTH}, 0.0, 0.1)
       call set_note_vibrato with:
         active=true, rate=rate_i, pitchDepth=depth_i,
         amplitudeDepth=0.2, onset=0.25, regularity=0.7,
         envelope=[{"time":0,"value":0},{"time":0.3,"value":0},{"time":0.6,"value":1},{"time":1,"value":1}],
         noteIds=[noteId].
  4. do not verify.
REPORT: "vibrato varied {len(HELD_IDS)} notes, {PRESET} digits,
Δrate ±{RANGE_RATE/2}, Δdepth ±{RANGE_PITCH_DEPTH/2}".
```

**Example fill**: `PRESET=e-digits`, `BASE_RATE=5.5`, `RANGE_RATE=1.5`,
`BASE_PITCH_DEPTH=0.025`, `RANGE_PITCH_DEPTH=0.015`. Envelope is the
bowed late-swell shape (hardcoded, same rationale as template 4).
Produces N undo entries — a batch `set_note_vibrato` with
`paramsPerNote[]` would be the proper solution.

---

## 15. Harmonize — stack an interval onto current selection

Duplicate the selection in place (no time shift), transposed by a pitch
ratio. Use for octave doublings, fifths, third-stacks. Multiple passes
give full chord voicings.

```
TASK: harmonize the user's current canvas selection at pitch ratio {RATIO}.
STEPS:
  1. call get_selected_notes. call this SRC.
  2. call duplicate_notes(offsetMs=0, pitchRatio={RATIO}, noteIds=SRC).
     capture returned ids as COPY.
  3. leave COPY as the canvas selection so I can keep harmonising.
REPORT: one line, "harmonized {n} notes at ×{RATIO}".
```

**Example fill**: `RATIO=1.5` (perfect fifth up), `RATIO=2.0` (octave
up), `RATIO=0.5` (octave down), `RATIO=1.25` (major third, just).
Run in sequence for triads: first ×1.25, reselect SRC, then ×1.5.

---

## 16. Retrograde — reverse pitch order on selection

Keep timings untouched but reverse the sequence of pitches across the
selection: note 0 takes note N-1's pitch, note 1 takes note N-2's, and
so on. Rhythm preserved, melody mirrored.

```
TASK: retrograde (pitch-reverse) the user's current canvas selection.
STEPS:
  1. call get_selected_notes. order by startTime → IDS, each with its
     pitchHz. let N = len(IDS). if N < 2, stop.
  2. for i in 0..N-1:
       target_pitch = IDS[N - 1 - i].pitchHz
       if target_pitch == IDS[i].pitchHz: continue  (skip no-ops)
       ratio_i = target_pitch / IDS[i].pitchHz
       call transpose_notes(ratio=ratio_i, noteIds=[IDS[i].id]).
  3. do not verify.
REPORT: one line, "retrograded {N} notes".
```

**Example fill**: no placeholders — this template takes no args.
Produces up to N undo entries (fewer when pitches repeat). A batch
`transpose_notes` with `ratiosPerNote[]` would collapse to one.

---

## 17. Pattern select — beat positions within bar

Select notes whose startTime lands on specific beats of the bar
(1-indexed: beat 1 is the downbeat). `WINDOW_MS` catches notes slightly
off the grid so swung or humanised phrasing still gets picked up.

```
TASK: select notes on beats {BEATS} of the bar.
STEPS:
  1. call get_composition_state. read bpm and timeSig numerator as
     beatsPerBar. let msPerBeat = 60000 / bpm,
     barMs = beatsPerBar * msPerBeat.
  2. resolve scope: call get_selected_notes. if selectedCount > 0,
     scope = those ids in startTime order. else scope = all notes of
     the current track in startTime order.
  3. for each note in scope:
       posInBar  = note.startTimeMs mod barMs
       beatIndex = round(posInBar / msPerBeat) mod beatsPerBar
       beat      = beatIndex + 1
       offGrid   = abs(posInBar - beatIndex * msPerBeat)
       keep if beat ∈ {BEATS} and offGrid <= {WINDOW_MS}.
  4. hold picks as the working selection.
REPORT: "beats {BEATS} (±{WINDOW_MS}ms): picked {len(picks)}/{scope_N}"
and id list.
```

**Example fill**: `BEATS=[1,3]`, `WINDOW_MS=30` (strong beats in 4/4).
`BEATS=[2,4]` for backbeat feel, `BEATS=[1]` for downbeats only.
Compound meters (6/8, 12/8) aren't special-cased — beatsPerBar is the
time-sig numerator, so "beats" here means eighth-note pulses in 6/8.

---

## 18. Multi-step — rhythmic echo tail

Produce {COPIES} copies of the current selection, each offset {BEATS}
beats later than the previous and softer by factor {DECAY}. Builds a
decaying echo trail, last copy left selected so you can keep editing
it.

```
TASK: echo the current selection {COPIES} times with decay.
STEPS:
  1. call get_selected_notes. call this SRC.
  2. call get_composition_state → bpm. let msPerBeat = 60000 / bpm,
     step = {BEATS} * msPerBeat.
  3. for k = 1..{COPIES}:
       call duplicate_notes(offsetMs=k*step, pitchRatio=1.0, noteIds=SRC).
       capture returned ids as COPY_k.
       call scale_dynamics(factor={DECAY}^k, noteIds=COPY_k).
  4. leave COPY_{COPIES} (the last, quietest echo) as the canvas selection.
REPORT: one line, "echoed {n} notes ×{COPIES}: step {BEATS} beats, decay {DECAY}".
```

**Example fill**: `COPIES=3`, `BEATS=2`, `DECAY=0.6` (three echoes,
each 2 beats after the previous, at 60/36/22% level). Produces
2×{COPIES} undo entries — one duplicate + one scale per copy.

---

## 19. Selection filter — duration range

```
TASK: select notes within a duration range.
STEPS:
  1. call select_notes with:
       mode="range",
       durationMinMs={MIN_MS},
       durationMaxMs={MAX_MS}.
REPORT: one line, "selected {matchedCount} notes in {MIN_MS}–{MAX_MS} ms".
```

**Example fill**: `MIN_MS=500`, `MAX_MS=100000` (long / sustained notes).
For staccato use `MIN_MS=0`, `MAX_MS=150`.

---

## 20. Selection filter — pitch + duration range

```
TASK: select notes within both a pitch range and a duration range.
STEPS:
  1. call select_notes with:
       mode="range",
       pitchMinHz={MIN_HZ},
       pitchMaxHz={MAX_HZ},
       durationMinMs={MIN_MS},
       durationMaxMs={MAX_MS}.
REPORT: one line,
"selected {matchedCount} notes: {MIN_HZ}–{MAX_HZ} Hz, {MIN_MS}–{MAX_MS} ms".
```

**Example fill**: `MIN_HZ=300`, `MAX_HZ=1200`, `MIN_MS=500`,
`MAX_MS=100000` (long notes in the middle register). Use extreme values
(0 or 20000 / 100000) on a slot to make it effectively unbounded.

---

## 21. Sounit build — 10-Band EQ with per-band score-curve envelopes

Adds a 10-Band EQ to the current canvas, plus 10 Envelope Engines wired
one-per-band. Each envelope is set to `followDynamics=1`, given the
score-curve name `band 1` … `band 10`, and renamed to match. After the
template finishes, painting a "band 3" curve on a note (e.g. via
`set_dynamics` mode `expressive`) drives the gain of EQ band 3 — full
per-band spectral shaping with no inspector clicks.

```
TASK: build a 10-Band EQ with 10 score-curve-driven envelopes, one per band.
STEPS:
  1. call edit_graph(action:"add", type:"10-Band EQ"). capture returned
     instance name as EQ.
  2. for i = 1..10:
       a. let NAME = "band " + i.
       b. call edit_graph(action:"add", type:"Envelope Engine",
                          params:{followDynamics:1.0,
                                  scoreCurveName:NAME}).
          capture returned instance name as TMP.
          NOTE: scoreCurveName is a string param — pass it as a string
          value, not a number. The tool routes string values to
          setStringParameter automatically.
       c. call edit_graph(action:"rename", instanceName:TMP,
                          newName:NAME).
       d. call edit_graph(action:"connect",
                          fromInstance:NAME, fromPort:"envelopeValue",
                          toInstance:EQ, toPort:"band" + i).
          (default function "passthrough" is correct — band gain becomes
          the score-curve value 0..1.)
  3. do not call get_graph_state at any point. the names from step 1 and
     step 2b are authoritative — use them directly.
REPORT: one line, "added 10-Band EQ ({EQ}) + 10 score-curve envelopes
band 1..10".
```

**Example fill**: no placeholders — this template takes no args. Run
once per sounit. To shape per band, target the named curve on a note,
e.g. `set_dynamics(mode:"expressive", name:"band 3", points:[...])`.

---

## Notes for the paper test

- If a template makes you think "I'd reword that" — flag it, don't reword silently.
- If you reach for a template and realise it doesn't exist, jot what it
  would have been. That's the roadmap for v1.
- Watch for templates that Anima executes wrong despite the explicit
  steps — that's a sign the lingo isn't compact enough, or a tool is
  missing (e.g. no duration filter on select_notes).
- After a few sessions, decide: build the slash palette, iterate on
  template content, or add missing tools.
