# AGENTS.md - Kala Project Guide

**For AI coding agents working on the Kala codebase.**

---

## Project Overview

**Kala** is a revolutionary compositional tool that breaks from traditional keyboard/MIDI thinking. Using a pen tablet as primary input, it captures expressive gesture as notation itself — the act of composing IS the performance. The application features:

- **Pen-tablet input** - No MIDI, no piano roll, direct gesture capture
- **Container-graph synthesis** - Visual sound design using "sounits" 
- **Tuning-agnostic foundation** - Voices share sonic DNA across any tuning system
- **Hybrid sound engine** - Physics modeling, wavetable synthesis, and parameter coupling
- **Real-time and pre-rendered audio** - Flexible playback options
- **Built-in AI agent (Anima)** - Assists with composition and sound design
- **Per-track variations** - Polyphonic routing across different sound graphs

**Technology Stack:**
- **Language:** C++17
- **Framework:** Qt 6.10+ (Widgets, Concurrent, Network)
- **Build System:** CMake 3.19+ (primary), qmake (legacy)
- **Compiler:** MinGW-w64 (Windows target)
- **Audio:** RTAudio 6.0.1, FFTW3 3.3.5
- **Platform:** Windows (primary deployment target)

---

## Build and Development

### Build Commands

**Debug build:**
```bash
cd build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug
cmake --build . --config Debug -j8
```

**Release build:**
```bash
cd build/Desktop_Qt_6_10_1_MinGW_64_bit-Release  
cmake --build . --config Release -j8
```

**Full clean build:**
```bash
# From project root
rm -rf build
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . -j8
```

### Dependencies

**Core dependencies (auto-fetched via CMake):**
- Qt 6.5+ (Components: Core, Widgets, Concurrent, Network)
- RTAudio 6.0.1 (audio I/O)

**Manual dependencies:**
- FFTW3 3.3.5 (pre-built Windows DLLs in `fftw-3.3.5-dll64/`)

**Required paths:**
- Qt: `C:/Qt/6.10.1/mingw_64/bin/`
- MinGW: `C:/Qt/Tools/mingw1310_64/bin/`

### Packaging

```bash
bash scripts/package.sh
```

Creates `dist/Kala/` folder with all dependencies and `dist/Kala.zip` for distribution.

### Testing

**No automated test suite currently exists.** Testing is done manually:
- Launch from Qt Creator or built executable
- Test audio playback, sounit loading, note input
- Verify AI agent connectivity if applicable

---

## Code Organization

### Directory Structure

```
Kala/
├── CMakeLists.txt           # Main build configuration
├── Kala.pro                 # Legacy qmake project (auto-generated)
├── main.cpp                 # Application entry point
├── kalamain.h/cpp           # Main window and orchestration
├── audioengine.h/cpp        # Real-time audio and playback
├── trackmanager.h/cpp       # Track lifecycle management
├── track.h/cpp              # Individual track implementation
├── sounitbuilder.h/cpp      # Sound design graph editor
├── scorecanvas.h/cpp        # Composition canvas and note input
├── canvas.h/cpp             # Container graph visualization
├── container.h/cpp          # Synthesis container widgets
├── sounitgraph.h/cpp        # Compiled audio graph
├── note.h/cpp               # Note data model
├── curve.h/cpp              # Parameter curve implementation
├── *model.h/cpp             # Instrument models (flute, guitar, etc.)
├── *processor.h/cpp         # Audio processing units
├── llmclient.h/cpp          # LLM API client
├── kalatools.h/cpp          # AI agent tool dispatcher
├── kalaagent.h/cpp          # AI agent conversation manager
├── companionpanel.h/cpp     # AI chat interface
├── help/                    # HTML documentation
├── scripts/                 # Build and utility scripts
└── fftw-3.3.5-dll64/        # FFTW library
```

### Key Module Divisions

**1. Core Application (kalamain.h/cpp)**
- Main window orchestration
- Menu and toolbar management
- Project file I/O (.kala format)
- Sounit file management (.sounit format)
- Integration of all subsystems

**2. Audio Engine (audioengine.h/cpp)**
- RTAudio integration for real-time output
- Pre-rendered buffer playback
- Track-based polyphonic rendering
- Graph-based synthesis dispatch

