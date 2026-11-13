# Kala File Format (.kala)

**Audience:** a coding agent building a separate app that must emit files Kala can open.
**Status:** describes the format as of 2026-11-13. The C++ source is authoritative — every
section cites the `file:line` that reads the field. If this document and the code disagree,
the code wins; update this document.

---

## 1. Mental model: two independent layers

A project is **composition data** plus an **optional synthesis graph per track**.

```
foo.kala                     ← this document. Tracks, notes, scale, tempo.
foo.kalarender               ← OPTIONAL binary render cache. Never write it.
foo-<instrument>.sounit      ← OPTIONAL synthesis graph, one file per track.
                                Documented in Claude-sounit-builder.md.
```

Key consequences for a writer:

- **A `.kala` can be notes-only.** `sounitFilePath: ""` is valid, and it is exactly what a
  brand-new project looks like (`KalaMain::newProject`, `kalamain.cpp:4901`). The score shows
  and edits the notes; the track is simply silent until the user loads a sounit into it later.
  This is the recommended first target.
- **Note timing and pitch are absolute.** `startTime`/`duration` are milliseconds and
  `pitchHz` is hertz. Tempo and scale do **not** transform them on load. They only draw the
  editing grid, name the lines, and drive bar/beat display. A writer never needs to integrate
  a tempo map to place audio correctly.
- **The graph is not embedded in the project.** Save it to a separate `.sounit` and point
  `sounitFilePath` at it (§10).

---

## 2. Encoding and units

| Thing | Rule |
|---|---|
| Encoding | UTF-8 JSON, one object at the root |
| Indentation | Kala writes 4-space indented; anything valid parses |
| Time | milliseconds, `double`, absolute from piece start |
| Pitch | hertz, `double` (A4 = 440.0). No MIDI note numbers anywhere |
| Score pitch range | the score draws **20–8000 Hz** (`scorecanvas.h:482`). Notes outside it are valid, load, and sound normally — they simply draw off-screen and cannot be edited with the mouse until scrolled/zoomed into view |
| Curve time | normalized 0.0–1.0 **within the note** (or within the phrase for `legatoCurve`) |
| Normalized values | 0.0–1.0 unless stated otherwise |
| Color | `"#rrggbb"` |
| IDs | unique strings; UUIDs in practice |
| Numbers | **finite only.** JSON has no `NaN` or `Infinity`; emitting one makes the whole file unparseable. Substitutes: `0`, or omit the key |
| File name | Kala's open dialog filters `*.kala`. The loader does not check the extension, but name files `.kala` so they open normally |

---

## 3. Top-level object

```json
{
    "version": 1,
    "fileType": "kala-project",
    "compositionSettings": { ... },
    "containerSettings": { ... },
    "scaleSettings": { ... },
    "tempoSettings": { ... },
    "trackCanvasStates": { ... },
    "tracks": [ ... ],
    "notes": [ ... ]
}
```

| Key | Type | Required | If absent |
|---|---|---|---|
| `version` | int | Written as `1`, never read | — |
| `fileType` | string | **Yes.** Must equal `"kala-project"` or load is refused | Load error dialog |
| `compositionSettings` | object | Strongly recommended | Kala keeps the previous project's settings |
| `containerSettings` | object | No | User's editor preferences are kept |
| `scaleSettings` | object | Strongly recommended | Kala keeps the previous project's scale |
| `tempoSettings` | object | Strongly recommended | Kala keeps the previous project's tempo |
| `trackCanvasStates` | object | No | See §10.3 — do not use this |
| `tracks` | array | No; empty → one default track | One empty `"Track 1"` |
| `notes` | array | No | No notes |

Every key inside every object is technically optional — Qt's `toDouble(default)` style
fallbacks supply the value. But **omitting top-level settings objects leaks state from the
previously open project**, because the loader only applies them when present. Always write
`compositionSettings`, `scaleSettings`, and `tempoSettings`.

`containerSettings` holds the sounit editor's per-container parameter min/max ranges — user
preferences, not composition data. Omit it; Kala will use the user's own.

