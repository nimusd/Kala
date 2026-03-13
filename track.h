#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QVector>
#include <QSet>
#include <QJsonObject>
#include <vector>
#include <QMap>
#include <mutex>
#include <atomic>
#include "note.h"
#include "canvas.h"
#include "sounitgraph.h"
#include "container.h"
#include "rendersegment.h"
#include "noterender.h"
#include "sounitvariation.h"

/**
 * Track - Self-contained audio track with sounit and notes
 *
 * Core design principle: Each Track is completely independent, owning all its data:
 * - Sounit (containers, connections, compiled graph)
 * - Notes (composition data)
 * - Render cache (pre-rendered audio segments)
 * - Metadata (name, color, volume, mute/solo)
 *
 * This eliminates the need for external state tracking and makes track switching trivial.
 *
 * Usage example:
 *   Track* track = new Track(0, "Piano", QColor("#3498db"));
 *   track->loadSounit("path/to/sounit.json");
 *   track->addNote(Note(0.0, 1000.0, 440.0, 0.7));
 *   track->prerender(44100);
 *   track->saveSounit("path/to/sounit.json");
 */
class Track : public QObject
{
    Q_OBJECT

public:
    // ========== Construction / Destruction ==========

    /**
     * Create a new track
     * @param trackId Unique track identifier (0-based index)
     * @param name Display name for the track
     * @param color Visual color for notes and UI
     * @param parent Qt parent object
     */
    explicit Track(int trackId, const QString &name, const QColor &color, QObject *parent = nullptr);

    ~Track();

    // ========== Track Metadata ==========

    int getTrackId() const { return m_trackId; }
    QString getName() const { return m_name; }
    void setName(const QString &name);

    QColor getColor() const { return m_color; }
    void setColor(const QColor &color);

    // ========== Sounit Management ==========

    /**
     * Get the canvas for editing this track's sounit
     * Each track owns its own Canvas instance with its own containers/connections.
     * Call show() on the returned canvas to display it in the Sound Engine tab.
     *
     * @return Pointer to this track's canvas (never null)
     */
    Canvas* getCanvas() const { return m_canvas; }

    /**
     * Get the compiled audio graph for this track
     * The graph is built from the canvas containers/connections.
     * Returns nullptr if graph is invalid or hasn't been built yet.
     *
     * @return Pointer to graph, or nullptr if invalid
     */
    SounitGraph* getGraph() const { return m_graph; }

    /**
     * Check if this track has a valid compiled graph for audio rendering
     */
    bool hasValidGraph() const { return m_graph != nullptr && m_graph->isValid(); }

    /**
     * Get the sounit name (from loaded file metadata)
     */
    QString getSounitName() const { return m_sounitName; }

    /**
     * Get the sounit comment (from loaded file metadata)
     */
    QString getSounitComment() const { return m_sounitComment; }

    /**
     * Get the file path this sounit was loaded from (empty if not loaded)
     */
    QString getSounitFilePath() const { return m_sounitFilePath; }

    /**
     * Check if sounit has unsaved changes
     */
    bool isSounitDirty() const { return m_sounitDirty; }

    /**
     * Clear the sounit dirty flag (used after project loading)
     */
    void clearSounitDirty() { m_sounitDirty = false; }

    /**
     * Load sounit from JSON file
     * Replaces all containers and connections in the canvas.
     * Invalidates the compiled graph and render cache.
     *
     * @param filePath Path to .sounit file
     * @return true if loaded successfully, false on error
     */
    bool loadSounit(const QString &filePath);

    /**
     * Save sounit to JSON file
     * Saves current canvas state (containers, connections, parameters).
     * Clears the dirty flag on success.
     *
     * @param filePath Path to save .sounit file
     * @return true if saved successfully, false on error
     */
    bool saveSounit(const QString &filePath);

    /**
     * Create new empty sounit
     * Clears all containers and connections, resets to default state.
     * Invalidates graph and render cache.
     */
    void newSounit();

