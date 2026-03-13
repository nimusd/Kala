# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Development Workflow

⚠️ **CRITICAL:** Always wait for express user request before writing code. Go step by step: **build, test, celebrate, repeat**.

Never write code proactively. The user will explicitly tell you when to implement features.

## Project Overview

**Kala** is a compositional instrument built on Qt 6 that breaks from keyboard thinking. It uses pen tablet input as the primary interface, capturing expressive gesture as notation itself. The sound engine uses "physics modeling" (invented laws governing mass, inertia, parameter coupling) to create sounds that are alive but controllable. No MIDI. No piano roll.

**Core Philosophy:**

- The act of composing IS the performance  
- Voices share sonic DNA built on tuning-agnostic foundation  
- Pre-render model for composition (not real-time performance)  
- No hard-coded limits — all numeric constraints are configurable

## Build Commands

This project uses **CMake** with **Qt 6.10.1 MinGW 64-bit**.

\# Configure (from project root)

cmake \-S . \-B build \-G "MinGW Makefiles"

\# Build

cmake \--build build

\# Run executable

./build/Desktop\_Qt\_6\_10\_1\_MinGW\_64\_bit-Debug/Kala.exe

The current build directory is `build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug/`.

**Note:** There are currently no tests defined in the project.

## Architecture Overview

### Three-Pillar System

1. **Sound Engine** — Physics-based synthesis with container architecture  
     
   - Renders phrases to audio buffers  
   - Real-time playback of pre-rendered content  
   - Live synthesis only for actively-edited content

   

2. **Data Layer** — Contract between systems  
     
   - Defines structures (Note, Phrase, Sounit, Curve, Scale)  
   - Manages persistence (planned: .kala, .sounit, .phrase files in JSON)

   

3. **Input Engine** — Gesture capture and notation display  
     
   - Wacom pen input (6 dimensions planned)  
   - Scale-centric staff display  
   - Curve-based note visualization

### Main Components

**KalaMain** (`kalamain.h/.cpp`)

- Application root window (QMainWindow)  
- Manages 3-tab interface: Sound Engine (tab 0), Composition (tab 1), Preferences (tab 2\)  
- Creates and orchestrates SounitBuilder and ScoreCanvasWindow  
- Signal: `onTabChanged(int)` shows/hides appropriate child windows

**SounitBuilder** (`sounitbuilder.h/.cpp`)

- Sound synthesis module (tab 0\)  
- Graph-based interface for connecting audio containers  
- Manages toolbar with 12 audio processor types (color-coded by category)  
- Handles connection editing between container ports  
- Displays container and connection inspectors  
- Current state: UI framework with functional editing logic

**Canvas** (`canvas.h/.cpp`)

- Core rendering surface for sound graph visualization  
- Draws Bezier curves for connections between containers  
- Manages pending connections (visual feedback during dragging)  
- Handles selection and deletion of containers/connections  
- Hit testing with 10px tolerance for connection clicks

**Container** (`container.h/.cpp`)

- Represents single audio processing unit in graph  
- Visual representation: colored header, input/output port circles  
- Drag-and-drop repositioning  
- Parameter management: `setParameter()`, `getParameter()`, batch updates with `beginParameterUpdate()` / `endParameterUpdate()`  
- Emits signals: `portClicked()`, `moved()`, `clicked()`, `parameterChanged()`

**ScoreCanvasWindow** (`scorecanvaswindow.h/.cpp`)

- Music composition interface (tab 1\)  
- Toolbar with draw/record tools for discrete/continuous notes  
- Current functional state:  
  - time line with now marker and loop setting  
  - note drawing in discrete and continuous modes  
  - Single and multi note select. Move selection around  
  - Delete, cut, copy and paste  
  - Zoom and pan  
  - Playback pre-rendered from loaded sounit.  
- Current half finished state  
  - Playback pre-rendered from loaded sounit (still some issues)  
  - Phrases and gesture (lots of issues)  
- Not implemented: Inspectors

### Container Categories (Audio Processors)

**Essential (Blue):**

- Harmonic Generator
- Spectrum to Signal
- Karplus Strong
- Signal Mixer
- Attack (STK-inspired instrument onset transients)

**Shaping (Orange):**

- Rolloff Processor  
- Formant Body  
- Breath Turbulence  
- Noise Color Filter

**Modifiers (Green):**

