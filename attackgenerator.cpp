#include "attackgenerator.h"
#include <cmath>
#include <algorithm>

static constexpr double PI = 3.14159265358979323846;
static constexpr double TWO_PI = 2.0 * PI;
static constexpr double TARGET_PEAK = 0.8;
static constexpr double FADE_PORTION = 0.1;  // Final 10% gets cosine fade

AttackGenerator::AttackGenerator(double sampleRate)
    : sampleRate(sampleRate)
    , attackType(AttackType::FluteChiff)
    , duration(0.25)
    , intensity(0.8)
    , mix(0.8)
    , noiseAmount(0.5)
    , jetRatio(0.32)
    , reedStiffness(0.5)
    , reedAperture(0.5)
    , blowPosition(0.5)
    , lipTension(0.5)
    , buzzQ(5.0)
    , hardness(0.5)
    , brightness(0.5)
    , tightness(0.5)
    , tone(0.5)
    , active(false)
    , sampleCount(0)
    , attackSamples(0)
    , currentPitch(440.0)
    , onePoleState1(0.0)
    , onePoleState2(0.0)
    , biquadX1(0.0), biquadX2(0.0)
    , biquadY1(0.0), biquadY2(0.0)
    , biquadA0(1.0), biquadA1(0.0), biquadA2(0.0)
    , biquadB1(0.0), biquadB2(0.0)
    , bq2X1(0.0), bq2X2(0.0)
    , bq2Y1(0.0), bq2Y2(0.0)
    , bq2A0(1.0), bq2A1(0.0), bq2A2(0.0)
    , bq2B1(0.0), bq2B2(0.0)
    , fmPhase(0.0)
    , fmModPhase(0.0)
    , fluteWobblePhase(0.0)
    , brassBendPhase(0.0)
    , drumSweepPhase(0.0)
    , saxSubPhase(0.0)
    , rngState(22222)
{
}

void AttackGenerator::trigger(double pitch)
{
    currentPitch = std::clamp(pitch, 20.0, 20000.0);

    attackSamples = static_cast<size_t>(duration * sampleRate);
    if (attackSamples < 2) attackSamples = 2;

    sampleCount = 0;
    active = true;

    // Reset all DSP state
    onePoleState1 = 0.0;
    onePoleState2 = 0.0;
    biquadX1 = biquadX2 = 0.0;
    biquadY1 = biquadY2 = 0.0;
    bq2X1 = bq2X2 = 0.0;
    bq2Y1 = bq2Y2 = 0.0;
    fmPhase = 0.0;
    fmModPhase = 0.0;
    fluteWobblePhase = 0.0;
    brassBendPhase = 0.0;
    drumSweepPhase = 0.0;
    saxSubPhase = 0.0;

    // Seed RNG with pitch-dependent value for variety
    rngState = static_cast<unsigned int>(pitch * 1000.0) ^ 0xDEADBEEF;

    // Type-specific setup
    if (attackType == AttackType::BrassBuzz) {
        setupBiQuadBandpass(currentPitch, buzzQ);
    } else if (attackType == AttackType::ClarinetOnset) {
        // Bore resonance at ~3x fundamental (clarinet overblows at the 12th)
        setupBiQuad2Bandpass(currentPitch * 3.0, 3.0);
    } else if (attackType == AttackType::SaxophoneHonk) {
        // Bore resonance at fundamental for nasal character
        setupBiQuad2Bandpass(currentPitch, 6.0);
    } else if (attackType == AttackType::DrumHit) {
        // Body resonance - low frequency thump
        double bodyFreq = 80.0 + tone * 200.0;
        setupBiQuad2Bandpass(bodyFreq, 3.0);
    }
}