    /**
     * Build/rebuild the audio graph from current canvas state
     * Must be called after editing containers/connections to hear changes.
     * Automatically invalidates render cache when graph changes.
     *
     * @param sampleRate Audio sample rate (e.g., 44100 or 48000)
     * @return true if graph is valid, false if invalid (will use fallback)
     */
    bool rebuildGraph(double sampleRate);

    // ========== Note Management ==========

    /**
     * Get all notes on this track (read-only)
     */
    const QList<Note>& getNotes() const { return m_notes; }

    /**
     * Get all notes on this track (mutable)
     * Use with caution - modifying notes requires calling markNotesDirty()
     */
    QList<Note>& getNotesRef() { return m_notes; }

    /**
     * Add a note to this track
     * Marks notes as dirty and invalidates affected render segments.
     *
     * @param note Note to add
     */
    void addNote(const Note &note);

    /**
     * Remove a note by its ID
     * Marks notes as dirty and invalidates affected render segments.
     *
     * @param noteId Unique note identifier
     * @return true if note was found and removed
     */
    bool removeNote(const QString &noteId);

    /**
     * Update an existing note
     * Finds note by ID and replaces it with new data.
     * Marks notes as dirty and invalidates affected render segments.
     *
     * @param note Updated note (ID must match existing note)
     * @return true if note was found and updated
     */
    bool updateNote(const Note &note);

    /**
     * Clear all notes
     * Invalidates entire render cache.
     */
    void clearNotes();

    /**
     * Check if notes have unsaved changes
     */
    bool areNotesDirty() const { return m_notesDirty; }

    /**
     * Mark notes as dirty (call this if you modify notes via getNotesRef())
     */
    void markNotesDirty() { m_notesDirty = true; emit notesDirtyChanged(true); }

    /**
     * Clear notes dirty flag (call after saving composition)
     */
    void clearNotesDirty() { m_notesDirty = false; emit notesDirtyChanged(false); }

    // ========== Variation Management ==========

    /**
     * Create a new variation from current canvas state
     * The variation is a snapshot of the current sounit graph.
     * Once created, variations are immutable.
     *
     * @param name Display name for the variation
     * @return Variation index (1-based), or 0 on failure
     */
    int createVariation(const QString &name);

    /**
     * Create a variation from external JSON graph data (no canvas involvement)
     * Used when loading a saved .sounit file as a variation instead of replacing
     * the base sounit. The main canvas is left untouched.
     *
     * @param graphData JSON object with "containers" and "connections" arrays
     * @param name Display name for the variation
     * @return Variation index (1-based), or 0 on failure
     */
    int createVariationFromJson(const QJsonObject &graphData, const QString &name);

    /**
     * Delete a variation by index
     * Notes using this variation will fall back to base sounit.
     *
     * @param index Variation index (1-based)
     * @return true if variation was found and deleted
     */
    bool deleteVariation(int index);

    /**
     * Get the number of variations (not counting base sounit)
     */
    int getVariationCount() const { return m_variations.size(); }

    /**
     * Get variation name by index
     * @param index Variation index (1-based)
     * @return Variation name, or empty string if index invalid
     */
    QString getVariationName(int index) const;

    /**
     * Set variation name
     * @param index Variation index (1-based)
     * @param name New name
     * @return true if variation was found and renamed
     */
    bool setVariationName(int index, const QString &name);

    /**
     * Get the graph for a specific variation
     * @param index 0 = base sounit, 1+ = variation index
     * @return SounitGraph pointer, or base graph if index invalid
     */
    SounitGraph* getGraphForVariation(int index);

    /**
     * Get list of all variation names (for populating UI menus)
     * Does NOT include base sounit.
     */
    QStringList getVariationNames() const;

    /**
     * Find a variation by name
     * @param name Variation name to search for
     * @return Variation index (1-based) if found, 0 if not found
     */
    int findVariationByName(const QString &name) const;

    /**
     * Create or update an internal variation from current canvas state.
     * Used by Note Mode to auto-save per-note graphs.
     *
     * @param existingIndex If > 0 and that variation isInternal, update in-place; otherwise create new
     * @return Variation index (1-based), or 0 on failure
     */
    int createOrUpdateInternalVariation(int existingIndex = 0);

