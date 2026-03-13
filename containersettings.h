#ifndef CONTAINERSETTINGS_H
#define CONTAINERSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QJsonObject>

/**
 * ContainerSettings - Global settings for container parameter limits
 *
 * This class stores configurable min/max values for all container sliders
 * and internal clamps. It replaces hard-coded values throughout the codebase.
 *
 * The settings are persisted using QSettings and can be modified via the
 * Settings tab in the application.
 *
 * Usage:
 *   ContainerSettings::instance().harmonicGenerator.maxHarmonics
 */
class ContainerSettings : public QObject
{
    Q_OBJECT

public:
    static ContainerSettings& instance();

    // Prevent copying
    ContainerSettings(const ContainerSettings&) = delete;
    ContainerSettings& operator=(const ContainerSettings&) = delete;

    // Save/Load settings
    void saveSettings();
    void loadSettings();
    void resetToDefaults();

    // JSON serialization for project files
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

    // ========== Harmonic Generator Settings ==========
    struct HarmonicGeneratorSettings {
        int maxHarmonics = 128;               // Max value for numHarmonics slider
        double fundamentalHzMin = 20.0;       // Min fundamental frequency
        double fundamentalHzMax = 8000.0;     // Max fundamental frequency
        double rolloffPowerMin = 0.2;         // Min rolloff power
        double rolloffPowerMax = 5.0;         // Max rolloff power
        double driftMax = 0.1;                // Max drift amount
        // PADsynth
        double padBandwidthMin = 1.0;         // Min bandwidth (cents)
        double padBandwidthMax = 200.0;       // Max bandwidth (cents)
        double padBandwidthScaleMin = 0.0;    // Min bandwidth scale
        double padBandwidthScaleMax = 2.0;    // Max bandwidth scale
    } harmonicGenerator;

    // ========== Rolloff Processor Settings ==========
    struct RolloffProcessorSettings {
        double lowRolloffMin = 0.0;
        double lowRolloffMax = 2.0;
        double highRolloffMin = 0.0;
        double highRolloffMax = 3.0;
        double crossoverMin = 1.0;
        double crossoverMax = 32.0;
        double transitionMin = 0.0;
        double transitionMax = 16.0;
    } rolloffProcessor;

    // ========== Spectrum to Signal Settings ==========
    struct SpectrumToSignalSettings {
        double normalizeMin = 0.0;
        double normalizeMax = 1.0;
    } spectrumToSignal;

    // ========== Formant Body Settings ==========
    struct FormantBodySettings {
        double f1FreqMin = 200.0;
        double f1FreqMax = 1000.0;
        double f2FreqMin = 800.0;
        double f2FreqMax = 3000.0;
        double qMin = 1.0;
        double qMax = 20.0;
        double mixMin = 0.0;
        double mixMax = 1.0;
    } formantBody;

    // ========== Breath Turbulence Settings ==========
    struct BreathTurbulenceSettings {
        double blendMin = 0.0;
        double blendMax = 1.0;
    } breathTurbulence;

    // ========== Noise Color Filter Settings ==========
    struct NoiseColorFilterSettings {
        double colorMin = 100.0;
        double colorMax = 8000.0;
        double filterQMin = 0.5;
        double filterQMax = 10.0;
    } noiseColorFilter;

    // ========== Physics System Settings ==========
    struct PhysicsSystemSettings {
        double massMin = 0.0;
        double massMax = 10.0;
        double springKMin = 0.0001;
        double springKMax = 1.0;
        double dampingMin = 0.5;
        double dampingMax = 0.9999;
        double impulseMin = 0.0;
        double impulseMax = 1000.0;
    } physicsSystem;

    // ========== Drift Engine Settings ==========
    struct DriftEngineSettings {
        double amountMin = 0.0;
        double amountMax = 0.1;
        double rateMin = 0.01;
        double rateMax = 10.0;
    } driftEngine;

    // ========== Gate Processor Settings ==========
    struct GateProcessorSettings {
        double velocityMin = 0.0;
        double velocityMax = 1.0;
        double attackTimeMin = 0.001;
        double attackTimeMax = 1.0;
        double releaseTimeMin = 0.001;
        double releaseTimeMax = 2.0;
        double velocitySensMin = 0.0;
        double velocitySensMax = 1.0;
    } gateProcessor;