double AttackGenerator::generateSample(double inputSignal)
{
    if (!active || attackSamples == 0) {
        return inputSignal;
    }

    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);

    // Generate burst based on attack type
    double burst = 0.0;
    switch (attackType) {
        case AttackType::FluteChiff:     burst = generateFluteChiff(); break;
        case AttackType::ClarinetOnset:  burst = generateClarinetOnset(); break;
        case AttackType::SaxophoneHonk:  burst = generateSaxophoneHonk(); break;
        case AttackType::BrassBuzz:      burst = generateBrassBuzz(); break;
        case AttackType::PianoHammer:    burst = generatePianoHammer(); break;
        case AttackType::DrumHit:        burst = generateDrumHit(); break;
    }

    // Apply intensity
    burst *= intensity;

    // Blending: burst blends with input signal (same pattern as KarplusStrongAttack Attack mode)
    double burstWeight = (1.0 - progress) * mix;

    // Apply cosine fade in final portion for smooth transition
    if (progress > (1.0 - FADE_PORTION)) {
        double fadeProgress = (progress - (1.0 - FADE_PORTION)) / FADE_PORTION;
        double fadeFactor = 0.5 * (1.0 + std::cos(fadeProgress * PI));
        burstWeight *= fadeFactor;
    }

    double inputWeight = 1.0 - burstWeight;
    double output = burst * burstWeight + inputSignal * inputWeight;

    sampleCount++;
    if (sampleCount >= attackSamples) {
        active = false;
    }

    return output;
}

void AttackGenerator::reset()
{
    active = false;
    sampleCount = 0;
    onePoleState1 = 0.0;
    onePoleState2 = 0.0;
    biquadX1 = biquadX2 = 0.0;
    biquadY1 = biquadY2 = 0.0;
    bq2X1 = bq2X2 = 0.0;
    bq2Y1 = bq2Y2 = 0.0;
    fmPhase = 0.0;
    fmModPhase = 0.0;
    fluteWobblePhase = 0.0;
    brassBendPhase = 0.0;
    drumSweepPhase = 0.0;
    saxSubPhase = 0.0;
}

// ========== Internal DSP Helpers ==========

double AttackGenerator::nextRandom()
{
    // Linear congruential generator -> [-1, 1]
    rngState = rngState * 1664525u + 1013904223u;
    return static_cast<double>(static_cast<int>(rngState >> 1)) / 1073741824.0 - 1.0;
}

double AttackGenerator::filterOnePole1(double input, double coefficient)
{
    onePoleState1 += coefficient * (input - onePoleState1);
    return onePoleState1;
}

double AttackGenerator::filterOnePole2(double input, double coefficient)
{
    onePoleState2 += coefficient * (input - onePoleState2);
    return onePoleState2;
}

void AttackGenerator::setupBiQuadBandpass(double freq, double q)
{
    // Clamp frequency to Nyquist
    freq = std::clamp(freq, 20.0, sampleRate * 0.45);
    double w0 = TWO_PI * freq / sampleRate;
    double sinW0 = std::sin(w0);
    double cosW0 = std::cos(w0);
    double alpha = sinW0 / (2.0 * q);

    double a0Inv = 1.0 / (1.0 + alpha);
    biquadA0 = (sinW0 / 2.0) * a0Inv;
    biquadA1 = 0.0;
    biquadA2 = -(sinW0 / 2.0) * a0Inv;
    biquadB1 = (-2.0 * cosW0) * a0Inv;
    biquadB2 = (1.0 - alpha) * a0Inv;
}

double AttackGenerator::processBiQuad(double input)
{
    double output = biquadA0 * input + biquadA1 * biquadX1 + biquadA2 * biquadX2
                  - biquadB1 * biquadY1 - biquadB2 * biquadY2;

    biquadX2 = biquadX1;
    biquadX1 = input;
    biquadY2 = biquadY1;
    biquadY1 = output;

    return output;
}

void AttackGenerator::setupBiQuad2Bandpass(double freq, double q)
{
    freq = std::clamp(freq, 20.0, sampleRate * 0.45);
    double w0 = TWO_PI * freq / sampleRate;
    double sinW0 = std::sin(w0);
    double cosW0 = std::cos(w0);
    double alpha = sinW0 / (2.0 * q);

    double a0Inv = 1.0 / (1.0 + alpha);
    bq2A0 = (sinW0 / 2.0) * a0Inv;
    bq2A1 = 0.0;
    bq2A2 = -(sinW0 / 2.0) * a0Inv;
    bq2B1 = (-2.0 * cosW0) * a0Inv;
    bq2B2 = (1.0 - alpha) * a0Inv;
}

