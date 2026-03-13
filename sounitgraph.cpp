#include "sounitgraph.h"
#include <QDebug>
#include <QSet>
#include <cmath>

// Helper function to apply connection functions
static double applyConnectionFunction(double currentValue, double sourceValue,
                                     const QString& function, double weight)
{
    if (function == "passthrough") {
        // Source replaces input entirely (ignores weight)
        return sourceValue;
    } else if (function == "add") {
        // Input + source × weight
        return currentValue + sourceValue * weight;
    } else if (function == "multiply") {
        // Input × source × weight
        return currentValue * sourceValue * weight;
    } else if (function == "subtract") {
        // Input - source × weight
        return currentValue - sourceValue * weight;
    } else if (function == "replace") {
        // Weighted blend: input × (1-weight) + source × weight
        return currentValue * (1.0 - weight) + sourceValue * weight;
    } else if (function == "modulate") {
        // Bipolar modulation around input: input + (source - 0.5) × weight × range
        // Using range = 2.0 for full bipolar range
        return currentValue + (sourceValue - 0.5) * weight * 2.0;
    }

    // Unknown function, default to passthrough
    return sourceValue;
}

SounitGraph::SounitGraph(double sampleRate)
    : sampleRate(sampleRate)
    , hasValidSignalOutput(false)
{
}

SounitGraph* SounitGraph::clone() const
{
    SounitGraph *copy = new SounitGraph(sampleRate);

    // Copy graph structure (shallow — Container pointers are read-only during rendering)
    copy->executionOrder = executionOrder;
    copy->signalOutputContainer = signalOutputContainer;
    copy->hasValidSignalOutput = hasValidSignalOutput;
    copy->m_currentIsLegato = m_currentIsLegato;
    copy->m_hasTailProcessor = m_hasTailProcessor;
    copy->m_needsIRPreroll = m_needsIRPreroll;
    copy->cachedConnections = cachedConnections;
    copy->m_postRenderIRContainer = m_postRenderIRContainer;

    // Deep-copy each ProcessorData entry
    for (auto it = processors.constBegin(); it != processors.constEnd(); ++it) {
        const ProcessorData &src = it.value();
        ProcessorData &dst = copy->processors[it.key()];

        dst.container = src.container;  // shallow — read-only during rendering

        // Clone each processor using compiler-generated copy constructor
        if (src.harmonicGen)          dst.harmonicGen = new HarmonicGenerator(*src.harmonicGen);
        if (src.rolloffProc)          dst.rolloffProc = new RolloffProcessor(*src.rolloffProc);
        if (src.spectrumBlender)      dst.spectrumBlender = new SpectrumBlender(*src.spectrumBlender);
        if (src.spectrumToSig)        dst.spectrumToSig = new SpectrumToSignal(*src.spectrumToSig);
        if (src.signalMixer)          dst.signalMixer = new SignalMixer(*src.signalMixer);
        if (src.formantBody)          dst.formantBody = new FormantBody(*src.formantBody);
        if (src.breathTurb)           dst.breathTurb = new BreathTurbulence(*src.breathTurb);
        if (src.noiseFilter)          dst.noiseFilter = new NoiseColorFilter(*src.noiseFilter);
        if (src.physicsSys)           dst.physicsSys = new PhysicsSystem(*src.physicsSys);
        if (src.envelopeEng)          dst.envelopeEng = new EnvelopeEngine(*src.envelopeEng);
        if (src.driftEng)             dst.driftEng = new DriftEngine(*src.driftEng);
        if (src.gateProc)             dst.gateProc = new GateProcessor(*src.gateProc);
        if (src.easingApp)            dst.easingApp = new EasingApplicator(*src.easingApp);
        if (src.karplusStrongAttack)  dst.karplusStrongAttack = new KarplusStrongAttack(*src.karplusStrongAttack);
        if (src.attackGen)            dst.attackGen = new AttackGenerator(*src.attackGen);
        if (src.lfo)                  dst.lfo = new LFO(*src.lfo);
        if (src.freqMapper)           dst.freqMapper = new FrequencyMapper(*src.freqMapper);
        if (src.wavetableSynth)       dst.wavetableSynth = new WavetableSynth(*src.wavetableSynth);
        if (src.bandpassEQ)           dst.bandpassEQ = new BandpassEQ(*src.bandpassEQ);
        if (src.combFilter)           dst.combFilter = new CombFilter(*src.combFilter);
        if (src.lowHighPassFilter)    dst.lowHighPassFilter = new LowHighPassFilter(*src.lowHighPassFilter);
        if (src.irConvolution)        dst.irConvolution = new IRConvolution(*src.irConvolution);
        if (src.recorderModel)        dst.recorderModel   = new RecorderModel(*src.recorderModel);
        if (src.bowedModel)           dst.bowedModel      = new BowedModel(*src.bowedModel);
        if (src.reedModel)       dst.reedModel  = new SaxophoneModel(*src.reedModel);
        if (src.tailProc)             dst.tailProc        = new TailProcessor(*src.tailProc);

        // Copy output values
        dst.spectrumOut = src.spectrumOut;
        dst.signalOut = src.signalOut;
        dst.controlOut = src.controlOut;
        dst.gateEnvelopeOut = src.gateEnvelopeOut;
        dst.gateStateOut = src.gateStateOut;
        dst.gateAttackTrigger = src.gateAttackTrigger;
        dst.gateReleaseTrigger = src.gateReleaseTrigger;
        dst.padWavetable = src.padWavetable;
        dst.padFundamentalHz = src.padFundamentalHz;
        dst.prevImpulse = src.prevImpulse;
        dst.irInConnected = src.irInConnected;
    }

    // Resolve post-render IR alias into the cloned processors map
    if (m_postRenderIRContainer && copy->processors.contains(m_postRenderIRContainer)) {
        copy->m_postRenderIRConv = copy->processors[m_postRenderIRContainer].irConvolution;
    }

    return copy;
}

void SounitGraph::buildFromCanvas(Canvas *canvas)
{
    qDebug() << "SounitGraph::buildFromCanvas - START";

    processors.clear();
    executionOrder.clear();
    cachedConnections.clear();
    signalOutputContainer = nullptr;
    hasValidSignalOutput = false;
    m_postRenderIRContainer = nullptr;
    m_postRenderIRConv = nullptr;

    if (!canvas) {
        qDebug() << "SounitGraph::buildFromCanvas - canvas is null!";
        return;
    }

    // Cache connections at build time - this makes the graph independent of live canvas edits
    cachedConnections = canvas->getConnections();
    qDebug() << "SounitGraph: Cached" << cachedConnections.size() << "connections";

    qDebug() << "SounitGraph: Finding containers...";
    // Create processor data for each container
    QList<Container*> containers = canvas->findChildren<Container*>();
    qDebug() << "SounitGraph: Found" << containers.size() << "containers";

    for (Container *container : containers) {
        qDebug() << "  - Container:" << container->getName();
        ProcessorData data;
        data.container = container;
        processors[container] = data;
    }

    qDebug() << "SounitGraph: Computing execution order...";
    // Compute execution order
    computeExecutionOrder(canvas);

    qDebug() << "SounitGraph: Creating processors...";
    // Create processor instances
    createProcessors();

    // Propagate PAD wavetable through spectrum chain (one-time, at build)
    // Walk execution order so upstream containers are processed first.
    for (Container *container : executionOrder) {
        if (!processors.contains(container)) continue;
        ProcessorData &proc = processors[container];
        const QString &name = container->getName();
        if (name == "Rolloff Processor" || name == "Spectrum Blender") {
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container &&
                    (conn.toPort == "spectrumIn" || conn.toPort == "spectrumA")) {
                    if (processors.contains(conn.fromContainer)) {
                        const ProcessorData &src = processors[conn.fromContainer];
                        if (!src.padWavetable.empty() && proc.padWavetable.empty()) {
                            proc.padWavetable = src.padWavetable;
                            proc.padFundamentalHz = src.padFundamentalHz;
                        }
                    }
                }
            }
        }
    }

    // Lift IR Convolution out of the graph if it is the final output container.
    // This makes reverb independent of note dynamics style (pen vs mouse).
    setupPostRenderIR();

    // Resolve irIn connections for IR Convolution containers
    for (auto it = processors.begin(); it != processors.end(); ++it) {
        if (!it.value().irConvolution) continue;
        Container *container = it.key();
        for (const Canvas::Connection &conn : cachedConnections) {
            if (conn.toContainer == container && conn.toPort == "irIn") {
                it.value().irInConnected = true;
                break;
            }
        }
    }

    qDebug() << "SounitGraph: Built graph with" << processors.size() << "containers";
    qDebug() << "SounitGraph: Execution order:" << executionOrder.size() << "containers";
    qDebug() << "SounitGraph: Valid signal output:" << hasValidSignalOutput;
    qDebug() << "SounitGraph: Post-render IR:" << (m_postRenderIRConv != nullptr ? "yes" : "no");
}

// ---------------------------------------------------------------------------
// setupPostRenderIR
// ---------------------------------------------------------------------------

void SounitGraph::setupPostRenderIR()
{
    // Only activate when the final signal output is an IR Convolution container.
    // When this is the case, lift it out of the in-graph execution loop and apply
    // it separately via processPostIR(), which receives the pre-outer-dynamics signal.
    if (!signalOutputContainer) return;
    if (signalOutputContainer->getName() != "IR Convolution") return;
    if (!processors.contains(signalOutputContainer)) return;

    ProcessorData &irProc = processors[signalOutputContainer];
    if (!irProc.irConvolution) return;

    // Find the container connected to the IR's signalIn port — that becomes the
    // new (dry) signal output of the graph.
    Container *preIRContainer = nullptr;
    for (const Canvas::Connection &conn : cachedConnections) {
        if (conn.toContainer == signalOutputContainer && conn.toPort == "signalIn") {
            if (processors.contains(conn.fromContainer)) {
                preIRContainer = conn.fromContainer;
                break;
            }
        }
    }

    if (!preIRContainer) {
        // IR has no signal input — misconfigured; leave in-graph.
        qDebug() << "SounitGraph: IR Convolution has no signalIn connection; keeping in-graph";
        return;
    }

    m_postRenderIRContainer = signalOutputContainer;
    m_postRenderIRConv      = irProc.irConvolution;

    // The new dry signal output is the container feeding into the IR.
    signalOutputContainer = preIRContainer;

    qDebug() << "SounitGraph: IR Convolution lifted to post-render; dry output from:"
             << preIRContainer->getName() << "(" << preIRContainer->getInstanceName() << ")";
}

// ---------------------------------------------------------------------------
// processPostIR
// ---------------------------------------------------------------------------

double SounitGraph::processPostIR(double irInput, double dryOutput, bool tailMode)
{
    if (!m_postRenderIRConv || !m_postRenderIRConv->hasIR()) {
        return dryOutput;
    }

    if (tailMode) {
        // First call in tail mode: restore static wetDry and set up optional injection.
        // prepareForTailMode() is idempotent so calling it every sample is safe.
        m_postRenderIRConv->prepareForTailMode(m_lastNormalDynamics);
    }

    // In tail mode the IR processes silence; the tail injection mechanism inside
    // processSampleWetOnly() replaces that silence with the peak-amplitude block
    // if the note ended with near-zero dynamics (pen note fade).
    double wet = m_postRenderIRConv->processSampleWetOnly(tailMode ? 0.0 : irInput);
    double wd  = m_postRenderIRConv->getWetDry();
    return dryOutput * (1.0 - wd) + wet * wd;
}

// ---------------------------------------------------------------------------
// computeExecutionOrder
// ---------------------------------------------------------------------------

void SounitGraph::computeExecutionOrder(Canvas *canvas)
{
    // Proper topological sort using Kahn's algorithm
    // This follows actual connections instead of fixed type-based ordering

    if (!canvas) {
        qDebug() << "SounitGraph::computeExecutionOrder - canvas is null";
        return;
    }

    // Build dependency graph
    // Key: container, Value: set of containers that depend on it (outgoing edges)
    QMap<Container*, QSet<Container*>> dependents;
    // Key: container, Value: number of incoming edges (dependencies)
    QMap<Container*, int> incomingEdgeCount;

    // Initialize all containers with zero incoming edges
    for (Container *container : processors.keys()) {
        incomingEdgeCount[container] = 0;
        dependents[container] = QSet<Container*>();
    }

    // Build the graph from connections
    const QVector<Canvas::Connection>& connections = canvas->getConnections();
    for (const Canvas::Connection &conn : connections) {
        // Connection goes from output to input, so fromContainer must execute before toContainer
        if (processors.contains(conn.fromContainer) && processors.contains(conn.toContainer)) {
            // Add edge: fromContainer → toContainer
            dependents[conn.fromContainer].insert(conn.toContainer);
            incomingEdgeCount[conn.toContainer]++;
        }
    }

    // Kahn's algorithm: Start with containers that have no dependencies
    QVector<Container*> queue;
    for (Container *container : processors.keys()) {
        if (incomingEdgeCount[container] == 0) {
            queue.append(container);
        }
    }

    // Process queue
    executionOrder.clear();
    while (!queue.isEmpty()) {
        // Remove a container with no incoming edges
        Container *current = queue.takeFirst();
        executionOrder.append(current);

        // Remove edges from current to its dependents
        for (Container *dependent : dependents[current]) {
            incomingEdgeCount[dependent]--;
            if (incomingEdgeCount[dependent] == 0) {
                queue.append(dependent);
            }
        }
    }

    // Check for cycles
    if (executionOrder.size() != processors.size()) {
        qDebug() << "SounitGraph::computeExecutionOrder - WARNING: Cycle detected!";
        qDebug() << "  Processed" << executionOrder.size() << "containers, but" << processors.size() << "total";
        qDebug() << "  Containers in cycle:";
        for (Container *container : processors.keys()) {
            if (!executionOrder.contains(container)) {
                qDebug() << "    -" << container->getName() << "(" << container->getInstanceName() << ")";
            }
        }
        // Continue anyway with partial order, but mark as invalid
        hasValidSignalOutput = false;
        return;
    }

    // Find the signal output container
    // Look for containers with "signalOut" port that have no outgoing signal connections
    // or simply the last container in execution order that has a signalOut port
    signalOutputContainer = nullptr;

    // Check containers in reverse execution order (last executed first)
    for (int i = executionOrder.size() - 1; i >= 0; i--) {
        Container *container = executionOrder[i];
        QStringList outputs = container->getOutputPorts();

        // Check if this container has a signalOut port
        if (outputs.contains("signalOut")) {
            // Check if this port has any outgoing connections
            bool hasOutgoingSignal = false;
            for (const Canvas::Connection &conn : connections) {
                if (conn.fromContainer == container && conn.fromPort == "signalOut") {
                    hasOutgoingSignal = true;
                    break;
                }
            }

            // If no outgoing signal connection, this is likely the final output
            if (!hasOutgoingSignal) {
                signalOutputContainer = container;
                hasValidSignalOutput = true;
                qDebug() << "SounitGraph: Signal output container:" << container->getName()
                         << "(" << container->getInstanceName() << ")";
                break;
            }
        }
    }

    // Debug output
    qDebug() << "SounitGraph: Topological sort complete";
    qDebug() << "  Execution order:";
    for (int i = 0; i < executionOrder.size(); i++) {
        Container *c = executionOrder[i];
        qDebug() << "    " << (i+1) << "." << c->getName() << "(" << c->getInstanceName() << ")";
    }

    if (!hasValidSignalOutput) {
        qDebug() << "SounitGraph: WARNING - No valid signal output found!";
    }
}