Validation is minimal: the loader checks JSON parseability and `fileType`. Everything else is
migrated or defaulted silently (`KalaMain::loadProject`, `kalamain.cpp:5557`).

---

## 4. `compositionSettings`

Written by `CompositionSettings::toJson` (`compositionsettings.cpp:51`), read by
`fromJson` (`compositionsettings.cpp:69`).

| Key | Type | Default on load | Meaning |
|---|---|---|---|
| `name` | string | `"Untitled"` | Composition title, shown in title bar and exports |
| `timeMode` | string | missing → `"Absolute"` | `"Musical"` or `"Absolute"`. **Write `"Musical"`.** Anything else becomes Absolute. Musical = bars:beats display, Absolute = min:sec |
| `tempo` | int | `60` | BPM. One "beat" is one pulse in Kala mode, one denominator-note in Western mode |
| `referenceTempo` | int | `tempo` | The BPM the note times were authored at. Only used by the UI when the user *changes* tempo (it rescales note times). For a writer: set it equal to `tempo` |
| `timeSigTop` | int | `5` | Pulses (Kala) or beats (Western) per bar. 1–99 |
| `timeSigBottom` | int | `4` | `0` = Kala pulse mode (no denominator). `4`, `8`, … = Western notation. Display only; never scales bar duration |
| `lengthMs` | double | `300000` | Declared piece length in ms. **This is the authority** |
| `lengthBars` | int | derived from `lengthMs` | Declared length in bars. Honoured as saved even if it disagrees; keep the two consistent |
| `sampleRate` | int | `48000` | Project audio rate. 44100 / 48000 / 96000 … |
| `bitDepth` | int | `24` | Export bit depth: 16 / 24 / 32 |
| `preRenderDelayMs` | int | `400` | Background render debounce. 50–2000 |
| `autoRender` | bool | `true` | Render on edit vs only on play |

**Bar arithmetic** (same everywhere in Kala — `scorecanvas.cpp:1230`, `timeline.cpp:114`):

```
pulseMs = 60000 / tempo
barMs   = pulseMs * timeSigTop
```

`timeSigBottom` never enters the duration math. At `tempo: 120`, `timeSigTop: 5`:
pulse = 500 ms, bar = 2500 ms.

On export, the rendered length is `max(lengthMs, end of rendered content)`
(`kalamain.cpp:5259`), so a declared length shorter than the notes is not a crash — it is
extended.

---

## 5. `scaleSettings`

Written at `kalamain.cpp:5473`, read at `kalamain.cpp:5623`.

```json
"scaleSettings": {
    "defaultScale": { "scaleId": 2, "name": "Equal Temperament", "tonicIndex": 0 },
    "baseFrequency": 16.3516,
    "scaleChanges": [
        { "timeMs": 0,    "scale": { ... }, "baseFrequency": 16.3516 },
        { "timeMs": 8000, "scale": { ... }, "baseFrequency": 16.3516 }
    ]
}
```

| Key | Type | Default | Meaning |
|---|---|---|---|
| `defaultScale` | object | `scaleId 0` | Scale used from time 0 until the first change |
| `baseFrequency` | double | `25.0` | Frequency of degree 0 of octave 0. Sets where the grid lines sit |
| `scaleChanges` | array | `[]` | Modulations. Each `{timeMs, scale, baseFrequency}` |

### 5.1 Scale object

`Scale::toJson` (`scale.cpp:1349`), `Scale::fromJson` (`scale.cpp:1371`).

| Key | Type | Default on load | Meaning |
|---|---|---|---|
| `scaleId` | int | `0` | **The only key that selects the scale.** Index into the hard-coded table in §5.2 |
| `name` | string | ignored | Human readability only |
| `tonicIndex` | int | omitted → none | Reference degree: the ET key (0 = C, 1 = C♯, …) or the tonic degree of a maqam/raga/JI scale. Applied only if within that scale's degree count |
| `centOffsets` | array of double | none | Per-degree microtonal inflection, ±50 cents. One entry per degree. Written only when any offset is non-zero |