    /**
     * Check if a variation is internal (auto-created by Note Mode)
     * @param index Variation index (1-based)
     * @return true if the variation exists and is internal
     */
    bool isInternalVariation(int index) const;

    /**
     * Load a variation's state into the main canvas for editing
     * This copies the variation's serialized graph data into the main canvas.
     * After loading, the canvas will show the variation's containers/connections.
     * Any edits will NOT affect the original variation (variations are immutable).
     *
     * @param index Variation index (1-based), or 0 to restore base sounit
     * @return true if loaded successfully
     */
    bool loadVariationToCanvas(int index);

    /**
     * Save current canvas state as the base sounit snapshot
     * Call this before loading any variation to preserve the base state.
     */
    void saveBaseCanvasState();

    /**
     * Check if base canvas state has been saved
     */
    bool hasBaseCanvasState() const { return !m_baseCanvasState.isEmpty(); }

    // ========== Audio Rendering ==========

    /**
     * Pre-render all notes into audio segments
     * Uses the compiled graph to render notes into the segment cache.
     * Intelligently re-renders only changed segments (partial re-render).
     *
     * @param sampleRate Audio sample rate
     * @param segmentDurationMs Duration of each segment in milliseconds (default: 1000ms)
     * @return true if render succeeded, false on error
     */
    bool prerender(double sampleRate, double segmentDurationMs = 1000.0);

    /**
     * Get render segments for audio playback
     * Access pre-rendered audio data. Each segment contains a buffer of samples.
     *
     * @return Vector of render segments (may be empty if not rendered yet)
     */
    const QVector<RenderSegment>& getRenderSegments() const { return m_renderSegments; }

    /**
     * Check if render cache needs updating
     * True if notes changed, graph changed, or never rendered.
     */
    bool isRenderDirty() const { return m_renderDirty; }

    /**
     * Force render cache to be dirty (invalidate all segments)
     */
    void invalidateRenderCache();

    /**
     * Clear render cache entirely (frees memory)
     */
    void clearRenderCache();

    // ========== Note-Based Rendering (New System) ==========

    /**
     * Pre-render a single note
     * Renders the note's audio into its NoteRender cache.
     *
     * @param note The note to render
     * @return true if render succeeded
     */
    bool prerenderNote(const Note &note);

    /**
     * Pre-render all dirty notes
     * Only renders notes that are marked dirty or have no cached render.
     * This is the main entry point for the new rendering system.
     *
     * @param sampleRate Audio sample rate
     * @return true if all renders succeeded
     */
    bool prerenderDirtyNotes(double sampleRate);

    /**
     * Cancel an in-progress render
     * Sets atomic flag checked by prerenderDirtyNotes/prerenderNote loops.
     * Safe to call from any thread (e.g., during processEvents in key handler).
     */
    void cancelRender();

    /**
     * Check if a render is currently in progress
     */
    bool isRendering() const { return m_rendering.load(); }

    /**
     * Invalidate a specific note's render cache
     * Call this when a note's content changes (pitch, duration, curves).
     * NOT needed when note just moves in time.
     *
     * @param noteId The note ID to invalidate
     */
    void invalidateNoteRender(const QString &noteId);

    /**
     * Invalidate all note renders
     * Call this when the graph changes (all notes need re-rendering).
     */
    void invalidateAllNoteRenders();

    /**
     * Get the render for a specific note
     * @param noteId The note ID
     * @return Pointer to NoteRender, or nullptr if not found
     */
    const NoteRender* getNoteRender(const QString &noteId) const;

    /**
     * Get all note renders (for cache serialization)
     * @return Reference to the note renders map
     */
    const QMap<QString, NoteRender>& getNoteRenders() const { return m_noteRenders; }

    /**
     * Set a note render (for cache loading)
     * @param noteId The note ID
     * @param render The render to set
     */
    void setNoteRender(const QString &noteId, const NoteRender &render);

    /**
     * Get current graph hash (for cache validation)
     * @return Current graph state hash
     */
    uint64_t getGraphHash() const { return m_graphHash; }