    // ========== Karplus Strong Settings ==========
    struct KarplusStrongSettings {
        double attackPortionMin = 0.01;
        double attackPortionMax = 1.0;
        double dampingMin = 0.0;
        double dampingMax = 1.0;
        double pluckPositionMin = 0.0;
        double pluckPositionMax = 1.0;
        double mixMin = 0.0;
        double mixMax = 1.0;
        double brightnessMin = 0.0;
        double brightnessMax = 1.0;
        double excitationSoftnessMin = 0.0;
        double excitationSoftnessMax = 1.0;
        double pluckHardnessMin = 0.0;
        double pluckHardnessMax = 1.0;
        double bodyResonanceMin = 0.0;
        double bodyResonanceMax = 1.0;
        double bodyFreqMin = 80.0;
        double bodyFreqMax = 400.0;
        double pickDirectionMin = 0.0;
        double pickDirectionMax = 1.0;
    } karplusStrong;

    // ========== Attack Settings ==========
    struct AttackSettings {
        double durationMin = 0.005;
        double durationMax = 5.0;
        double intensityMin = 0.0;
        double intensityMax = 1.0;
        double mixMin = 0.0;
        double mixMax = 1.0;
        // Air Jet
        double noiseAmountMin = 0.0;
        double noiseAmountMax = 1.0;
        double jetRatioMin = 0.0;
        double jetRatioMax = 1.0;
        // Reed (Clarinet/Saxophone)
        double reedStiffnessMin = 0.0;
        double reedStiffnessMax = 1.0;
        double reedApertureMin = 0.0;
        double reedApertureMax = 1.0;
        double blowPositionMin = 0.0;
        double blowPositionMax = 1.0;
        // Brass
        double lipTensionMin = 0.0;
        double lipTensionMax = 1.0;
        double buzzQMin = 0.5;
        double buzzQMax = 50.0;
        // FM Strike
        double hardnessMin = 0.0;
        double hardnessMax = 1.0;
        double brightnessMin = 0.0;
        double brightnessMax = 1.0;
        // Membrane Sweep
        double tightnessMin = 0.0;
        double tightnessMax = 1.0;
        double toneMin = 0.0;
        double toneMax = 1.0;
    } attack;

    // ========== LFO Settings ==========
    struct LFOSettings {
        double frequencyMin = 0.01;
        double frequencyMax = 20.0;
        double amplitudeMin = 0.0;
        double amplitudeMax = 1.0;
    } lfo;

    // ========== Wavetable Synth Settings ==========
    struct WavetableSynthSettings {
        double positionMin = 0.0;
        double positionMax = 1.0;
    } wavetableSynth;

    // ========== Frequency Mapper Settings ==========
    struct FrequencyMapperSettings {
        double lowFreqMin = 1.0;
        double lowFreqMax = 500.0;
        double highFreqMin = 500.0;
        double highFreqMax = 20000.0;
        double outputMinMin = -2.0;   // output floor slider lower bound
        double outputMinMax = 1.0;    // output floor slider upper bound
        double outputMaxMin = 0.0;    // output ceiling slider lower bound
        double outputMaxMax = 2.0;    // output ceiling slider upper bound
    } frequencyMapper;

    // ========== Bandpass EQ Settings ==========
    struct BandpassEQSettings {
        double gainMin = 0.0;
        double gainMax = 2.0;    // 0 = silent, 1 = unity, 2 = boost
        double qMin = 0.5;
        double qMax = 20.0;
    } bandpassEQ;

    // ========== Comb Filter Settings ==========
    struct CombFilterSettings {
        double delayTimeMin = 0.1;
        double delayTimeMax = 100.0;
        double feedbackMin = -0.99;
        double feedbackMax = 0.99;
        double dampingMin = 0.0;
        double dampingMax = 1.0;
    } combFilter;

    // ========== LP/HP Filter Settings ==========
    struct LowHighPassFilterSettings {
        double cutoffMin = 20.0;
        double cutoffMax = 20000.0;
        double resonanceMin = 0.5;
        double resonanceMax = 20.0;
    } lowHighPassFilter;