- Physics System
- Easing Applicator
- Envelope Engine
- Drift Engine
- ~~Gate Processor~~ (hidden from UI - redundant with Envelope Engine, code retained)
- LFO (Low Frequency Oscillator)

### Scale System

**Overview:**

Kala uses a flexible scale system that supports different tuning systems (Just Intonation, Pythagorean, etc.). The scale determines the frequency ratios for the 7 scale degrees (C, D, E, F, G, A, B) and is used to generate the colored horizontal lines on the score canvas.

**Core Components:**

**Scale Class** (`scale.h/.cpp`)

- Encapsulates a tuning system with 7 frequency ratios  
- Provides factory methods for predefined scales  
- Supports scale selection and retrieval by ID

**Predefined Scales:**

The scale system supports **35 predefined scales** from around the world, with 5-8 notes each.

**ScaleDialog** (`scaledialog.h/.cpp/.ui`)

- Simple dialog for selecting scale and base frequency  
- Components:  
  - **Scale dropdown:** Select from available tuning systems  
  - **Base frequency spinbox:** Set reference frequency (10-1000 Hz, default: 25 Hz)  
  - **OK/Cancel buttons:** Accept or reject scale change  
- When opened, shows current scale and base frequency  
- User selects new scale/frequency and clicks OK to apply

**Scale Display:**

- Located in Composition tab's Scale panel (bottom of main window)  
- Shows format: "1 · \[Scale Name\] · \[Base Freq\] Hz"  
- Example: "1 · Pythagorean · 25.00 Hz"  
- Click "Edit" button to open ScaleDialog

**ScoreCanvas Integration:**

- `currentScale` member stores active scale  
- `baseFrequency` member stores reference frequency (default: 25 Hz)  
- `generateScaleLines()` creates scale lines using:  
  - Base frequency × 2^octave × scale ratio  
  - Covers octaves 0-9 (typically 25 Hz to 8000 Hz range)  
  - **Variable note counts**: Supports 5-note (pentatonic), 7-note (heptatonic), and 8-note scales  
- `setScale()` updates scale and regenerates lines  
- `setBaseFrequency()` updates base frequency and regenerates lines

**Scale Line Properties:**

- Each line has:  
  - `frequencyHz`: Calculated frequency for this degree/octave  
  - `scaleDegree`: 1 to N (where N \= number of notes in scale)  
  - `color`: Extended color palette supporting up to 10 notes:  
    - 1: Red (Tonic)  
    - 2: Orange  
    - 3: Yellow  
    - 4: Green  
    - 5: Blue (Fifth in 7-note scales)  
    - 6: Indigo  
    - 7: Violet  
    - 8: Deep Pink  
    - 9: Cyan  
    - 10: Gray  
  - `noteName`: Scale-specific names (e.g., "C4", "Sa0", "Gong3", "1-2")  
  - `isThicker`: Tonic (degree 1\) and Fifth (degree 5, if scale has 5+ notes) are drawn with thicker lines


  
**Current Limitations:**

- No UI for deleting/editing existing modulations  
- Timeline visual markers disabled (will be reimplemented with better design)

**Future Enhancements:**

- **Modulation management UI**: Dialog to view/edit/delete all modulations
- **Click-to-remove modulation**: Right-click demarcation line to delete
- Additional Japanese scales (Iwato, In Sen, Yo, Kumoi)
- Bohlen-Pierce scale (non-octave scale)  
- Microtonal scales (31-TET, 53-TET, 72-TET)

**Files:**

- scale.h/cpp \- Scale data structure with 20 predefined world scales  
- scaledialog.h/cpp/ui \- Simple scale selection dialog  
- scorecanvas.h/cpp \- Scale rendering, modulation storage, time-based lookup, time-aware scale line rendering  
- timeline.h/cpp \- Timeline rendering (visual modulation markers temporarily disabled)  
- kalamain.cpp \- Scale dialog integration, modulation handling (onScaleEditClicked)

**Recent Changes (2026-01-03):**

**Visual Rendering:**

- Modified `scorecanvas.cpp::paintEvent()` to call `drawTimeAwareScaleLines()` instead of drawing all scale lines globally  
- Added `scorecanvas.cpp::drawTimeAwareScaleLines()` \- segments canvas into time regions and draws appropriate scale lines for each  
- Added `scorecanvas.cpp::generateScaleLinesForScale()` \- generates scale lines for a specific scale/baseFreq combination  
- Added `scorecanvas.cpp::drawScaleLineInRegion()` \- draws a scale line clipped to a horizontal pixel range  
- Modified `timeline.cpp::paintEvent()` \- commented out call to `drawScaleChangeMarkers()` (line 175\)

