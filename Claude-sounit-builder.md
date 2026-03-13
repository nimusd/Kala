# Claude Sounit Builder Reference

Everything needed to design and generate `.sounit` files for Kala without opening the app.

---

## File Format

```json
{
    "connections": [ ... ],
    "containers": [ ... ],
    "sounit": {
        "comment": "description of the instrument",
        "created": "2026-03-05T10:00:00",
        "name": "Instrument Name",
        "version": "1.0"
    }
}
```

### Container object

```json
{
    "color": "#3498db",
    "instanceName": "Harmonic Generator 1",
    "type": "Harmonic Generator",
    "position": { "x": 100, "y": 200 },
    "parameters": { ... },
    "customDna": { ... },      // optional — only when dnaSelect == -1
    "digitString": "...",      // optional — digit formula DNA
    "customDnaName": "..."     // optional
}
```

### Connection object

```json
{
    "fromContainer": "Harmonic Generator 1",
    "fromPort": "spectrum",
    "toContainer": "Rolloff Processor 1",
    "toPort": "spectrumIn",
    "function": "passthrough",
    "weight": 1
}
```

`weight` scales the source value. For control ports (e.g. digitWindowOffset), weight=50 maps 0–1 signal to 0–50 offset range.

### Connection Functions

| Function | Formula | Use Case |
|----------|---------|----------|
| `passthrough` | `output = sourceValue` | Direct signal or spectrum routing |
| `add` | `output = current + source × weight` | Additive modulation |
| `multiply` | `output = current × source × weight` | Amplitude / gain control |
| `subtract` | `output = current − source × weight` | Inverse control |
| `replace` | `output = current × (1−weight) + source × weight` | Smooth parameter crossfading |
| `modulate` | `output = current + (source − 0.5) × weight × 2.0` | Bipolar CV-style modulation (LFO→param) |

**Notes:**
- `passthrough` (weight ignored): use for signal/spectrum routing and simple control overrides
- `add`/`subtract`: add offset to a param (e.g. envelope adding to a base rolloff value)
- `multiply`: scale a param by a control signal (tremolo, gain shaping)
- `replace`: crossfade between the param's default and the source value; weight=1 = full source
- `modulate`: bipolar — source 0.5 = no change, source 0.0 = subtract weight, source 1.0 = add weight. Use for LFO→pitch vibrato, LFO→rolloff timbral wobble

---

## Container Types & Parameters

### Harmonic Generator
**Ports out:** `spectrum`
**Ports in:** `purity`, `drift`, `digitWindowOffset`

```json
"parameters": {
    "dnaSelect": 0,          // 0=Vocal Bright, 1=Vocal Warm, 2=Brass, 3=Reed, 4=String, 5=Pure Sine, -1=Custom
    "numHarmonics": 64,
    "purity": 0,             // 0=harmonic, higher=inharmonic (detuned partials)
    "drift": 0,              // static drift amount (use Drift Engine for dynamic)
    "padEnabled": 0,         // 1 to enable PADsynth
    "padBandwidth": 40,      // cents (1–200), spectral smearing width
    "padBandwidthScale": 1.0,// how bandwidth scales with harmonic number (0–2)
    "padProfileShape": 0,    // 0=Gaussian
    "padFftSize": 262144,    // wavetable size
    "digitWindowOffset": 0   // base offset into digit string
}
```

**Custom DNA** (when dnaSelect=-1):
```json
"customDna": {
    "count": 64,
    "name": "Spectrum Name",
    "amplitudes": [ 0.5, 0.25, ... ]   // 64 values, 0.0–1.0
}
```

**Digit Formula DNA** (1/7, 1/17, etc.):
```json
"digitString": "142857142857...",   // 1000+ digits, stored as string
"parameters": {
    "dnaSelect": -1,
    "digitWindowOffset": 0
}
```
Note: `customDna` is NOT needed when using `digitString`. The digit string itself defines the spectrum.
Connect Envelope Engine (score curve mode) → `digitWindowOffset` with weight=50–100 for live sweep.

---

### Spectrum to Signal
**Ports in:** `spectrumIn`, `pitchMultiplier`
**Ports out:** `signalOut`

```json
"parameters": {
    "normalize": 1,
    "pitchMultiplier": 1.0   // pitch ratio: 1.0=no change, 2.0=octave up, 0.5=octave down
}
```

---

### Rolloff Processor
**Ports in:** `spectrumIn`, `highRolloff`, `lowRolloff`
**Ports out:** `spectrumOut`