void SounitGraph::createProcessors()
{
    for (auto it = processors.begin(); it != processors.end(); ++it) {
        Container *container = it.key();
        ProcessorData &data = it.value();

        if (container->getName() == "Harmonic Generator") {
            data.harmonicGen = new HarmonicGenerator(sampleRate);

            int dnaSelect = static_cast<int>(container->getParameter("dnaSelect", 0.0));

            // Check if using custom DNA pattern
            if (dnaSelect == -1) {
                // Load custom DNA pattern from container
                int customDnaCount = static_cast<int>(container->getParameter("customDnaCount", 0.0));

                if (customDnaCount > 0) {
                    std::vector<double> customAmplitudes;
                    customAmplitudes.reserve(customDnaCount);

                    for (int i = 0; i < customDnaCount; i++) {
                        QString paramName = QString("customDna_%1").arg(i);
                        double amp = container->getParameter(paramName, 0.0);
                        customAmplitudes.push_back(amp);
                    }

                    qDebug() << "Loading custom DNA with" << customDnaCount << "harmonics";
                    data.harmonicGen->setCustomAmplitudes(customAmplitudes);

                    // If this custom DNA was built from a digit formula, also load the
                    // digit string so the digitWindowOffset port can modulate it live
                    if (!container->getDigitString().isEmpty()) {
                        int baseOffset = static_cast<int>(container->getParameter("digitWindowOffset", 0.0));
                        data.harmonicGen->setDigitString(container->getDigitString(), baseOffset);
                        qDebug() << "  Digit formula string loaded, length ="
                                 << container->getDigitString().length()
                                 << ", baseOffset =" << baseOffset;
                    }
                } else {
                    // No custom pattern stored, fall back to rolloff-based generation
                    qDebug() << "Custom DNA selected but no pattern stored, using rolloff";
                    data.harmonicGen->setNumHarmonics(
                        static_cast<int>(container->getParameter("numHarmonics", 64.0)));
                    data.harmonicGen->setRolloffPower(container->getParameter("rolloff", 1.82));
                    data.harmonicGen->setDnaPreset(-1);
                }
            } else {
                // Using preset DNA
                int numHarms = static_cast<int>(container->getParameter("numHarmonics", 64.0));
                double rolloff = container->getParameter("rolloff", 1.82);
                qDebug() << "  [SounitGraph] HarmonicGenerator config: DNA=" << dnaSelect
                         << ", numHarmonics=" << numHarms
                         << ", rolloff=" << rolloff;
                data.harmonicGen->setNumHarmonics(numHarms);
                data.harmonicGen->setRolloffPower(rolloff);
                data.harmonicGen->setDnaPreset(dnaSelect);
            }

            // Note: purity and drift are now controlled via input ports, not stored parameters

            // PADsynth: read params and pre-generate wavetable if enabled
            bool padEnabled = container->getParameter("padEnabled", 0.0) > 0.5;
            if (padEnabled) {
                data.harmonicGen->setPadEnabled(true);
                data.harmonicGen->setPadBandwidth(container->getParameter("padBandwidth", 40.0));
                data.harmonicGen->setPadBandwidthScale(container->getParameter("padBandwidthScale", 1.0));
                data.harmonicGen->setPadProfileShape(static_cast<int>(container->getParameter("padProfileShape", 0.0)));
                data.harmonicGen->setPadFftSize(static_cast<int>(container->getParameter("padFftSize", 262144.0)));

                // Generate wavetable using a reference pitch (131 Hz ≈ C3)
                double refPitch = 131.0;
                data.harmonicGen->generatePadWavetable(refPitch, sampleRate);
                data.padWavetable = data.harmonicGen->getPadWavetable();
                data.padFundamentalHz = refPitch;
                qDebug() << "  [SounitGraph] PADsynth enabled for HG, wavetable size:" << data.padWavetable.size();
            }

        } else if (container->getName() == "Rolloff Processor") {
            data.rolloffProc = new RolloffProcessor();
            data.rolloffProc->setLowRolloff(container->getParameter("lowRolloff", 0.3));
            data.rolloffProc->setHighRolloff(container->getParameter("highRolloff", 1.0));
            data.rolloffProc->setCrossoverHarmonic(static_cast<int>(container->getParameter("crossover", 8.0)));
            data.rolloffProc->setTransitionWidth(static_cast<int>(container->getParameter("transition", 4.0)));

        } else if (container->getName() == "Spectrum Blender") {
            data.spectrumBlender = new SpectrumBlender();
            data.spectrumBlender->setPosition(container->getParameter("position", 0.5));

        } else if (container->getName() == "Spectrum to Signal") {
            data.spectrumToSig = new SpectrumToSignal(sampleRate);
            data.spectrumToSig->setNormalize(container->getParameter("normalize", 1.0));

        } else if (container->getName() == "Formant Body") {
            data.formantBody = new FormantBody(sampleRate);
            data.formantBody->setF1Freq(container->getParameter("f1Freq", 500.0));
            data.formantBody->setF2Freq(container->getParameter("f2Freq", 1500.0));
            data.formantBody->setF1Q(container->getParameter("f1Q", 8.0));
            data.formantBody->setF2Q(container->getParameter("f2Q", 10.0));
            data.formantBody->setDirectMix(container->getParameter("directMix", 0.3));
            data.formantBody->setF1F2Balance(container->getParameter("f1f2Balance", 0.6));

        } else if (container->getName() == "Breath Turbulence") {
            data.breathTurb = new BreathTurbulence();
            data.breathTurb->setBlend(container->getParameter("blend", 0.10));
            // For now, use default blend curve (sqrt)

        } else if (container->getName() == "Noise Color Filter") {
            data.noiseFilter = new NoiseColorFilter(sampleRate);
            data.noiseFilter->setColor(container->getParameter("color", 2000.0));
            data.noiseFilter->setFilterQ(container->getParameter("filterQ", 1.0));

            // Set noise type
            int noiseTypeValue = static_cast<int>(container->getParameter("noiseType", 0.0));
            data.noiseFilter->setNoiseType(static_cast<NoiseColorFilter::NoiseType>(noiseTypeValue));

            // Set filter type (0=Lowpass, 1=Highpass, 2=Bandpass)
            int filterTypeValue = static_cast<int>(container->getParameter("filterType", 1.0));
            data.noiseFilter->setFilterType(static_cast<NoiseColorFilter::FilterType>(filterTypeValue));

        } else if (container->getName() == "Physics System") {
            data.physicsSys = new PhysicsSystem();
            data.physicsSys->setMass(container->getParameter("mass", 0.5));
            data.physicsSys->setSpringK(container->getParameter("springK", 0.001));
            data.physicsSys->setDamping(container->getParameter("damping", 0.995));
            data.physicsSys->setImpulseAmount(container->getParameter("impulseAmount", 100.0));

        } else if (container->getName() == "Envelope Engine") {
            data.envelopeEng = new EnvelopeEngine();
            int envSelect = static_cast<int>(container->getParameter("envelopeSelect", 0.0));

            // If custom envelope is selected (index 5), set envelope type to Custom
            // and load the custom envelope data
            if (envSelect == 5 && container->hasCustomEnvelopeData()) {
                data.envelopeEng->setEnvelopeType(EnvelopeEngine::EnvelopeType::Custom);
                EnvelopeData customData = container->getCustomEnvelopeData();
                data.envelopeEng->setCustomEnvelope(customData.points);
            } else {
                // Standard envelope types (0-4)
                data.envelopeEng->setEnvelopeSelect(envSelect);
            }

            data.envelopeEng->setTimeScale(container->getParameter("timeScale", 1.0));
            data.envelopeEng->setValueScale(container->getParameter("valueScale", 1.0));
            data.envelopeEng->setValueOffset(container->getParameter("valueOffset", 0.0));

        } else if (container->getName() == "Drift Engine") {
            data.driftEng = new DriftEngine(sampleRate);
            data.driftEng->setAmount(container->getParameter("amount", 0.005));
            data.driftEng->setRate(container->getParameter("rate", 0.5));

            // Set drift pattern
            int patternValue = static_cast<int>(container->getParameter("driftPattern", 2.0));
            data.driftEng->setDriftPattern(static_cast<DriftEngine::DriftPattern>(patternValue));

        } else if (container->getName() == "Gate Processor") {
            data.gateProc = new GateProcessor(sampleRate);
            data.gateProc->setVelocity(container->getParameter("velocity", 1.0));
            data.gateProc->setAttackTime(container->getParameter("attackTime", 0.01));
            data.gateProc->setReleaseTime(container->getParameter("releaseTime", 0.1));
            data.gateProc->setAttackCurve(static_cast<int>(container->getParameter("attackCurve", 0.0)));
            data.gateProc->setReleaseCurve(static_cast<int>(container->getParameter("releaseCurve", 0.0)));
            data.gateProc->setVelocitySens(container->getParameter("velocitySens", 0.5));

        } else if (container->getName() == "Easing Applicator") {
            data.easingApp = new EasingApplicator();
            data.easingApp->setEasingSelect(static_cast<int>(container->getParameter("easingSelect", 0.0)));
            // For now, use default easing mode (InOut)

        } else if (container->getName() == "Karplus Strong") {
            data.karplusStrongAttack = new KarplusStrongAttack(sampleRate);
            int modeVal = static_cast<int>(container->getParameter("mode", 0.0));
            data.karplusStrongAttack->setMode(static_cast<KSMode>(modeVal));
            data.karplusStrongAttack->setAttackPortion(
                container->getParameter("attackPortion", 0.15));
            data.karplusStrongAttack->setDamping(
                container->getParameter("damping", 0.5));
            data.karplusStrongAttack->setPluckPosition(
                container->getParameter("pluckPosition", 0.5));
            data.karplusStrongAttack->setMix(
                container->getParameter("mix", 0.8));
            data.karplusStrongAttack->setBrightness(
                container->getParameter("brightness", 0.5));
            data.karplusStrongAttack->setExcitationType(
                static_cast<int>(container->getParameter("excitationType", 0.0)));
            data.karplusStrongAttack->setExcitationSoftness(
                container->getParameter("blendRatio", 0.5));
            data.karplusStrongAttack->setPluckHardness(
                container->getParameter("pluckHardness", 0.0));
            data.karplusStrongAttack->setPickDirection(
                container->getParameter("pickDirection", 0.5));
            data.karplusStrongAttack->setBodyResonance(
                container->getParameter("bodyResonance", 0.0));
            data.karplusStrongAttack->setBodyFreq(
                container->getParameter("bodyFreq", 200.0));

        } else if (container->getName() == "Attack") {
            data.attackGen = new AttackGenerator(sampleRate);
            data.attackGen->setAttackType(static_cast<int>(container->getParameter("attackType", 0.0)));
            data.attackGen->setDuration(container->getParameter("duration", 0.15));
            data.attackGen->setIntensity(container->getParameter("intensity", 0.8));
            data.attackGen->setMix(container->getParameter("mix", 0.8));
            // Flute params
            data.attackGen->setNoiseAmount(container->getParameter("noiseAmount", 0.5));
            data.attackGen->setJetRatio(container->getParameter("jetRatio", 0.32));
            // Clarinet/Sax params
            data.attackGen->setReedStiffness(container->getParameter("reedStiffness", 0.5));
            data.attackGen->setReedAperture(container->getParameter("reedAperture", 0.5));
            // Sax only
            data.attackGen->setBlowPosition(container->getParameter("blowPosition", 0.5));
            // Brass params
            data.attackGen->setLipTension(container->getParameter("lipTension", 0.5));
            data.attackGen->setBuzzQ(container->getParameter("buzzQ", 5.0));
            // Piano params
            data.attackGen->setHardness(container->getParameter("hardness", 0.5));
            data.attackGen->setBrightness(container->getParameter("brightness", 0.5));
            // Drum params
            data.attackGen->setTightness(container->getParameter("tightness", 0.5));
            data.attackGen->setTone(container->getParameter("tone", 0.5));

        } else if (container->getName() == "LFO") {
            data.lfo = new LFO(sampleRate);
            data.lfo->setFrequency(container->getParameter("frequency", 1.0));
            data.lfo->setAmplitude(container->getParameter("amplitude", 1.0));
            data.lfo->setWaveType(static_cast<int>(container->getParameter("waveType", 0.0)));

        } else if (container->getName() == "Frequency Mapper") {
            data.freqMapper = new FrequencyMapper();
            data.freqMapper->setLowFreq(container->getParameter("lowFreq", 25.0));
            data.freqMapper->setHighFreq(container->getParameter("highFreq", 8000.0));
            data.freqMapper->setOutputMin(container->getParameter("outputMin", 0.0));
            data.freqMapper->setOutputMax(container->getParameter("outputMax", 1.0));
            data.freqMapper->setCurveType(static_cast<int>(container->getParameter("curveType", 1.0)));
            data.freqMapper->setInvert(container->getParameter("invert", 0.0) > 0.5);

        } else if (container->getName() == "Signal Mixer") {
            data.signalMixer = new SignalMixer();
            data.signalMixer->setGainA(container->getParameter("gainA", 1.0));
            data.signalMixer->setGainB(container->getParameter("gainB", 1.0));

        } else if (container->getName() == "Wavetable Synth") {
            data.wavetableSynth = new WavetableSynth(sampleRate);

            int cycleLenParam = static_cast<int>(container->getParameter("cycleLength", 2048.0));
            data.wavetableSynth->setCycleLength(cycleLenParam);

            // Load wavetable data: either from container's WAV data or from preset
            if (container->hasWavetableData()) {
                const std::vector<float> &wavData = container->getWavetableData();
                data.wavetableSynth->loadFromWav(wavData.data(),
                                                  static_cast<int>(wavData.size()),
                                                  cycleLenParam);
            } else {
                int presetSelect = static_cast<int>(container->getParameter("wavetableSelect", 0.0));
                data.wavetableSynth->loadPreset(static_cast<WavetableSynth::Preset>(presetSelect));
            }

            data.wavetableSynth->setPosition(container->getParameter("position", 0.0));
            int outputModeVal = static_cast<int>(container->getParameter("outputMode", 0.0));
            data.wavetableSynth->setOutputMode(static_cast<WavetableSynth::OutputMode>(outputModeVal));

        } else if (container->getName() == "10-Band EQ") {
            data.bandpassEQ = new BandpassEQ(sampleRate);
            for (int i = 0; i < BandpassEQ::NUM_BANDS; i++) {
                data.bandpassEQ->setGain(i, container->getParameter(QString("band%1").arg(i + 1), 1.0));
                data.bandpassEQ->setQ(i, container->getParameter(QString("q%1").arg(i + 1), 2.0));
            }

        } else if (container->getName() == "Comb Filter") {
            data.combFilter = new CombFilter(sampleRate);
            data.combFilter->setMode(static_cast<int>(container->getParameter("mode", 0.0)));
            data.combFilter->setDelayTime(container->getParameter("delayTime", 5.0));
            data.combFilter->setFeedback(container->getParameter("feedback", 0.5));
            data.combFilter->setDamping(container->getParameter("damping", 0.0));

        } else if (container->getName() == "LP/HP Filter") {
            data.lowHighPassFilter = new LowHighPassFilter(sampleRate);
            data.lowHighPassFilter->setMode(static_cast<int>(container->getParameter("mode", 0.0)));
            data.lowHighPassFilter->setCutoff(container->getParameter("cutoff", 1000.0));
            data.lowHighPassFilter->setResonance(container->getParameter("resonance", 0.707));

        } else if (container->getName() == "IR Convolution") {
            data.irConvolution = new IRConvolution(sampleRate);
            // setStaticWetDry stores the graph-build value so prepareForTailMode()
            // can restore it if dynamics-driven modulation drove wetDry to zero.
            data.irConvolution->setStaticWetDry(container->getParameter("wetDry", 0.5));
            data.irConvolution->setPredelay(container->getParameter("predelay", 0.0));
            data.irConvolution->setHighDamp(container->getParameter("highDamp", 20000.0));
            data.irConvolution->setLowCut(container->getParameter("lowCut", 20.0));
            data.irConvolution->setDecay(container->getParameter("decay", 1.0));
            data.irConvolution->setFadeInPct(container->getParameter("fadeInPct", 10.0));
            data.irConvolution->setCaptureLengthSeconds(container->getParameter("captureLength", 1.0));
            if (container->hasIRData()) {
                data.irConvolution->setImpulseResponse(container->getIRData());
            }

        } else if (container->getName() == "Recorder") {
            data.recorderModel = new RecorderModel(sampleRate);
            data.recorderModel->setBreathPressure(container->getParameter("breathPressure", 0.81));
            data.recorderModel->setJetRatio(container->getParameter("jetRatio", 0.35));
            data.recorderModel->setNoiseGain(container->getParameter("noiseGain", 0.015));
            data.recorderModel->setVibratoFreq(container->getParameter("vibratoFreq", 0.0));
            data.recorderModel->setVibratoGain(container->getParameter("vibratoGain", 0.0));
            data.recorderModel->setEndReflection(container->getParameter("endReflection", 0.73));
            data.recorderModel->setJetReflection(container->getParameter("jetReflection", 0.59));

        } else if (container->getName() == "Bowed") {
            data.bowedModel = new BowedModel(sampleRate);
            data.bowedModel->setBowPressure(container->getParameter("bowPressure", 0.75));
            data.bowedModel->setBowVelocity(container->getParameter("bowVelocity", 0.5));
            data.bowedModel->setBowPosition(container->getParameter("bowPosition", 0.127));
            data.bowedModel->setNLType(static_cast<int>(container->getParameter("nlType", 0.0)));
            data.bowedModel->setNLAmount(container->getParameter("nlAmount", 0.0));
            data.bowedModel->setNLFreqMod(container->getParameter("nlFreqMod", 10.0));
            data.bowedModel->setNLAttack(container->getParameter("nlAttack", 0.1));

        } else if (container->getName() == "Reed") {
            data.reedModel = new SaxophoneModel(sampleRate);
            data.reedModel->setBreathPressure(container->getParameter("breathPressure", 0.7));
            data.reedModel->setReedStiffness(container->getParameter("reedStiffness", 0.5));
            data.reedModel->setReedAperture(container->getParameter("reedAperture", 0.5));
            data.reedModel->setBlowPosition(container->getParameter("blowPosition", 0.2));
            data.reedModel->setNoiseGain(container->getParameter("noiseGain", 0.2));
            data.reedModel->setVibratoFreq(container->getParameter("vibratoFreq", 5.735));
            data.reedModel->setVibratoGain(container->getParameter("vibratoGain", 0.0));
            data.reedModel->setNLType(static_cast<int>(container->getParameter("nlType", 0.0)));
            data.reedModel->setNLAmount(container->getParameter("nlAmount", 0.0));
            data.reedModel->setNLFreqMod(container->getParameter("nlFreqMod", 10.0));
            data.reedModel->setNLAttack(container->getParameter("nlAttack", 0.1));

        } else if (container->getName() == "Note Tail") {
            data.tailProc = new TailProcessor(sampleRate);
            data.tailProc->setTailLength(container->getParameter("tailLength", 200.0));
        }
    }

    // Cache whether a TailProcessor is present — when true, signal sources are NOT silenced
    // in tail mode so the TailProcessor can apply its fade to the live ongoing signal.
    m_hasTailProcessor = false;
    for (auto it = processors.constBegin(); it != processors.constEnd(); ++it) {
        if (it.value().tailProc) { m_hasTailProcessor = true; break; }
    }
}