**Snapping (Note Creation):**

- Added `scorecanvas.cpp::snapToNearestScaleLineAtTime()` \- time-aware snapping that generates scale lines for the active scale at a specific time  
- Modified `scorecanvas.cpp::snapToNearestScaleLine()` \- now delegates to time-aware version with time=0 for backward compatibility  
- Updated all discrete note creation and dragging to use `snapToNearestScaleLineAtTime()` with note's start time  
- Updated `quantizePitchCurveToScale()` to accept Note parameter and use time-aware scale generation  
- All note snapping now respects the scale active at the note's time position

**Audio Rendering:**

- No changes needed \- notes store absolute frequencies (Hz) which are rendered correctly  
- Once a note is snapped to the correct scale line, that frequency is stored and played back accurately

**Files Modified:**

- scorecanvas.h \- Added method declarations for time-aware rendering and snapping  
- scorecanvas.cpp \- Implemented time-aware scale line rendering and snapping  
- timeline.cpp \- Disabled visual modulation markers (line 175\)

### Signal/Slot Architecture

All inter-component communication uses Qt signals/slots:

Container Port Click Flow:

User clicks port circle → Container::portClicked() signal

→ SounitBuilder::onPortClicked() slot

→ Stores start point OR creates connection

→ Canvas::addConnection() or Canvas::startPendingConnection()

→ Canvas::update() triggers repaint

Container Movement Flow:

Container::moved() signal

→ Canvas::update() (redraws all connections)

Graph Rebuild Flow:

Container parameter change → Container::parameterChanged() signal

→ SounitBuilder::rebuildGraph() slot

→ AudioEngine::buildGraph() rebuilds audio graph

Connection added/removed → Canvas::graphChanged() signal

→ SounitBuilder::rebuildGraph() slot

Container added → Direct call to SounitBuilder::rebuildGraph()

## Design Patterns

- **Model-View-Controller:** Canvas (view/model), SounitBuilder (controller), Container (domain)  
- **Signal-Slot:** Decoupled component communication via Qt meta-object system  
- **Composite Pattern:** Canvas contains multiple Container widgets  
- **Strategy Pattern:** Toolbar buttons use lambdas to configure different container types  
- **Event-Driven Interaction:** Mouse events → port selection → pending connections → model updates → repaints

## Code Conventions

### Rendering and Visual Feedback