Note names and grid positions come from `baseFrequency * 2^octave * ratio[degree]`
(`scorecanvas.cpp:531`). None of this affects audio — it is a drawing/edit grid over
absolute-Hz notes. If your app emits `pitchHz` directly, pick `scaleId: 2` (ET) and a
`baseFrequency` that makes the grid land where you expect (16.3516 = C0 for ET), or simply
reuse another project's `scaleSettings` verbatim.

### 5.2 Scale IDs

Do **not** invent IDs; unknown IDs silently fall back to Just Intonation. Full table at
`scale.cpp:1257`.

| ID | Name | ID | Name |
|---|---|---|---|
| 0 | Just Intonation | 29 | Maqam Athar Kurd |
| 1 | Pythagorean | 30 | Persian Shur |
| 2 | Equal Temperament | 31 | Persian Mahur |
| 3 | Quarter-Comma Meantone | 32 | Persian Segah |
| 4 | Maqam Rast | 33 | Persian Chahargah |
| 5 | Maqam Bayati | 34 | Persian Homayun |
| 6 | Maqam Saba | 35 | Ionian (Major) |
| 7 | Maqam Hijaz | 36 | Dorian |
| 8 | Maqam Nahawand | 37 | Phrygian |
| 9 | Maqam Kurd | 38 | Lydian |
| 10 | Raga Bhairav | 39 | Mixolydian |
| 11 | Raga Yaman | 40 | Aeolian (Natural Minor) |
| 12 | Raga Todi | 41 | Locrian |
| 13 | Raga Bhairavi | 42 | Melodic Minor |
| 14 | Raga Shankarabharana | 43 | Dorian b2 |
| 15 | Raga Mayamalavagowla | 44 | Lydian Augmented |
| 16 | Pelog | 45 | Lydian Dominant |
| 17 | Slendro | 46 | Mixolydian b6 |
| 18 | Chinese Pentatonic | 47 | Locrian #2 |
| 19 | Hirajoshi | 48 | Altered (Super Locrian) |
| 20 | Whole Tone | 49 | Maqam Rast (5-Limit JI) |
| 21 | Half Diminished | 50 | Maqam Bayati (5-Limit JI) |
| 22 | Raga Kafi | 51 | Maqam Saba (5-Limit JI) |
| 23 | Raga Asavari | 52 | Maqam Hijaz (5-Limit JI) |
| 24 | Raga Khamaj | 53 | Maqam Nahawand (5-Limit JI) |
| 25 | Raga Marwa | 54 | Maqam Kurd (5-Limit JI) |
| 26 | Raga Purvi | 55 | Maqam Sikah (5-Limit JI) |
| 27 | Maqam Sikah | 56 | Maqam Ajam (5-Limit JI) |
| 28 | Maqam Ajam | 57 | Maqam Athar Kurd (5-Limit JI) |

`centOffsets` exist because `scaleId` selects a fixed ratio table. To retune individual
degrees without inventing a new table, add offsets.

---

## 6. `tempoSettings`

Written at `kalamain.cpp:5491`, read at `kalamain.cpp:5658`. **These are separate fields from
`compositionSettings` and must be kept consistent with them.**

```json
"tempoSettings": {
    "defaultTempo": 120,
    "defaultTimeSigNum": 5,
    "defaultTimeSigDenom": 0,
    "tempoChanges": [
        { "timeMs": 10000, "tempoTimeSignature": {
              "bpm": 140, "timeSigNumerator": 7,
              "timeSigDenominator": 0, "gradualTransition": false } }
    ]
}
```

| Key | Type | Default on load | Meaning |
|---|---|---|---|
| `defaultTempo` | double | **`60`** | Tempo from time 0 to the first change |
| `defaultTimeSigNum` | int | `4` | Pulses/beats per bar from time 0 |
| `defaultTimeSigDenom` | int | `4` | `0` = Kala mode; otherwise Western denominator |
| `tempoChanges` | array | `[]` | Markers. `timeMs` is absolute ms |