double AttackGenerator::processBiQuad2(double input)
{
    double output = bq2A0 * input + bq2A1 * bq2X1 + bq2A2 * bq2X2
                  - bq2B1 * bq2Y1 - bq2B2 * bq2Y2;

    bq2X2 = bq2X1;
    bq2X1 = input;
    bq2Y2 = bq2Y1;
    bq2Y1 = output;

    return output;
}

double AttackGenerator::computeADSR(double attackTime, double decayTime,
                                     double sustainLevel, double releaseTime)
{
    double timeInSeconds = static_cast<double>(sampleCount) / sampleRate;
    double totalTime = static_cast<double>(attackSamples) / sampleRate;
    double releaseStart = totalTime - releaseTime;
    if (releaseStart < attackTime + decayTime) {
        releaseStart = attackTime + decayTime;
    }

    if (timeInSeconds < attackTime) {
        // Attack phase
        return (attackTime > 0.0) ? timeInSeconds / attackTime : 1.0;
    } else if (timeInSeconds < attackTime + decayTime) {
        // Decay phase
        double decayProgress = (timeInSeconds - attackTime) / decayTime;
        return 1.0 - (1.0 - sustainLevel) * decayProgress;
    } else if (timeInSeconds < releaseStart) {
        // Sustain phase
        return sustainLevel;
    } else {
        // Release phase
        double releaseProgress = (timeInSeconds - releaseStart) / releaseTime;
        releaseProgress = std::clamp(releaseProgress, 0.0, 1.0);
        return sustainLevel * (1.0 - releaseProgress);
    }
}

// ========== Per-Type Generation ==========

double AttackGenerator::generateFluteChiff()
{
    // Flute chiff: breathy, airy, with pitch instability and band-limited noise
    // Key character: the jet stream is unstable at note onset, creating a breathy
    // "chh" sound with pitch wandering before the note stabilizes
    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);

    double envelope = computeADSR(duration * 0.1, duration * 0.25, 0.5, duration * 0.35);

    // Pitch instability: random walk that decays over time (flute-specific!)
    // The jet takes time to lock onto the resonance frequency
    fluteWobblePhase += (nextRandom() * 0.05) * (1.0 - progress);
    double pitchWobble = 1.0 + fluteWobblePhase * jetRatio * 0.1;

    // Band-limited noise in the "breathy" range (2-6 kHz)
    double noise = nextRandom() * noiseAmount;
    // High-pass: remove low frequencies for airy quality
    double hpCoeff = 1.0 - std::exp(-TWO_PI * 2000.0 / sampleRate);
    double hiPassed = noise - filterOnePole1(noise, hpCoeff);
    // Low-pass: remove harsh high frequencies
    double lpCoeff = 1.0 - std::exp(-TWO_PI * (4000.0 + jetRatio * 4000.0) / sampleRate);
    double bandNoise = filterOnePole2(hiPassed, lpCoeff);

    // Jet table nonlinearity (from STK JetTable: f(x) = x^3 - x)
    // Models the turbulent jet deflection at the edge of the embouchure
    double jetInput = bandNoise * (0.5 + jetRatio) * pitchWobble;
    double jetOutput = jetInput * jetInput * jetInput - jetInput;

    // Mix in a faint pitched component that fades in (the note "finding" its pitch)
    double t = static_cast<double>(sampleCount) / sampleRate;
    double pitchedComponent = std::sin(TWO_PI * currentPitch * pitchWobble * t) * progress * 0.2;

    double result = (jetOutput * (1.0 - progress * 0.5) + pitchedComponent) * envelope;
    return std::clamp(result, -1.0, 1.0) * TARGET_PEAK;
}

