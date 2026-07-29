# Integrate Faust Flute DSP Container in Sounit Builder

This plan describes the implementation steps to integrate the Faust-generated Nonlinear Waveguide Flute model (`flute.cpp`) as a new container/synthesizer unit in Kala's Sounit Builder, under a new **Waveguides** menu.

The Faust model (`mydsp` class) has 0 audio inputs and 2 audio outputs (stereo). We average left/right to produce a mono signal matching Kala's existing waveguide container pattern. The model inherits from Faust's `dsp` base class and registers its parameters via `buildUserInterface(UI*)` — we provide stub `dsp` and `UI` interfaces in `faust_dummy.h` plus a `ParameterMapUI` subclass that captures `FAUSTFLOAT*` pointers keyed by parameter name, so the wrapper can read/write them by name at runtime.

---

## Faust Parameter Triage

The Faust model exposes ~22 parameters via `buildUserInterface`. Not all should be modulatable Kala ports. The decisions:

### Exposed as modulatable Kala ports (7 inputs)
| Faust name | Kala port | Faust default | Notes |
|---|---|---|---|
| `Pressure` (fHslider15) | `breathPressure` | 0.9 | Range 0.0–1.5; clamp to 0.0–1.0 in wrapper |
| `Noise Gain` (fHslider11) | `noiseGain` | 0.1 | Range 0.0–1.0 |
| `Nonlinearity` (fHslider3) | `nonlinearity` | 0.0 | Range 0.0–1.0 |
| `Nonlinearity Attack` (fHslider4) | `nlAttack` | 0.1 s | Range 0.0–2.0 s; same semantics as Bowed/Reed/Clarinet `nlAttack` |
| `Modulation_Frequency` (fHslider5) | `modFrequency` | 220 Hz | Range 20–1000 Hz |
| `Modulation_Type` (fEntry1) | `modType` | 0 | 0–4 (int); see tooltip in flute.cpp:809-812 |
| `pitchMultiplier` | `pitchMultiplier` | 1.0 | Kala convention — not a Faust param; multiplies pitch before setting `freq` |

### Hardcoded to sensible defaults (NOT exposed as ports)
| Faust name | Value | Reason |
|---|---|---|
| `gain` (fEntry2) | driven by dynamics | Set from `currentDynamics` each tick; not user-modulatable |
| `gate` (fButton0) | 1 (on) / 0 (tail) | Set to 0 during tail mode so internal envelope releases |
| `freq` (fEntry0) | set from pitch | Driven by `pitch * pitchMultiplier` each tick |
| `Glob_Env_Attack` (fHslider1) | 0.01 | Minimal — Kala's Envelope Engine handles note shaping |
| `Glob_Env_Release` (fHslider0) | 0.05 | Short — let tail mode handle ring-out |
| `Press_Env_Attack` (fHslider13) | 0.01 | Minimal — breath pressure is driven by external Envelope Engine |
| `Press_Env_Decay` (fHslider14) | 0.01 | Minimal |
| `Press_Env_Release` (fHslider12) | 0.05 | Short |
| `Pressure_Env` (fCheckbox0) | 1 | Always enabled; actual pressure shape comes from `breathPressure` port |

### Neutralized (Faust built-in effects — Kala has its own)
| Faust name | Value | Reason |
|---|---|---|
| `Vibrato_Freq` (fHslider6) | — | Not used; vibrato envelope fRec24 stays at 0 |
| `Vibrato_Gain` (fHslider10) | **0.0** | Faust vibrato is excitation-level (sine added to waveguide drive signal). Kala does true physical pitch vibrato via LFO→pitchMultiplier (modulates delay-line lengths). Neutralize to avoid double-vibrato. |
| `Vibrato_Attack` (fHslider9) | 0.5 | Irrelevant when gain=0 |
| `Vibrato_Begin` (fHslider8) | 0.1 | Irrelevant when gain=0 |
| `Vibrato_Release` (fHslider7) | 0.2 | Irrelevant when gain=0 |
| `reverbGain` (fHslider2) | **0.0** | Kala has IR Convolution for reverb |
| `roomSize` (fHslider17) | 0.72 | Irrelevant when reverbGain=0 |
| `pan angle` (fHslider16) | **0.5** | Center — Kala has its own Pan container |
| `spatial width` (fHslider18) | **0.0** | Mono — Kala has its own Pan container |

### Gate → tail mode mapping

