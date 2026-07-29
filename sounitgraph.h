#ifndef SOUNITGRAPH_H
#define SOUNITGRAPH_H

#include "container.h"
#include "canvas.h"
#include "harmonicgenerator.h"
#include "spectrumtosignal.h"
#include "signalmixer.h"
#include "rolloffprocessor.h"
#include "spectrumblender.h"
#include "formantbody.h"
#include "breathturbulence.h"
#include "noisecolorfilter.h"
#include "physicssystem.h"
#include "envelopeengine.h"
#include "driftengine.h"
#include "gateprocessor.h"
#include "easingapplicator.h"
#include "karplusstrongattack.h"
#include "attackgenerator.h"
#include "lfo.h"
#include "frequencymapper.h"
#include "wavetablesynth.h"
#include "bandpasseq.h"
#include "combfilter.h"
#include "lowhighpassfilter.h"
#include "irconvolution.h"
#include "recordermodel.h"
#include "bowedmodel.h"
#include "saxophonemodel.h"
#include "saxophonemodel2.h"
#include "flutemodel.h"
#include "pianomodel.h"
#include "bassmodel.h"
#include "tibetanbowlmodel.h"
#include "tailprocessor.h"
#include "percussionmodel.h"
#include "panprocessor.h"
#include "spectrum.h"
#include <QMap>
#include <QVector>

/**
 * SounitGraph - Executes a graph of connected containers
 *
 * Manages processor instances for each container and executes them
 * in topological order to generate audio.
 */
class SounitGraph
{
public:
    SounitGraph(double sampleRate = 44100.0);

    // Build the graph from canvas containers and connections
    void buildFromCanvas(Canvas *canvas);

    // Generate a single audio sample
    // pitch = fundamental frequency in Hz
    // noteProgress = 0.0 to 1.0, represents position within note duration
    // isLegato = true if this note continues from previous (skip attack triggers)
    // tailMode = true to mute signal sources and let processors drain (reverb/decay ring-out)
    // scoreCurveValues / scoreCurveNames are required (no defaults) so every caller
    // is forced to think about expressive curve routing. Pass {}, {} explicitly
    // when no curves apply (e.g. IR pre-roll); in that no-score-context case,
    // followDynamics expressive-curve branches fall back to unity (act as no-ops).
    // With a non-empty list, a note missing the named curve falls back to zero.
    double generateSample(double pitch, double noteProgress, bool isLegato, bool tailMode,
                          double currentDynamics,
                          const QVector<double> &scoreCurveValues,
                          const QStringList &scoreCurveNames);

    // Post-render IR: when IR Convolution is the last container, it is applied here
    // instead of inside the graph so reverb is independent of note dynamics style.
    // irInput  = graph output * amplitude (pre-outer-currentDynamics)
    // dryOutput = graph output * amplitude * currentDynamics (fully processed dry signal)
    // Returns the wet/dry mix.  Call only when hasPostRenderIR() is true.
    bool hasPostRenderIR() const { return m_postRenderIRConv != nullptr; }
    double processPostIR(double irInput, double dryOutput, bool tailMode);

    // Reset all processors (call when starting new note)
    // isLegato = true to preserve state for legato transition (don't reset K-S etc.)
    void reset(bool isLegato = false);

    // Check if graph is valid and can produce audio
    bool isValid() const { return hasValidSignalOutput; }

    // Returns true if graph contains containers that produce a tail (reverb, string decay, etc.)
    bool hasTail() const;

    // Returns true if graph contains a TailProcessor (Note Tail container)
    bool hasTailProcessor() const { return m_hasTailProcessor; }

    // Returns true if graph contains a Saxophone container
    bool hasSaxophoneModel() const;

    // Returns true if graph contains a Percussion container.
    // Used by the audio engine to treat notes as triggers (cap note-body length).
    bool hasPercussion() const;


    // Returns true if graph contains a Karplus-Strong with string damping enabled
    bool hasStringDamping() const;

    // Returns true if graph contains a Pan container
    bool hasPan() const { return m_panContainer != nullptr; }

    // Returns the current pan value (-1.0 to +1.0) from the Pan container, or 0.0 if none
    double getPanValue() const;

    // Deep-copy the graph (each processor is cloned; Container pointers are shared read-only)
    SounitGraph* clone() const;

private:
    struct ProcessorData {
        Container *container = nullptr;

        // Processor instances (only one will be used per container)
        // Using raw pointers for QMap compatibility
        HarmonicGenerator *harmonicGen = nullptr;
        RolloffProcessor *rolloffProc = nullptr;
        SpectrumBlender *spectrumBlender = nullptr;
        SpectrumToSignal *spectrumToSig = nullptr;
        SignalMixer *signalMixer = nullptr;
        FormantBody *formantBody = nullptr;
        BreathTurbulence *breathTurb = nullptr;
        NoiseColorFilter *noiseFilter = nullptr;
        PhysicsSystem *physicsSys = nullptr;
        EnvelopeEngine *envelopeEng = nullptr;
        DriftEngine *driftEng = nullptr;
        GateProcessor *gateProc = nullptr;
        EasingApplicator *easingApp = nullptr;
        KarplusStrongAttack *karplusStrongAttack = nullptr;
        AttackGenerator *attackGen = nullptr;
        LFO *lfo = nullptr;
        FrequencyMapper *freqMapper = nullptr;
        WavetableSynth *wavetableSynth = nullptr;
        BandpassEQ *bandpassEQ = nullptr;
        CombFilter *combFilter = nullptr;
        LowHighPassFilter *lowHighPassFilter = nullptr;
        IRConvolution *irConvolution = nullptr;
        RecorderModel   *recorderModel   = nullptr;
        BowedModel      *bowedModel      = nullptr;
        SaxophoneModel  *reedModel       = nullptr;
        SaxophoneModel2 *saxophoneModel  = nullptr;
        FluteModel      *fluteModel      = nullptr;
        PianoModel      *pianoModel      = nullptr;
        BassModel       *bassModel       = nullptr;
        TibetanBowlModel *tibetanBowlModel = nullptr;
        TailProcessor    *tailProc         = nullptr;
        PercussionModel  *percussionModel  = nullptr;
        PanProcessor     *panProc          = nullptr;