double AttackGenerator::generateClarinetOnset()
{
    // Clarinet onset: starts with an overblown "squeak" (high register chirp)
    // then quickly settles into the reed buzz. Very distinctive.
    // Key character: brief high-pitched squeal from overblowing, then woody buzz
    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);
    double t = static_cast<double>(sampleCount) / sampleRate;

    // Two-phase envelope: fast squeak, then slower reed onset
    double squeakDuration = duration * 0.15;  // First 15% is the squeak
    double squeak = 0.0;
    double reed = 0.0;

    if (t < squeakDuration) {
        // Phase 1: Overblown squeak (3rd harmonic / 12th)
        // Clarinet overblows at the twelfth (3x fundamental)
        double squeakProgress = t / squeakDuration;
        double squeakEnv = std::sin(PI * squeakProgress) * 0.35;  // Reduced amplitude
        double squeakPitch = currentPitch * 3.0 * (1.0 + (1.0 - squeakProgress) * reedStiffness * 0.15);
        squeak = std::sin(TWO_PI * squeakPitch * t) * squeakEnv;

        // Add some reed buzz to the squeak
        double noise = nextRandom() * 0.2;
        squeak += noise * squeakEnv * 0.3;

        // Through the bore resonance filter
        squeak = processBiQuad2(squeak);
    }

    // Phase 2: Reed table onset (ramps up as squeak fades)
    double reedOnset = std::clamp((t - squeakDuration * 0.5) / (duration * 0.3), 0.0, 1.0);
    double reedDecay = std::clamp(1.0 - (progress - 0.5) / 0.5, 0.0, 1.0);
    double reedEnv = reedOnset * reedDecay;

    // Reed table nonlinearity with more aggressive clipping for woody buzz
    double noise = nextRandom();
    double reedOffset = reedAperture * 0.4 + 0.4;
    double reedSlope = reedStiffness * -1.2 - 0.4;  // More aggressive than before
    double reedInput = noise * reedEnv;
    double reedOut = reedOffset + reedSlope * reedInput;

    // Harder clipping for the characteristic clarinet "edge"
    if (reedOut > 0.8) reedOut = 0.8 + (reedOut - 0.8) * 0.1;
    if (reedOut < -0.8) reedOut = -0.8 + (reedOut + 0.8) * 0.1;
    reedOut -= reedOffset;

    // Through bore resonance to add body
    reed = processBiQuad2(reedOut) * reedEnv;

    double result = squeak * 0.3 + reed * 0.8;
    return std::clamp(result, -1.0, 1.0) * TARGET_PEAK;
}

double AttackGenerator::generateSaxophoneHonk()
{
    // Saxophone honk: nasal, brassy, with sub-harmonic emphasis and pressure wobble
    // Key character: the "wah" quality from the conical bore, sub-harmonics from
    // the mouthpiece, and a characteristic pressure "wobble" at onset
    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);
    double t = static_cast<double>(sampleCount) / sampleRate;

    // Slow-building envelope with characteristic "swell"
    double envAttack = std::clamp(t / (duration * 0.3), 0.0, 1.0);
    // Apply exponential curve for the characteristic sax "bloom"
    envAttack = envAttack * envAttack;
    double envDecay = std::clamp(1.0 - (progress - 0.6) / 0.4, 0.0, 1.0);
    double envelope = envAttack * envDecay;

    // Pressure wobble: low-frequency oscillation in blow pressure (2-5 Hz)
    // This is the embouchure instability characteristic of sax onset
    double wobbleFreq = 3.0 + blowPosition * 4.0;
    double wobble = std::sin(TWO_PI * wobbleFreq * t) * 0.3 * (1.0 - progress);

    // Reed table with wobble modulation
    double noise = nextRandom();
    double reedOffset = reedAperture * 0.4 + 0.4;
    double reedSlope = reedStiffness * -0.8 - 0.3;
    double reedInput = noise * envelope * (1.0 + wobble);
    double reedOut = reedOffset + reedSlope * reedInput;
    reedOut = std::clamp(reedOut, -1.0, 1.0);
    reedOut -= reedOffset;

    // Bore resonance at fundamental for nasal character
    double boreOut = processBiQuad2(reedOut);

    // Sub-harmonic component (half the fundamental) - characteristic of sax's
    // conical bore exciting sub-harmonics through the mouthpiece
    saxSubPhase += TWO_PI * (currentPitch * 0.5) / sampleRate;
    if (saxSubPhase > TWO_PI) saxSubPhase -= TWO_PI;
    double subHarmonic = std::sin(saxSubPhase) * envelope * blowPosition * 0.4;

    // Low-pass for warmth (sax is warmer than clarinet)
    double lpCoeff = 1.0 - std::exp(-TWO_PI * (1500.0 + blowPosition * 2000.0) / sampleRate);
    double warmOut = filterOnePole1(boreOut, lpCoeff);

    double result = warmOut * 0.7 + subHarmonic + reedOut * 0.2;
    return std::clamp(result, -1.0, 1.0) * TARGET_PEAK;
}

