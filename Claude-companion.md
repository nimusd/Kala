# Kala Companion — AI Agent Documentation

The Kala Companion is a built-in AI agent that lives inside Kala. It can answer questions
about the application, build sounit graphs on the user's behalf, and write music directly
onto the score canvas. It supports any OpenAI-compatible API (Ollama, OpenAI, Groq,
OpenRouter) and the Anthropic API.

---

## Architecture Overview

```
User types in CompanionPanel
        │
        ▼
  KalaAgent::sendUserMessage()
        │  builds: system prompt + history + tool schemas
        ▼
  LLMClient::sendChatRequest()          ← HTTP, QNetworkAccessManager
        │
        ▼
  LLM API  (Ollama / OpenAI / Anthropic / Groq / OpenRouter)
        │
        ▼
  KalaAgent::onResponseReceived()
        │
        ├─ finish_reason == "tool_calls"  →  KalaTools::dispatchTool()  →  loop (max 12 rounds)
        │
        └─ finish_reason == "stop"        →  emit messageReady()  →  CompanionPanel
```

All code runs on the main Qt thread. `QNetworkReply::finished` fires on the main thread,
so no locking or thread hopping is needed.

---

## Files

| File | Role |
|------|------|
| `llmconfig.h` | Config struct (`LLMProvider`, URL, API key, model, tokens, temperature) |
| `llmclient.h/.cpp` | Stateless HTTP client. Speaks OpenAI format to callers; adapts Anthropic internally |
| `kalatools.h/.cpp` | Tool dispatcher. Bridges LLM tool calls to Kala's C++ APIs |
| `kalaagent.h/.cpp` | Conversation loop. Owns history, manages tool-call rounds, builds system prompt |
| `companionpanel.h/.cpp` | Chat UI widget (QTextBrowser log + QPlainTextEdit input) |

Modified files: `kalamain.h`, `kalamain.cpp`, `sounitbuilder.h`, `sounitbuilder.cpp`, `CMakeLists.txt`

---

## llmconfig.h

```cpp
enum class LLMProvider { OpenAICompatible, Anthropic };

struct LLMConfig {
    LLMProvider provider  = LLMProvider::OpenAICompatible;
    QString     baseUrl   = "http://localhost:11434/v1";  // Ollama default
    QString     apiKey    = "ollama";
    QString     model     = "qwen3-coder:30b";
    int         maxTokens = 4096;
    double      temperature = 0.7;
};
```

Persisted to QSettings under keys: `llm/provider`, `llm/baseUrl`, `llm/apiKey`,
`llm/model`, `llm/maxTokens`, `llm/temperature`.

Provider auto-fill URLs (set by the Preferences UI combobox):
- Ollama: `http://localhost:11434/v1`
- OpenAI: `https://api.openai.com/v1`
- Groq: `https://api.groq.com/openai/v1`
- OpenRouter: `https://openrouter.ai/api/v1`
- Anthropic: `https://api.anthropic.com/v1` (uses `LLMProvider::Anthropic`)

---

## LLMClient

Single public method:

```cpp
void sendChatRequest(const QJsonArray &messages,
                     const QJsonArray &tools,
                     ResponseCallback callback);
// callback: void(QJsonObject response, QString errorString)
```

Always speaks **OpenAI format** to callers. Anthropic adaptation is entirely internal:
- `adaptRequestForAnthropic()` — extracts system message to top level, renames
  `parameters` → `input_schema` in tool schemas.
- `adaptResponseFromAnthropic()` — normalises `content[]` array back to
  `choices[0].message` shape.

---

## KalaAgent

- Holds `QJsonArray m_messages` — conversation history (system prompt NOT stored here,
  prepended fresh on each request by `buildRequestMessages()`).
- `sendUserMessage(text)` → appends user message → `continueConversation(12)`.
- Tool call loop: appends assistant message → executes tools → appends tool results →
  calls `continueConversation(roundsLeft - 1)`. Capped at **12 rounds**.
- Signals: `messageReady(text, role)`, `thinkingStarted()`, `thinkingFinished()`.
- `role` values: `"assistant"`, `"tool_info"`, `"error"`.

System prompt (`buildSystemPrompt()`):
- Starts with `/no_think` (suppresses Qwen3 chain-of-thought for faster responses).
- Sections: Kala overview → Container types (all 25) → Connection functions →
  Typical signal chains → Key rules & gotchas → DNA presets →
  File library tools → Composition tools (notes, scale, tempo).

---