`tempoTimeSignature` object (`tempotimesignature.cpp:3`):

| Key | Type | Default |
|---|---|---|
| `bpm` | double | `120` |
| `timeSigNumerator` | int | `4` |
| `timeSigDenominator` | int | `4` |
| `gradualTransition` | bool | `false` — when true, BPM ramps linearly to the next marker |

Keep `compositionSettings.tempo == tempoSettings.defaultTempo`,
`timeSigTop == defaultTimeSigNum`, and `timeSigBottom == defaultTimeSigDenom`. They are
stored twice and the UI reads both.

---

## 7. `tracks`

Array of track objects, index = the `trackIndex` notes refer to. Written by `Track::toJson`
(`track.cpp:1178`), read by `fromJson` (`track.cpp:1235`).

### 7.1 Track object

| Key | Type | Default | Meaning |
|---|---|---|---|
| `trackId` | int | existing | Identifier. Give unique ints (0, 1, 2 …) |
| `name` | string | `"Track N"` | Display name |
| `color` | string | `"#3498db"` | Track color |
| `sounitFilePath` | string | `""` | Path to the external `.sounit`. Empty = no graph (§10) |
| `sounitName` | string | `""` | Instrument name. Overrides the name inside the sounit file |
| `muted` | bool | `false` | |
| `solo` | bool | `false` | |
| `volume` | double | `1.0` | Mixer fader, clamped 0.0–1.0 |
| `gain` | double | `1.0` | Make-up gain, clamped 0.0–8.0 |
| `pan` | double | `0.0` | −1.0 left … +1.0 right |
| `notes` | array | `[]` | A copy of this track's notes. See §8 note below |
| `variations` | array | `[]` | Alternative graphs (§10.2) |
| `sampleRate` | double | `44100` | Rate the graphs were compiled at |
| `graphHash` | int64 | `0` | Render-cache key. Wrong value only costs a re-render |
| `clip` | object | omitted | Phase-9 bake/VL-freeze audio clip. **Omit it** |

### 7.2 Notes are stored twice

Kala writes every note both in the top-level `notes[]` and in its track's `notes[]`.
**The top-level array is authoritative**: on load it becomes the score (`Phrase`), and the
track's copy is overwritten from it whenever playback or background rendering starts
(`scorecanvaswindow.cpp:1926`, `scorecanvaswindow.cpp:2830`).

For a writer: emit notes in the top-level `notes[]` only. Omitting per-track `notes` is safe
and avoids the two copies drifting apart.

### 7.3 `clip` — attach a WAV to a track

A track may carry one WAV file alongside its notes. Kala mixes the clip and the track's
rendered notes together, in both playback and export. This is the only way to carry a source
recording inside a project, which makes it useful for a transcription: the original can be
heard against the generated notes. Omit the key entirely for a normal track.

```json
"clip": { "filePath": "C:/audio/bird.wav", "offsetMs": 0, "onsetMs": 0,
          "manualNudgeMs": 0, "durationMs": 4000, "gain": 1.0,
          "enabled": true, "liveMidi": false }
```

| Key | Type | Default | Meaning |
|---|---|---|---|
| `filePath` | string | — | WAV on disk. Mono or stereo, any sample rate (resampled to the track rate). PCM/float WAV only — no MP3/FLAC |
| `offsetMs` | double | `0` | Timeline position in ms where the clip's first retained sample plays |
| `onsetMs` | double | `0` | Leading audio in the file to discard at load. After trimming, sample 0 lands at `offsetMs` |
| `manualNudgeMs` | double | `0` | Fine trim added to `offsetMs` (positive = later) |
| `durationMs` | double | `0` | Declared clip length. Extends the track's content end; playback itself stops at the WAV's real length |
| `gain` | double | `1.0` | Clip gain, applied before track volume/gain |
| `enabled` | bool | `true` | `false` mutes the clip only; the notes keep playing |
| `liveMidi` | bool | `false` | Phase-9 freeze override; leave `false` |