**3. Track System (trackmanager.h/cpp, track.h/cpp)**
- Multi-track composition support
- Per-track sounit and variation management
- Note storage and rendering cache
- Volume, pan, mute controls

**4. Score Canvas (scorecanvas.h/cpp, scorecanvaswindow.h/cpp)**
- Pen-based note input and editing
- Dynamics curves, pitch curves, vibrato
- Expressive curve system
- Undo/Redo command stack
- Scale and tempo management

**5. Sounit Builder (sounitbuilder.h/cpp, canvas.h/cpp, container.h/cpp)**
- Visual graph editor for sound design
- 26 container types (synthesis, processing, modulation)
- Port-based connections with functions
- Parameter inspection and editing
- Sounit save/load (.sounit JSON format)

**6. Audio Graph (sounitgraph.h/cpp)**
- Container graph compilation
- Real-time audio processing
- Parameter modulation
- Spectrum/signal flow

**7. Instrument Models (*model.h/cpp)**
- Faust-generated physical models
- Instruments: flute, guitar, oud, piano, bass, Tibetan bowl, bowed, saxophone, percussion, recorder
- Per-note polyphonic rendering

**8. AI Agent (llmclient.h/cpp, kalatools.h/cpp, kalaagent.h/cpp, companionpanel.h/cpp)**
- OpenAI-compatible and Anthropic API support
- Tool-based interaction (50+ tools)
- Composition and sound design assistance
- Template system for common operations

**9. Audio Processors (*processor.h/cpp)**
- HarmonicGenerator, SpectrumToSignal, EnvelopeEngine
- Filters (bandpass, comb, low/highpass, IR convolution)
- Modulators (LFO, DriftEngine, PhysicsSystem)
- Effects (RolloffProcessor, SpectrumBlender, FormantBody)

---

## Development Conventions

### Code Style

**Naming conventions:**
- Classes: `PascalCase` (e.g., `AudioEngine`, `SounitBuilder`)
- Methods: `camelCase` (e.g., `getSampleRate()`, `setPitchHz()`)
- Member variables: `m_camelCase` (e.g., `m_sampleRate`, `m_currentTrack`)
- Constants: `UPPER_SNAKE_CASE` or `kPascalCase`
- Files: `lowercase` or `PascalCase` matching class names

**Qt-specific:**
- Use Qt containers (`QVector`, `QMap`, `QString`) over STL
- Signal/slot connections for component communication
- `Q_PROPERTY` for exposed properties
- Qt-style memory management (parent-child hierarchy)

**Memory management:**
- Use `QScopedPointer`/`QSharedPointer` where appropriate
- Follow Qt parent-child automatic cleanup
- Explicit `delete` only for non-Qt objects
- Raw pointers for non-owning references

**File organization:**
- Header files: class declarations, inline methods
- Implementation files: method definitions
- UI files: `.ui` files for Qt Designer layouts
- Separate `commands` files for undo/redo operations

### Project-Specific Patterns

**1. Container-Based Synthesis**
```cpp
// Creating a container
Container* container = new Container(type, name, color, inputs, outputs);
container->setParameter("frequency", 440.0);
canvas->addContainer(container);

// Connecting containers
Canvas::Connection conn;
conn.fromContainer = sourceContainer;
conn.fromPort = "spectrum";
conn.toContainer = targetContainer;
conn.toPort = "spectrumIn";
conn.function = "passthrough";
conn.weight = 1.0;
canvas->addConnection(conn);
```

**2. Note Creation and Manipulation**
```cpp
// Create a note
Note note(startTime, duration, pitchHz, dynamics);
note.setTrackIndex(trackIndex);
note.setDynamicsCurve(curve);
track->addNote(note);

// Modify with undo support
AddNoteCommand* cmd = new AddNoteCommand(track, note);
scoreCanvas->getUndoStack()->push(cmd);
```

**3. Audio Graph Building**
```cpp
// Build graph for a track
SounitGraph* graph = new SounitGraph(sampleRate);
graph->buildFromCanvas(canvas, trackIndex);
audioEngine->buildGraph(graph, trackIndex);
```