void SounitGraph::reset(bool isLegato)
{
    for (auto &data : processors) {
        if (data.harmonicGen) {
            data.harmonicGen->reset();
        }
        if (data.spectrumToSig) {
            data.spectrumToSig->reset();
        }
        if (data.formantBody) {
            // For legato, don't reset formant filters - let them continue smoothly
            if (!isLegato) {
                data.formantBody->reset();
            }
        }
        if (data.noiseFilter) {
            data.noiseFilter->reset();
        }
        if (data.physicsSys) {
            data.physicsSys->reset();
        }
        if (data.driftEng) {
            data.driftEng->reset();
        }
        if (data.gateProc) {
            data.gateProc->reset();
            // Trigger note on when starting a new note
            // For legato notes, skip attack and go directly to sustain
            data.gateProc->noteOn(1.0, isLegato);
        }
        if (data.karplusStrongAttack) {
            // For legato notes, DON'T reset K-S - let the string keep vibrating
            // This preserves the sound from the previous note
            if (!isLegato) {
                data.karplusStrongAttack->reset();
            }
        }
        if (data.attackGen) {
            // For legato notes, don't reset - let attack fade naturally
            if (!isLegato) {
                data.attackGen->reset();
            }
        }
        if (data.lfo) {
            data.lfo->reset();
        }
        if (data.freqMapper) {
            data.freqMapper->reset();
        }
        if (data.wavetableSynth) {
            data.wavetableSynth->reset();
        }
        if (data.bandpassEQ) {
            if (!isLegato) {
                data.bandpassEQ->reset();
            }
        }
        if (data.combFilter) {
            if (!isLegato) {
                data.combFilter->reset();
            }
        }
        if (data.lowHighPassFilter) {
            if (!isLegato) {
                data.lowHighPassFilter->reset();
            }
        }
        if (data.irConvolution) {
            if (!isLegato) {
                data.irConvolution->reset();
            }
        }
        if (data.recorderModel) {
            data.recorderModel->reset(isLegato);
        }
        if (data.bowedModel) {
            data.bowedModel->reset(isLegato);
        }
        if (data.reedModel) {
            data.reedModel->reset(isLegato);
        }
        if (data.tailProc) {
            data.tailProc->reset();
        }
        // RolloffProcessor and BreathTurbulence have no state to reset
    }

    // Schedule IR pre-roll on the first generateSample() call if any IR Conv has a live irIn source
    m_needsIRPreroll = false;
    if (!isLegato) {
        for (auto it = processors.constBegin(); it != processors.constEnd(); ++it) {
            if (it.value().irInConnected) {
                m_needsIRPreroll = true;
                break;
            }
        }
    }
}

bool SounitGraph::hasTail() const
{
    for (auto it = processors.constBegin(); it != processors.constEnd(); ++it) {
        const ProcessorData &data = it.value();
        if (data.irConvolution && data.irConvolution->hasIR()) return true;
        if (data.karplusStrongAttack) return true;
        if (data.combFilter) return true;
        if (data.recorderModel) return true;
        if (data.bowedModel) return true;
        if (data.reedModel) return true;
        if (data.tailProc) return true;
    }
    return false;
}

