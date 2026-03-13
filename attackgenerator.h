#ifndef ATTACKGENERATOR_H
#define ATTACKGENERATOR_H

#include <cstddef>

/**
 * AttackType - The six STK-inspired excitation types
 */
enum class AttackType {
    FluteChiff = 0,
    ClarinetOnset = 1,
    SaxophoneHonk = 2,
    BrassBuzz = 3,
    PianoHammer = 4,
    DrumHit = 5
};

/**
 * AttackGenerator - Generates STK-inspired attack transients
 *
 * Produces shaped excitation bursts that blend with an input signal
 * during the attack portion of a note. Each attack type implements
 * a different DSP algorithm derived from STK source concepts.
 *
 * API follows the same pattern as KarplusStrongAttack:
 *   trigger(pitch) -> generateSample(inputSignal) -> reset() -> isActive()
 */
class AttackGenerator
{
public:
    AttackGenerator(double sampleRate = 44100.0);

    void trigger(double pitch);
    double generateSample(double inputSignal = 0.0);
    void reset();
    bool isActive() const { return active; }

    // Common parameter setters
    void setAttackType(AttackType type) { attackType = type; }
    void setAttackType(int type);
    void setDuration(double d);
    void setIntensity(double i);
    void setMix(double m);
    void setSampleRate(double rate);

    // Flute parameters
    void setNoiseAmount(double n);
    void setJetRatio(double j);

    // Clarinet/Saxophone parameters
    void setReedStiffness(double s);
    void setReedAperture(double a);

    // Saxophone only
    void setBlowPosition(double p);

    // Brass parameters
    void setLipTension(double t);
    void setBuzzQ(double q);

    // Piano parameters
    void setHardness(double h);
    void setBrightness(double b);

    // Drum parameters
    void setTightness(double t);
    void setTone(double t);

    // Getters
    AttackType getAttackType() const { return attackType; }
    double getDuration() const { return duration; }
    double getIntensity() const { return intensity; }
    double getMix() const { return mix; }

private:
    double sampleRate;

    // Common parameters
    AttackType attackType;
    double duration;
    double intensity;
    double mix;

    // Flute parameters
    double noiseAmount;
    double jetRatio;

    // Clarinet/Saxophone parameters
    double reedStiffness;
    double reedAperture;

    // Saxophone only
    double blowPosition;

    // Brass parameters
    double lipTension;
    double buzzQ;

    // Piano parameters
    double hardness;
    double brightness;

    // Drum parameters
    double tightness;
    double tone;

    // State
    bool active;
    size_t sampleCount;
    size_t attackSamples;
    double currentPitch;

    // OnePole filter states (two independent filters)
    double onePoleState1;
    double onePoleState2;

    // BiQuad filter state (for brass lip resonance)
    double biquadX1, biquadX2;
    double biquadY1, biquadY2;
    double biquadA0, biquadA1, biquadA2;
    double biquadB1, biquadB2;

    // Second BiQuad (for clarinet bore / sax bore / drum body)
    double bq2X1, bq2X2;
    double bq2Y1, bq2Y2;
    double bq2A0, bq2A1, bq2A2;
    double bq2B1, bq2B2;

    // FM synthesis state (for piano)
    double fmPhase;
    double fmModPhase;

    // Flute pitch wobble phase
    double fluteWobblePhase;

    // Brass pitch bend state
    double brassBendPhase;

    // Drum pitch sweep phase
    double drumSweepPhase;

    // Sax sub-harmonic oscillator phase
    double saxSubPhase;

    // RNG state (simple linear congruential)
    unsigned int rngState;

    // Internal DSP helpers
    double nextRandom();
    double filterOnePole1(double input, double coefficient);
    double filterOnePole2(double input, double coefficient);
    void setupBiQuadBandpass(double freq, double q);
    double processBiQuad(double input);
    void setupBiQuad2Bandpass(double freq, double q);
    double processBiQuad2(double input);
    double computeADSR(double attackTime, double decayTime, double sustainLevel, double releaseTime);

    // Per-type generate functions
    double generateFluteChiff();
    double generateClarinetOnset();
    double generateSaxophoneHonk();
    double generateBrassBuzz();
    double generatePianoHammer();
    double generateDrumHit();
};

#endif // ATTACKGENERATOR_H
