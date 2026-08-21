# CLAUDE.md

Thin orientation file. Authoritative references live in the companion docs below.

## Project

**Kala** — Qt 6 / CMake / MinGW music composition app. Pen-tablet input (no MIDI),
container-graph synthesis ("sounits"), pre-rendered playback, per-track variations
that route polyphonically across notes.

## Build

```
cd build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug
/c/Qt/Tools/mingw1310_64/bin/mingw32-make.exe -j8
```

Release build uses Ninja (not make — its folder had stale qmake Makefiles, now removed):

```
cd build/Desktop_Qt_6_10_1_MinGW_64_bit-Release
/c/Qt/Tools/Ninja/ninja.exe
```

After build, the user can launch directly from Qt Creator — no rebuild needed.

## Working style

- **Don't write code proactively.** Wait for explicit user request.
- Build → test → celebrate → repeat.

## Reminders

- **Tempo map inspector** Kala-mode support is done (2026-06-30). TempoDialog shows pulses + pulse duration ms in Kala mode, BPM + num/denom in Western mode. Mode is global per project.

## Where to actually look things up

- **`Claude-sounit-builder.md`** — sounit JSON format, all container types, ports,
  parameters, DNA presets, signal-chain patterns. Read this before designing
  sounit files or working with the synthesis graph.
- **`Claude-vl70m-midi.md`** — VL70-m MIDI output project brief: roadmap (8 phases),
  settled decisions, per-phase test criteria, phase status in "Right now". Read
  before any MIDI/hardware work. Companion references: `VL70mE2.md` (module
  manual + MIDI data format), `VL-Wizard Manual (draft).md` (voice editor).
- **`Claude-companion.md`** — Anima (AI agent) architecture and full tool reference.
  Read this before adding/modifying agent tools.
- **`anima-templates.md`** — Anima conversation templates (number-pattern, etc.).
- **`docs/md format/`** — design specifications (architecture, port specs).

## Track / Note ownership (still current)

```
Track owns:
  ├─ Canvas (containers + connections)        — base sounit
  ├─ SounitGraph (compiled audio graph)       — base
  ├─ QList<SounitVariation*>                  — named variations, each with own canvas + graph
  ├─ QList<Note>                              — composition data
  ├─ QHash<QString, NoteRender> noteRenders  — per-note pre-rendered audio cache
  └─ Metadata (name, color, volume, pan, mute)

Note:
  ├─ id, startTime, duration, pitchHz
  ├─ trackIndex, variationIndex               — variationIndex picks which graph renders this note
  ├─ dynamics curve + named expressive curves
  ├─ pitch curve (glissando)
  ├─ vibrato
  └─ renderDirty flag
```