The Faust `gate` button (`fButton0`) controls an internal attack/release envelope. When gate=0, the release timer starts and excitation fades smoothly while delay lines continue to ring. This maps cleanly to Kala's tail mode:

- Normal playback: `gate = 1.0f`
- Tail mode: `gate = 0.0f` → internal envelope releases, delay lines drain naturally

`hasTail()` returns `true` for the Flute container — same as Recorder and other waveguide models.

---

## Proposed Changes

### Core DSP and Model Wrappers

#### [NEW] `faust_dummy.h`
Define the `dsp` and `UI` base classes expected by Faust-generated code, plus `ParameterMapUI`.

`dsp` virtual interface: `getNumInputs()`, `getNumOutputs()`, `init(int sample_rate)`, `clone()`, `getSampleRate()`, `buildUserInterface(UI*)`, `compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)`, `instanceInit(int)`, `instanceConstants(int)`, `instanceClear()`.

`UI` virtual interface: `openVerticalBox(const char*)`, `openHorizontalBox(const char*)`, `closeBox()`, `addHorizontalSlider(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT)`, `addNumEntry(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT)`, `addButton(const char*, FAUSTFLOAT*)`, `addCheckButton(const char*, FAUSTFLOAT*)`, `declare(FAUSTFLOAT**, const char*, const char*)`.

`ParameterMapUI` extends `UI`: overrides the `add*` methods to store `{name → FAUSTFLOAT*}` mappings in a `QHash<QString, FAUSTFLOAT*>`. Used once during `FluteModel` construction to capture all parameter pointers, then discarded.