- **Bezier curves** for connections: Cubic interpolation with 50+ pixel control offset  
- **Dashed lines** for pending connections during dragging  
- **Color coding:** Connection color matches source container  
- **Selection highlighting:** Blue (\#0066CC) for selected connections, 1px border for containers  
- **Hit testing:** 10px tolerance for connection selection

### Connection Logic

- Connections are **directional**: Output → Input only  
- Validation prevents same-type connections (no input→input or output→output)  
- First port click starts pending connection; second click completes or cancels  
- Connection direction auto-flips: if start is input, swap from/to in Connection struct

### Port Representation

struct PortInfo {

QString name;          // Port identifier

bool isOutput;         // Direction

QLabel \\\*circle;        // Visual indicator (●)

};

### Connection Representation

struct Connection {

Container \\\*fromContainer;  // Source (must be output port)

QString fromPort;

Container \\\*toContainer;    // Destination (must be input port)

QString toPort;

};

## Threading Model (Planned)

Per Architecture v1.4:

- **Audio Thread (highest priority):** Mix pre-rendered buffers, run live engine for active selection. Never allocates memory, waits on locks, or touches UI.  
- **UI Thread (normal):** User interaction, display updates. No heavy computation.  
- **Render Thread (low priority):** Pre-render dirty phrases in background.

**Communication:**

- UI ↔ Render: Qt signals/slots  
- UI → Audio: Lock-free SPSC queue for commands  
- Audio → UI: Lock-free SPSC queue for status updates

**Current State:** Threading partly implemented, not optimized.

## File Structure

C:\\c++code\\Kala\\

├── CMakeLists.txt              \# Build configuration

├── main.cpp                    \# Entry point (minimal)

├── kalamain.h/.cpp/.ui      \# Main tabbed window

├── sounitbuilder.h/.cpp/.ui    \# Sound engine module

├── canvas.h/.cpp               \# Graph rendering surface

├── container.h/.cpp/.ui        \# Audio unit widget

├── scorecanvaswindow.h/.cpp/.ui \# Composition module (stub)

├── docs/md format/             \# Design specifications

│   ├── Kala Architecture v1.4.md

│   ├── kala\_sounit\_builder\_canvas\_specification\_v1\_0.md

│   ├── kala\_score\_canvas\_specification\_v1.md

│   ├── Container Port Specification v1.2.md

│   └── \[other spec files\]

└── build/                      \# Qt build artifacts

## Important Specifications

**Container Port Design Principles** (from Container Port Specification v1.2):

1. **No Hard-Coded Limits:** All numeric constraints configurable via Preferences  
2. **Everything Modulatable is an Input:** Any parameter that benefits from modulation is exposed as input port with default value  
3. **Modular Composition:** Instantiate containers multiple times, chain freely, signal path must end with Signal output

**Connection Functions** (how outputs combine with inputs):

- `passthrough` — source replaces input entirely  
- `add` — input \+ source × weight  
- `multiply` — input × source × weight  
- `subtract` — input \- source × weight  
- `replace` — weighted blend  
- `modulate` — bipolar modulation around input

**Data Types:**

- Signal: Audio-rate stream (-1.0 to 1.0)  
- Spectrum: Array of harmonic amplitudes  
- Control: Single value (control or audio rate)  
- Trigger: Discrete event

## Current Implementation Status

### Implemented

- Main tabbed window with show/hide logic for child windows  
- SounitBuilder with 12 container types via toolbar  
- Canvas with Bezier curve rendering for connections  
- Container drag-and-drop, port click signaling  
- Connection creation with direction validation  
- Selection and deletion (Del key) for containers and connections  
- **Continuous note support**: Pitch and dynamics curves are sampled per-sample during rendering  
  - Discrete notes: constant pitch/dynamics  
  - Continuous notes: pitch/dynamics vary over time via curves  
  - Audio engine samples `getPitchAt(progress)` and `getDynamicsAt(progress)` every sample

### Not Yet or Partly Implemented

- Data persistence (save/load)  
- Undo/redo (planned: Qt QUndoStack)  
- Connection validation beyond direction checking  
- Marquee selection in sounit builder (works in composition)  
- Zoom/pan navigation in sounit builder  (works in composition)  
- Threading model

- ## Preferences tab

### Recently Added

- **Render Cache Binary File (2026-01-16)** \- Pre-rendered audio is now saved alongside project files for instant loading.  
    
  - .

## Key Gotchas

1. **Connection Direction:** Connections internally always store output→input, but UI logic may start from input side (direction flip happens in `SounitBuilder::onPortClicked`)  
     
2. **Qt Designer UI Files:** Changes to .ui files require rebuild to regenerate `ui_*.h` headers  
     
3. **MOC (Meta-Object Compiler):** Any class with Q\_OBJECT macro needs MOC processing — CMake handles this automatically  
   

### Architecture Overview

Track owns:

└─ Canvas (containers \+ connections)

└─ SounitGraph (compiled audio graph)

└─ QList\<Note\> (composition data)

└─ QMap\<QString, NoteRender\> noteRenders  ← NEW: Per-note audio cache

└─ Metadata (name, color, volume, etc.)

Note:

└─ id, startTime, duration, pitchHz, curves...

└─ bool renderDirty  ← NEW: Flag for re-rendering

NoteRender (new struct):

└─ std::vector\<float\> samples    ← Rendered audio for this note

└─ double sampleRate             ← Sample rate used

└─ uint64\_t graphHash            ← Hash of graph state when rendered

└─ uint64\_t noteHash             ← Hash of note properties when rendered

## Related Documentation

All design specifications are in `docs/md format/`. Key documents:

- **Kala Architecture v1.4.md** — System overview, threading, scope  
- **kala\_sounit\_builder\_canvas\_specification\_v1\_0.md** — Sound engine canvas interaction patterns  
- **kala\_score\_canvas\_specification\_v1.md** — Composition interface design  
- **Container Port Specification v1.2.md** — Complete port definitions for all 12 container types  
- **Kala Input Engine Specification v1.2.md** — Pen input capture design

Refer to these when implementing features or understanding design intent.  