The WAV is read when the project loads. A missing or unreadable file logs a warning and leaves
the clip silent — the project still opens. Put the recording on its own track if it should be
muted or soloed independently of the transcription; put it on the same track if it should
always play with the notes.

---

## 8. `notes` — the composition

Written by `Note::toJson` (`note.cpp:386`), read by `fromJson` (`note.cpp:458`).
This array is what the score displays and edits.

| Key | Type | Default | Meaning |
|---|---|---|---|
| `id` | string | random UUID | Must be unique across the project. Duplicate IDs corrupt editing |
| `startTime` | double | `0` | ms from piece start |
| `duration` | double | `1000` | ms |
| `pitchHz` | double | `440` | Base pitch, used when `pitchCurve` is empty |
| `trackIndex` | int | `0` | Which `tracks[]` entry renders it |
| `variationIndex` | int | `0` | `0` = base graph; `n` = `tracks[i].variations[n-1]` (1-based) |
| `quantized` | bool | `false` | Editor flag: continuous note was snapped to scale |
| `legato` | bool | `false` | Starts smoothly from the previous note, no re-attack |
| `linkedAsLegato` | bool | `false` | Group/visual flag set by "Link as Legato". Audio uses `legato` |
| `pitchCurve` | Curve | empty | Pitch over time in **Hz**. Empty → constant `pitchHz`. Multiply for glissando |
| `dynamicsCurve` | Curve | empty | Loudness 0–1. **Must be non-empty or the note is silent** (see §8.2) |
| `bottomCurve` | Curve | empty | Visual bottom edge of the note blob. No audio effect |
| `legatoCurve` | Curve | omitted | Phrase-wide dynamics shape, 0–1 across the whole legato phrase. Omit unless reproducing a linked phrase |
| `vibrato` | Vibrato | defaults | See §8.3 |
| `segments` | array | omitted | Editor metadata for quantized notes. No audio effect |
| `expressiveCurves` | array | omitted | Named extra curves, `[{"name": ..., "curve": ...}]` |
| `envelopeControlPoints` | object | omitted | Editor shape metadata, keyed by curve name |

Notes on the same track are independent: overlaps are legal and their rendered audio is summed
(polyphony). The order of `notes[]` carries no meaning — emit it sorted by `startTime` for
readability, but Kala does not require it. Negative `startTime` is not meaningful; use 0.

### 8.1 Curve

`Curve::toJson` (`curve.cpp:146`):

```json
"pitchCurve": {
    "points": [
        { "time": 0.0, "value": 261.63, "pressure": 1.0 },
        { "time": 1.0, "value": 392.00, "pressure": 1.0 }
    ]
}
```

| Field | Type | Meaning |
|---|---|---|
| `time` | double | 0.0 = note start, 1.0 = note end. Clamped, interpolated linearly |
| `value` | double | Parameter value (Hz for pitch, 0–1 for dynamics/other curves) |
| `pressure` | double | Pen pressure 0–1. **Cosmetic metadata; never read for audio.** Emit `1.0` |

One point is enough (constant). Zero points means "empty curve".

### 8.2 Two silent-note traps

1. **Empty `dynamicsCurve` renders at 0.0**, because `Curve::valueAt` returns 0.0 for an empty
   curve (`curve.cpp:45`) and the renderer samples it directly
   (`audioengine.cpp:1033`). Always emit at least
   `{"points":[{"time":0,"value":0.7,"pressure":1},{"time":1,"value":0.7,"pressure":1}]}`.
2. **A track with no graph is silent** even with valid notes. If you want sound on first
   open, you must supply a sounit (§10).

### 8.3 Vibrato

`Vibrato::toJson` (`vibrato.h:110`). Omit the whole object for no vibrato.