    // ========== IR Convolution Settings ==========
    struct IRConvolutionSettings {
        double wetDryMin = 0.0;
        double wetDryMax = 1.0;
        double predelayMin = 0.0;
        double predelayMax = 500.0;   // ms
        double decayMin = 0.0;
        double decayMax = 1.0;
        double highDampMin = 200.0;
        double highDampMax = 20000.0; // Hz
        double fadeInPctMin = 0.0;
        double fadeInPctMax = 100.0;  // % of pre-delay
        double lowCutMin = 20.0;
        double lowCutMax = 2000.0;    // Hz
        double captureLengthMin = 0.1;
        double captureLengthMax = 20.0;  // seconds
    } irConvolution;

    // ========== Recorder Settings ==========
    struct RecorderSettings {
        double breathPressureMin = 0.0;
        double breathPressureMax = 1.0;
        double jetRatioMin       = 0.05;
        double jetRatioMax       = 0.60;
        double noiseGainMin      = 0.0;
        double noiseGainMax      = 0.5;
        double vibratoFreqMin    = 0.0;
        double vibratoFreqMax    = 20.0;
        double vibratoGainMin    = 0.0;
        double vibratoGainMax    = 1.0;
        double endReflectionMin  = -1.0;
        double endReflectionMax  =  1.0;
        double jetReflectionMin  = -1.0;
        double jetReflectionMax  =  1.0;
    } recorder;

    // ========== Bowed Settings ==========
    struct BowedSettings {
        double bowPressureMin = 0.0;
        double bowPressureMax = 1.0;
        double bowVelocityMin = 0.0;
        double bowVelocityMax = 1.0;
        double bowPositionMin = 0.0;
        double bowPositionMax = 1.0;
        // Nonlinear waveguide (Michon)
        double nlTypeMin     = 0.0;
        double nlTypeMax     = 4.0;
        double nlAmountMin   = 0.0;
        double nlAmountMax   = 1.0;
        double nlFreqModMin  = 0.0;
        double nlFreqModMax  = 200.0;
        double nlAttackMin   = 0.001;
        double nlAttackMax   = 2.0;
    } bowed;

    // ========== Reed Settings ==========
    struct ReedSettings {
        double breathPressureMin = 0.0;
        double breathPressureMax = 1.0;
        double reedStiffnessMin  = 0.0;
        double reedStiffnessMax  = 1.0;
        double reedApertureMin   = 0.0;
        double reedApertureMax   = 1.0;
        double blowPositionMin   = 0.0;
        double blowPositionMax   = 1.0;
        double noiseGainMin      = 0.0;
        double noiseGainMax      = 0.4;
        double vibratoFreqMin    = 0.0;
        double vibratoFreqMax    = 12.0;
        double vibratoGainMin    = 0.0;
        double vibratoGainMax    = 0.5;
        // Nonlinear allpass (Michon)
        double nlTypeMin         = 0.0;
        double nlTypeMax         = 4.0;
        double nlAmountMin       = 0.0;
        double nlAmountMax       = 1.0;
        double nlFreqModMin      = 0.0;
        double nlFreqModMax      = 200.0;
        double nlAttackMin       = 0.001;
        double nlAttackMax       = 2.0;
    } reed;

    // ========== Easing Settings ==========
    // Default parameter values for easing functions (used by both Sound Engine and Score)
    struct EasingSettings {
        // Back easing: overshoot amount (how far past target)
        double overshootDefault = 0.5;    // Lower for subtle rhythmic effects (math default: 1.70158)
        double overshootMin = 0.0;
        double overshootMax = 2.0;

        // Elastic easing: spring behavior
        double amplitudeDefault = 0.5;    // Lower for subtle bounce (math default: 1.0)
        double amplitudeMin = 0.1;
        double amplitudeMax = 2.0;
        double periodDefault = 0.4;       // Slightly longer period (math default: 0.3)
        double periodMin = 0.1;
        double periodMax = 1.0;

        // Wobble easing: decaying oscillation
        double frequencyDefault = 2.0;    // Lower frequency (math default: 3.0)
        double frequencyMin = 0.5;
        double frequencyMax = 5.0;
        double decayDefault = 0.7;        // Slightly faster decay (math default: 0.8)
        double decayMin = 0.1;
        double decayMax = 1.0;
    } easing;

signals:
    void settingsChanged();

private:
    ContainerSettings();
    ~ContainerSettings() = default;
};

#endif // CONTAINERSETTINGS_H