```json
"parameters": {
    "crossover": 8,       // harmonic number where rolloff starts
    "highRolloff": 0.1,   // 0=full highs, 1=cut all highs (above crossover)
    "lowRolloff": 0,      // same for below crossover
    "transition": 4       // transition band width in harmonics
}
```
When driven by envelope/drift, the connection overrides the static param value.
Set static value = starting/fallback value.

---

### Note Tail
**Ports in:** `signalIn`
**Ports out:** `signalOut`

```json
"parameters": { "tailLength": 150 }   // ms, 1–9999
```
Always the last container in signal chain. Prevents end-of-note clicks via cosine fade.

---

### Envelope Engine
**Ports out:** `envelopeValue`

```json
"parameters": {
    "envelopeSelect": 5,   // 5 = custom envelope
    "timeScale": 1,
    "valueOffset": 0,
    "valueScale": 1
},
"customEnvelope": {
    "loopMode": 0,
    "name": "My Envelope",
    "points": [
        { "curveType": 1, "time": 0.0, "value": 0.0 },
        { "curveType": 1, "time": 0.5, "value": 0.8 },
        { "curveType": 1, "time": 1.0, "value": 0.3 }
    ]
}
```
`time` is 0–1 (fraction of note duration). `curveType`: 1=smooth.
**Score curve mode**: set `envelopeSelect` to the score-curve enum value (check app). Used to drive `digitWindowOffset` from composition.

---

### Drift Engine
**Ports out:** `driftOut`

```json
"parameters": {
    "amount": 0.025,   // magnitude of drift (0–1)
    "rate": 1.2        // speed of drift variation
}
```
Typically drives Rolloff `highRolloff` or HG `drift` port for organic liveliness.

---

### LFO
**Color:** Green. **Ports in:** `frequency`, `amplitude`, `waveType`
**Ports out:** `valueOut`

```json
"parameters": {
    "frequency": 1.0,  // Hz (0.01–100.0)
    "amplitude": 1.0,  // peak amplitude (0.0–1.0)
    "waveType": 0      // 0=Sine, 1=Triangle, 2=Square, 3=Sawtooth
}
```

Output is **bipolar**: swings from `-amplitude` to `+amplitude`. Does not reset at note boundaries — phase runs continuously across all notes.

Use `modulate` connection function to drive params with LFO (keeps them centered). Use `add` for offset modulation.

| Effect | Frequency | WaveType |
|--------|-----------|----------|
| Vibrato | 4–8 Hz | Sine |
| Tremolo | 2–10 Hz | Sine |
| Timbral wobble | 0.1–1 Hz | Sine/Triangle |
| Slow drift | 0.01–0.1 Hz | Sine |
| Rhythmic gate | 1–4 Hz | Square |

**Tempo reference:** at 120 BPM — quarter note = 2.0 Hz, eighth = 4.0 Hz, sixteenth = 8.0 Hz.

---

### Spectrum Blender
**Color:** Orange. **Ports in:** `spectrumA`, `spectrumB`, `position`
**Ports out:** `spectrumOut`

```json
"parameters": {
    "position": 0.5    // 0.0=pure A, 1.0=pure B (overridden when modulated)
}
```

Formula: `output[h] = A[h] × (1 - position) + B[h] × position` for each harmonic h.

Use for timbral morphing between two HG sources. Drive `position` with:
- Envelope Engine → morph from A to B over note lifetime
- LFO → oscillate between timbres (timbral vibrato)
- Drift Engine → slow organic timbral wandering

Place before Rolloff Processor and Spectrum to Signal in the chain. Can chain two blenders for A→B→C three-way morph.

---

### Signal Mixer
**Ports in:** `signalA`, `signalB`
**Ports out:** `signalOut`

```json
"parameters": {
    "gainA": 1.0,
    "gainB": 0.4
}
```

---

### Attack
**Ports in:** `brightness`
**Ports out:** `signalOut`

```json
"parameters": {
    "attackType": 1,
    "brightness": 0.07,
    "duration": 0.18,
    "hardness": 0.06,
    "intensity": 0.16,
    "mix": 0.58,
    "noiseAmount": 0.5,
    "tightness": 0.94,
    "tone": 0.42,
    "blowPosition": 0.5,
    "buzzQ": 5,
    "jetRatio": 0.32,
    "lipTension": 0.5,
    "reedAperture": 0.71,
    "reedStiffness": 0.13
}
```