**4. AI Tool Implementation**
```cpp
// In kalatools.cpp
QJsonObject KalaTools::add_note(const QJsonObject& params) {
    double startTime = params["startTime"].toDouble();
    double duration = params["duration"].toDouble();
    double pitchHz = params["pitchHz"].toDouble();
    
    Note note(startTime, duration, pitchHz, 0.7);
    QString id = m_scoreCanvas->addNote(note);
    
    QJsonObject result;
    result["id"] = id;
    result["message"] = "Note added";
    return result;
}
```

### Constants and Magic Numbers

**Audio defaults:**
- Sample rate: 44100 Hz (configurable: 44100/48000/96000/192000)
- Buffer size: 256 frames (configurable)
- Pitch range: 20 Hz - 20 kHz

**Note defaults:**
- Default dynamics: 0.7
- Default duration: 500 ms
- Minimum duration: 10 ms

**Canvas defaults:**
- Default zoom: 1.0
- Container colors: Category-based (Essential=Blue, Shaping=Orange, Modifiers=Green, Filters=Purple)

---

## Testing Strategy

### Manual Testing Areas

**1. Audio Engine:**
- Real-time playback stability
- Pre-rendered buffer accuracy
- Multi-track polyphonic rendering
- Device change handling

**2. Sounit Builder:**
- Container creation and deletion
- Parameter editing and graph updates
- Connection management
- Save/load functionality

**3. Score Canvas:**
- Pen input responsiveness
- Note selection and manipulation
- Curve editing (dynamics, pitch, expressive)
- Undo/Redo reliability

**4. AI Agent:**
- Tool execution accuracy
- API connectivity (OpenAI, Anthropic, Ollama)
- Response formatting
- Error handling

### Known Issues to Watch For

**Audio glitches:**
- Buffer underflow during complex scenes
- Parameter modulation timing issues
- Graph rebuild overhead

**Memory leaks:**
- Container deletion cleanup
- Note render cache invalidation
- AI response history growth

**Platform-specific:**
- Windows audio device enumeration
- Pen tablet driver compatibility
- File path handling (forward vs backslash)

---

## Security Considerations

### File I/O Security

**Project files (.kala):**
- Contains user composition data
- Validate JSON structure on load
- Sanitize file paths to prevent directory traversal

**Sounit files (.sounit):**
- Contains sound design graphs
- Validate container types and parameters
- Limit container count to prevent DoS

**External files:**
- WAV files for wavetables and IR convolution
- Validate file format and dimensions
- Limit file size to prevent memory exhaustion

### API Security

**LLM API keys:**
- Stored in QSettings (plaintext)
- Never log or expose API keys
- Support for local models (Ollama) reduces exposure

**Network requests:**
- Use HTTPS for API calls
- Validate response JSON structure
- Implement timeout handling

### User Input Validation

**Pen input:**
- Validate coordinate ranges
- Prevent negative durations
- Clamp pitch to audible range

**AI agent input:**
- Sanitize tool parameters
- Validate file paths
- Limit recursion depth

---

## Deployment Process

### Build Configuration

**Release build requirements:**
- MinSizeRel or Release configuration
- All Qt libraries deployed via `windeployqt`
- FFTW3 DLL included
- Help documentation copied

### Distribution Structure

```
Kala/
├── Kala.exe                  # Main executable
├── Qt6*.dll                  # Qt runtime libraries
├── platforms/                # Qt platform plugins
├── libfftw3f-3.dll          # FFTW library
├── help/                     # HTML documentation
└── [other Qt dependencies]
```

### Version Management

**Version info:**
- Stored in project metadata
- Displayed in About dialog
- Used for project file compatibility

**Project file format:**
- JSON-based with version field
- Backward compatibility maintained when possible
- Migration logic for older formats

### Installation

**No installer currently.** Distribution via:
- ZIP archive containing portable folder
- User extracts and runs `Kala.exe`
- Optional desktop shortcut creation

---

## Important References

### Internal Documentation

**`CLAUDE.md`** - Quick reference for AI agents
**`Claude-sounit-builder.md`** - Complete sounit format specification, container types, ports, parameters
**`Claude-companion.md`** - Anima AI agent architecture and tool reference (50+ tools documented)
**`anima-templates.md`** - Pre-built AI agent templates for common operations

### External Documentation