    /**
     * Set graph hash (used during project loading to preserve cache validity)
     * @param hash The hash value to set
     */
    void setGraphHash(uint64_t hash) { m_graphHash = hash; }

    /**
     * Check if any notes need rendering
     */
    bool hasRenderWork() const;

    /**
     * Get mixed audio buffer for a time range (for playback)
     * Mixes all note renders that overlap the time range.
     * Thread-safe: locks internal mutex during access.
     *
     * @param startTimeMs Start time in milliseconds
     * @param durationMs Duration in milliseconds
     * @return Mixed audio buffer
     */
    std::vector<float> getMixedBuffer(double startTimeMs, double durationMs);

    /**
     * Returns the actual end time including rendered tails (reverb/decay ring-out).
     */
    double getRenderedEndTimeMs() const;

    /**
     * Thread-safe note synchronization
     * Replaces all notes with the provided list.
     * Use this instead of getNotesRef() when called from UI thread during playback.
     *
     * @param notes New notes to set
     */
    void syncNotes(const QList<Note> &notes);

    /**
     * Lock the playback mutex
     * Call this before accessing notes/renders from UI thread during playback setup.
     * Must be paired with unlockPlayback().
     */
    void lockPlayback() { m_playbackMutex.lock(); }

    /**
     * Unlock the playback mutex
     */
    void unlockPlayback() { m_playbackMutex.unlock(); }

    // ========== Playback State ==========

    /**
     * Check if track is muted (silent during playback)
     */
    bool isMuted() const { return m_muted; }
    void setMuted(bool muted);

    /**
     * Check if track is soloed (only soloed tracks play)
     */
    bool isSolo() const { return m_solo; }
    void setSolo(bool solo);

    /**
     * Get track volume (0.0 = silent, 1.0 = full)
     */
    float getVolume() const { return m_volume; }
    void setVolume(float volume);

    /**
     * Get track gain (0.0 = silent, 1.0 = unity, 8.0 = +18dB)
     */
    float getGain() const { return m_gain; }
    void setGain(float gain);

    /**
     * Get track pan (-1.0 = full left, 0.0 = center, 1.0 = full right)
     */
    float getPan() const { return m_pan; }
    void setPan(float pan);

    // ========== Serialization ==========

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

signals:
    // ========== Signals ==========

    /**
     * Emitted when track name changes
     */
    void nameChanged(const QString &name);

    /**
     * Emitted when track color changes
     */
    void colorChanged(const QColor &color);

    /**
     * Emitted when track gain changes
     */
    void gainChanged(float gain);

    /**
     * Emitted when sounit is loaded or created
     */
    void sounitLoaded(const QString &sounitName);

    /**
     * Emitted when sounit dirty state changes
     */
    void sounitDirtyChanged(bool dirty);

    /**
     * Emitted when graph is rebuilt
     */
    void graphRebuilt(bool isValid);

    /**
     * Emitted when a note is added
     */
    void noteAdded(const Note &note);

    /**
     * Emitted when a note is removed
     */
    void noteRemoved(const QString &noteId);

    /**
     * Emitted when a note is updated
     */
    void noteUpdated(const Note &note);

    /**
     * Emitted when notes dirty state changes
     */
    void notesDirtyChanged(bool dirty);

    /**
     * Emitted when render starts
     */
    void renderStarted();

    /**
     * Emitted during rendering (progress 0-100)
     */
    void renderProgressChanged(int percent);

    /**
     * Emitted when render completes
     */
    void renderCompleted();

    /**
     * Emitted when render is cancelled via Escape key
     */
    void renderCancelled();

    /**
     * Emitted when render cache is invalidated
     */
    void renderCacheInvalidated();

    /**
     * Emitted when mute state changes
     */
    void muteChanged(bool muted);

    /**
     * Emitted when solo state changes
     */
    void soloChanged(bool solo);

    /**
     * Emitted when volume changes
     */
    void volumeChanged(float volume);

    /**
     * Emitted when pan changes
     */
    void panChanged(float pan);

    /**
     * Emitted when a variation is created
     */
    void variationCreated(int index, const QString &name);