**attackType values** (from attackgenerator.h):
| Value | Name | Relevant params |
|-------|------|-----------------|
| 0 | FluteChiff | noiseAmount, jetRatio |
| 1 | ClarinetOnset | reedStiffness, reedAperture |
| 2 | SaxophoneHonk | reedStiffness, reedAperture, blowPosition |
| 3 | BrassBuzz | lipTension, buzzQ |
| 4 | PianoHammer | hardness, brightness |
| 5 | DrumHit | tightness, tone |

**DrumHit (attackType=5)** internals:
- Membrane: sine sweeps from `200 + tone×600` Hz down to `60 + tone×100` Hz
- Sweep rate: `5 + tightness×40` (higher tightness = faster sweep)
- Membrane decay: `8 + tightness×25`
- Body resonance (BiQuad): `80 + tone×200` Hz
- Noise snap filtered at `500 + tone×8000` Hz
- **Pitch is NOT note-pitch-tracked** — use Frequency Mapper → Attack:tone to make it follow the score

**Percussion signal chain (tested pattern):**
`Attack (DrumHit=5) → IR Convolution (guitar/body IR, wetDry=0.20–0.30) → output`
`Frequency Mapper → Attack:tone` (maps note pitch to drum's resonant frequency range)
- IR Convolution adds physical body resonance that bare membrane synthesis lacks. IR has hasTail()=true — no Note Tail needed.
- Low wetDry (0.15–0.30) = subtle body color, not a room effect
- Optional: second Frequency Mapper → IR:wetDry (low notes=wetter/resonant, high notes=drier/shorter rim character)

---

### IR Convolution
**Color:** Purple. **Ports in:** `signalIn`, `irIn`, `wetDry`, `predelay`, `highDamp`, `lowCut`
**Ports out:** `signalOut`

```json
"parameters": {
    "wetDry": 0.5,       // 0.0=dry, 1.0=wet (modulatable)
    "predelay": 0.0,     // ms before wet signal (0–500, modulatable)
    "highDamp": 20000.0, // HP damp cutoff Hz (200–20000, modulatable)
    "lowCut": 20.0,      // Low-cut Hz (20–2000, modulatable)
    "decay": 1.0,        // IR tail shortening (0–1, inspector only)
    "fadeInPct": 10.0    // Wet fade-in % of predelay (0–100, inspector only)
}
```

IR is loaded via inspector (WAV file). **IR data is embedded in the JSON when saved by the app** — do not write `irFilePath` in generated sounits; user must assign IR in-app and re-save.

`irIn` port: connect any signal source for cross-synthesis. The engine pre-renders `captureLength` seconds of that signal at the note's pitch and uses it as the IR. `captureLength` (0.1–20.0s) is inspector-only.

IR Convolution has `hasTail()=true` — reverb tail renders after note ends. No Note Tail needed.

---

### Physics System
**Ports in:** `targetValue`
**Ports out:** `currentValue`

```json
"parameters": {
    "damping": 0.94,
    "impulseAmount": 900,
    "mass": 9.4,
    "springK": 0.9
}
```

---

### Easing Applicator
**Ports in:** `startValue`
**Ports out:** `easedValue`

```json
"parameters": {
    "easingSelect": 24,
    "endValue": 1,
    "startValue": 0
}
```

---

## Available DNA Spectra

Location: `C:\Users\nimus\Music\kala\spectrum\`

| File | Character | Good for |
|------|-----------|----------|
| `fundamentalplus_near-sine.dna.json` | Dominant fundamental, 8 harmonics then silence | Clean bass foundation, layering |
| `octaves_only_organ.dna.json` | Only octave harmonics (1,2,4,8,16,32,64) | Organ bass, pure anchors |
| `Harmonic Series Mathematical.dna.json` | 1/n decay — full, rich, natural | General purpose, PADsynth base |
| `vocal_warm.dna.json` | Strong fundamental, gentle rolloff, boosted 2nd–4th | Warm bass, cello-like |
| `vocal_bright.dna.json` | 1/n^1.2 + formant boosts at 3–5 and 8–12 | Mid/soprano voices |
| `odd_only_clarinet.dna.json` | Odd harmonics only | Reed-like, nasal bass |
| `odd_dominant.dna.json` | Odd harmonics dominant (not exclusive) | Woody, clarinet-adjacent |
| `even_only_hollow.dna.json` | Even harmonics only | Hollow, flute-adjacent |
| `prime_numbers.dna.json` | Primes only (2,3,5,7,11...) | Sparse, bright, unusual |
| `bell_curve_gaussian.dna.json` | Bell curve peaking around harmonic 16–17 | Bell-like, metallic |
| `Logistic Map Chaos.dna.json` | Chaotic, dense low-mid then sparse | Aggressive, unpredictable |
| `fibonacci_sequence.dna.json` | Fibonacci positions | Organic, balanced |
| `phi.dna.json` | Golden ratio decay | Warm, slightly irrational feel |
| `brass.dna.json` | Odd-boosted 1/n^1.5 | Trumpet-adjacent |
| `string.dna.json` | Boosted harmonics 5–10 | Violin-like |
| `inverse_bright.dna.json` | Inverted rolloff, high harmonics dominant | Bright, cutting |
| `pi -Modulated Spectrum.dna.json` | π-modulated | Quasi-random, mathematical |
| `Phi-Modulated Primes.dna.json` | φ × primes | Complex, mathematical |
| `susvib_A2_v1_1.dna.json` | Sustained vibrato capture | Real instrument character |
| ... and more | | |

---

## Digit Formula Strings

Pre-computed repeating rational decimals:

**1/7** (period 6: `142857`):
```
1428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428571428
```

**1/17** (period 16: `0588235294117647`):
```
0588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647058823529411764705882352941176470588235294117647
```

**1/13** (period 6: `076923`):
```
076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923076923
```

**1/11** (period 2: `09`):
```
090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909
```

For longer periods use the in-app Digit Formula dialog and save the sounit.

---

## Typical Signal Chains

**Simple additive:**
`HG → Rolloff → StoS → Note Tail`

**With brightness animation:**
`HG → Rolloff → StoS → Note Tail`
`Envelope Engine → Rolloff:highRolloff`

**With organic life:**
`HG → Rolloff → StoS → Note Tail`
`Drift Engine → Rolloff:highRolloff`
(or `Drift Engine → HG:drift`)

**Layered:**
`HG1 → StoS1 → Mixer:signalA`
`HG2 → Rolloff → StoS2 → Mixer:signalB`
`Mixer → Note Tail`

**Attack transient + HG sustain:**
`Attack → Mixer:signalA`
`HG → Rolloff → StoS → Mixer:signalB`
`Mixer → Note Tail`
`Envelope Engine → Rolloff:highRolloff` (or → Attack:brightness)

**Timbral morph (Spectrum Blender):**
`HG1 → Spectrum Blender:spectrumA`
`HG2 → Spectrum Blender:spectrumB`
`Spectrum Blender → Rolloff → StoS → Note Tail`
`Envelope Engine → Spectrum Blender:position` (or LFO, or Drift)

**Bowed hybrid (waveguide + additive body):**
`Bowed → Mixer:signalA` (gainA=0.12–0.14 — waveguide = timbral tint only)
`HG → Rolloff → StoS → Formant Body → Mixer:signalB` (gainB=1.0)
`Envelope Engine → Bowed:bowVelocity` (followDynamics=1)
`Drift → HG:drift`

**Breath turbulence (wind instrument):**
`HG → Rolloff → StoS → Formant Body → Breath Turbulence:voiceIn`
`Noise Color Filter → Breath Turbulence:noiseIn`
`Breath Turbulence → Note Tail`
`Envelope Engine → Breath Turbulence:blend` (optional — dynamic breathiness)

**Placed in space (IR room):**
`[Signal chain] → EQ → IR Convolution → signalOut`
(No Note Tail needed — IR Convolution has hasTail()=true)

**LFO vibrato:**
`LFO (freq=5.5, amp=0.02, Sine) → StoS:pitchMultiplier` (function=modulate, weight=0.04)

---

### Karplus Strong
**Ports in:** `signalIn`, `trigger`, `mode`, `attackPortion`, `damping`, `pluckPosition`, `mix`, `brightness`, `excitationType`, `blendRatio`, `pluckHardness`, `bodyResonance`, `bodyFreq`, `pickDirection`, `stringDamping`, `pitchMultiplier`
**Ports out:** `signalOut`

```json
"parameters": {
    "mode": 0,              // 0=String (KS is whole sound), 1=Attack (KS pluck transient on top of signalIn)
    "attackPortion": 0.15,  // portion of decay to include (0–1)
    "damping": 0.35,        // decay rate — 0=longest, 1=quickest. Usually driven by Frequency Mapper
    "pluckPosition": 0.82,  // 0=bridge (bright), 1=neck (mellow/dark)
    "brightness": 0.12,     // loop filter stretch 0=dark, 1=bright
    "excitationType": 3,    // 0=Noise, 1=SoftPluck, 2=Blend, 3=FingerPluck
    "blendRatio": 0.25,     // excitation softness / blend ratio (0–1)
    "pluckHardness": 0.15,  // 0=wide/soft, 1=narrow/hard
    "pickDirection": 0.5,   // 0=downstroke, 0.5=neutral, 1=upstroke
    "bodyResonance": 0.42,  // body resonator wet/dry (0=off)
    "bodyFreq": 105,        // body resonator center Hz (80–400)
    "mix": 0.8,             // Attack mode: pluck prominence (0–1)
    "stringDamping": 0,     // if >=0.5, hasTail()=true (needed for bowed-style use)
    "pitchMultiplier": 1.0  // pitch ratio: 1.0=no change, 2.0=octave up, 0.5=octave down
}
```

Note Tail NOT needed for KS String mode — the string decays naturally.
Note Tail IS needed in KS Attack mode (mode=1) since the HG sustain body doesn't decay.

---

### Wavetable Synth
**Ports in:** `position`, `pitchMultiplier`
**Ports out:** `signalOut`, `spectrum`

```json
"parameters": {
    "wavetableSelect": 0,   // 0=Saw, 1=Square, 2=Triangle, 3=Pulse, 4=SuperSaw, 5=Formant
    "position": 0.0,        // frame morph position (0–1, for multi-frame WAV files)
    "outputMode": 0,        // 0=Both, 1=SignalOnly, 2=SpectrumOnly
    "pitchMultiplier": 1.0  // pitch ratio: 1.0=no change, 2.0=octave up, 0.5=octave down
}
```

---

### Recorder (fipple flute waveguide)
**Ports in:** `breathPressure`, `jetRatio`, `noiseGain`, `vibratoFreq`, `vibratoGain`, `endReflection`, `jetReflection`, `pitchMultiplier`
**Ports out:** `signalOut`

```json
"parameters": {
    "breathPressure": 0.81,  // driving force — must be ~0.75–0.90 for oscillation
    "jetRatio": 0.35,        // jet delay / bore delay (0.05–0.60); main timbre control
    "noiseGain": 0.015,      // breath turbulence (0.0–0.5)
    "vibratoFreq": 0.0,      // vibrato Hz (0–20)
    "vibratoGain": 0.0,      // vibrato depth (0=off)
    "endReflection": 0.73,   // bore open-end reflection (positive = open tube)
    "jetReflection": 0.59,   // jet-to-bore coupling
    "pitchMultiplier": 1.0   // pitch ratio: 1.0=no change, 2.0=octave up, 0.5=octave down
}
```

---

### Bowed (bowed string waveguide)
**Ports in:** `bowPressure`, `bowVelocity`, `bowPosition`, `nlType`, `nlAmount`, `nlFreqMod`, `nlAttack`, `pitchMultiplier`
**Ports out:** `signalOut`

```json
"parameters": {
    "bowPressure": 0.75,  // friction force (0–1); main character control
    "bowVelocity": 0.50,  // bow speed / loudness (0–1)
    "bowPosition": 0.127, // contact point: 0.05–0.12=sul ponticello, 0.127=normale, 0.25+=sul tasto
    "nlType": 0,          // 0=bridge signal, 1=leaky avg, 2=signal², 3=sine@nlFreqMod, 4=sine@pitch
    "nlAmount": 0.0,      // nonlinearity depth (0=bypassed)
    "nlFreqMod": 10.0,    // oscillator Hz for nlType 3 (0–200)
    "nlAttack": 0.10,     // nonlinearity ramp-in seconds
    "pitchMultiplier": 1.0 // pitch ratio: 1.0=no change, 2.0=octave up, 0.5=octave down
}
```

---

### Reed / Saxophone (conical bore waveguide)
**Ports in:** `breathPressure`, `reedStiffness`, `reedAperture`, `blowPosition`, `noiseGain`, `vibratoFreq`, `vibratoGain`, `nlType`, `nlAmount`, `nlFreqMod`, `nlAttack`, `pitchMultiplier`
**Ports out:** `signalOut`

```json
"parameters": {
    "breathPressure": 0.70,  // driving force — must be ~0.55+ for oscillation
    "reedStiffness": 0.50,   // reed slope: 0.2=soft/round, 0.7+=stiff/bright/nasal
    "reedAperture": 0.50,    // reed resting opening offset
    "blowPosition": 0.20,    // split ratio of bore delay lines (0.05=clarinet, 0.20=sax, 0.40=oboe)
    "noiseGain": 0.20,       // breath turbulence (0–0.4)
    "vibratoFreq": 5.735,    // vibrato Hz (0–12)
    "vibratoGain": 0.0,      // vibrato depth (0=off)
    "nlType": 0,             // 0=open-end, 1=leaky avg, 2=signal², 3=sine@nlFreqMod, 4=sine@pitch
    "nlAmount": 0.0,         // nonlinearity depth (0=bypassed, inside loop)
    "nlFreqMod": 10.0,       // oscillator Hz for nlType 3 (0–200)
    "nlAttack": 0.10,        // nonlinearity ramp-in seconds
    "pitchMultiplier": 1.0   // pitch ratio: 1.0=no change, 2.0=octave up, 0.5=octave down
}
```

---

**Frequency Mapper** — maps note pitch to a control value. Reads pitch from engine context automatically.
**Ports in:** `curve`, `pitchMultiplier`  **Ports out:** `controlOut`
```json
"parameters": {
    "curveType": 1,
    "lowFreq": 40,          // Hz — maps low pitch
    "highFreq": 600,        // Hz — maps high pitch
    "outputMin": 0.18,      // value at lowFreq (invert=1: low pitch → low damping = long sustain)
    "outputMax": 0.52,      // value at highFreq
    "invert": 1,            // 1=invert (low pitch → outputMin, high pitch → outputMax)
    "pitchMultiplier": 1.0  // scales the pitch seen by the mapper (1.0=no change)
}
```

**Formant Body** (Orange) — vowel-like resonance, instrument body model
**Ports in:** `signalIn`, `f1Freq`, `f2Freq`, `f1Q`, `f2Q`, `f1f2Balance`, `directMix`
**Ports out:** `signalOut`
```json
"parameters": {
    "f1Freq": 520,        // first formant Hz (typ 300–1000)
    "f1Q": 8,             // first formant Q
    "f2Freq": 1650,       // second formant Hz (typ 800–3000)
    "f2Q": 10,            // second formant Q
    "f1f2Balance": 0.68,  // 0=f1 only, 1=f2 only, 0.5=equal
    "directMix": 0.28     // dry signal mix (0=fully filtered, 1=dry only)
}
```
Cello/viola range: f1=520–580 Hz, f2=1650–1850 Hz. Higher f2 = brighter/violin. Low directMix = more colored. Place after Spectrum to Signal in chain.

**10-Band EQ** (Purple)
**Ports in:** `signalIn`, `band1`–`band10`, `q1`–`q10`
**Ports out:** `signalOut`
```json
"parameters": {
    "band1": 0.7,   // sub bass
    "band2": 0.85,  // bass
    "band3": 1.0,   // low mid
    "band4": 1.0,   // mid
    "band5": 0.9,
    "band6": 0.75,
    "band7": 0.55,
    "band8": 0.3,
    "band9": 0.1,
    "band10": 0.0,  // air
    "q1": 2, ...    // Q for each band (0.5–20)
}
```
Values: 0=mute, 1=full. Flat ~0.7–0.8 across all bands.

**EQ bandpass shape for bowed strings** (observed in project):
`band1=0.06, band2=0.11, band3=0.15, band4=0.59, band5=0.69, band6=1.0, band7=0.34, band8=0.32, band9=0.1, band10=0, all Q=2`

**Noise Color Filter** (Orange) — noise generator + biquad filter
**Ports in:** `audioIn` (optional external audio), `color`, `filterQ`, `pitchMultiplier`
**Ports out:** `noiseOut`
```json
"parameters": {
    "color": 2000.0,     // filter cutoff Hz (100–12000)
    "filterQ": 1.0,      // resonance (0.5–20.0)
    "filterType": 1,     // 0=Lowpass, 1=Highpass (default), 2=Bandpass
    "noiseType": 0,      // 0=White, 1=Pink, 2=Brown
    "followPitch": false, // when true, filter tracks note fundamental (ignores color)
    "pitchMultiplier": 1.0 // multiplier for followPitch mode (0.5–8.0)
}
```
Color guide: 100–500 Hz = dark rumble, 500–1500 = warm breath ('H'), 1500–3000 = 'SH', 3000–6000 = 'S', 6000–12000 = hissy.
For flute-like pitched breath: `filterType=Bandpass, followPitch=true, filterQ=5–15`.
Output goes to `noiseIn` of Breath Turbulence.

**Breath Turbulence** (Orange) — blends voice signal with noise
**Ports in:** `voiceIn`, `noiseIn`, `blend`
**Ports out:** `signalOut`
```json
"parameters": {
    "blend": 0.1    // 0.0=voice only, 1.0=noise only
}
```
Formula: `output = voice × (1 - blend) + noise × blend`
Place after Formant Body (or StoS) in chain. Connect Noise Color Filter → `noiseIn`.
Blend guidelines: 0.05–0.15 subtle (flute), 0.2–0.35 medium (sax), 0.4–0.6 pronounced.
Drive `blend` with Envelope Engine for dynamic breath evolution.

**Comb Filter** (Purple) — delay-based resonator for metallic textures
**Ports in:** `signalIn`, `delayTime`, `feedback`, `damping`
**Ports out:** `signalOut`

**LP/HP Filter** (Purple) — resonant lowpass or highpass
**Ports in:** `signalIn`, `cutoff`, `resonance`
**Ports out:** `signalOut`
```json
"parameters": {
    "mode": 0,       // 0=Lowpass, 1=Highpass
    "cutoff": 342,   // Hz
    "resonance": 0.7 // Q factor
}
```

---

## Orchestra Project — Status

### Bass register — DONE
`C:\Users\nimus\Music\kala\sounit\bass\`

**Additive/spectral:**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `organ-bass.sounit` | Octave harmonics, pure, dark | `crossover`, `highRolloff` |
| `warm-pad-bass.sounit` | Vocal Warm + PADsynth 28c + Drift | `padBandwidth`, drift `amount` |
| `digit-bass-1-7.sounit` | 1/7 digit pattern, PADsynth 15c | `padBandwidth`, `digitWindowOffset` weight |
| `pad-bloom-bass.sounit` | Harmonic Series, wide PADsynth 65c, slow bloom | envelope shape, `padBandwidth` |
| `layered-bass.sounit` | Near-sine foundation + Harmonic Series texture | `gainB`, envelope shape |

**Karplus-Strong (plucked):**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `upright-bass.sounit` | FingerPluck, dark, near-neck, acoustic | `pluckPosition`, `bodyFreq`, FM outputMin/Max |
| `electric-bass.sounit` | Noise excitation, punchy, EQ shaped | `pluckHardness`, `brightness`, `pickDirection` |
| `harp-bass.sounit` | SoftPluck, near-neck, very long decay | FM `outputMin` (ring length), `bodyFreq` |
| `muted-bass.sounit` | Blend, near-bridge, high damping, percussive | `damping`, `pluckHardness`, `bodyFreq` |
| `hybrid-pluck-sustain.sounit` | KS Attack transient + HG near-sine sustain | KS `mix`, envelope shape, `crossover` |

### Alto register — DONE
`C:\Users\nimus\Music\kala\sounit\alto\`

**Additive/spectral:**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `vocal-melody.sounit` | Vocal Warm + PADsynth 10c, Attack, active lowRolloff | Envelope max, padBandwidth |
| `reed-agile.sounit` | Odd Only Clarinet, no PADsynth, fastest tail (40ms) | Drift amount, Attack intensity |
| `string-expressive.sounit` | String DNA, Attack, Envelope → swell | highRolloff max, Drift rate |
| `digit-melody-1-17.sounit` | 1/17 digit pattern, offset=1, PADsynth 10c | digitWindowOffset weight, padBandwidth |

**Karplus-Strong (plucked):**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `pizzicato.sounit` | FingerPluck, medium position, alto FM range | pluckPosition, bodyFreq |
| `lute-melodic.sounit` | Blend excitation, EQ sub-cut, warm | FM outputMin/Max, padBandwidth |

**Physical modeling:**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `bowed-lyric.sounit` | Bowed + String DNA + Formant (f1=580/f2=1850) | bowPosition, Envelope max |
| `bowed-intense.sounit` | Bowed near bridge + wider PADsynth, aggressive swell | nlAmount, Envelope 2 peak |
| `saxophone.sounit` | Reed (nlType=2) + Vocal Warm, followDynamics | reedStiffness, gainB |
| `oboe.sounit` | Reed (nlType=3) + Formant (f1=850/f2=2700) nasal | f1Freq, reedAperture |
| `clarinet-reed.sounit` | Reed (nlType=1) + Odd Only Clarinet DNA | reedStiffness, gainB |

### Soprano register — DONE
`C:\Users\nimus\Music\kala\sounit\soprano\`

**Additive/spectral:**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `pure-melody.sounit` | Near-sine DNA, no PADsynth, Attack, 38ms tail | Envelope max, crossover |
| `bright-voice.sounit` | Vocal Bright (dnaSelect=0), PADsynth 10c, Attack | Envelope peak (brightness ceiling) |
| `harmony-blend.sounit` | Harmonic Series, PADsynth 22c, no Attack, long tail | padBandwidth, Envelope max |
| `bell-shimmer.sounit` | Gaussian bell DNA, purity=0.015, Drift→shimmer | purity, Drift amount |
| `digit-1-13.sounit` | 1/13 digit pattern (076923), offset=1, Attack | digitWindowOffset weight |

**Physical modeling:**
| File | Concept | Key params to tweak |
|------|---------|-------------------|
| `soprano-violin.sounit` | Bowed (bowPos=0.12, bridge) + String DNA + Formant (f1=900/f2=2800) | bowPosition, Envelope 2 peak, f1/f2Freq |
| `soprano-recorder.sounit` | Recorder + Pure Sine HG body (gain 0.28), followDynamics | jetRatio, noiseGain |
| `soprano-oboe.sounit` | Reed + Reed DNA + Formant (f1=1100/f2=3400), stiff reed | reedStiffness, f1Freq, gainB |
| `soprano-whistle.sounit` | Pure Recorder only, high jetRatio (0.46), most airy | jetRatio, noiseGain |

---

## Color Conventions

| Color | Category | Containers |
|-------|----------|------------|
| `#3498db` | Blue — Essential | HG, StoS, Signal Mixer, Note Tail, Attack, Karplus Strong, Wavetable Synth, Recorder, Bowed, Reed |
| `#e67e22` | Orange — Shaping | Rolloff Processor, Spectrum Blender, Formant Body, Noise Color Filter, Breath Turbulence |
| `#27ae60` | Green — Modifiers | Envelope Engine, Drift Engine, LFO, Physics System, Easing Applicator, Frequency Mapper |
| `#9b59b6` | Purple — Filters | 10-Band EQ, Comb Filter, LP/HP Filter, IR Convolution |

---

## Sounit Design Lessons (from testing)

- **PADsynth safe range**: `padBandwidth` 4–7 cents, `padBandwidthScale` 0.09–0.25. Values above ~12/0.5 cause ensemble detuning / doubling artifacts, especially in upper register. Use even lower (4/0.09) for very focused solo sound.
- **Bowed hybrid gainA**: In Bowed+HG mixers, `gainA` (waveguide) must be 0.12–0.14. Higher values cause beating/detuning because waveguide and additive body have slight tuning discrepancy. The bow is a timbral tint, not the main body.
- **IR files**: `irData` with embedded WAV samples is the correct format (written by app on save). Never write `irFilePath` in generated sounits — it won't load. Workflow: load sounit → assign IR in inspector → save (app embeds data).
- **IR library location**: `C:\Users\nimus\Music\kala\impulse responses\perfect space\Real Spaces\Noisevault Studio\`
- **Variation methodology** (user pattern): Keep core synthesis → add EQ bandpass + IR Convolution (replace Note Tail) = "placed in space" variant. Remove bow velocity envelope = "static" variant.
- **Note Tail vs IR Convolution**: Never use both — IR Convolution already has hasTail()=true. Use Note Tail only for pure synthesis chains (no IR).
- **Follow Dynamics**: `Envelope Engine` with `followDynamics: 1` (and `envelopeSelect: 0`) routes the score pen-pressure directly as a control signal — no custom envelope needed.

---

## Key Rules

1. `digitString` in container JSON AND `dnaSelect: -1` in parameters — both needed for digit formula DNA
2. `customDna` object in container JSON AND `dnaSelect: -1` in parameters — both needed for custom spectrum
3. Note Tail must be last in the signal chain (has `hasTail()=true`, engine enters tail mode)
4. When a control port (e.g. `highRolloff`, `digitWindowOffset`) is connected, the connection overrides the static parameter value — static value = fallback/default
5. `weight` on a connection to `digitWindowOffset` acts as range multiplier: weight=100 → 0–1 control signal maps to 0–100 offset range
6. PADsynth generates a wavetable at note-on — `padFftSize=262144` is the standard quality setting
7. All sounit files saved to `C:\Users\nimus\Music\kala\sounit\` — load via File > Load Sounit in app
8. **`pitchMultiplier` port** — available on all synthesis containers (Spectrum to Signal, Karplus Strong, Wavetable Synth, Recorder, Bowed, Reed, Frequency Mapper) and Noise Color Filter (when followPitch is on). Scales the note pitch before synthesis: 1.0=no change, 2.0=octave up, 0.5=octave down. Any ratio works — system is tuning-agnostic. Connect LFO for vibrato, Envelope Engine for pitch glide, Physics System for spring pitch wobble. Static value in params sets the default when unconnected. Backward compatible: old sounits lacking the param load as 1.0.