double AttackGenerator::generateBrassBuzz()
{
    // Brass buzz: starts below pitch and "lips up" to the note, with
    // a characteristic spit-noise burst at the very beginning
    // Key character: pitch bend upward, buzzy lip vibration, initial "spit"
    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);
    double t = static_cast<double>(sampleCount) / sampleRate;

    double envelope = computeADSR(duration * 0.15, duration * 0.2, 0.65, duration * 0.3);

    // Phase 1: Initial "spit" burst (first ~5% of duration)
    double spitDuration = duration * 0.05;
    double spit = 0.0;
    if (t < spitDuration) {
        double spitEnv = 1.0 - (t / spitDuration);
        spit = nextRandom() * spitEnv * spitEnv * 0.8;  // Fast exponential noise burst
    }

    // Pitch bend: brass instruments "lip up" to the note
    // Start ~semitone below and bend up, speed controlled by lipTension
    double bendSpeed = 2.0 + lipTension * 15.0;  // Higher tension = faster arrival
    double bendAmount = std::exp(-bendSpeed * progress);
    double bendSemitones = -1.5 * bendAmount;  // Start 1.5 semitones flat
    double effectivePitch = currentPitch * std::pow(2.0, bendSemitones / 12.0);

    // Update lip resonance filter at bent pitch
    double lipFreq = effectivePitch * (0.8 + lipTension * 0.4);
    setupBiQuadBandpass(lipFreq, buzzQ);

    // Generate excitation noise
    double noise = nextRandom();
    double excitation = noise * envelope;

    // Through lip resonance
    double resonated = processBiQuad(excitation);

    // Lip area mapping: asymmetric clipping (from STK brass)
    // This creates the distinctive "blatty" brass harmonics
    double buzzed = resonated * std::abs(resonated);  // Squaring for odd harmonics

    // Add a faint pitched oscillation that strengthens over time
    // (the lip buzz locking onto the resonant frequency)
    brassBendPhase += TWO_PI * effectivePitch / sampleRate;
    if (brassBendPhase > TWO_PI) brassBendPhase -= TWO_PI;
    double pitched = std::sin(brassBendPhase) * progress * 0.3;

    double result = (spit + buzzed * 0.7 + pitched) * envelope;
    return std::clamp(result, -1.0, 1.0) * TARGET_PEAK;
}

double AttackGenerator::generatePianoHammer()
{
    // Piano hammer: FM burst with fast ADSR (inspired by STK Rhodey)
    // Already sounds distinctive - keep the same algorithm with minor refinements
    double envelope = computeADSR(0.001, duration * 0.3, 0.0, duration * 0.5);

    // FM synthesis
    double carrierRatio = 1.0 + hardness * 14.0;  // 1x to 15x fundamental
    double modIndex = brightness * 5.0;            // Modulation depth

    // Modulation index decays with envelope for natural hammer behavior
    // (brighter at impact, duller as it decays)
    double dynamicModIndex = modIndex * (0.3 + 0.7 * envelope);

    // Modulator
    double modFreq = currentPitch * carrierRatio * 0.5;
    fmModPhase += TWO_PI * modFreq / sampleRate;
    if (fmModPhase > TWO_PI) fmModPhase -= TWO_PI;
    double modulator = std::sin(fmModPhase) * dynamicModIndex;

    // Carrier
    double carrierFreq = currentPitch * carrierRatio;
    fmPhase += TWO_PI * (carrierFreq + modulator * carrierFreq) / sampleRate;
    if (fmPhase > TWO_PI) fmPhase -= TWO_PI;
    double carrier = std::sin(fmPhase);

    // Add a subtle noise component for the "thunk" of hammer on string
    double thunkEnv = std::exp(-50.0 * static_cast<double>(sampleCount) / sampleRate);
    double thunk = nextRandom() * thunkEnv * hardness * 0.3;

    return std::clamp((carrier + thunk) * envelope, -1.0, 1.0) * TARGET_PEAK;
}