| Key | Type | Default | Meaning |
|---|---|---|---|
| `active` | bool | `false` | |
| `rate` | double | `5.0` | Hz, typically 3–8 |
| `pitchDepth` | double | `0.02` | Fractional pitch deviation (0.02 = 2%) |
| `amplitudeDepth` | double | `0.3` | 0–1 |
| `onset` | double | `0.2` | Fraction into the note where vibrato begins |
| `regularity` | double | `0.5` | 0 = mechanical, 1 = organic variation |
| `envelope` | array | `[(0,0),(1,1)]` | Intensity over the note: `{time, value, curveType}` |
| `rateEnvelope` | array | `[(0,0),(1,1)]` | Rate multiplier over the note |

`curveType`: `0` linear, `1` smooth (cosine), `2` step. Same shape for
`envelopeControlPoints` values and for `customEnvelope` points.

### 8.4 Segment

Editor metadata produced when a continuous note is quantized. Only meaningful alongside
`quantized: true` and a stepped `pitchCurve`. Safe to omit.

```json
{ "startTime": 0.0, "endTime": 1.0, "pitchHz": 261.63,
  "variationIndex": -1, "useNoteVibrato": true }
```

`time` values are normalized within the note. `variationIndex: -1` means inherit the note's.
When `useNoteVibrato` is false, an embedded `vibrato` object is also written.

### 8.5 Extra expressive curves

`expressiveCurves` are named 0–1 curves in addition to dynamics. An Envelope Engine
container in the graph can follow one by setting its `scoreCurveName` string parameter to
the same name (`audioengine.cpp:1092`). Without such a container they are inert but are
preserved and displayed.

```json
"expressiveCurves": [
    { "name": "Brightness", "curve": { "points": [ ... ] } }
]
```

`envelopeControlPoints` stores the editor's original control-point shape per curve name
(`"Dynamics"` for the dynamics curve) so the curve editor can reload exact points. Purely
editorial.

---

## 9. How the pieces interact at load

`KalaMain::loadProject` (`kalamain.cpp:5557`), in order:

1. Parse JSON, check `fileType == "kala-project"`.
2. Apply `compositionSettings` → sets project sample rate and length.
3. Apply `scaleSettings` → default scale, modulations, timeline markers.
4. Apply `tempoSettings` → default tempo/time signature, markers.
5. Create tracks from `tracks[]`. For each track with a non-empty `sounitFilePath`, load that
   `.sounit` into the track's canvas. A missing/unreadable sounit logs a warning and leaves
   the canvas empty — **the project still opens**.
6. Restore `trackCanvasStates["0"]` into track 0's canvas, if present (§10.3).
7. Load top-level `notes[]` into the score phrase.
8. Optionally load `.kalarender`; otherwise everything is re-rendered on demand.

Rendering after that: notes are grouped by `trackIndex`, pushed to their tracks, and each
note is rendered through its track's base graph or `variationIndex` graph.

---

## 10. The optional synthesis graph

If the project should make sound on first open, write one `.sounit` per track and reference
it by path.

### 10.1 `.sounit` files

The format is documented in `Claude-sounit-builder.md`. Root shape:

```json
{
    "sounit": { "name": "...", "version": "1.0", "created": "...",
                "comment": "...", "expressiveCurveNames": [ ... ] },
    "containers": [ ... ],
    "connections": [ ... ]
}
```

Set `tracks[i].sounitFilePath` to that file's path. The path is passed straight to
`QFile::open` — there is no project-relative resolution, so use an absolute path or a path
valid from Kala's working directory. `tracks[i].sounitName` overrides the name stored in the
file (this is how user renames survive).

### 10.2 Variations

Per-track alternative graphs. `variationIndex` on a note selects one (1-based).

```json
"variations": [
    { "name": "Sul ponticello",
      "graphData": { "containers": [ ... ], "connections": [ ... ],
                     "expressiveCurveNames": [ ... ] },
      "graphHash": 0 }
]
```

`graphData` is the same container/connection arrays as a `.sounit`, minus the `sounit`
metadata object. `isInternal` (optional, only written when true) marks tool-generated
variations; omit it. Notes with `variationIndex: 1` use `variations[0]`, and so on.

### 10.3 `trackCanvasStates` — do not rely on it

```json
"trackCanvasStates": { "0": { "sounit": {...}, "containers": [...], "connections": [...] } }
```