#### [NEW] `flutemodel.h`
Declare the `FluteModel` class. Forward-declare `mydsp` (do NOT include `flute.cpp` here — see compilation strategy below). Store a `mydsp*` (raw pointer, consistent with Kala's existing pattern of raw processor pointers in `ProcessorData`).

Public interface follows the RecorderModel pattern:
```cpp
class FluteModel {
public:
    explicit FluteModel(double sampleRate = 44100.0);
    ~FluteModel();
    FluteModel(const FluteModel&);            // deep copy (calls mydsp::clone)
    FluteModel& operator=(const FluteModel&);

    void reset(bool isLegato = false);
    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    // Per-sample setters (called every tick with modulated values)
    void setBreathPressure(double p);
    void setNoiseGain(double g);
    void setNonlinearity(double n);
    void setNlAttack(double a);
    void setModFrequency(double f);
    void setModType(int t);
    void setPitchMultiplier(double m);  // stored, applied to pitch in tick()

private:
    double sampleRate;
    mydsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;  // populated once at construction
    double pitchMultiplier = 1.0;

    void setFaustParam(const QString& name, FAUSTFLOAT value);
    FAUSTFLOAT getFaustParam(const QString& name) const;
};
```

#### [NEW] `flutemodel.cpp`
Implement `FluteModel`. Key details:

- **Constructor**: instantiates `mydsp`, calls `init(sampleRate)`, creates a `ParameterMapUI`, calls `dspInstance->buildUserInterface(&ui)` to populate `paramMap`. Then hardcodes neutral defaults for all non-exposed parameters via `paramMap`.
- **Destructor**: `delete dspInstance`
- **Copy constructor**: calls `dspInstance->clone()` (Faust provides this)
- **`tick()`**: sets `freq` = `pitch * pitchMultiplier`, `gain` = `currentDynamics`, `gate` = `tailMode ? 0.0f : 1.0f`, `Pressure` = `breathPressure`, etc. Calls `compute(1, inputs, outputs)`. Returns `(outputs[0][0] + outputs[1][0]) * 0.5f`.
- **`reset()`**: calls `instanceClear()` (not `instanceInit()` — preserve sample rate constants). For `isLegato`, skip clear to preserve delay line state.

#### Compilation strategy for `flute.cpp`

`flute.cpp` defines `static float ftbl0mydspSIG0[65536]` (256 KB) and several static functions at file scope. Including it from multiple translation units would cause duplicate symbol errors. Strategy:

- `flute.cpp` — compiled as its own translation unit (added to CMakeLists.txt)
- `flutemodel.h` — forward-declares `mydsp`; stores `mydsp*` only
- `flutemodel.cpp` — `#include "flute.cpp"` to get the full `mydsp` class definition

#### [MODIFY] `flute.cpp`
Add `#include "faust_dummy.h"` at the top (after the existing `#ifndef __mydsp_H__` guard) so that `class mydsp : public dsp` resolves. No other changes — Faust-generated code should not be hand-edited beyond this include.

---

### Synthesis Graph & Routing

#### [MODIFY] `sounitgraph.h`
- `#include "flutemodel.h"`
- Add `FluteModel* fluteModel = nullptr;` to `ProcessorData`
- Add `delete fluteModel;` to `ProcessorData` destructor

#### [MODIFY] `sounitgraph.cpp`
Follow the Recorder pattern throughout:

- **`clone()` (copy constructor, line ~86)**: deep-copy `fluteModel` when present
- **`createProcessors()` (line ~691)**: add `else if (container->getName() == "Flute")` block — instantiate `FluteModel`, set default params from container
- **`reset()` (line ~856)**: add `if (data.fluteModel) data.fluteModel->reset(isLegato);`
- **`hasTail()` (line ~924)**: add `if (data.fluteModel) return true;` — the waveguide delay lines ring down
- **Tail mode sample generation (line ~1037)**: add Flute case — call `fluteModel->tick(pitch * pitchMult, 1.0, m_currentIsLegato, true, m_currentDynamics)` (note: `tailMode=true` so gate=0 internally)
- **`executeContainer()` (line ~2046)**: add Flute case. Read modulated values from connections (same pattern as Recorder). Call setters each sample, then `proc.signalOut = proc.fluteModel->tick(pitch * pitchMult, noteProgress, m_currentIsLegato, false, m_currentDynamics)`

Connection port mapping in `executeContainer` (controlOut [0,1] scaled to parameter range):

| Port | Scaling |
|---|---|
| `breathPressure` | direct [0,1], clamp to [0,1] |
| `noiseGain` | direct [0,1], clamp to [0,1] |
| `nonlinearity` | direct [0,1], clamp to [0,1] |
| `nlAttack` | sv × 2.0 → [0, 2] seconds |
| `modFrequency` | sv × 980 + 20 → [20, 1000] Hz |
| `modType` | sv × 4, round to int → [0,4] |
| `pitchMultiplier` | direct (no scaling — raw ratio) |

---

### UI & Configuration

Container color: `rgb(230, 81, 133)` (pink/magenta — distinct from existing waveguide blue).

#### [MODIFY] `canvas.cpp`
Register ports in `getPortsForContainerType()`:
```
Inputs:  {"breathPressure", "noiseGain", "nonlinearity", "nlAttack",
          "modFrequency", "modType", "pitchMultiplier"}
Outputs: {"signalOut"}
```

#### [MODIFY] `sounitbuilder.ui`
- Create a new menu `menuWaveguides` with title `"Waveguide"`
- Add menu action `actionFlute` with text `"Flute"`
- Insert `menuWaveguides` into `menuBar`

#### [MODIFY] `sounitbuilder.cpp`
- Connect `actionFlute` → `onAddContainer("Flute", QColor(230, 81, 133), inputs, outputs)`
- Add parameter defaults in `onCreateContainer()` for `"Flute"`:
  - `breathPressure` = 0.9
  - `noiseGain` = 0.1
  - `nonlinearity` = 0.0
  - `nlAttack` = 0.1
  - `modFrequency` = 220.0
  - `modType` = 0.0
  - `pitchMultiplier` = 1.0

#### [MODIFY] `kalamain.h`
- Declare `void populateFluteInspector();`

#### [MODIFY] `kalamain.cpp`
- Add `"Flute"` case in `onContainerSelected()` → call `populateFluteInspector()`
- Implement `populateFluteInspector()` — create sliders for the 7 parameters above, matching the layout pattern of `populateRecorderInspector()`
- Add description in `getContainerDescription()` for `"Flute"`

---

### Build System

#### [MODIFY] `CMakeLists.txt`
Add `flutemodel.cpp` and `flute.cpp` to the source file list.

---

## Verification Plan

### Build
- Run CMake/Make to verify clean compilation

### Manual
1. Launch Kala, open Sounit Builder, click **Waveguide → Flute**
2. Verify container appears on canvas with correct pink color and 7 input ports + 1 output port
3. Open inspector — verify all 7 sliders populated and responsive
4. Connect a signal chain: trigger/gate + pitch input → Flute → signal output; verify sound
5. Verify real-time parameter modulation (sliders change timbre)
6. Verify note-off → tail mode rings down naturally (no hard cutoff)
7. Verify stereo output is properly mono (no channel imbalance)