double AttackGenerator::generateDrumHit()
{
    // Drum hit: membrane-like behavior with downward pitch sweep + body resonance
    // Key character: the pitch drops rapidly after impact (membrane physics),
    // plus a resonant body thump at a low frequency
    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);
    double t = static_cast<double>(sampleCount) / sampleRate;

    // Two-component sound: membrane (pitched, sweeping down) + noise (snap/rattle)

    // Component 1: Membrane with downward pitch sweep
    // Real drum membranes start at a high frequency and sweep down to the resonant pitch
    double sweepRate = 5.0 + tightness * 40.0;  // Tighter = faster sweep
    double startPitch = 200.0 + tone * 600.0;    // Higher tone = higher start
    double endPitch = 60.0 + tone * 100.0;       // Rest frequency
    double sweepPitch = endPitch + (startPitch - endPitch) * std::exp(-sweepRate * t);

    drumSweepPhase += TWO_PI * sweepPitch / sampleRate;
    if (drumSweepPhase > TWO_PI) drumSweepPhase -= TWO_PI;
    double membrane = std::sin(drumSweepPhase);

    // Membrane decays exponentially
    double membraneDecay = 8.0 + tightness * 25.0;
    double membraneEnv = std::exp(-membraneDecay * t);
    membrane *= membraneEnv;

    // Component 2: Noise burst for the "snap" of the stick hit
    double noiseDecay = 15.0 + tightness * 40.0;
    double noiseEnv = std::exp(-noiseDecay * t);
    double noise = nextRandom() * noiseEnv;

    // Filter noise based on tone
    double cutoffHz = 500.0 + tone * 8000.0;
    double coeff = 1.0 - std::exp(-TWO_PI * cutoffHz / sampleRate);
    double filteredNoise = filterOnePole1(noise, coeff);

    // Component 3: Body resonance thump (low frequency boom)
    double bodyResonance = processBiQuad2(membrane + filteredNoise * 0.3);
    double bodyEnv = std::exp(-(4.0 + tightness * 10.0) * t);

    // Mix: membrane + noise snap + body thump
    double result = membrane * 0.5 + filteredNoise * 0.4 + bodyResonance * bodyEnv * 0.4;
    return std::clamp(result, -1.0, 1.0) * TARGET_PEAK;
}

// ========== Parameter Setters ==========

void AttackGenerator::setAttackType(int type)
{
    if (type >= 0 && type <= 5) {
        attackType = static_cast<AttackType>(type);
    }
}

void AttackGenerator::setDuration(double d)
{
    duration = std::clamp(d, 0.005, 5.0);
}

void AttackGenerator::setIntensity(double i)
{
    intensity = std::clamp(i, 0.0, 1.0);
}

void AttackGenerator::setMix(double m)
{
    mix = std::clamp(m, 0.0, 1.0);
}

void AttackGenerator::setSampleRate(double rate)
{
    if (rate > 0.0) {
        sampleRate = rate;
    }
}

void AttackGenerator::setNoiseAmount(double n)
{
    noiseAmount = std::clamp(n, 0.0, 1.0);
}

void AttackGenerator::setJetRatio(double j)
{
    jetRatio = std::clamp(j, 0.0, 1.0);
}

void AttackGenerator::setReedStiffness(double s)
{
    reedStiffness = std::clamp(s, 0.0, 1.0);
}

void AttackGenerator::setReedAperture(double a)
{
    reedAperture = std::clamp(a, 0.0, 1.0);
}

void AttackGenerator::setBlowPosition(double p)
{
    blowPosition = std::clamp(p, 0.0, 1.0);
}

void AttackGenerator::setLipTension(double t)
{
    lipTension = std::clamp(t, 0.0, 1.0);
}

void AttackGenerator::setBuzzQ(double q)
{
    buzzQ = std::clamp(q, 0.5, 50.0);
}

void AttackGenerator::setHardness(double h)
{
    hardness = std::clamp(h, 0.0, 1.0);
}

void AttackGenerator::setBrightness(double b)
{
    brightness = std::clamp(b, 0.0, 1.0);
}

void AttackGenerator::setTightness(double t)
{
    tightness = std::clamp(t, 0.0, 1.0);
}

void AttackGenerator::setTone(double t)
{
    tone = std::clamp(t, 0.0, 1.0);
}