## KalaTools — Complete Tool Reference

Constructor: `KalaTools(SounitBuilder*, TrackManager*, ScoreCanvasWindow*, QObject*)`.

Must call `setCurrentTrackIndex(int)` when the user switches to a different track
(done in `KalaMain::switchToSounit()`).

### Library path

The library root defaults to `~/Music/kala`. It is stored in QSettings under
`library/kalaRoot` and used by all four scan tools. The agent can update it at any time
via `set_library_root`.

---

### Sounit / Graph tools

#### `get_graph_state`
Returns the current sounit graph (containers + connections) as compact JSON.
Skips large binary blobs (wavetable data, IR samples, custom DNA arrays).

Response: `{ result: { sounitName, containers: [...], connections: [...] } }`

Each container: `{ type, instanceName, position: {x,y}, parameters: {...} }`
Each connection: `{ from, fromPort, to, toPort, function, weight }`

---

#### `get_sounit_list`
Scans `<libraryRoot>/sounit` recursively for all `.sounit` files.
Also includes any file from the last-used directory (QSettings `lastDirectory/sounit`).

Response: `{ result: [ { name: "alto/bowed-expressive", path: "..." }, ... ] }`

Names are relative paths within the sounit folder, with `.sounit` stripped.

---

#### `clear_graph`
Calls `Track::newSounit()` + `SounitBuilder::rebuildGraph()`.
Clears all containers and connections. No undo (it's a full reset).

---

#### `add_container`
Parameters: `type` (required), `params` (optional object), `position` (optional `{x,y}`).

Uses `SounitBuilder::onAddContainer()` to create the container with all defaults,
then applies any param overrides. Finds the new container by diffing instance names
before/after. Returns the assigned instance name (e.g. `"Envelope Engine 2"`).

Valid types: all 25 container types (see system prompt CONTAINER TYPES section).

---

#### `set_parameter`
Parameters: `instanceName`, `param`, `value` (number).

Sets a single numeric parameter. The audio graph rebuilds automatically via the
existing `parameterChanged()` signal chain.

Note: string parameters (e.g. IR file path, DNA file path) cannot be set this way —
use `load_ir` for IR, and inform the user to set custom DNA in the HG inspector.

---

#### `connect_containers`
Parameters: `fromInstance`, `fromPort`, `toInstance`, `toPort`,
`function` (default `"passthrough"`), `weight` (default `1.0`).

Validates port names against `Canvas::getPortsForContainerType()`.
Pushes an `AddConnectionCommand` through the undo stack.

Valid functions: `passthrough`, `add`, `subtract`, `multiply`, `replace`, `modulate`.

---

#### `remove_container`
Parameters: `instanceName`.

Pushes a `DeleteContainerCommand` through the undo stack. All connections to/from
the container are removed automatically.

---

#### `load_sounit`
Parameters: `filePath`.

Calls `Track::loadSounit()`, reconnects container signals, rebuilds graph.
Replaces the entire current graph.

---

#### `save_sounit`
Parameters: `filePath`, `name` (optional display name).

Calls `Track::saveSounit()`. The sounit name is embedded in the saved file.

---

#### `play_preview`
No parameters. Calls `SounitBuilder::triggerPlay()` — same as pressing Play in the
sounit builder.

---

#### `load_ir`
Parameters: `instanceName`, `filePath`.

Loads a WAV file into an IR Convolution container using `WavetableSynth::loadWavFile()`
then `Container::setIRData()` + `Container::setIRFilePath()`, then rebuilds the graph.
This is the full equivalent of clicking "Load IR File..." in the inspector.

Requires the target container to be of type `"IR Convolution"`.

---

### Library browser tools

#### `get_spectrum_list`
Scans `<libraryRoot>/spectrum` for `*.dna.json` files.
Response: `{ result: [ { name: "fibonacci_sequence", path: "..." }, ... ] }`

---

#### `load_spectrum`
Parameters: `instanceName`, `filePath`.

Reads a `.dna.json` file and loads its amplitude array directly into a Harmonic Generator
container using the same numeric parameter storage the app uses internally:
- Sets `dnaSelect = -1` (custom mode)
- Sets `customDnaCount = N`
- Sets `customDna_0` … `customDna_N-1` to the amplitude values
- Sets `container->setCustomDnaName()` so the inspector label shows correctly

File format: `{ "name": "...", "numHarmonics": 64, "amplitudes": [0.49, 0.21, ...] }`

Requires the target container to be of type `"Harmonic Generator"`.

---

#### `get_project_list`
Scans `<libraryRoot>/projects` for `*.kala` files.
Response: `{ result: [ { name: "oudish hijaz 2", path: "..." }, ... ] }`

---

#### `get_envelope_list`
Scans `<libraryRoot>/envelopes` for `*.env.json` files.
Response: `{ result: [ { name: "swell", path: "..." }, ... ] }`

---

#### `load_envelope`
Parameters: `instanceName`, `filePath`.

Reads a `.env.json` file and applies it to an Envelope Engine container.
Sets `envelopeSelect = 7` (custom), calls `setCustomEnvelopeData()`, rebuilds graph.

File format:
```json
{ "name": "Swell", "loopMode": 0,
  "points": [ {"time": 0, "value": 0, "curveType": 1}, ... ] }
```
`curveType`: 0=Linear, 1=Smooth, 2=Step.

---

#### `set_envelope_shape`
Parameters: `instanceName`, `points` (array, required), `name` (optional), `loopMode` (optional, default 0).

Builds an envelope from scratch without any file. Each point: `{time, value, curveType}`.
Time and value are 0–1 (normalised to note lifetime). `curveType` defaults to 1 (Smooth).
First/last points are auto-clamped to `time=0` / `time=1` if omitted.

Common shapes:
```
Swell:   [{0,0},{0.5,1},{1,0}]
ASR:     [{0,0},{0.05,1},{0.8,0.7},{1,0}]
Fade in: [{0,0},{1,1}]
Bell:    [{0,0},{0.1,1},{0.4,0.3},{1,0}]
```

`loopMode`: 0=None, 1=Loop, 2=Ping-pong.

---

#### `get_ir_list`
Scans `<libraryRoot>/impulse responses` recursively for `*.wav` files.
Names are relative paths within the IR folder (e.g. `"Theatre41/convolved_viola.wav"`).

---

#### `set_library_root`
Parameters: `path` (absolute directory path).

Saves to QSettings `library/kalaRoot`. All subsequent scan tools use this path.
The directory must exist at call time.

---

### Composition tools

These tools operate on `ScoreCanvas` via `ScoreCanvasWindow::getScoreCanvas()`.
All note mutations go through the `QUndoStack` and are fully undoable with Ctrl+Z.

---

#### `get_composition_state`
Parameters: `trackIndex` (optional, filter to one track).

Returns all notes + current scale + tempo + time signature.

Response:
```json
{
  "result": {
    "noteCount": 8,
    "notes": [
      { "id": "...", "startTime": 0, "duration": 500, "pitchHz": 440,
        "dynamics": 0.7, "trackIndex": 0 }
    ],
    "scale": "Just Intonation",
    "baseFreqHz": 261.63,
    "tempo": 120,
    "timeSigNum": 4,
    "timeSigDenom": 4
  }
}
```

---

#### `get_track_list`
No parameters. Returns all tracks with index, name, and loaded sounit name.

---

#### `add_note`
Parameters: `startTime` (ms), `duration` (ms), `pitchHz`, `dynamics` (default 0.7),
`trackIndex` (default = current track).

Pushes an `AddNoteCommand`. Returns `{ id, message }` where `id` is the note's UUID
(needed for `delete_note`).

---

#### `delete_note`
Parameters: `id` (UUID string from `get_composition_state` or `add_note`).

Pushes a `DeleteNoteCommand`. Finds the note by UUID, then deletes by index.

---

#### `clear_notes`
Parameters: `trackIndex` (optional).

Without `trackIndex`: clears all notes (all tracks) via a macro undo command.
With `trackIndex`: clears only notes on that track.

---

#### `set_scale`
Parameters: `scaleId` OR `scaleName` (one required), `baseFreqHz` (optional).

Sets the tuning scale and regenerates scale lines. Does **not** transpose existing notes.

Key scale IDs:
```
 0  Just Intonation       1  Pythagorean        2  Equal Temperament
 3  Quarter-Comma Meantone
 4  Maqam Rast            5  Maqam Bayati       7  Maqam Hijaz
10  Raga Bhairav         11  Raga Yaman        13  Raga Bhairavi
16  Chinese Pentatonic   17  Hirajoshi         20  Whole Tone
30  Persian Shur         31  Persian Mahur     32  Persian Segah
```

---

#### `set_tempo`
Parameters: `bpm` (20–400).

Sets `ScoreCanvas::setDefaultTempo()`. Controls bar lines and musical time display.

---

### Transport tools

#### `play_score`
No parameters. Starts score canvas playback from the current now-marker position.
Equivalent to pressing Play in the score canvas window. Use after composing or modifying
notes to let the user hear the result.

---

#### `stop_score`
No parameters. Stops score canvas playback.

---

### Undo / Redo

#### `undo`
No parameters. Undoes the last action on the score canvas (Ctrl+Z equivalent).
Returns the name of the undone action. Use to recover from AI tool mistakes.

---

#### `redo`
No parameters. Redoes the last undone action (Ctrl+Y equivalent).

---

### Tempo / time-signature tools

#### `set_time_signature`
Parameters: `bpm` (optional), `numerator` (optional), `denominator` (optional), `gradual` (optional bool).

Sets the default (opening) tempo and time signature of the score — the marker at time 0.
All parameters are optional; omitted values keep their current setting. Supports undo.

---

#### `get_time_signature_changes`
No parameters. Returns all tempo/time-signature markers in the score (including the default at time 0).
Each entry: `{ timeMs, bpm, numerator, denominator, gradual }`.

---

#### `add_time_signature_change`
Parameters: `timeMs` (required, > 0), `numerator` (required), `denominator` (required),
`bpm` (optional, inherits current bpm if omitted), `gradual` (optional bool).

Inserts (or replaces) a tempo/time-signature change at the given position. Equivalent to setting the
now-marker in the score canvas and clicking "+" on the time signature panel. Supports undo.

---

#### `remove_time_signature_change`
Parameters: `timeMs` (required, > 0).

Removes the tempo/time-signature marker at the given position. Returns an error if no marker exists
there. The default marker at time 0 cannot be removed. Supports undo.

---

### Engine / project settings tools

#### `get_engine_settings`
No parameters.

Returns the full Sound Engine Settings as a JSON object with two top-level keys:
- `containerSettings` — all container slider ranges, nested by group name (see below)
- `project` — `{ compositionName, sampleRate, bitDepth, lengthMs }`

Call this first to see current values and exact key names before calling `set_engine_settings`.

---

#### `set_engine_settings`
Parameters: `containerSettings` (optional object), `project` (optional object).

Applies a **partial** update — only groups/fields you include are changed. Everything else is preserved.
Merges at both the group level and the field level, so sending `{"driftEngine": {"amountMax": 0.3}}`
only changes `amountMax` without touching `amountMin`.

Changes are saved immediately to QSettings. Inspector slider ranges reflect the new values the next
time a panel opens.

`containerSettings` groups and their fields:
```
harmonicGenerator  maxHarmonics, fundamentalHzMin/Max, rolloffPowerMin/Max, driftMax,
                   padBandwidthMin/Max, padBandwidthScaleMin/Max
rolloffProcessor   lowRolloffMin/Max, highRolloffMin/Max, crossoverMin/Max, transitionMin/Max
spectrumToSignal   normalizeMin/Max
formantBody        f1FreqMin/Max, f2FreqMin/Max, qMin/Max, mixMin/Max
breathTurbulence   blendMin/Max
noiseColorFilter   colorMin/Max, filterQMin/Max
physicsSystem      massMin/Max, springKMin/Max, dampingMin/Max, impulseMin/Max
driftEngine        amountMin/Max, rateMin/Max
gateProcessor      velocityMin/Max, attackTimeMin/Max, releaseTimeMin/Max, velocitySensMin/Max
karplusStrong      attackPortionMin/Max, dampingMin/Max, pluckPositionMin/Max, mixMin/Max,
                   brightnessMin/Max, excitationSoftnessMin/Max, pluckHardnessMin/Max,
                   bodyResonanceMin/Max, bodyFreqMin/Max, pickDirectionMin/Max
attack             durationMin/Max, intensityMin/Max, mixMin/Max, noiseAmountMin/Max,
                   jetRatioMin/Max, reedStiffnessMin/Max, reedApertureMin/Max,
                   blowPositionMin/Max, lipTensionMin/Max, buzzQMin/Max,
                   hardnessMin/Max, brightnessMin/Max, tightnessMin/Max, toneMin/Max
lfo                frequencyMin/Max, amplitudeMin/Max
wavetableSynth     positionMin/Max
frequencyMapper    lowFreqMin/Max, highFreqMin/Max, outputMinMin/Max, outputMaxMin/Max
bandpassEQ         gainMin/Max, qMin/Max
combFilter         delayTimeMin/Max, feedbackMin/Max, dampingMin/Max
lowHighPassFilter  cutoffMin/Max, resonanceMin/Max
irConvolution      wetDryMin/Max, predelayMin/Max, decayMin/Max, highDampMin/Max,
                   fadeInPctMin/Max, lowCutMin/Max, captureLengthMin/Max
recorder           breathPressureMin/Max, jetRatioMin/Max, noiseGainMin/Max,
                   vibratoFreqMin/Max, vibratoGainMin/Max, endReflectionMin/Max, jetReflectionMin/Max
bowed              bowPressureMin/Max, bowVelocityMin/Max, bowPositionMin/Max,
                   nlTypeMin/Max, nlAmountMin/Max, nlFreqModMin/Max, nlAttackMin/Max
reed               breathPressureMin/Max, reedStiffnessMin/Max, reedApertureMin/Max,
                   blowPositionMin/Max, noiseGainMin/Max, vibratoFreqMin/Max, vibratoGainMin/Max,
                   nlTypeMin/Max, nlAmountMin/Max, nlFreqModMin/Max, nlAttackMin/Max
easing             overshootDefault/Min/Max, amplitudeDefault/Min/Max, periodDefault/Min/Max,
                   frequencyDefault/Min/Max, decayDefault/Min/Max
```

`project` fields: `compositionName` (string), `sampleRate` (int: 44100/48000/96000/192000),
`bitDepth` (int: 16/24/32), `lengthMs` (number).

**Note**: `settingsChanged()` is not emitted — the in-memory singleton and QSettings are updated,
but open inspector panels do not hot-reload their slider ranges. The user must reopen the inspector.

---

### Project tools

#### `open_project`
Parameters: `filePath` (required, absolute path to `.kala` file).

Loads a project file into the running session, replacing the current state.
Use `get_project_list` first to browse available projects.

---

#### `save_project`
Parameters: `filePath` (optional).

Saves the current project. If `filePath` is omitted, saves to the existing project path.
Required for new/unsaved projects.

---

### Track management tools

#### `add_track`
Parameters: `name` (required), `color` (optional hex string).

Adds a new instrument track. Returns `{ trackIndex }` — use immediately with `load_sounit`
to assign an instrument. Each track is completely independent (sounit + notes).

---

#### `rename_track`
Parameters: `trackIndex` (required), `name` (required).

Renames a track. Updates both the data model and the track selector UI.

---

#### `delete_track`
Parameters: `trackIndex` (required).

Deletes a track and all its notes. Track indices above the deleted one shift down.
Cannot delete the last track. **Not undoable** — use with care.

---

### Score canvas tools

These mirror the right-click context menu operations on the score canvas.
All operations accept an optional `noteIds` array. Empty (or omitted) = all notes on the current track.

#### `get_variation_list`
No parameters. Returns `[{index, name}, ...]` for all sounit variations on the current track.
Index 0 = base sounit; 1+ = named variations (e.g. "Wood Room", "Plate").

#### `apply_variation`
Parameters: `variationIndex` (int, required), `noteIds` (string array, optional).
Assigns a variation index to each targeted note by calling `note.setVariationIndex()` +
`setRenderDirty()`. Does **not** push an undo command (matches UI behavior — variations are
not undoable in the right-click menu either).

#### `create_variation`
Parameters: `name` (string, required).

Snapshots the current canvas state as a named variation. Equivalent to the "Save as Variation"
button in the sounit builder. The base sounit on the canvas is unchanged.
Emits `variationsChanged` to refresh the variation selector UI.
Returns the 1-based variation index.

#### `create_variation_from_sounit`
Parameters: `filePath` (string, required), `name` (string, required).

Loads a `.sounit` file as a named variation without touching the current canvas.
Reads containers + connections from the file, calls `Track::createVariationFromJson()`.
Use `get_sounit_list` first to find the file path.
Emits `variationsChanged` to refresh the UI.

#### `delete_variation`
Parameters: `variationIndex` (int, required, 1-based).

Deletes a variation by index. Cannot delete index 0 (base sounit).
Emits `variationsChanged` to refresh the UI.

#### `rename_variation`
Parameters: `variationIndex` (int, required, 1-based), `name` (string, required).

Renames an existing variation. Emits `variationsChanged` to refresh the UI.

#### `switch_variation`
Parameters: `variationIndex` (int, required).

Loads a variation onto the canvas for editing (index 0 = base sounit).
Calls `Track::loadVariationToCanvas()`, rebuilds the audio graph, reconnects container signals.
After this call, all sounit-editing tools (`add_container`, `set_parameter`, etc.) operate on
the loaded variation's graph. Call `create_variation` to snapshot edits as a new variation when done.

#### `apply_dynamics_curve`
Parameters: `points` (array of `{time, value, curveType}`), `noteIds` (optional),
`weight` (0–1, default 1.0), `perNote` (bool, default false).
Pushes `ApplyDynamicsCurveCommand`. Points use time=0→first note, time=1→last note.
`perNote=true` applies the curve within each individual note's own timeline.

#### `scale_timing`
Parameters: `proportion` (> 0), `noteIds` (optional).
Pushes `ScaleTimingCommand`. Multiplies all start times and durations by `proportion`.
proportion=2.0 = half tempo, 0.5 = double tempo.

#### `apply_rhythm_easing`
Parameters: `easingType` (int, default 0=Linear), `anchorMode` (int, default 3=AnchorBoth),
`weight` (0–1, default 1.0), `noteIds` (optional, minimum 2).
Pushes `ApplyRhythmicEasingCommand`. Redistributes note onset times using the easing curve.
Uses `Easing::Type` enum (0=Linear … 28=Wobble) and `AnchorMode` (0=None, 1=First, 2=Last, 3=Both).

#### `link_legato`
Parameters: `noteIds` (string array, required, ≥ 2).
Pushes `LinkAsLegatoCommand`. Notes sustain into each other without envelope retrigger.

#### `unlink_legato`
Parameters: `noteIds` (optional).
Pushes `UnlinkLegatoCommand`. Restores independent envelope retriggering per note.

#### `quantize_to_scale`
Parameters: `noteIds` (optional).
Calls `sc->snapSelectedNotesToScale()` after `sc->selectNotes(indices)`.
Snaps pitch to nearest scale degree.

#### `make_continuous`
Parameters: `noteIds` (optional).
Calls `sc->makeSelectedNotesContinuous()` after selection. Note extends until next note starts.

#### `make_discrete`
Parameters: `noteIds` (optional).
Calls `sc->makeSelectedNotesDiscrete()` after selection. Note plays its exact stated duration.

**Implementation note**: `quantize_to_scale`, `make_continuous`, `make_discrete` use
`applyWithSelection()` which calls `sc->selectNotes()` (now public) before the operation.
`selectNotes` was moved from private to public in `scorecanvas.h` for this purpose.

#### `transpose_notes`
Parameters: `ratio` (number, e.g. 2.0 = octave up, 0.5 = octave down) OR `scaleDegrees` (int, +up/-down),
`noteIds` (optional).

Transposes pitch of targeted notes. `ratio` multiplies pitchHz directly — use for interval transposition
(2.0 = octave, 1.5 = fifth, 4/3 = fourth in JI). `scaleDegrees` walks the scale active at each note's
start time; handles octave wrapping across the full pitch range. Pitch curves (glissandi) are scaled
proportionally. Supports undo.

---

#### `set_note_pitch`
Parameters: `pitchHz` (number) OR `values` (number array), `noteIds` (optional).

Sets the absolute pitch of targeted notes by frequency. Single `pitchHz` applies to all; `values`
array sets per-note pitches (must match note count). Glissando/pitch curves are preserved, scaled
proportionally to the new pitch. Supports undo.

---

#### `stretch_notes`
Parameters: `factor` (required, > 0), `noteIds` (optional).

Multiplies the duration of targeted notes by `factor` without changing their start times.
`factor > 1` = longer notes, `factor < 1` = shorter notes. Supports undo.

---

#### `set_note_duration`
Parameters: `duration` (number, ms) OR `values` (number array, same length as `noteIds`),
`noteIds` (optional).

Sets note duration(s). Single `duration` applies the same value to all targeted notes.
`values` array sets per-note durations (must match `noteIds` length). Supports undo.

---

#### `duplicate_notes`
Parameters: `offsetMs` (required), `pitchRatio` (optional, default 1.0), `trackIndex` (optional),
`noteIds` (optional).

Copies a set of notes and places them at `offsetMs` ms after the start of the source phrase.
To place copies immediately after: `offsetMs = (maxEndTime − minStartTime)`.
`pitchRatio` transposes copies (2.0 = octave up). Returns `{ count, noteIds[] }` of new notes —
pipe these directly into `scale_timing`, `apply_rhythm_easing`, `transpose_notes`, etc. Supports undo.

---

#### `get_note_vibrato`
Parameters: `noteIds` (optional, first match inspected).

Returns the vibrato settings on a note: `active`, `rate` (Hz), `pitchDepth`, `amplitudeDepth`,
`onset`, `regularity`, and `envelope` (intensity curve points).

---

#### `set_note_vibrato`
Parameters: all optional — `active`, `rate`, `pitchDepth`, `amplitudeDepth`, `onset`,
`regularity`, `envelope` (array of `{time, value, curveType}`), `noteIds`.

Sets vibrato on a selection of notes. Unspecified parameters are preserved from the first
targeted note's current settings, so you can change just `rate` without touching the rest.
The `envelope` shapes how vibrato intensity builds over the note lifetime — default is a
linear rise; a delayed-then-full shape like `[{0,0},{0.3,0},{0.6,1},{1,1}]` gives a late
swell typical of bowed strings. Uses `SetVibratoCommand` — supports undo.

Typical starting values: `rate=5.5`, `pitchDepth=0.025`, `amplitudeDepth=0.2`, `onset=0.25`,
`regularity=0.7`.

---

#### `get_note_curves`
Parameters: `noteIds` (optional, first match inspected).

Returns all expressive curves on a note: dynamics (index 0) plus any named curves
(`Brightness`, `Breathiness`, `Bow Pressure`, etc.). Response: `{ noteId, curveCount, curves[] }`
where each curve has `{ index, name, pointCount, points[] }`.

---

#### `set_note_curve`
Parameters: `points` (required), `name` (default `"Dynamics"`), `noteIds` (optional),
`weight` (0–1, default 1.0), `perNote` (bool, default false).

Sets a named expressive curve on notes. `"Dynamics"` targets the main dynamics curve.
Any other name creates/replaces a named expressive curve which modulates whatever synthesis
parameter is routed to that name. Same `points` format as `set_envelope_shape`.
Supports undo via `AddExpressiveCurveCommand` / `ApplyDynamicsCurveCommand`.

---

#### `get_selected_notes`
No parameters. Returns the IDs of notes currently selected on the canvas by the user.
Use when the user says "work on my selection" / "apply this to the selected notes".
Returns `{ selectedCount, noteIds[], message }`.
The returned `noteIds` can be passed to any tool that accepts a `noteIds` array.

#### `select_notes`
Parameters: `pitchMinHz` (optional), `pitchMaxHz` (optional), `trackIndex` (optional).

Selects notes by pitch range and/or track. At least one filter should be provided.
Calls `sc->selectNotes()` — notes are visually highlighted on the canvas.
Returns `{ matchedCount, noteIds[], message }` so IDs can be piped into other tools.

#### `shift_notes`
Parameters: `offsetMs` (required), `pitchHz` (optional), `pitchToleranceCents` (optional, default 50), `noteIds` (optional).

Shifts note start times by `offsetMs` (positive = later, negative = earlier).
Optional pitch filter: only shift notes within `pitchToleranceCents` of `pitchHz`.
Clamps: rejects the operation if any note would shift before time 0.
Pushes a single macro `QUndoCommand` — all shifts undo together with Ctrl+Z.

#### `set_note_dynamics`
Parameters: `noteIds` (string array, required), `values` (number array, required, same length).

Sets an individual dynamics value (0.0–1.0) per note. `noteIds[i]` gets `values[i]`.
Use for per-note humanization: call `get_composition_state`, compute per-note dynamics
in the LLM (beat pattern + small random offset), then call this once with all IDs and values.
Reuses `SetBeatDynamicsCommand` — single undo command restores all previous dynamics.

#### `apply_beat_dynamics`
Parameters: `pattern` (number array, required), `noteIds` (optional).

Applies a per-beat accent pattern across measures. `pattern[0]` = beat 1, `pattern[1]` = beat 2, etc.
Each note's beat position is computed from `getTimeSignatureAtTime()` + `getDefaultTempo()`,
so mid-piece time signature changes are handled correctly.
Pattern wraps if shorter than the time signature numerator.
Pushes `SetBeatDynamicsCommand` — single undo.

Example (5/4, strong downbeat): `pattern: [0.9, 0.35, 0.7, 0.5, 0.35]`

**New undo command**: `SetBeatDynamicsCommand` (scorecanvascommands.h/.cpp) — stores old
dynamics curves per note for undo, sets constant dynamics via `note.setDynamics()` on redo.
Used by both `apply_beat_dynamics` and `set_note_dynamics`.

---

## CompanionPanel

Chat UI widget (`QWidget`). Catppuccin dark theme.

- `QTextBrowser m_log` — read-only HTML log.
- `QPlainTextEdit m_input` — Enter = send, Shift+Enter = newline.
- Enter key intercepted via `eventFilter`.
- `appendMessage(text, role)` roles:
  - `"user"` — green header with timestamp
  - `"assistant"` — blue header, body runs through `markdownToHtml()`
  - `"tool_info"` — small line (green for success `✓`, orange for error `⚠`)
  - `"error"` — red text
- `setThinking(bool)` — shows/hides "Thinking…" label, disables input during generation.
- Static `markdownToHtml()` handles: `###`/`##`/`#` headings, `**bold**`, `*italic*`,
  `` `inline code` ``, ` ``` ` fenced code blocks, `- `/ `* ` bullet lists, blank-line
  paragraph breaks.

---

## KalaMain integration

### Construction (constructor body)

```cpp
// Load config from QSettings
m_llmConfig = { ... };   // reads llm/* keys

// Instantiate agent stack
m_kalaTools    = new KalaTools(sounitBuilder, trackManager, scoreCanvasWindow, this);
m_kalaAgent    = new KalaAgent(m_llmConfig, m_kalaTools, this);
m_companionPanel = new CompanionPanel(this);

// Dock widget
m_companionDock = new QDockWidget("AI Companion", this);
m_companionDock->setWidget(m_companionPanel);
addDockWidget(Qt::RightDockWidgetArea, m_companionDock);

// Wire signals
connect(m_companionPanel, &CompanionPanel::userMessageSubmitted,
        m_kalaAgent,       &KalaAgent::sendUserMessage);
connect(m_kalaAgent, &KalaAgent::messageReady,
        m_companionPanel,  &CompanionPanel::appendMessage);
connect(m_kalaAgent, &KalaAgent::thinkingStarted, ...);
connect(m_kalaAgent, &KalaAgent::thinkingFinished, ...);
```

### Track sync

`switchToSounit(trackIndex)` calls `m_kalaTools->setCurrentTrackIndex(trackIndex)`
so the agent always operates on the track the user is looking at.

### Preferences tab

"AI Companion" section with: provider combo (auto-fills URL), base URL field,
API key field (masked), model field, Apply button.
Apply saves to QSettings, calls `m_kalaAgent->setConfig()` and `clearHistory()`.

### View menu

`actionCompanion` (Ctrl+Shift+A) toggles `m_companionDock->setVisible()`.

---

## Adding a New Tool — Checklist

1. **Declare** the method in `kalatools.h`:
   ```cpp
   QJsonObject toolMyNewTool(const QJsonObject &args);
   ```

2. **Add dispatch** in `kalatools.cpp::dispatchTool()`:
   ```cpp
   if (toolName == "my_new_tool") return toolMyNewTool(args);
   ```

3. **Implement** the method in `kalatools.cpp`.
   Return `ok("message")` on success or `error("reason")` on failure.

4. **Add schema** in `kalatools.cpp::getToolSchemas()`:
   ```cpp
   schemas.append(QJsonObject{
       {"type", "function"},
       {"function", QJsonObject{
           {"name", "my_new_tool"},
           {"description", "..."},
           {"parameters", QJsonObject{
               {"type", "object"},
               {"properties", QJsonObject{
                   {"myParam", strProp("Description of myParam")}
               }},
               {"required", QJsonArray{"myParam"}}
           }}
       }}
   });
   ```
   Helper builders: `strProp(desc)`, `numProp(desc)`, `objProp(desc, props)`.

5. **Update the system prompt** in `kalaagent.cpp::buildSystemPrompt()` if the tool
   needs usage guidance or fits into an existing section.

---

## Known Limitations / Future Work

- **Score canvas redraw**: `set_scale` now emits `scaleSettingsChanged` so the inspector
  label and frequency labels update immediately. `set_tempo` was already correct
  (`setDefaultTempo` emits `tempoSettingsChanged` internally).

- **Multi-track note writing**: The agent writes notes to all tracks via `add_note` with
  explicit `trackIndex`. For complex arrangements, the tool call count can hit the 12-round
  cap. If needed, increase `kMaxToolRounds` in `kalaagent.h`.

- **`duplicate_notes` + undo**: The `PasteNotesCommand` is pushed once and fully undoable,
  but the returned note IDs are only valid for the current redo state. If the user undoes
  and redoes, the IDs are regenerated — don't cache them across undo/redo cycles.