    /**
     * Emitted when a variation is deleted
     */
    void variationDeleted(int index);

    /**
     * Emitted when a variation is renamed
     */
    void variationRenamed(int index, const QString &name);

private slots:
    /**
     * Handle canvas graph changes (connections added/removed)
     */
    void onCanvasGraphChanged();

    /**
     * Handle container parameter changes
     */
    void onContainerParameterChanged();

    /**
     * Handle sounit name changes (from canvas)
     */
    void onSounitNameChanged(const QString &name);

private:
    // ========== Member Variables ==========

    // Track identity
    int m_trackId;
    QString m_name;
    QColor m_color;

    // Sounit (sound graph)
    Canvas *m_canvas;              // Owned by this Track - contains containers/connections
    SounitGraph *m_graph;          // Owned by this Track - compiled audio graph
    QString m_sounitName;          // From loaded file metadata
    QString m_sounitComment;       // From loaded file metadata
    QString m_sounitFilePath;      // Path to loaded .sounit file (empty if new)
    bool m_sounitDirty;            // Has unsaved sounit changes?
    bool m_loadingInProgress;      // Suppress dirty marking during loading
    double m_sampleRate;           // Sample rate used for graph compilation

    // Notes (composition)
    QList<Note> m_notes;           // All notes on this track
    bool m_notesDirty;             // Has unsaved note changes?

    // Render cache (segment-based - legacy)
    QVector<RenderSegment> m_renderSegments;  // Pre-rendered audio data
    QList<Note> m_cachedNotes;                // Notes used for last render (for change detection)
    bool m_renderDirty;                       // Needs re-render?
    double m_segmentDurationMs;               // Segment size for rendering

    // Render cache (note-based - new system)
    QMap<QString, NoteRender> m_noteRenders;  // Per-note rendered audio (key = note ID)
    uint64_t m_graphHash;                     // Hash of current graph state (for invalidation)

    // Variations (immutable snapshots of the sounit)
    QList<SounitVariation*> m_variations;     // Owned pointers, index 0 in list = variation 1
    QJsonObject m_baseCanvasState;            // Saved base sounit state for restoration

    // Playback state
    bool m_muted;
    bool m_solo;
    float m_volume;  // 0.0 to 1.0
    float m_gain;    // 0.0 to 8.0 (unity = 1.0, max = +18dB)
    float m_pan;     // -1.0 (left) to 1.0 (right), 0.0 = center

    // Thread safety
    mutable std::mutex m_playbackMutex;  // Protects m_notes and m_noteRenders during playback

    // Render cancellation
    std::atomic<bool> m_cancelRender{false};
    std::atomic<bool> m_rendering{false};

    // ========== Helper Methods ==========

    /**
     * Connect all canvas signals to track slots
     */
    void connectCanvasSignals();

    /**
     * Mark sounit as dirty (unsaved changes)
     */
    void markSounitDirty();

    /**
     * Invalidate render segments that overlap with a note's time range
     */
    void invalidateSegmentsForNote(const Note &note);

    /**
     * Detect which notes changed and invalidate affected segments
     */
    void detectNoteChangesAndInvalidate();

    /**
     * Pure rendering logic for a single note (no side effects on m_noteRenders)
     * Thread-safe: only reads shared state, writes to local outRender buffer.
     *
     * @param note The note to render
     * @param graph Pre-cloned graph instance (caller owns)
     * @param outRender Output render buffer
     * @return true if render succeeded
     */
    bool renderNoteImpl(const Note &note, SounitGraph *graph, NoteRender &outRender) const;

    /**
     * Build a SounitGraph from serialized JSON data
     * Used for compiling variation graphs.
     * Populates the variation's compiledGraph and sourceCanvas fields.
     *
     * @param graphData JSON object containing containers and connections
     * @param sampleRate Sample rate for the graph
     * @param outVariation Variation to populate with graph and canvas (takes ownership)
     */
    void buildGraphFromJson(const QJsonObject &graphData, double sampleRate, SounitVariation* outVariation);
};

#endif // TRACK_H