bool SounitGraph::hasStringDamping() const
{
    for (auto it = processors.constBegin(); it != processors.constEnd(); ++it) {
        if (it.value().karplusStrongAttack) {
            Container *container = it.key();
            if (container->getParameter("stringDamping", 0.0) >= 0.5) {
                return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// runIRPreroll
// ---------------------------------------------------------------------------

void SounitGraph::runIRPreroll(double pitch)
{
    // Clone the post-reset graph to use as a scratch pad for capture.
    // Container pointers are shared (read-only), so getParameter() etc. work correctly.
    SounitGraph *preroll = this->clone();
    preroll->m_needsIRPreroll = false;  // prevent recursion

    // Find the max captureLength across all irIn-connected IR Conv containers
    double maxCaptureSecs = 0.0;
    for (auto it = processors.constBegin(); it != processors.constEnd(); ++it) {
        if (it.value().irInConnected && it.value().container) {
            double secs = it.value().container->getParameter("captureLength", 1.0);
            maxCaptureSecs = std::max(maxCaptureSecs, secs);
        }
    }

    int maxCapBuf = static_cast<int>(20.0 * sampleRate);
    int captureSamples = std::min(std::max(1, static_cast<int>(maxCaptureSecs * sampleRate)), maxCapBuf);

    // Run the pre-roll; the auto-commit in pushIRSample() fires once the threshold is reached
    for (int i = 0; i < captureSamples; i++) {
        preroll->generateSample(pitch, 0.5, false, false, 1.0, {});
    }

    // Copy committed IRs from the pre-roll clone back into this graph
    for (auto it = processors.begin(); it != processors.end(); ++it) {
        if (!it.value().irInConnected || !it.value().irConvolution) continue;
        Container *container = it.key();
        if (!preroll->processors.contains(container)) continue;
        const ProcessorData &preData = preroll->processors[container];
        if (!preData.irConvolution || !preData.irConvolution->hasIR()) continue;
        it.value().irConvolution->setImpulseResponse(preData.irConvolution->getIR());
        it.value().irConvolution->freezeCapture();  // stop capture during main render
    }

    delete preroll;
}

double SounitGraph::generateSample(double pitch, double noteProgress, bool isLegato, bool tailMode, double currentDynamics, const QVector<double> &scoreCurveValues)
{
    if (!hasValidSignalOutput) {
        return 0.0;
    }

    // On the first sample of each non-legato note, pre-render the irIn source
    // to capture the IR before any convolution is applied.
    if (m_needsIRPreroll && !tailMode) {
        m_needsIRPreroll = false;
        runIRPreroll(pitch);
    }

    // Store state for use by executeContainer
    m_currentIsLegato = isLegato;
    m_tailMode = tailMode;
    m_currentDynamics = currentDynamics;
    m_scoreCurveValues = scoreCurveValues;
    if (!tailMode) {
        m_lastNormalDynamics = currentDynamics;  // Snapshot dynamics before tail mode begins
    }

    // Execute containers in order
    for (Container *container : executionOrder) {
        if (processors.contains(container)) {
            executeContainer(processors[container], pitch, noteProgress);
        }
    }

    // Return final signal output
    if (signalOutputContainer && processors.contains(signalOutputContainer)) {
        return processors[signalOutputContainer].signalOut;
    }

    return 0.0;
}

void SounitGraph::executeContainer(ProcessorData &proc, double pitch, double noteProgress)
{
    Container *container = proc.container;
    if (!container) return;

    if (m_tailMode) {
        const QString &name = container->getName();

        // Physical models always use their special tail-mode tick regardless of TailProcessor:
        // they need excitation switched off so their bore/string drains naturally.
        // Recorder: keep running in tail mode but with excitation off so bore rings down
        if (name == "Recorder" && proc.recorderModel) {
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);
            for (const Canvas::Connection &c : cachedConnections)
                if (c.toContainer == container && c.toPort == "pitchMultiplier" && processors.contains(c.fromContainer))
                    pitchMult = processors[c.fromContainer].controlOut;
            proc.signalOut = proc.recorderModel->tick(pitch * pitchMult, 1.0,
                                                      m_currentIsLegato, true);
            return;
        }
        // Bowed: keep running in tail mode so string rings down naturally
        if (name == "Bowed" && proc.bowedModel) {
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);
            for (const Canvas::Connection &c : cachedConnections)
                if (c.toContainer == container && c.toPort == "pitchMultiplier" && processors.contains(c.fromContainer))
                    pitchMult = processors[c.fromContainer].controlOut;
            proc.signalOut = proc.bowedModel->tick(pitch * pitchMult, 1.0,
                                                   m_currentIsLegato, true);
            return;
        }
        // Reed: let bore drain in tail mode
        if (name == "Reed" && proc.reedModel) {
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);
            for (const Canvas::Connection &c : cachedConnections)
                if (c.toContainer == container && c.toPort == "pitchMultiplier" && processors.contains(c.fromContainer))
                    pitchMult = processors[c.fromContainer].controlOut;
            proc.signalOut = proc.reedModel->tick(pitch * pitchMult, 1.0,
                                                       m_currentIsLegato, true);
            return;
        }
        // IR Convolution in tail mode:
        //   - Post-render IR: processing is deferred to processPostIR(); only skip here.
        //   - In-graph IR: drain FDL with prepareForTailMode() + processSample(0.0).
        //     prepareForTailMode() runs once (idempotent) and:
        //       1. Restores wetDry to the static parameter value (dynamics-driven
        //          modulation may have driven it to zero at the note's last sample).
        //       2. If the note ended softly (pen dynamics faded to near-zero), injects
        //          a decaying replay of the last peak-amplitude block so the FDL has
        //          energy to ring out naturally.
        if (name == "IR Convolution" && proc.irConvolution) {
            if (proc.container == m_postRenderIRContainer) {
                proc.signalOut = 0.0;  // handled by processPostIR()
                return;
            }
            proc.irConvolution->prepareForTailMode(m_lastNormalDynamics);
            proc.signalOut = proc.irConvolution->processSample(0.0);
            return;
        }

        // Simple signal generators: silence them so IR/reverb tails drain cleanly.
        // Exception: when a TailProcessor is present it owns the fade, so these sources
        // keep generating to give it a live signal to apply the cosine fade to.
        if (!m_hasTailProcessor) {
            if (name == "Harmonic Generator" || name == "Spectrum to Signal" ||
                name == "Wavetable Synth" || name == "Noise Color Filter" ||
                name == "Breath Turbulence" || name == "Attack") {
                proc.signalOut = 0.0;
                proc.spectrumOut = Spectrum();
                return;
            }
        }
    }

    if (container->getName() == "Harmonic Generator") {
        if (proc.harmonicGen) {
            // Initialize input values with defaults
            double purityValue = 0.0;  // Default purity (0.0 = pure DNA, 1.0 = flat spectrum)
            double driftValue = 0.0;   // Default drift

            // Check for connections to purity and drift ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "purity") {
                        // Get purity modulation from source container (control output 0.0-1.0)
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            purityValue = applyConnectionFunction(purityValue, sourceValue,
                                                                conn.function, conn.weight);
                            // Clamp to valid range
                            purityValue = qBound(0.0, purityValue, 1.0);
                        }
                    } else if (conn.toPort == "drift") {
                        // Get drift modulation from source container (control output)
                        if (processors.contains(conn.fromContainer)) {
                            // Scale to drift range (0.0 to 0.1)
                            double sourceValue = processors[conn.fromContainer].controlOut * 0.1;
                            driftValue = applyConnectionFunction(driftValue, sourceValue,
                                                               conn.function, conn.weight);
                            // Clamp to valid range
                            driftValue = qBound(0.0, driftValue, 0.1);
                        }
                    }
                }
            }

            // Apply final modulated values
            proc.harmonicGen->setPurity(purityValue);
            proc.harmonicGen->setDrift(driftValue);

            // Digit window offset: only active when the generator has a digit string loaded
            if (!proc.harmonicGen->getDigitString().isEmpty()) {
                double offsetValue = container->getParameter("digitWindowOffset", 0.0);
                for (const Canvas::Connection &conn : cachedConnections) {
                    if (conn.toContainer == container && conn.toPort == "digitWindowOffset") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            offsetValue = applyConnectionFunction(offsetValue, sourceValue,
                                                                  conn.function, conn.weight);
                        }
                    }
                }
                int intOffset = std::max(0, static_cast<int>(std::round(offsetValue)));
                proc.harmonicGen->setDigitWindowOffset(intOffset);
            }

            // Generate harmonic spectrum using HarmonicGenerator's pre-calculated amplitudes
            // (which include DNA preset characteristics and purity blending)
            int numHarmonics = proc.harmonicGen->getNumHarmonics();
            proc.spectrumOut.resize(numHarmonics);

            // Use the pre-calculated amplitudes from HarmonicGenerator
            // (already normalized and DNA-aware)
            for (int h = 0; h < numHarmonics; h++) {
                double amp = proc.harmonicGen->getHarmonicAmplitude(h);
                proc.spectrumOut.setAmplitude(h, amp);
            }
        }

    } else if (container->getName() == "Rolloff Processor") {
        if (proc.rolloffProc) {
            // Initialize input values with defaults
            Spectrum inputSpectrum;
            double lowRolloff = container->getParameter("lowRolloff", 0.3);
            double highRolloff = container->getParameter("highRolloff", 1.0);
            int crossoverHarmonic = static_cast<int>(container->getParameter("crossover", 8.0));
            int transitionWidth = static_cast<int>(container->getParameter("transition", 4.0));

            // Find connections to input ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "spectrumIn") {
                        // Get spectrum from source container (passthrough only for spectrum data)
                        if (processors.contains(conn.fromContainer)) {
                            inputSpectrum = processors[conn.fromContainer].spectrumOut;
                        }
                    } else if (conn.toPort == "lowRolloff") {
                        // Get lowRolloff modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to rolloff range (0.0-2.0)
                            double sourceValue = processors[conn.fromContainer].controlOut * 2.0;
                            lowRolloff = applyConnectionFunction(lowRolloff, sourceValue,
                                                                 conn.function, conn.weight);
                            lowRolloff = qBound(0.0, lowRolloff, 2.0);
                        }
                    } else if (conn.toPort == "highRolloff") {
                        // Get highRolloff modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to rolloff range (0.0-3.0)
                            double sourceValue = processors[conn.fromContainer].controlOut * 3.0;
                            highRolloff = applyConnectionFunction(highRolloff, sourceValue,
                                                                  conn.function, conn.weight);
                            highRolloff = qBound(0.0, highRolloff, 3.0);
                        }
                    } else if (conn.toPort == "crossover") {
                        // Get crossover modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to harmonic range (1-32)
                            double sourceValue = 1.0 + processors[conn.fromContainer].controlOut * 31.0;
                            double crossoverDouble = applyConnectionFunction(crossoverHarmonic, sourceValue,
                                                                             conn.function, conn.weight);
                            crossoverHarmonic = static_cast<int>(qBound(1.0, crossoverDouble, 32.0));
                        }
                    } else if (conn.toPort == "transition") {
                        // Get transition width modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to width range (0-16)
                            double sourceValue = processors[conn.fromContainer].controlOut * 16.0;
                            double transitionDouble = applyConnectionFunction(transitionWidth, sourceValue,
                                                                              conn.function, conn.weight);
                            transitionWidth = static_cast<int>(qBound(0.0, transitionDouble, 16.0));
                        }
                    }
                }
            }

            // Process spectrum with dual-band rolloff curve
            proc.rolloffProc->processSpectrumDualBand(inputSpectrum, proc.spectrumOut,
                                                       lowRolloff, highRolloff,
                                                       crossoverHarmonic, transitionWidth);
        }

    } else if (container->getName() == "Spectrum Blender") {
        if (proc.spectrumBlender) {
            Spectrum spectrumA;
            Spectrum spectrumB;
            double position = container->getParameter("position", 0.5);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "spectrumA") {
                        if (processors.contains(conn.fromContainer)) {
                            spectrumA = processors[conn.fromContainer].spectrumOut;
                        }
                    } else if (conn.toPort == "spectrumB") {
                        if (processors.contains(conn.fromContainer)) {
                            spectrumB = processors[conn.fromContainer].spectrumOut;
                        }
                    } else if (conn.toPort == "position") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            position = applyConnectionFunction(position, sourceValue,
                                                              conn.function, conn.weight);
                            position = qBound(0.0, position, 1.0);
                        }
                    }
                }
            }

            proc.spectrumBlender->process(spectrumA, spectrumB, position, proc.spectrumOut);
        }

    } else if (container->getName() == "Spectrum to Signal") {
        if (proc.spectrumToSig) {
            // Clear wavetable each frame; will be set if source has PAD data
            proc.spectrumToSig->clearWavetable();

            // Initialize input values with defaults
            Spectrum inputSpectrum;
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);

            // Find connections to spectrumIn and pitchMultiplier ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "spectrumIn") {
                        // Get spectrum from source container (passthrough only for spectrum data)
                        if (processors.contains(conn.fromContainer)) {
                            inputSpectrum = processors[conn.fromContainer].spectrumOut;

                            // Check if source has a PAD wavetable
                            const ProcessorData &srcProc = processors[conn.fromContainer];
                            if (!srcProc.padWavetable.empty()) {
                                proc.spectrumToSig->setWavetable(&srcProc.padWavetable, srcProc.padFundamentalHz);
                            }
                        }
                    } else if (conn.toPort == "pitchMultiplier") {
                        if (processors.contains(conn.fromContainer))
                            pitchMult = processors[conn.fromContainer].controlOut;
                    }
                }
            }

            // Generate audio sample from spectrum with modulated pitch
            proc.signalOut = proc.spectrumToSig->generateSample(inputSpectrum, pitch * pitchMult);
        }

    } else if (container->getName() == "Formant Body") {
        if (proc.formantBody) {
            // Initialize input values with defaults
            double inputSignal = 0.0;
            double f1Freq = container->getParameter("f1Freq", 500.0);
            double f2Freq = container->getParameter("f2Freq", 1500.0);
            double f1Q = container->getParameter("f1Q", 8.0);
            double f2Q = container->getParameter("f2Q", 10.0);
            double directMix = container->getParameter("directMix", 0.3);
            double f1f2Balance = container->getParameter("f1f2Balance", 0.6);

            // Find connections to all input ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "signalIn") {
                        // Get signal from source container (passthrough only for signal data)
                        if (processors.contains(conn.fromContainer)) {
                            inputSignal = processors[conn.fromContainer].signalOut;
                        }
                    } else if (conn.toPort == "f1Freq") {
                        // Get f1Freq modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to f1Freq range (200-1000 Hz)
                            double sourceValue = 200.0 + processors[conn.fromContainer].controlOut * 800.0;
                            f1Freq = applyConnectionFunction(f1Freq, sourceValue,
                                                           conn.function, conn.weight);
                            f1Freq = qBound(200.0, f1Freq, 1000.0);
                        }
                    } else if (conn.toPort == "f2Freq") {
                        // Get f2Freq modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to f2Freq range (500-3000 Hz)
                            double sourceValue = 500.0 + processors[conn.fromContainer].controlOut * 2500.0;
                            f2Freq = applyConnectionFunction(f2Freq, sourceValue,
                                                           conn.function, conn.weight);
                            f2Freq = qBound(500.0, f2Freq, 3000.0);
                        }
                    } else if (conn.toPort == "f1Q") {
                        // Get f1Q modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to Q range (1.0-20.0)
                            double sourceValue = 1.0 + processors[conn.fromContainer].controlOut * 19.0;
                            f1Q = applyConnectionFunction(f1Q, sourceValue,
                                                        conn.function, conn.weight);
                            f1Q = qBound(1.0, f1Q, 20.0);
                        }
                    } else if (conn.toPort == "f2Q") {
                        // Get f2Q modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to Q range (1.0-20.0)
                            double sourceValue = 1.0 + processors[conn.fromContainer].controlOut * 19.0;
                            f2Q = applyConnectionFunction(f2Q, sourceValue,
                                                        conn.function, conn.weight);
                            f2Q = qBound(1.0, f2Q, 20.0);
                        }
                    } else if (conn.toPort == "directMix") {
                        // Get directMix modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            directMix = applyConnectionFunction(directMix, sourceValue,
                                                              conn.function, conn.weight);
                            directMix = qBound(0.0, directMix, 1.0);
                        }
                    } else if (conn.toPort == "f1f2Balance") {
                        // Get f1f2Balance modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            f1f2Balance = applyConnectionFunction(f1f2Balance, sourceValue,
                                                                conn.function, conn.weight);
                            f1f2Balance = qBound(0.0, f1f2Balance, 1.0);
                        }
                    }
                }
            }

            // Update parameters (from static config or modulated values)
            proc.formantBody->setF1Freq(f1Freq);
            proc.formantBody->setF2Freq(f2Freq);
            proc.formantBody->setF1Q(f1Q);
            proc.formantBody->setF2Q(f2Q);
            proc.formantBody->setDirectMix(directMix);
            proc.formantBody->setF1F2Balance(f1f2Balance);

            // Process the signal through formant filters
            proc.signalOut = proc.formantBody->processSample(inputSignal);
        }

    } else if (container->getName() == "Breath Turbulence") {
        if (proc.breathTurb) {
            // Initialize input values with defaults
            double voiceIn = 0.0;
            double noiseIn = 0.0;
            double blend = container->getParameter("blend", 0.10);

            // Find connections to voiceIn, noiseIn, and blend ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "voiceIn") {
                        // Get signal from source container (passthrough only for signal data)
                        if (processors.contains(conn.fromContainer)) {
                            voiceIn = processors[conn.fromContainer].signalOut;
                        }
                    } else if (conn.toPort == "noiseIn") {
                        // Get noise signal from source container (passthrough only for signal data)
                        if (processors.contains(conn.fromContainer)) {
                            noiseIn = processors[conn.fromContainer].signalOut;
                        }
                    } else if (conn.toPort == "blend") {
                        // Get blend modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            blend = applyConnectionFunction(blend, sourceValue,
                                                          conn.function, conn.weight);
                            blend = qBound(0.0, blend, 1.0);
                        }
                    }
                }
            }

            // Update blend parameter (from static config or modulated value)
            proc.breathTurb->setBlend(blend);

            // Process the blend
            proc.signalOut = proc.breathTurb->processSample(voiceIn, noiseIn);
        }

    } else if (container->getName() == "Noise Color Filter") {
        if (proc.noiseFilter) {
            // Initialize input values with defaults
            double color = container->getParameter("color", 2000.0);
            double filterQ = container->getParameter("filterQ", 1.0);
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);
            double audioIn = 0.0;
            bool hasAudioIn = false;
            bool followPitch = container->getParameter("followPitch", 0.0) > 0.5;

            // Find connections to input ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "audioIn") {
                        // Get audio signal from source container (passthrough only for signal data)
                        if (processors.contains(conn.fromContainer)) {
                            audioIn = processors[conn.fromContainer].signalOut;
                            hasAudioIn = true;
                        }
                    } else if (conn.toPort == "color") {
                        // Get color modulation from source container (ignored when followPitch is on)
                        if (!followPitch && processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to color range (100-8000 Hz)
                            double sourceValue = processors[conn.fromContainer].controlOut * 7900.0 + 100.0;
                            color = applyConnectionFunction(color, sourceValue,
                                                          conn.function, conn.weight);
                            color = qBound(100.0, color, 8000.0);
                        }
                    } else if (conn.toPort == "filterQ") {
                        // Get filterQ modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to filterQ range (0.5-10.0)
                            double sourceValue = processors[conn.fromContainer].controlOut * 9.5 + 0.5;
                            filterQ = applyConnectionFunction(filterQ, sourceValue,
                                                            conn.function, conn.weight);
                            filterQ = qBound(0.5, filterQ, 10.0);
                        }
                    } else if (conn.toPort == "pitchMultiplier") {
                        if (processors.contains(conn.fromContainer))
                            pitchMult = processors[conn.fromContainer].controlOut;
                    }
                }
            }

            // followPitch: override color with note frequency * multiplier (connection-modulated)
            if (followPitch) {
                color = pitch * pitchMult;
                color = qBound(20.0, color, sampleRate * 0.45);
            }

            // Update parameters
            proc.noiseFilter->setColor(color);
            proc.noiseFilter->setFilterQ(filterQ);

            // Update filter type
            int filterTypeValue = static_cast<int>(container->getParameter("filterType", 1.0));
            proc.noiseFilter->setFilterType(static_cast<NoiseColorFilter::FilterType>(filterTypeValue));

            // Update noise type
            int noiseTypeValue = static_cast<int>(container->getParameter("noiseType", 0.0));
            proc.noiseFilter->setNoiseType(static_cast<NoiseColorFilter::NoiseType>(noiseTypeValue));

            // Process or generate noise
            if (hasAudioIn) {
                // Use external audio input
                proc.signalOut = proc.noiseFilter->processSample(audioIn);
            } else {
                // Generate internal noise
                proc.signalOut = proc.noiseFilter->generateSample();
            }
        }

    } else if (container->getName() == "Physics System") {
        if (proc.physicsSys) {
            // Initialize input values with defaults
            double targetValue = 0.0;
            double mass = container->getParameter("mass", 0.5);
            double springK = container->getParameter("springK", 0.001);
            double damping = container->getParameter("damping", 0.995);
            double impulseAmount = container->getParameter("impulseAmount", 100.0);
            double impulse = 0.0;

            // Find connections to input ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "targetValue") {
                        // Get control value from source container
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            targetValue = applyConnectionFunction(targetValue, sourceValue,
                                                                conn.function, conn.weight);
                        }
                    } else if (conn.toPort == "mass") {
                        // Get mass modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to mass range (0.0-10.0)
                            double sourceValue = processors[conn.fromContainer].controlOut * 10.0;
                            mass = applyConnectionFunction(mass, sourceValue,
                                                         conn.function, conn.weight);
                            mass = qBound(0.0, mass, 10.0);
                        }
                    } else if (conn.toPort == "springK") {
                        // Get springK modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to springK range (0.0001-1.0)
                            double sourceValue = processors[conn.fromContainer].controlOut * 0.9999 + 0.0001;
                            springK = applyConnectionFunction(springK, sourceValue,
                                                            conn.function, conn.weight);
                            springK = qBound(0.0001, springK, 1.0);
                        }
                    } else if (conn.toPort == "damping") {
                        // Get damping modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to damping range (0.5-0.9999)
                            double sourceValue = processors[conn.fromContainer].controlOut * 0.4999 + 0.5;
                            damping = applyConnectionFunction(damping, sourceValue,
                                                            conn.function, conn.weight);
                            damping = qBound(0.5, damping, 0.9999);
                        }
                    } else if (conn.toPort == "impulseAmount") {
                        // Get impulseAmount modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to impulseAmount range (0-1000)
                            double sourceValue = processors[conn.fromContainer].controlOut * 1000.0;
                            impulseAmount = applyConnectionFunction(impulseAmount, sourceValue,
                                                                  conn.function, conn.weight);
                            impulseAmount = qBound(0.0, impulseAmount, 1000.0);
                        }
                    } else if (conn.toPort == "impulse") {
                        // Get impulse trigger from source container (check specific port)
                        if (processors.contains(conn.fromContainer)) {
                            impulse = getOutputValue(processors[conn.fromContainer], conn.fromPort);
                        }
                    }
                }
            }

            // Update physics parameters
            proc.physicsSys->setMass(mass);
            proc.physicsSys->setSpringK(springK);
            proc.physicsSys->setDamping(damping);
            proc.physicsSys->setImpulseAmount(impulseAmount);

            // Handle impulse trigger (rising edge detection)
            // Trigger when impulse crosses threshold (0.5) from below
            if (proc.prevImpulse < 0.5 && impulse >= 0.5) {
                proc.physicsSys->applyImpulse(impulseAmount);
            }
            proc.prevImpulse = impulse;

            // Process physics simulation
            proc.controlOut = proc.physicsSys->processSample(targetValue);
        }

    } else if (container->getName() == "Envelope Engine") {
        if (proc.envelopeEng) {
            // Initialize input values with defaults
            double effectiveProgress = noteProgress;
            double timeScale = container->getParameter("timeScale", 1.0);
            double valueScale = container->getParameter("valueScale", 1.0);
            double valueOffset = container->getParameter("valueOffset", 0.0);

            // Check for connections to modulatable parameters
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "timeScale") {
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to useful range (0.1-5.0)
                            double sourceValue = 0.1 + processors[conn.fromContainer].controlOut * 4.9;
                            timeScale = applyConnectionFunction(timeScale, sourceValue,
                                                              conn.function, conn.weight);
                            timeScale = qBound(0.1, timeScale, 5.0);
                        }
                    } else if (conn.toPort == "valueScale") {
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to 0.0-2.0
                            double sourceValue = processors[conn.fromContainer].controlOut * 2.0;
                            valueScale = applyConnectionFunction(valueScale, sourceValue,
                                                               conn.function, conn.weight);
                            valueScale = qBound(0.0, valueScale, 2.0);
                        }
                    } else if (conn.toPort == "valueOffset") {
                        if (processors.contains(conn.fromContainer)) {
                            // Scale control output (0.0-1.0) to -1.0 to 1.0
                            double sourceValue = processors[conn.fromContainer].controlOut * 2.0 - 1.0;
                            valueOffset = applyConnectionFunction(valueOffset, sourceValue,
                                                                conn.function, conn.weight);
                            valueOffset = qBound(-1.0, valueOffset, 1.0);
                        }
                    }
                }
            }

            // Update envelope parameters
            int envSelect = static_cast<int>(container->getParameter("envelopeSelect", 0.0));

            // Handle custom envelope data if selected
            if (envSelect == 5 && container->hasCustomEnvelopeData()) {
                proc.envelopeEng->setEnvelopeType(EnvelopeEngine::EnvelopeType::Custom);
                EnvelopeData customData = container->getCustomEnvelopeData();
                proc.envelopeEng->setCustomEnvelope(customData.points);
            } else {
                // Standard envelope types
                proc.envelopeEng->setEnvelopeSelect(envSelect);
            }

            proc.envelopeEng->setTimeScale(timeScale);
            proc.envelopeEng->setValueScale(valueScale);
            proc.envelopeEng->setValueOffset(valueOffset);

            // Set envelope-specific timing parameters (for standard envelopes)
            proc.envelopeEng->setAttackTime(container->getParameter("envAttack", 0.1));
            proc.envelopeEng->setDecayTime(container->getParameter("envDecay", 0.2));
            proc.envelopeEng->setSustainLevel(container->getParameter("envSustain", 0.7));
            proc.envelopeEng->setReleaseTime(container->getParameter("envRelease", 0.2));
            proc.envelopeEng->setFadeTime(container->getParameter("envFadeTime", 0.5));

            // Follow Score Curve: bypass the envelope shape and output a named score curve
            // value directly (still scaled/offset by valueScale/valueOffset).
            // scoreCurveIndex 0 = dynamics (default), 1+ = additional named curves.
            if (container->getParameter("followDynamics", 0.0) > 0.5) {
                int scoreIdx = static_cast<int>(container->getParameter("scoreCurveIndex", 0.0));
                double scoreVal = (scoreIdx > 0 && scoreIdx < m_scoreCurveValues.size())
                                  ? m_scoreCurveValues[scoreIdx]
                                  : m_currentDynamics;
                proc.controlOut = qBound(0.0, 1.0, scoreVal * valueScale + valueOffset);
            } else {
                // Normal mode: process envelope shape with note progress
                proc.controlOut = proc.envelopeEng->process(effectiveProgress, m_currentIsLegato);
            }
        }

    } else if (container->getName() == "Drift Engine") {
        if (proc.driftEng) {
            // Initialize input values with defaults
            double amount = container->getParameter("amount", 0.005);
            double rate = container->getParameter("rate", 0.5);

            // Find connections to input ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "amount") {
                        // Get amount modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to amount range (0.0-0.1)
                            double sourceValue = processors[conn.fromContainer].controlOut * 0.1;
                            amount = applyConnectionFunction(amount, sourceValue,
                                                           conn.function, conn.weight);
                            amount = qBound(0.0, amount, 0.1);
                        }
                    } else if (conn.toPort == "rate") {
                        // Get rate modulation from source container
                        if (processors.contains(conn.fromContainer)) {
                            // Scale controlOut (0-1) to rate range (0.01-10.0)
                            double sourceValue = processors[conn.fromContainer].controlOut * 9.99 + 0.01;
                            rate = applyConnectionFunction(rate, sourceValue,
                                                         conn.function, conn.weight);
                            rate = qBound(0.01, rate, 10.0);
                        }
                    }
                }
            }

            // Update drift parameters
            proc.driftEng->setAmount(amount);
            proc.driftEng->setRate(rate);

            // Update drift pattern
            int patternValue = static_cast<int>(container->getParameter("driftPattern", 2.0));
            proc.driftEng->setDriftPattern(static_cast<DriftEngine::DriftPattern>(patternValue));

            // Generate drift value (detuning multiplier around 1.0)
            proc.controlOut = proc.driftEng->generateSample();
        }

    } else if (container->getName() == "Gate Processor") {
        if (proc.gateProc) {
            // Update gate parameters (in case they changed)
            proc.gateProc->setVelocity(container->getParameter("velocity", 1.0));
            proc.gateProc->setAttackTime(container->getParameter("attackTime", 0.01));
            proc.gateProc->setReleaseTime(container->getParameter("releaseTime", 0.1));
            proc.gateProc->setAttackCurve(static_cast<int>(container->getParameter("attackCurve", 0.0)));
            proc.gateProc->setReleaseCurve(static_cast<int>(container->getParameter("releaseCurve", 0.0)));
            proc.gateProc->setVelocitySens(container->getParameter("velocitySens", 0.5));

            // Process gate state machine
            proc.gateProc->processSample();

            // Store all outputs
            proc.gateEnvelopeOut = proc.gateProc->getEnvelopeOut();
            proc.gateStateOut = static_cast<double>(proc.gateProc->getStateOut());
            proc.gateAttackTrigger = proc.gateProc->getAttackTrigger() ? 1.0 : 0.0;
            proc.gateReleaseTrigger = proc.gateProc->getReleaseTrigger() ? 1.0 : 0.0;

            // Default controlOut to envelopeOut for backward compatibility
            proc.controlOut = proc.gateEnvelopeOut;
        }

    } else if (container->getName() == "Easing Applicator") {
        if (proc.easingApp) {
            // Initialize input values from container parameters (inspector defaults)
            double startValue = container->getParameter("startValue", 0.0);
            double endValue = container->getParameter("endValue", 1.0);
            double progress = 0.5;

            // Find connections to input ports
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "startValue") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            startValue = applyConnectionFunction(startValue, sourceValue,
                                                               conn.function, conn.weight);
                        }
                    } else if (conn.toPort == "endValue") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            endValue = applyConnectionFunction(endValue, sourceValue,
                                                             conn.function, conn.weight);
                        }
                    } else if (conn.toPort == "progress") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            progress = applyConnectionFunction(progress, sourceValue,
                                                             conn.function, conn.weight);
                            progress = qBound(0.0, progress, 1.0);
                        }
                    }
                }
            }

            // Update easing parameters (in case they changed)
            proc.easingApp->setEasingSelect(static_cast<int>(container->getParameter("easingSelect", 0.0)));

            // Set custom parameters for Back/Elastic/Wobble easings
            proc.easingApp->setOvershoot(container->getParameter("easingOvershoot", 1.70158));
            proc.easingApp->setAmplitude(container->getParameter("easingAmplitude", 1.0));
            proc.easingApp->setPeriod(container->getParameter("easingPeriod", 0.3));
            proc.easingApp->setFrequency(container->getParameter("easingFrequency", 3.0));
            proc.easingApp->setDecay(container->getParameter("easingDecay", 0.8));

            // Process easing
            proc.controlOut = proc.easingApp->process(startValue, endValue, progress);
        }

    } else if (container->getName() == "Karplus Strong") {
        if (proc.karplusStrongAttack) {
            // Initialize input values with defaults
            double inputSignal = 0.0;
            int effectiveMode = static_cast<int>(container->getParameter("mode", 0.0));
            double effectiveAttackPortion = container->getParameter("attackPortion", 0.15);
            double effectiveDamping = container->getParameter("damping", 0.5);
            double effectivePluckPosition = container->getParameter("pluckPosition", 0.5);
            double effectiveMix = container->getParameter("mix", 0.8);
            double effectiveBrightness = container->getParameter("brightness", 0.5);
            int effectiveExcitationType = static_cast<int>(container->getParameter("excitationType", 0.0));
            double effectiveExcitationSoftness = container->getParameter("blendRatio", 0.5);
            double effectivePluckHardness = container->getParameter("pluckHardness", 0.0);
            double effectivePickDirection = container->getParameter("pickDirection", 0.5);
            double effectiveBodyResonance = container->getParameter("bodyResonance", 0.0);
            double effectiveBodyFreq = container->getParameter("bodyFreq", 200.0);
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);
            bool shouldTrigger = false;

            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "signalIn" && processors.contains(conn.fromContainer)) {
                        // Get input signal from upstream container (for Attack mode)
                        inputSignal = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "trigger" && processors.contains(conn.fromContainer)) {
                        double trigVal = processors[conn.fromContainer].controlOut;
                        if (trigVal > 0.5) shouldTrigger = true;
                    } else if (conn.toPort == "mode" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveMode = sourceVal > 0.5 ? 1 : 0;
                    } else if (conn.toPort == "attackPortion" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveAttackPortion = applyConnectionFunction(
                            effectiveAttackPortion, sourceVal, conn.function, conn.weight);
                        effectiveAttackPortion = qBound(0.01, effectiveAttackPortion, 1.0);
                    } else if (conn.toPort == "damping" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveDamping = applyConnectionFunction(
                            effectiveDamping, sourceVal, conn.function, conn.weight);
                        effectiveDamping = qBound(0.0, effectiveDamping, 1.0);
                    } else if (conn.toPort == "pluckPosition" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectivePluckPosition = applyConnectionFunction(
                            effectivePluckPosition, sourceVal, conn.function, conn.weight);
                        effectivePluckPosition = qBound(0.0, effectivePluckPosition, 1.0);
                    } else if (conn.toPort == "mix" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveMix = applyConnectionFunction(
                            effectiveMix, sourceVal, conn.function, conn.weight);
                        effectiveMix = qBound(0.0, effectiveMix, 1.0);
                    } else if (conn.toPort == "brightness" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveBrightness = applyConnectionFunction(
                            effectiveBrightness, sourceVal, conn.function, conn.weight);
                        effectiveBrightness = qBound(0.0, effectiveBrightness, 1.0);
                    } else if (conn.toPort == "excitationType" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveExcitationType = static_cast<int>(std::round(sourceVal));
                        effectiveExcitationType = qBound(0, effectiveExcitationType, 2);
                    } else if (conn.toPort == "blendRatio" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveExcitationSoftness = applyConnectionFunction(
                            effectiveExcitationSoftness, sourceVal, conn.function, conn.weight);
                        effectiveExcitationSoftness = qBound(0.0, effectiveExcitationSoftness, 1.0);
                    } else if (conn.toPort == "pluckHardness" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectivePluckHardness = applyConnectionFunction(
                            effectivePluckHardness, sourceVal, conn.function, conn.weight);
                        effectivePluckHardness = qBound(0.0, effectivePluckHardness, 1.0);
                    } else if (conn.toPort == "pickDirection" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectivePickDirection = applyConnectionFunction(
                            effectivePickDirection, sourceVal, conn.function, conn.weight);
                        effectivePickDirection = qBound(0.0, effectivePickDirection, 1.0);
                    } else if (conn.toPort == "bodyResonance" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveBodyResonance = applyConnectionFunction(
                            effectiveBodyResonance, sourceVal, conn.function, conn.weight);
                        effectiveBodyResonance = qBound(0.0, effectiveBodyResonance, 1.0);
                    } else if (conn.toPort == "bodyFreq" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveBodyFreq = applyConnectionFunction(
                            effectiveBodyFreq, sourceVal, conn.function, conn.weight);
                        effectiveBodyFreq = qBound(80.0, effectiveBodyFreq, 400.0);
                    } else if (conn.toPort == "pitchMultiplier" && processors.contains(conn.fromContainer)) {
                        pitchMult = processors[conn.fromContainer].controlOut;
                    }
                }
            }
            double effectivePitch = pitch * pitchMult;

            // Update parameters
            proc.karplusStrongAttack->setMode(static_cast<KSMode>(effectiveMode));
            proc.karplusStrongAttack->setAttackPortion(effectiveAttackPortion);
            proc.karplusStrongAttack->setDamping(effectiveDamping);
            proc.karplusStrongAttack->setPluckPosition(effectivePluckPosition);
            proc.karplusStrongAttack->setMix(effectiveMix);
            proc.karplusStrongAttack->setBrightness(effectiveBrightness);
            proc.karplusStrongAttack->setExcitationType(effectiveExcitationType);
            proc.karplusStrongAttack->setExcitationSoftness(effectiveExcitationSoftness);
            proc.karplusStrongAttack->setPluckHardness(effectivePluckHardness);
            proc.karplusStrongAttack->setPickDirection(effectivePickDirection);
            proc.karplusStrongAttack->setBodyResonance(effectiveBodyResonance);
            proc.karplusStrongAttack->setBodyFreq(effectiveBodyFreq);
            proc.karplusStrongAttack->setPitch(effectivePitch);

            // Trigger on note start (noteProgress near 0) or explicit trigger
            // For legato notes, skip the trigger - let the string keep vibrating from previous note
            if (!m_currentIsLegato && (noteProgress < 0.001 || shouldTrigger)) {
                proc.karplusStrongAttack->trigger(effectivePitch);
            }

            // Generate sample with input signal (used in Attack mode)
            proc.signalOut = proc.karplusStrongAttack->generateSample(inputSignal);
        }

    } else if (container->getName() == "Attack") {
        if (proc.attackGen) {
            // Initialize input values with defaults
            double inputSignal = 0.0;
            int effectiveAttackType = static_cast<int>(container->getParameter("attackType", 0.0));
            double effectiveDuration = container->getParameter("duration", 0.15);
            double effectiveIntensity = container->getParameter("intensity", 0.8);
            double effectiveMix = container->getParameter("mix", 0.8);
            double effectiveNoiseAmount = container->getParameter("noiseAmount", 0.5);
            double effectiveJetRatio = container->getParameter("jetRatio", 0.32);
            double effectiveReedStiffness = container->getParameter("reedStiffness", 0.5);
            double effectiveReedAperture = container->getParameter("reedAperture", 0.5);
            double effectiveBlowPosition = container->getParameter("blowPosition", 0.5);
            double effectiveLipTension = container->getParameter("lipTension", 0.5);
            double effectiveBuzzQ = container->getParameter("buzzQ", 5.0);
            double effectiveHardness = container->getParameter("hardness", 0.5);
            double effectiveBrightness = container->getParameter("brightness", 0.5);
            double effectiveTightness = container->getParameter("tightness", 0.5);
            double effectiveTone = container->getParameter("tone", 0.5);
            bool shouldTrigger = false;

            // Use cached connections
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "signalIn" && processors.contains(conn.fromContainer)) {
                        inputSignal = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "trigger" && processors.contains(conn.fromContainer)) {
                        double trigVal = processors[conn.fromContainer].controlOut;
                        if (trigVal > 0.5) shouldTrigger = true;
                    } else if (conn.toPort == "attackType" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveAttackType = static_cast<int>(sourceVal * 6.0);
                        effectiveAttackType = qBound(0, effectiveAttackType, 5);
                    } else if (conn.toPort == "duration" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveDuration = applyConnectionFunction(effectiveDuration, sourceVal * 5.0, conn.function, conn.weight);
                        effectiveDuration = qBound(0.005, effectiveDuration, 5.0);
                    } else if (conn.toPort == "intensity" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveIntensity = applyConnectionFunction(effectiveIntensity, sourceVal, conn.function, conn.weight);
                        effectiveIntensity = qBound(0.0, effectiveIntensity, 1.0);
                    } else if (conn.toPort == "mix" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveMix = applyConnectionFunction(effectiveMix, sourceVal, conn.function, conn.weight);
                        effectiveMix = qBound(0.0, effectiveMix, 1.0);
                    } else if (conn.toPort == "noiseAmount" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveNoiseAmount = applyConnectionFunction(effectiveNoiseAmount, sourceVal, conn.function, conn.weight);
                        effectiveNoiseAmount = qBound(0.0, effectiveNoiseAmount, 1.0);
                    } else if (conn.toPort == "jetRatio" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveJetRatio = applyConnectionFunction(effectiveJetRatio, sourceVal, conn.function, conn.weight);
                        effectiveJetRatio = qBound(0.0, effectiveJetRatio, 1.0);
                    } else if (conn.toPort == "reedStiffness" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveReedStiffness = applyConnectionFunction(effectiveReedStiffness, sourceVal, conn.function, conn.weight);
                        effectiveReedStiffness = qBound(0.0, effectiveReedStiffness, 1.0);
                    } else if (conn.toPort == "reedAperture" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveReedAperture = applyConnectionFunction(effectiveReedAperture, sourceVal, conn.function, conn.weight);
                        effectiveReedAperture = qBound(0.0, effectiveReedAperture, 1.0);
                    } else if (conn.toPort == "blowPosition" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveBlowPosition = applyConnectionFunction(effectiveBlowPosition, sourceVal, conn.function, conn.weight);
                        effectiveBlowPosition = qBound(0.0, effectiveBlowPosition, 1.0);
                    } else if (conn.toPort == "lipTension" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveLipTension = applyConnectionFunction(effectiveLipTension, sourceVal, conn.function, conn.weight);
                        effectiveLipTension = qBound(0.0, effectiveLipTension, 1.0);
                    } else if (conn.toPort == "buzzQ" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveBuzzQ = applyConnectionFunction(effectiveBuzzQ, sourceVal * 50.0, conn.function, conn.weight);
                        effectiveBuzzQ = qBound(0.5, effectiveBuzzQ, 50.0);
                    } else if (conn.toPort == "hardness" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveHardness = applyConnectionFunction(effectiveHardness, sourceVal, conn.function, conn.weight);
                        effectiveHardness = qBound(0.0, effectiveHardness, 1.0);
                    } else if (conn.toPort == "brightness" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveBrightness = applyConnectionFunction(effectiveBrightness, sourceVal, conn.function, conn.weight);
                        effectiveBrightness = qBound(0.0, effectiveBrightness, 1.0);
                    } else if (conn.toPort == "tightness" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveTightness = applyConnectionFunction(effectiveTightness, sourceVal, conn.function, conn.weight);
                        effectiveTightness = qBound(0.0, effectiveTightness, 1.0);
                    } else if (conn.toPort == "tone" && processors.contains(conn.fromContainer)) {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveTone = applyConnectionFunction(effectiveTone, sourceVal, conn.function, conn.weight);
                        effectiveTone = qBound(0.0, effectiveTone, 1.0);
                    }
                }
            }

            // Update parameters
            proc.attackGen->setAttackType(effectiveAttackType);
            proc.attackGen->setDuration(effectiveDuration);
            proc.attackGen->setIntensity(effectiveIntensity);
            proc.attackGen->setMix(effectiveMix);
            proc.attackGen->setNoiseAmount(effectiveNoiseAmount);
            proc.attackGen->setJetRatio(effectiveJetRatio);
            proc.attackGen->setReedStiffness(effectiveReedStiffness);
            proc.attackGen->setReedAperture(effectiveReedAperture);
            proc.attackGen->setBlowPosition(effectiveBlowPosition);
            proc.attackGen->setLipTension(effectiveLipTension);
            proc.attackGen->setBuzzQ(effectiveBuzzQ);
            proc.attackGen->setHardness(effectiveHardness);
            proc.attackGen->setBrightness(effectiveBrightness);
            proc.attackGen->setTightness(effectiveTightness);
            proc.attackGen->setTone(effectiveTone);

            // Trigger on note start or explicit trigger
            // For legato notes, skip the trigger
            if (!m_currentIsLegato && (noteProgress < 0.001 || shouldTrigger)) {
                proc.attackGen->trigger(pitch);
            }

            // Generate sample with input signal
            proc.signalOut = proc.attackGen->generateSample(inputSignal);
        }

    } else if (container->getName() == "Recorder") {
        if (proc.recorderModel) {
            // Start with stored parameter values; allow modulation via input ports
            double effectiveBreathPressure = container->getParameter("breathPressure", 0.81);
            double effectiveJetRatio       = container->getParameter("jetRatio", 0.35);
            double effectiveNoiseGain      = container->getParameter("noiseGain", 0.015);
            double effectiveVibratoFreq    = container->getParameter("vibratoFreq", 0.0);
            double effectiveVibratoGain    = container->getParameter("vibratoGain", 0.0);
            double effectiveEndReflection  = container->getParameter("endReflection", 0.73);
            double effectiveJetReflection  = container->getParameter("jetReflection", 0.59);
            double pitchMult               = container->getParameter("pitchMultiplier", 1.0);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer != container) continue;
                if (!processors.contains(conn.fromContainer)) continue;
                double sv = processors[conn.fromContainer].controlOut;

                if (conn.toPort == "pitchMultiplier") {
                    pitchMult = sv;
                } else if (conn.toPort == "breathPressure") {
                    effectiveBreathPressure = applyConnectionFunction(
                        effectiveBreathPressure, sv, conn.function, conn.weight);
                    effectiveBreathPressure = qBound(0.0, effectiveBreathPressure, 1.0);
                } else if (conn.toPort == "jetRatio") {
                    // controlOut [0,1] mapped to jetRatio [0.05, 0.60]
                    double scaled = 0.05 + sv * 0.55;
                    effectiveJetRatio = applyConnectionFunction(
                        effectiveJetRatio, scaled, conn.function, conn.weight);
                    effectiveJetRatio = qBound(0.05, effectiveJetRatio, 0.60);
                } else if (conn.toPort == "noiseGain") {
                    // controlOut [0,1] mapped to [0, 0.5]
                    double scaled = sv * 0.5;
                    effectiveNoiseGain = applyConnectionFunction(
                        effectiveNoiseGain, scaled, conn.function, conn.weight);
                    effectiveNoiseGain = qBound(0.0, effectiveNoiseGain, 0.5);
                } else if (conn.toPort == "vibratoFreq") {
                    // controlOut [0,1] mapped to [0, 20] Hz
                    double scaled = sv * 20.0;
                    effectiveVibratoFreq = applyConnectionFunction(
                        effectiveVibratoFreq, scaled, conn.function, conn.weight);
                    effectiveVibratoFreq = qBound(0.0, effectiveVibratoFreq, 20.0);
                } else if (conn.toPort == "vibratoGain") {
                    effectiveVibratoGain = applyConnectionFunction(
                        effectiveVibratoGain, sv, conn.function, conn.weight);
                    effectiveVibratoGain = qBound(0.0, effectiveVibratoGain, 1.0);
                }
            }

            proc.recorderModel->setBreathPressure(effectiveBreathPressure);
            proc.recorderModel->setJetRatio(effectiveJetRatio);
            proc.recorderModel->setNoiseGain(effectiveNoiseGain);
            proc.recorderModel->setVibratoFreq(effectiveVibratoFreq);
            proc.recorderModel->setVibratoGain(effectiveVibratoGain);
            proc.recorderModel->setEndReflection(effectiveEndReflection);
            proc.recorderModel->setJetReflection(effectiveJetReflection);

            proc.signalOut = proc.recorderModel->tick(pitch * pitchMult, noteProgress, m_currentIsLegato, false);
        }

    } else if (container->getName() == "Bowed") {
        if (proc.bowedModel) {
            double effectiveBowPressure = container->getParameter("bowPressure", 0.75);
            double effectiveBowVelocity = container->getParameter("bowVelocity", 0.5);
            double effectiveBowPosition = container->getParameter("bowPosition", 0.127);
            int    effectiveNLType      = static_cast<int>(container->getParameter("nlType", 0.0));
            double effectiveNLAmount    = container->getParameter("nlAmount", 0.0);
            double effectiveNLFreqMod   = container->getParameter("nlFreqMod", 10.0);
            double effectiveNLAttack    = container->getParameter("nlAttack", 0.1);
            double pitchMult            = container->getParameter("pitchMultiplier", 1.0);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer != container) continue;
                if (!processors.contains(conn.fromContainer)) continue;
                double sv = processors[conn.fromContainer].controlOut;

                if (conn.toPort == "pitchMultiplier") {
                    pitchMult = sv;
                } else if (conn.toPort == "bowPressure") {
                    effectiveBowPressure = applyConnectionFunction(
                        effectiveBowPressure, sv, conn.function, conn.weight);
                    effectiveBowPressure = qBound(0.0, effectiveBowPressure, 1.0);
                } else if (conn.toPort == "bowVelocity") {
                    effectiveBowVelocity = applyConnectionFunction(
                        effectiveBowVelocity, sv, conn.function, conn.weight);
                    effectiveBowVelocity = qBound(0.0, effectiveBowVelocity, 1.0);
                } else if (conn.toPort == "bowPosition") {
                    effectiveBowPosition = applyConnectionFunction(
                        effectiveBowPosition, sv, conn.function, conn.weight);
                    effectiveBowPosition = qBound(0.0, effectiveBowPosition, 1.0);
                } else if (conn.toPort == "nlAmount") {
                    effectiveNLAmount = applyConnectionFunction(
                        effectiveNLAmount, sv, conn.function, conn.weight);
                    effectiveNLAmount = qBound(0.0, effectiveNLAmount, 1.0);
                } else if (conn.toPort == "nlFreqMod") {
                    // controlOut [0,1] mapped to [0, 200] Hz
                    double scaled = sv * 200.0;
                    effectiveNLFreqMod = applyConnectionFunction(
                        effectiveNLFreqMod, scaled, conn.function, conn.weight);
                    effectiveNLFreqMod = qBound(0.0, effectiveNLFreqMod, 200.0);
                } else if (conn.toPort == "nlAttack") {
                    // controlOut [0,1] mapped to [0, 2] s
                    double scaled = sv * 2.0;
                    effectiveNLAttack = applyConnectionFunction(
                        effectiveNLAttack, scaled, conn.function, conn.weight);
                    effectiveNLAttack = qBound(0.001, effectiveNLAttack, 2.0);
                }
                // nlType is an integer selector — not modulated via connection
            }

            proc.bowedModel->setBowPressure(effectiveBowPressure);
            proc.bowedModel->setBowVelocity(effectiveBowVelocity);
            proc.bowedModel->setBowPosition(effectiveBowPosition);
            proc.bowedModel->setNLType(effectiveNLType);
            proc.bowedModel->setNLAmount(effectiveNLAmount);
            proc.bowedModel->setNLFreqMod(effectiveNLFreqMod);
            proc.bowedModel->setNLAttack(effectiveNLAttack);

            proc.signalOut = proc.bowedModel->tick(pitch * pitchMult, noteProgress, m_currentIsLegato, false);
        }

    } else if (container->getName() == "Reed") {
        if (proc.reedModel) {
            double effectiveBreathPressure = container->getParameter("breathPressure", 0.7);
            double effectiveReedStiffness  = container->getParameter("reedStiffness",  0.5);
            double effectiveReedAperture   = container->getParameter("reedAperture",   0.5);
            double effectiveBlowPosition   = container->getParameter("blowPosition",   0.2);
            double effectiveNoiseGain      = container->getParameter("noiseGain",      0.2);
            double effectiveVibratoFreq    = container->getParameter("vibratoFreq",  5.735);
            double effectiveVibratoGain    = container->getParameter("vibratoGain",    0.0);
            int    effectiveNLType         = static_cast<int>(container->getParameter("nlType",    0.0));
            double effectiveNLAmount       = container->getParameter("nlAmount",   0.0);
            double effectiveNLFreqMod      = container->getParameter("nlFreqMod",  10.0);
            double effectiveNLAttack       = container->getParameter("nlAttack",   0.1);
            double pitchMult               = container->getParameter("pitchMultiplier", 1.0);

            // Apply modulated inputs from connected ports
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer != container) continue;
                double sourceVal = getOutputValue(processors[conn.fromContainer], conn.fromPort);
                if (conn.toPort == "pitchMultiplier") {
                    pitchMult = sourceVal;
                } else if (conn.toPort == "breathPressure") {
                    effectiveBreathPressure = applyConnectionFunction(
                        effectiveBreathPressure, sourceVal, conn.function, conn.weight);
                    effectiveBreathPressure = qBound(0.0, effectiveBreathPressure, 1.0);
                } else if (conn.toPort == "reedStiffness") {
                    effectiveReedStiffness = applyConnectionFunction(
                        effectiveReedStiffness, sourceVal, conn.function, conn.weight);
                    effectiveReedStiffness = qBound(0.0, effectiveReedStiffness, 1.0);
                } else if (conn.toPort == "reedAperture") {
                    effectiveReedAperture = applyConnectionFunction(
                        effectiveReedAperture, sourceVal, conn.function, conn.weight);
                    effectiveReedAperture = qBound(0.0, effectiveReedAperture, 1.0);
                } else if (conn.toPort == "blowPosition") {
                    effectiveBlowPosition = applyConnectionFunction(
                        effectiveBlowPosition, sourceVal, conn.function, conn.weight);
                    effectiveBlowPosition = qBound(0.001, effectiveBlowPosition, 0.999);
                } else if (conn.toPort == "noiseGain") {
                    effectiveNoiseGain = applyConnectionFunction(
                        effectiveNoiseGain, sourceVal, conn.function, conn.weight);
                    effectiveNoiseGain = qBound(0.0, effectiveNoiseGain, 0.4);
                } else if (conn.toPort == "vibratoFreq") {
                    effectiveVibratoFreq = applyConnectionFunction(
                        effectiveVibratoFreq, sourceVal, conn.function, conn.weight);
                    effectiveVibratoFreq = qBound(0.0, effectiveVibratoFreq, 12.0);
                } else if (conn.toPort == "vibratoGain") {
                    effectiveVibratoGain = applyConnectionFunction(
                        effectiveVibratoGain, sourceVal, conn.function, conn.weight);
                    effectiveVibratoGain = qBound(0.0, effectiveVibratoGain, 0.5);
                } else if (conn.toPort == "nlAmount") {
                    effectiveNLAmount = applyConnectionFunction(
                        effectiveNLAmount, sourceVal, conn.function, conn.weight);
                    effectiveNLAmount = qBound(0.0, effectiveNLAmount, 1.0);
                } else if (conn.toPort == "nlFreqMod") {
                    effectiveNLFreqMod = applyConnectionFunction(
                        effectiveNLFreqMod, sourceVal, conn.function, conn.weight);
                    effectiveNLFreqMod = qBound(0.0, effectiveNLFreqMod, 200.0);
                } else if (conn.toPort == "nlAttack") {
                    effectiveNLAttack = applyConnectionFunction(
                        effectiveNLAttack, sourceVal, conn.function, conn.weight);
                    effectiveNLAttack = qBound(0.001, effectiveNLAttack, 2.0);
                }
                // nlType is an integer selector — not modulated via connection
            }

            proc.reedModel->setBreathPressure(effectiveBreathPressure);
            proc.reedModel->setReedStiffness(effectiveReedStiffness);
            proc.reedModel->setReedAperture(effectiveReedAperture);
            proc.reedModel->setBlowPosition(effectiveBlowPosition);
            proc.reedModel->setNoiseGain(effectiveNoiseGain);
            proc.reedModel->setVibratoFreq(effectiveVibratoFreq);
            proc.reedModel->setVibratoGain(effectiveVibratoGain);
            proc.reedModel->setNLType(effectiveNLType);
            proc.reedModel->setNLAmount(effectiveNLAmount);
            proc.reedModel->setNLFreqMod(effectiveNLFreqMod);
            proc.reedModel->setNLAttack(effectiveNLAttack);

            proc.signalOut = proc.reedModel->tick(pitch * pitchMult, noteProgress, m_currentIsLegato, false);
        }

    } else if (container->getName() == "LFO") {
        if (proc.lfo) {
            // Initialize input values with defaults from parameters
            double frequency = container->getParameter("frequency", 1.0);
            double amplitude = container->getParameter("amplitude", 1.0);
            int waveType = static_cast<int>(container->getParameter("waveType", 0.0));

            // Find connections to input ports and apply modulation
            // Use cached connections (snapshot at build time) for graph independence
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "frequency" && processors.contains(conn.fromContainer)) {
                        // Get frequency modulation from source container
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        // Scale controlOut (0-1) to reasonable LFO frequency range (0.01-20 Hz)
                        sourceValue = sourceValue * 19.99 + 0.01;
                        frequency = applyConnectionFunction(frequency, sourceValue,
                                                           conn.function, conn.weight);
                        frequency = qBound(0.01, frequency, 100.0);
                    } else if (conn.toPort == "amplitude" && processors.contains(conn.fromContainer)) {
                        // Get amplitude modulation from source container
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        amplitude = applyConnectionFunction(amplitude, sourceValue,
                                                           conn.function, conn.weight);
                        amplitude = qBound(0.0, amplitude, 1.0);
                    } else if (conn.toPort == "waveType" && processors.contains(conn.fromContainer)) {
                        // Get wave type from source container (quantize to 0-3)
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        waveType = static_cast<int>(sourceValue * 4.0);
                        waveType = qBound(0, waveType, 3);
                    }
                }
            }

            // Update LFO parameters
            proc.lfo->setFrequency(frequency);
            proc.lfo->setAmplitude(amplitude);
            proc.lfo->setWaveType(waveType);

            // Generate LFO sample (outputs -amplitude to +amplitude based on wave type)
            proc.controlOut = proc.lfo->process();
        }

    } else if (container->getName() == "Frequency Mapper") {
        if (proc.freqMapper) {
            // Read parameters
            double lowFreq = container->getParameter("lowFreq", 25.0);
            double highFreq = container->getParameter("highFreq", 8000.0);
            double outputMin = container->getParameter("outputMin", 0.0);
            double outputMax = container->getParameter("outputMax", 1.0);
            int curveType = static_cast<int>(container->getParameter("curveType", 1.0));
            bool invert = container->getParameter("invert", 0.0) > 0.5;

            // Check for external curve input and pitchMultiplier connections
            bool hasCurveInput = false;
            double externalCurve = 0.0;
            double pitchMult = container->getParameter("pitchMultiplier", 1.0);
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer != container || !processors.contains(conn.fromContainer)) continue;
                if (conn.toPort == "curve") {
                    externalCurve = processors[conn.fromContainer].controlOut;
                    externalCurve = applyConnectionFunction(0.0, externalCurve,
                                                           conn.function, conn.weight);
                    externalCurve = qBound(0.0, externalCurve, 1.0);
                    hasCurveInput = true;
                } else if (conn.toPort == "pitchMultiplier") {
                    pitchMult = processors[conn.fromContainer].controlOut;
                }
            }
            double effectivePitch = pitch * pitchMult;

            // Update processor parameters
            proc.freqMapper->setLowFreq(lowFreq);
            proc.freqMapper->setHighFreq(highFreq);
            proc.freqMapper->setOutputMin(outputMin);
            proc.freqMapper->setOutputMax(outputMax);
            proc.freqMapper->setCurveType(curveType);
            proc.freqMapper->setInvert(invert);

            // Map pitch to control output
            if (hasCurveInput) {
                proc.controlOut = proc.freqMapper->map(effectivePitch, externalCurve);
            } else {
                proc.controlOut = proc.freqMapper->map(effectivePitch);
            }
        }

    } else if (container->getName() == "Wavetable Synth") {
        if (proc.wavetableSynth) {
            // Read position and pitchMultiplier inputs from connections
            double positionVal = container->getParameter("position", 0.0);
            double pitchMult   = container->getParameter("pitchMultiplier", 1.0);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "position") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            positionVal = applyConnectionFunction(positionVal, sourceValue,
                                                                  conn.function, conn.weight);
                            positionVal = qBound(0.0, positionVal, 1.0);
                        }
                    } else if (conn.toPort == "pitchMultiplier") {
                        if (processors.contains(conn.fromContainer))
                            pitchMult = processors[conn.fromContainer].controlOut;
                    }
                }
            }
            double effectivePitch = pitch * pitchMult;

            proc.wavetableSynth->setPosition(positionVal);

            // Generate sample (direct waveform playback)
            WavetableSynth::OutputMode mode = proc.wavetableSynth->getOutputMode();

            if (mode == WavetableSynth::Both || mode == WavetableSynth::SignalOnly) {
                proc.signalOut = proc.wavetableSynth->generateSample(effectivePitch);
            } else {
                // Still need to advance phase even in spectrum-only mode
                proc.wavetableSynth->generateSample(effectivePitch);
                proc.signalOut = 0.0;
            }

            if (mode == WavetableSynth::Both || mode == WavetableSynth::SpectrumOnly) {
                proc.spectrumOut = proc.wavetableSynth->getCurrentSpectrum();
            }
        }

    } else if (container->getName() == "10-Band EQ") {
        if (proc.bandpassEQ) {
            double inputSignal = 0.0;

            // Load default gains from container parameters
            double bandGains[BandpassEQ::NUM_BANDS];
            for (int i = 0; i < BandpassEQ::NUM_BANDS; i++) {
                bandGains[i] = container->getParameter(QString("band%1").arg(i + 1), 1.0);
            }

            // Check connections
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "signalIn") {
                        if (processors.contains(conn.fromContainer)) {
                            inputSignal = processors[conn.fromContainer].signalOut;
                        }
                    } else {
                        // Check band1..band10 modulation inputs
                        for (int i = 0; i < BandpassEQ::NUM_BANDS; i++) {
                            QString portName = QString("band%1").arg(i + 1);
                            if (conn.toPort == portName && processors.contains(conn.fromContainer)) {
                                // Scale control output (0-1) to gain range (0-2)
                                double sourceValue = processors[conn.fromContainer].controlOut * 2.0;
                                bandGains[i] = applyConnectionFunction(bandGains[i], sourceValue,
                                                                       conn.function, conn.weight);
                                bandGains[i] = qBound(0.0, bandGains[i], 2.0);
                            }
                        }
                    }
                }
            }

            // Apply gains (Q is set from inspector only, not modulatable)
            for (int i = 0; i < BandpassEQ::NUM_BANDS; i++) {
                proc.bandpassEQ->setGain(i, bandGains[i]);
                proc.bandpassEQ->setQ(i, container->getParameter(QString("q%1").arg(i + 1), 2.0));
            }

            proc.signalOut = proc.bandpassEQ->processSample(inputSignal);
        }

    } else if (container->getName() == "Comb Filter") {
        if (proc.combFilter) {
            double inputSignal = 0.0;
            double delayTime = container->getParameter("delayTime", 5.0);
            double feedback = container->getParameter("feedback", 0.5);
            double damping = container->getParameter("damping", 0.0);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container && processors.contains(conn.fromContainer)) {
                    if (conn.toPort == "signalIn") {
                        inputSignal = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "delayTime") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        delayTime = applyConnectionFunction(delayTime, sourceValue * 100.0,
                                                            conn.function, conn.weight);
                        delayTime = qBound(0.1, delayTime, 100.0);
                    } else if (conn.toPort == "feedback") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        feedback = applyConnectionFunction(feedback, sourceValue * 1.98 - 0.99,
                                                           conn.function, conn.weight);
                        feedback = qBound(-0.99, feedback, 0.99);
                    } else if (conn.toPort == "damping") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        damping = applyConnectionFunction(damping, sourceValue,
                                                          conn.function, conn.weight);
                        damping = qBound(0.0, damping, 1.0);
                    }
                }
            }

            // Mode is set from inspector only (not modulatable)
            proc.combFilter->setMode(static_cast<int>(container->getParameter("mode", 0.0)));
            proc.combFilter->setDelayTime(delayTime);
            proc.combFilter->setFeedback(feedback);
            proc.combFilter->setDamping(damping);

            proc.signalOut = proc.combFilter->processSample(inputSignal);
        }

    } else if (container->getName() == "LP/HP Filter") {
        if (proc.lowHighPassFilter) {
            double inputSignal = 0.0;
            double cutoff = container->getParameter("cutoff", 1000.0);
            double resonance = container->getParameter("resonance", 0.707);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container && processors.contains(conn.fromContainer)) {
                    if (conn.toPort == "signalIn") {
                        inputSignal = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "cutoff") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        // Exponential scaling: 0→20Hz, 0.5→632Hz, 1→20000Hz
                        double modulatedCutoff = 20.0 * pow(1000.0, sourceValue);
                        cutoff = applyConnectionFunction(cutoff, modulatedCutoff,
                                                         conn.function, conn.weight);
                        cutoff = qBound(20.0, cutoff, 20000.0);
                    } else if (conn.toPort == "resonance") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        // Scale 0-1 to 0.5-20
                        double modulatedQ = 0.5 + sourceValue * 19.5;
                        resonance = applyConnectionFunction(resonance, modulatedQ,
                                                            conn.function, conn.weight);
                        resonance = qBound(0.5, resonance, 20.0);
                    }
                }
            }

            // Mode is set from inspector only (not modulatable)
            proc.lowHighPassFilter->setMode(static_cast<int>(container->getParameter("mode", 0.0)));
            proc.lowHighPassFilter->setCutoff(cutoff);
            proc.lowHighPassFilter->setResonance(resonance);

            proc.signalOut = proc.lowHighPassFilter->processSample(inputSignal);
        }

    } else if (container->getName() == "IR Convolution") {
        if (proc.irConvolution) {
            double inputSignal = 0.0;
            double irInValue   = 0.0;
            double wetDry = container->getParameter("wetDry", 0.5);
            double predelay = container->getParameter("predelay", 0.0);
            double highDamp = container->getParameter("highDamp", 20000.0);
            double lowCut = container->getParameter("lowCut", 20.0);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container && processors.contains(conn.fromContainer)) {
                    if (conn.toPort == "signalIn") {
                        inputSignal = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "irIn") {
                        irInValue = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "wetDry") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        wetDry = applyConnectionFunction(wetDry, sourceValue,
                                                         conn.function, conn.weight);
                        wetDry = qBound(0.0, wetDry, 1.0);
                    } else if (conn.toPort == "predelay") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        predelay = applyConnectionFunction(predelay, sourceValue,
                                                            conn.function, conn.weight);
                        predelay = qBound(0.0, predelay, 500.0);
                    } else if (conn.toPort == "highDamp") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        highDamp = applyConnectionFunction(highDamp, sourceValue,
                                                            conn.function, conn.weight);
                        highDamp = qBound(200.0, highDamp, 20000.0);
                    } else if (conn.toPort == "lowCut") {
                        double sourceValue = processors[conn.fromContainer].controlOut;
                        lowCut = applyConnectionFunction(lowCut, sourceValue,
                                                          conn.function, conn.weight);
                        lowCut = qBound(20.0, lowCut, 2000.0);
                    }
                }
            }

            if (proc.irInConnected) {
                proc.irConvolution->pushIRSample(irInValue);
            }

            proc.irConvolution->setWetDry(wetDry);
            proc.irConvolution->setPredelay(predelay);
            proc.irConvolution->setHighDamp(highDamp);
            proc.irConvolution->setLowCut(lowCut);

            if (proc.container == m_postRenderIRContainer) {
                // Post-render: parameters are updated above so connection-driven
                // modulation (e.g. wetDry from Envelope Engine) still works each sample.
                // Actual convolution is deferred to processPostIR() in the audio engine.
                proc.signalOut = 0.0;
            } else {
                proc.signalOut = proc.irConvolution->processSample(inputSignal);
            }
        }

    } else if (container->getName() == "Note Tail") {
        if (proc.tailProc) {
            double inputSignal = 0.0;
            double effectiveTailLength = container->getParameter("tailLength", 200.0);
            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container && processors.contains(conn.fromContainer)) {
                    if (conn.toPort == "signalIn") {
                        inputSignal = processors[conn.fromContainer].signalOut;
                    } else if (conn.toPort == "length") {
                        double sourceVal = processors[conn.fromContainer].controlOut;
                        effectiveTailLength = applyConnectionFunction(
                            effectiveTailLength, sourceVal, conn.function, conn.weight);
                        effectiveTailLength = qBound(1.0, effectiveTailLength, 9999.0);
                    }
                }
            }
            proc.tailProc->setTailLength(effectiveTailLength);
            proc.signalOut = proc.tailProc->processSample(inputSignal, m_tailMode);
        }

    } else if (container->getName() == "Signal Mixer") {
        if (proc.signalMixer) {
            double signalA = 0.0;
            double signalB = 0.0;
            double gainA = container->getParameter("gainA", 1.0);
            double gainB = container->getParameter("gainB", 1.0);

            for (const Canvas::Connection &conn : cachedConnections) {
                if (conn.toContainer == container) {
                    if (conn.toPort == "signalA") {
                        if (processors.contains(conn.fromContainer)) {
                            signalA = processors[conn.fromContainer].signalOut;
                        }
                    } else if (conn.toPort == "signalB") {
                        if (processors.contains(conn.fromContainer)) {
                            signalB = processors[conn.fromContainer].signalOut;
                        }
                    } else if (conn.toPort == "gainA") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            gainA = applyConnectionFunction(gainA, sourceValue,
                                                           conn.function, conn.weight);
                            gainA = qBound(0.0, gainA, 2.0);
                        }
                    } else if (conn.toPort == "gainB") {
                        if (processors.contains(conn.fromContainer)) {
                            double sourceValue = processors[conn.fromContainer].controlOut;
                            gainB = applyConnectionFunction(gainB, sourceValue,
                                                           conn.function, conn.weight);
                            gainB = qBound(0.0, gainB, 2.0);
                        }
                    }
                }
            }

            proc.signalOut = proc.signalMixer->process(signalA, signalB, gainA, gainB);
        }
    }
}

double SounitGraph::getInputValue(Container *container, const QString &portName, double defaultValue)
{
    // For future: traverse connections to get input values
    // For now, just return default
    return container->getParameter(portName, defaultValue);
}

double SounitGraph::getOutputValue(const ProcessorData &proc, const QString &portName) const
{
    // Gate Processor has multiple named outputs
    if (portName == "envelopeOut") {
        return proc.gateEnvelopeOut;
    } else if (portName == "stateOut") {
        return proc.gateStateOut;
    } else if (portName == "attackTrigger") {
        return proc.gateAttackTrigger;
    } else if (portName == "releaseTrigger") {
        return proc.gateReleaseTrigger;
    }

    // Default: check if it's a signal or control output
    if (portName == "signalOut" || portName.contains("signal", Qt::CaseInsensitive)) {
        return proc.signalOut;
    }

    // Default to controlOut for any other named output
    return proc.controlOut;
}