**`help/`** - User-facing HTML documentation (generated from `docs/` via Pandoc)
**`docs/md format/`** - Design specifications and architecture documents

### Key Implementation Files

**Audio synthesis:**
- `sounitgraph.cpp` - Graph compilation and processing
- `harmonicgenerator.cpp` - Spectrum generation
- `spectrumtosignal.cpp` - Spectrum to audio conversion

**Instrument models:**
- `flute.cpp`, `guitar.cpp`, `oud.cpp`, etc. - Faust-generated models
- `instrument.h` - LookupTable utility for calibration data

**AI integration:**
- `kalatools.cpp` - Tool implementations (3000+ lines)
- `kalaagent.cpp` - Conversation management
- `llmclient.cpp` - API client with Anthropic adaptation

---

## Common Tasks

### Adding a New Container Type

1. Define ports in `canvas.cpp::getPortsForContainerType()`
2. Add default parameters in `sounitbuilder.cpp`
3. Create UI in `kalamain.cpp::populate*Inspector()`
4. Implement audio processing in corresponding `*processor.cpp` or model
5. Update documentation in `Claude-sounit-builder.md`

### Adding a New AI Tool

1. Add tool declaration in `kalatools.h`
2. Implement tool in `kalatools.cpp`
3. Add to tool schema in `kalaagent.cpp::buildToolSchemas()`
4. Update `Claude-companion.md` with documentation
5. Test with various AI providers

### Modifying Note Behavior

1. Update `note.h` data model if needed
2. Modify rendering in `note.cpp` or `track.cpp`
3. Update undo commands in `scorecanvascommands.cpp`
4. Test AI agent tools that manipulate notes
5. Verify project file save/load

### Audio Graph Debugging

1. Check `audioengine.cpp` for graph building
2. Verify `sounitgraph.cpp` processing chain
3. Monitor parameter modulation in container connections
4. Use spectrum visualizer for signal inspection
5. Check render cache invalidation in `track.cpp`

---

## Platform-Specific Notes

### Windows Development

**Required software:**
- Qt 6.10+ with MinGW compiler
- Git for Windows (for bash scripts)
- Pandoc (for documentation building)
- Visual Studio or Qt Creator as IDE

**Known Windows quirks:**
- Path separators: Use forward slashes in CMake, backslashes in Windows API
- Audio device: Use WASAPI (default) or ASIO (requires additional setup)
- Console output: May not appear in GUI apps - use `qDebug()` or file logging

### Performance Considerations

**Real-time constraints:**
- Audio callback must complete within buffer time (~5.8ms at 44.1kHz/256 frames)
- Avoid memory allocation in audio thread
- Use lock-free queues for cross-thread communication

**Rendering optimization:**
- Per-note render cache with hash-based invalidation
- Lazy rendering for off-screen notes
- Background rendering for long compositions

---

## Troubleshooting

### Build Issues

**CMake can't find Qt:**
```bash
set CMAKE_PREFIX_PATH=C:/Qt/6.10.1/mingw_64
```

**Linker errors with FFTW:**
- Ensure `libfftw3f.dll.a` is in the include path
- Verify DLL architecture matches (64-bit)

### Runtime Issues

**No audio output:**
- Check audio device selection in preferences
- Verify RTAudio initialization in `audioengine.cpp`
- Test with different buffer sizes

**Pen input not working:**
- Verify tablet driver installation
- Check Qt tablet support in `kalamain.cpp`
- Test with different tablet models

### AI Agent Issues

**API connection failures:**
- Verify API key in QSettings
- Check network connectivity
- Test with different LLM providers

**Tool execution errors:**
- Check parameter validation in `kalatools.cpp`
- Verify object pointers are valid
- Review error messages in companion panel

---

## Contact and Contribution

**Project status:** Active development
**Primary language:** English (code comments and documentation)
**Coding standards:** Qt best practices with project-specific conventions

**Before making changes:**
1. Read the relevant reference documentation
2. Understand existing patterns in similar code
3. Test thoroughly with audio and AI features
4. Update documentation if adding new features

**Areas needing attention:**
- Automated test suite
- Linux/macOS porting
- Performance profiling and optimization
- Extended instrument model library

---

*This AGENTS.md file is maintained as part of the Kala project. Last updated based on codebase exploration.*