        // Data storage for this container's outputs
        Spectrum spectrumOut;
        double signalOut = 0.0;
        double controlOut = 0.0;

        // When true, downstream consumers should ignore this processor's output
        // (skip the connection entirely so downstream parameters keep their design-time
        // value). Set by the Envelope Engine when followDynamics=on but the named
        // score curve isn't present on the current note. Re-evaluated every sample.
        bool inactive = false;

        // Gate Processor specific outputs
        double gateEnvelopeOut = 0.0;
        double gateStateOut = 0.0;
        double gateAttackTrigger = 0.0;
        double gateReleaseTrigger = 0.0;

        // PADsynth wavetable (owned by this ProcessorData, S2S points into it)
        std::vector<float> padWavetable;
        double padFundamentalHz = 0.0;

        // True when irIn port has an incoming connection (set at build time)
        bool irInConnected = false;

        // State tracking for Physics System impulse trigger
        double prevImpulse = 0.0;

        // Destructor to clean up processors
        ~ProcessorData() {
            delete harmonicGen;
            delete rolloffProc;
            delete spectrumBlender;
            delete spectrumToSig;
            delete signalMixer;
            delete formantBody;
            delete breathTurb;
            delete noiseFilter;
            delete physicsSys;
            delete envelopeEng;
            delete driftEng;
            delete gateProc;
            delete easingApp;
            delete karplusStrongAttack;
            delete attackGen;
            delete lfo;
            delete freqMapper;
            delete wavetableSynth;
            delete bandpassEQ;
            delete combFilter;
            delete lowHighPassFilter;
            delete irConvolution;
            delete recorderModel;
            delete bowedModel;
            delete reedModel;
            delete saxophoneModel;
            delete fluteModel;
            delete pianoModel;
            delete bassModel;
            delete tibetanBowlModel;
            delete tailProc;
            delete percussionModel;
            delete panProc;
        }
    };

    double sampleRate;
    QMap<Container*, ProcessorData> processors;
    QVector<Container*> executionOrder;  // Topological order
    Container *signalOutputContainer = nullptr;  // Final output (pre-IR when post-render IR is active)
    bool hasValidSignalOutput = false;
    bool m_currentIsLegato = false;  // Current note's legato state (for executeContainer)
    bool m_tailMode = false;         // True during tail: signal sources output 0, processors drain
    bool m_hasTailProcessor = false; // True if a TailProcessor exists; sources keep running in tail mode
    bool m_needsIRPreroll = false;   // True after reset() when any IR Conv has irInConnected
    double m_currentDynamics = 1.0;      // Note's dynamics value at current sample (for followDynamics)
    double m_lastNormalDynamics = 1.0;   // Dynamics at the last non-tail sample (for prepareForTailMode)
    QVector<double> m_scoreCurveValues;  // All expressive curve values for current sample (index 0=dynamics, 1+=named)
    QStringList m_scoreCurveNames;       // Parallel name list (index 0 = "Dynamics", 1+ = curve names)

    // Post-render IR Convolution: when IR is the final container it is lifted out of the
    // graph and applied by processPostIR() with the pre-outer-dynamics signal as input.
    // This prevents the reverb from being doubly suppressed by dynamics curves (once by
    // the in-graph Envelope Engine, once by the audio engine's outer currentDynamics).
    Container    *m_postRenderIRContainer = nullptr;  // The IR container that was lifted out
    IRConvolution *m_postRenderIRConv     = nullptr;  // Alias into processors[m_postRenderIRContainer]

    // Pan container tracking (for stereo rendering)
    Container *m_panContainer = nullptr;

    // Cached connections - snapshot taken at build time so graph is independent of live canvas edits
    QVector<Canvas::Connection> cachedConnections;

    void computeExecutionOrder(Canvas *canvas);
    void createProcessors();
    void setupPostRenderIR();   // Called after createProcessors(); lifts final IR out of graph
    void executeContainer(ProcessorData &proc, double pitch, double noteProgress);
    double getInputValue(Container *container, const QString &portName, double defaultValue);
    double getOutputValue(const ProcessorData &proc, const QString &portName) const;
    void runIRPreroll(double pitch);  // Pre-renders irIn source to capture IR before note starts

    // True when fromContainer has a processor and is not marked inactive.
    // Used in executeContainer to gate connection reads — inactive sources contribute nothing.
    bool isSourceActive(Container *fromContainer) const {
        auto it = processors.constFind(fromContainer);
        return it != processors.constEnd() && !it->inactive;
    }
};

#endif // SOUNITGRAPH_H