This is a UI snapshot of the canvas that was open when the project was saved. The loader
applies **only key `"0"`**, and only to track 0 (`kalamain.cpp:5768`). It is not how a track's
graph is normally restored — `sounitFilePath` is.

If you absolutely need a self-contained single-track project with no external file, you can
embed the graph here, but note the container encoding differs slightly from `.sounit`:
custom Harmonic Generator DNA must be written as ordinary numeric parameters
(`dnaSelect: -1`, `customDnaCount: N`, `customDna_0`…`customDna_N`) plus a separate
`customDnaName` string, not as a `customDna` object.

Valid container `type` strings (from `Canvas::getPortsForContainerType`, `canvas.cpp:819`):

```
Harmonic Generator   Spectrum to Signal   Rolloff Processor   Formant Body
Breath Turbulence    Noise Color Filter   Physics System      Easing Applicator
Envelope Engine      Drift Engine         Gate Processor      Karplus Strong
Attack               LFO                  Frequency Mapper    Note Tail
Signal Mixer         Spectrum Blender     Wavetable Synth     10-Band EQ
Comb Filter          LP/HP Filter         IR Convolution      Recorder
Flute                singlePluck          doublePluck         Piano
Bass                 Tibetan Bowl         Bowed               Reed
Clarinet             Percussion           Pan                 VL70-m
```

Legacy names `Karplus Strong Attack` and `Saxophone` are auto-migrated to `Karplus Strong`
and `Clarinet`.

Container object:

```json
{
    "type": "Harmonic Generator",
    "instanceName": "Harmonic Generator 1",
    "color": "#3498db",
    "position": { "x": 100, "y": 200 },
    "parameters": { "purity": 1.0 },
    "stringParameters": { "scoreCurveName": "Dynamics" }
}
```

Connections reference containers by `instanceName`:

```json
{ "fromContainer": "Harmonic Generator 1", "fromPort": "spectrum",
  "toContainer": "Rolloff Processor 1", "toPort": "spectrumIn",
  "function": "passthrough", "weight": 1.0 }
```

If a container writes `customEnvelope`, its `points[].curveType` uses `0` linear, `1` smooth,
`2` step, and `loopMode` uses `0` none, `1` loop, `2` ping-pong.

`instanceName` must be unique within the canvas. Port names, connection functions, container
parameters, and signal-chain recipes are all in `Claude-sounit-builder.md`.

---

## 11. `.kalarender` — do not write it

Binary sidecar with the same basename as the `.kala` (`foo.kala` → `foo.kalarender`),
containing pre-rendered note audio. If it is absent, or its hashes/sample rate don't match,
Kala renders from scratch. Both outcomes are correct. Omit it.

---

## 12. Minimal complete example

One track, one bar of notes, no graph (silent until a sounit is attached).
120 BPM, 5 pulses per bar → pulse 500 ms, bar 2500 ms.

```json
{
    "version": 1,
    "fileType": "kala-project",
    "compositionSettings": {
        "name": "Notes Only",
        "timeMode": "Musical",
        "tempo": 120,
        "referenceTempo": 120,
        "timeSigTop": 5,
        "timeSigBottom": 0,
        "lengthMs": 2500,
        "lengthBars": 1,
        "sampleRate": 48000,
        "bitDepth": 24,
        "preRenderDelayMs": 400,
        "autoRender": true
    },
    "scaleSettings": {
        "defaultScale": { "scaleId": 2, "name": "Equal Temperament", "tonicIndex": 0 },
        "baseFrequency": 16.3516,
        "scaleChanges": []
    },
    "tempoSettings": {
        "defaultTempo": 120,
        "defaultTimeSigNum": 5,
        "defaultTimeSigDenom": 0,
        "tempoChanges": []
    },
    "tracks": [
        {
            "trackId": 0,
            "name": "Track 1",
            "color": "#3498db",
            "sounitFilePath": "",
            "sounitName": "Untitled Sounit",
            "muted": false,
            "solo": false,
            "volume": 1.0,
            "gain": 1.0,
            "pan": 0.0,
            "variations": [],
            "sampleRate": 48000,
            "graphHash": 0
        }
    ],
    "notes": [
        {
            "id": "5f0c6c2e-1111-4a2b-9a55-000000000001",
            "startTime": 0,
            "duration": 500,
            "pitchHz": 261.63,
            "trackIndex": 0,
            "variationIndex": 0,
            "quantized": false,
            "legato": false,
            "linkedAsLegato": false,
            "pitchCurve": { "points": [] },
            "dynamicsCurve": { "points": [
                { "time": 0, "value": 0.7, "pressure": 1.0 },
                { "time": 1, "value": 0.7, "pressure": 1.0 }
            ] },
            "bottomCurve": { "points": [
                { "time": 0, "value": 0.6, "pressure": 1.0 },
                { "time": 1, "value": 0.6, "pressure": 1.0 }
            ] }
        },
        {
            "id": "5f0c6c2e-1111-4a2b-9a55-000000000002",
            "startTime": 500,
            "duration": 1000,
            "pitchHz": 329.63,
            "trackIndex": 0,
            "variationIndex": 0,
            "quantized": false,
            "legato": false,
            "linkedAsLegato": false,
            "pitchCurve": { "points": [] },
            "dynamicsCurve": { "points": [
                { "time": 0, "value": 0.5, "pressure": 1.0 },
                { "time": 1, "value": 0.9, "pressure": 1.0 }
            ] },
            "bottomCurve": { "points": [
                { "time": 0, "value": 0.6, "pressure": 1.0 },
                { "time": 1, "value": 0.6, "pressure": 1.0 }
            ] }
        }
    ]
}
```

What the user sees on open: a score with two notes on track 1, a 1-bar timeline, ET grid.
Pressing play produces silence until a sounit is loaded onto the track.

To make it audible, write `sounit.generated.json` in the same folder, then set:

```json
"sounitFilePath": "C:/path/to/sounit.generated.sounit",
"sounitName": "My Instrument"
```

---

## 13. Writer's checklist

- [ ] `fileType` is exactly `"kala-project"`.
- [ ] `compositionSettings`, `scaleSettings`, `tempoSettings` are always present.
- [ ] `tempo`/`timeSigTop`/`timeSigBottom` agree between composition and tempo settings.
- [ ] `lengthMs == lengthBars * (60000/tempo * timeSigTop)` (rounded up).
- [ ] Every note has a **non-empty `dynamicsCurve`**, or it will be silent.
- [ ] Every note `id` is unique.
- [ ] `trackIndex` is within `tracks[]`.
- [ ] `variationIndex` is `0` unless the track actually has `variations`.
- [ ] Times are ms and pitches are Hz — no beat numbers, no MIDI note numbers.
- [ ] Curve `time` is 0–1 within the note, and curve points are sorted.
- [ ] Every number is finite — no `NaN`/`Infinity`, which JSON cannot represent.
- [ ] `.sounit` paths exist and are resolvable from Kala's working directory.
- [ ] Any `clip.filePath` exists and is a PCM/float WAV (mono or stereo, any rate).
- [ ] No `.kalarender` is written.
- [ ] Optional per-track `notes` omitted (top-level `notes` is authoritative).

---

## 14. Loader behaviour quick reference

| Situation | Result |
|---|---|
| Invalid JSON | Error dialog, project not opened (`kalamain.cpp:5571`) |
| `fileType` missing or different | "Not a valid Kala project file", not opened |
| Unknown container `type` | That container is skipped with a warning; rest of graph loads |
| `sounitFilePath` missing on disk | Track loads with an empty canvas; project still opens |
| Unknown `scaleId` | Falls back to Just Intonation |
| Unknown/extra JSON keys | Ignored |
| Missing optional objects | Defaults, or previous project's state for the settings objects |
| `lengthBars` disagrees with `lengthMs` | Both kept as saved; `lengthMs` governs export length |
| Notes beyond `lengthMs` | Still loaded; export extends to the rendered content end |
