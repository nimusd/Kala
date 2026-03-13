#include "containersettings.h"
#include <QDebug>
#include <QJsonObject>

ContainerSettings::ContainerSettings()
    : QObject(nullptr)
{
    loadSettings();
}

ContainerSettings& ContainerSettings::instance()
{
    static ContainerSettings instance;
    return instance;
}

void ContainerSettings::saveSettings()
{
    QSettings settings("Kala", "ContainerSettings");

    // Harmonic Generator
    settings.beginGroup("HarmonicGenerator");
    settings.setValue("maxHarmonics", harmonicGenerator.maxHarmonics);
    settings.setValue("fundamentalHzMin", harmonicGenerator.fundamentalHzMin);
    settings.setValue("fundamentalHzMax", harmonicGenerator.fundamentalHzMax);
    settings.setValue("rolloffPowerMin", harmonicGenerator.rolloffPowerMin);
    settings.setValue("rolloffPowerMax", harmonicGenerator.rolloffPowerMax);
    settings.setValue("driftMax", harmonicGenerator.driftMax);
    settings.setValue("padBandwidthMin", harmonicGenerator.padBandwidthMin);
    settings.setValue("padBandwidthMax", harmonicGenerator.padBandwidthMax);
    settings.setValue("padBandwidthScaleMin", harmonicGenerator.padBandwidthScaleMin);
    settings.setValue("padBandwidthScaleMax", harmonicGenerator.padBandwidthScaleMax);
    settings.endGroup();

    // Rolloff Processor
    settings.beginGroup("RolloffProcessor");
    settings.setValue("lowRolloffMin", rolloffProcessor.lowRolloffMin);
    settings.setValue("lowRolloffMax", rolloffProcessor.lowRolloffMax);
    settings.setValue("highRolloffMin", rolloffProcessor.highRolloffMin);
    settings.setValue("highRolloffMax", rolloffProcessor.highRolloffMax);
    settings.setValue("crossoverMin", rolloffProcessor.crossoverMin);
    settings.setValue("crossoverMax", rolloffProcessor.crossoverMax);
    settings.setValue("transitionMin", rolloffProcessor.transitionMin);
    settings.setValue("transitionMax", rolloffProcessor.transitionMax);
    settings.endGroup();

    // Spectrum to Signal
    settings.beginGroup("SpectrumToSignal");
    settings.setValue("normalizeMin", spectrumToSignal.normalizeMin);
    settings.setValue("normalizeMax", spectrumToSignal.normalizeMax);
    settings.endGroup();

    // Formant Body
    settings.beginGroup("FormantBody");
    settings.setValue("f1FreqMin", formantBody.f1FreqMin);
    settings.setValue("f1FreqMax", formantBody.f1FreqMax);
    settings.setValue("f2FreqMin", formantBody.f2FreqMin);
    settings.setValue("f2FreqMax", formantBody.f2FreqMax);
    settings.setValue("qMin", formantBody.qMin);
    settings.setValue("qMax", formantBody.qMax);
    settings.setValue("mixMin", formantBody.mixMin);
    settings.setValue("mixMax", formantBody.mixMax);
    settings.endGroup();

    // Breath Turbulence
    settings.beginGroup("BreathTurbulence");
    settings.setValue("blendMin", breathTurbulence.blendMin);
    settings.setValue("blendMax", breathTurbulence.blendMax);
    settings.endGroup();

    // Noise Color Filter
    settings.beginGroup("NoiseColorFilter");
    settings.setValue("colorMin", noiseColorFilter.colorMin);
    settings.setValue("colorMax", noiseColorFilter.colorMax);
    settings.setValue("filterQMin", noiseColorFilter.filterQMin);
    settings.setValue("filterQMax", noiseColorFilter.filterQMax);
    settings.endGroup();

    // Physics System
    settings.beginGroup("PhysicsSystem");
    settings.setValue("massMin", physicsSystem.massMin);
    settings.setValue("massMax", physicsSystem.massMax);
    settings.setValue("springKMin", physicsSystem.springKMin);
    settings.setValue("springKMax", physicsSystem.springKMax);
    settings.setValue("dampingMin", physicsSystem.dampingMin);
    settings.setValue("dampingMax", physicsSystem.dampingMax);
    settings.setValue("impulseMin", physicsSystem.impulseMin);
    settings.setValue("impulseMax", physicsSystem.impulseMax);
    settings.endGroup();

    // Drift Engine
    settings.beginGroup("DriftEngine");
    settings.setValue("amountMin", driftEngine.amountMin);
    settings.setValue("amountMax", driftEngine.amountMax);
    settings.setValue("rateMin", driftEngine.rateMin);
    settings.setValue("rateMax", driftEngine.rateMax);
    settings.endGroup();

    // Gate Processor
    settings.beginGroup("GateProcessor");
    settings.setValue("velocityMin", gateProcessor.velocityMin);
    settings.setValue("velocityMax", gateProcessor.velocityMax);
    settings.setValue("attackTimeMin", gateProcessor.attackTimeMin);
    settings.setValue("attackTimeMax", gateProcessor.attackTimeMax);
    settings.setValue("releaseTimeMin", gateProcessor.releaseTimeMin);
    settings.setValue("releaseTimeMax", gateProcessor.releaseTimeMax);
    settings.setValue("velocitySensMin", gateProcessor.velocitySensMin);
    settings.setValue("velocitySensMax", gateProcessor.velocitySensMax);
    settings.endGroup();

    // Karplus Strong
    settings.beginGroup("KarplusStrong");
    settings.setValue("attackPortionMin", karplusStrong.attackPortionMin);
    settings.setValue("attackPortionMax", karplusStrong.attackPortionMax);
    settings.setValue("dampingMin", karplusStrong.dampingMin);
    settings.setValue("dampingMax", karplusStrong.dampingMax);
    settings.setValue("pluckPositionMin", karplusStrong.pluckPositionMin);
    settings.setValue("pluckPositionMax", karplusStrong.pluckPositionMax);
    settings.setValue("mixMin", karplusStrong.mixMin);
    settings.setValue("mixMax", karplusStrong.mixMax);
    settings.setValue("brightnessMin", karplusStrong.brightnessMin);
    settings.setValue("brightnessMax", karplusStrong.brightnessMax);
    settings.setValue("excitationSoftnessMin", karplusStrong.excitationSoftnessMin);
    settings.setValue("excitationSoftnessMax", karplusStrong.excitationSoftnessMax);
    settings.setValue("pluckHardnessMin", karplusStrong.pluckHardnessMin);
    settings.setValue("pluckHardnessMax", karplusStrong.pluckHardnessMax);
    settings.setValue("bodyResonanceMin", karplusStrong.bodyResonanceMin);
    settings.setValue("bodyResonanceMax", karplusStrong.bodyResonanceMax);
    settings.setValue("bodyFreqMin", karplusStrong.bodyFreqMin);
    settings.setValue("bodyFreqMax", karplusStrong.bodyFreqMax);
    settings.setValue("pickDirectionMin", karplusStrong.pickDirectionMin);
    settings.setValue("pickDirectionMax", karplusStrong.pickDirectionMax);
    settings.endGroup();

    // Attack
    settings.beginGroup("Attack");
    settings.setValue("durationMin", attack.durationMin);
    settings.setValue("durationMax", attack.durationMax);
    settings.setValue("intensityMin", attack.intensityMin);
    settings.setValue("intensityMax", attack.intensityMax);
    settings.setValue("mixMin", attack.mixMin);
    settings.setValue("mixMax", attack.mixMax);
    settings.setValue("noiseAmountMin", attack.noiseAmountMin);
    settings.setValue("noiseAmountMax", attack.noiseAmountMax);
    settings.setValue("jetRatioMin", attack.jetRatioMin);
    settings.setValue("jetRatioMax", attack.jetRatioMax);
    settings.setValue("reedStiffnessMin", attack.reedStiffnessMin);
    settings.setValue("reedStiffnessMax", attack.reedStiffnessMax);
    settings.setValue("reedApertureMin", attack.reedApertureMin);
    settings.setValue("reedApertureMax", attack.reedApertureMax);
    settings.setValue("blowPositionMin", attack.blowPositionMin);
    settings.setValue("blowPositionMax", attack.blowPositionMax);
    settings.setValue("lipTensionMin", attack.lipTensionMin);
    settings.setValue("lipTensionMax", attack.lipTensionMax);
    settings.setValue("buzzQMin", attack.buzzQMin);
    settings.setValue("buzzQMax", attack.buzzQMax);
    settings.setValue("hardnessMin", attack.hardnessMin);
    settings.setValue("hardnessMax", attack.hardnessMax);
    settings.setValue("brightnessMin", attack.brightnessMin);
    settings.setValue("brightnessMax", attack.brightnessMax);
    settings.setValue("tightnessMin", attack.tightnessMin);
    settings.setValue("tightnessMax", attack.tightnessMax);
    settings.setValue("toneMin", attack.toneMin);
    settings.setValue("toneMax", attack.toneMax);
    settings.endGroup();

    // LFO
    settings.beginGroup("LFO");
    settings.setValue("frequencyMin", lfo.frequencyMin);
    settings.setValue("frequencyMax", lfo.frequencyMax);
    settings.setValue("amplitudeMin", lfo.amplitudeMin);
    settings.setValue("amplitudeMax", lfo.amplitudeMax);
    settings.endGroup();

    // Wavetable Synth
    settings.beginGroup("WavetableSynth");
    settings.setValue("positionMin", wavetableSynth.positionMin);
    settings.setValue("positionMax", wavetableSynth.positionMax);
    settings.endGroup();

    // Frequency Mapper
    settings.beginGroup("FrequencyMapper");
    settings.setValue("lowFreqMin", frequencyMapper.lowFreqMin);
    settings.setValue("lowFreqMax", frequencyMapper.lowFreqMax);
    settings.setValue("highFreqMin", frequencyMapper.highFreqMin);
    settings.setValue("highFreqMax", frequencyMapper.highFreqMax);
    settings.setValue("outputMinMin", frequencyMapper.outputMinMin);
    settings.setValue("outputMinMax", frequencyMapper.outputMinMax);
    settings.setValue("outputMaxMin", frequencyMapper.outputMaxMin);
    settings.setValue("outputMaxMax", frequencyMapper.outputMaxMax);
    settings.endGroup();

    // Bandpass EQ
    settings.beginGroup("BandpassEQ");
    settings.setValue("gainMin", bandpassEQ.gainMin);
    settings.setValue("gainMax", bandpassEQ.gainMax);
    settings.setValue("qMin", bandpassEQ.qMin);
    settings.setValue("qMax", bandpassEQ.qMax);
    settings.endGroup();

    // Comb Filter
    settings.beginGroup("CombFilter");
    settings.setValue("delayTimeMin", combFilter.delayTimeMin);
    settings.setValue("delayTimeMax", combFilter.delayTimeMax);
    settings.setValue("feedbackMin", combFilter.feedbackMin);
    settings.setValue("feedbackMax", combFilter.feedbackMax);
    settings.setValue("dampingMin", combFilter.dampingMin);
    settings.setValue("dampingMax", combFilter.dampingMax);
    settings.endGroup();

    // LP/HP Filter
    settings.beginGroup("LowHighPassFilter");
    settings.setValue("cutoffMin", lowHighPassFilter.cutoffMin);
    settings.setValue("cutoffMax", lowHighPassFilter.cutoffMax);
    settings.setValue("resonanceMin", lowHighPassFilter.resonanceMin);
    settings.setValue("resonanceMax", lowHighPassFilter.resonanceMax);
    settings.endGroup();

    // Recorder
    settings.beginGroup("Recorder");
    settings.setValue("breathPressureMin", recorder.breathPressureMin);
    settings.setValue("breathPressureMax", recorder.breathPressureMax);
    settings.setValue("jetRatioMin",       recorder.jetRatioMin);
    settings.setValue("jetRatioMax",       recorder.jetRatioMax);
    settings.setValue("noiseGainMin",      recorder.noiseGainMin);
    settings.setValue("noiseGainMax",      recorder.noiseGainMax);
    settings.setValue("vibratoFreqMin",    recorder.vibratoFreqMin);
    settings.setValue("vibratoFreqMax",    recorder.vibratoFreqMax);
    settings.setValue("vibratoGainMin",    recorder.vibratoGainMin);
    settings.setValue("vibratoGainMax",    recorder.vibratoGainMax);
    settings.setValue("endReflectionMin",  recorder.endReflectionMin);
    settings.setValue("endReflectionMax",  recorder.endReflectionMax);
    settings.setValue("jetReflectionMin",  recorder.jetReflectionMin);
    settings.setValue("jetReflectionMax",  recorder.jetReflectionMax);
    settings.endGroup();

    // Reed
    settings.beginGroup("Reed");
    settings.setValue("breathPressureMin", reed.breathPressureMin);
    settings.setValue("breathPressureMax", reed.breathPressureMax);
    settings.setValue("reedStiffnessMin",  reed.reedStiffnessMin);
    settings.setValue("reedStiffnessMax",  reed.reedStiffnessMax);
    settings.setValue("reedApertureMin",   reed.reedApertureMin);
    settings.setValue("reedApertureMax",   reed.reedApertureMax);
    settings.setValue("blowPositionMin",   reed.blowPositionMin);
    settings.setValue("blowPositionMax",   reed.blowPositionMax);
    settings.setValue("noiseGainMin",      reed.noiseGainMin);
    settings.setValue("noiseGainMax",      reed.noiseGainMax);
    settings.setValue("vibratoFreqMin",    reed.vibratoFreqMin);
    settings.setValue("vibratoFreqMax",    reed.vibratoFreqMax);
    settings.setValue("vibratoGainMin",    reed.vibratoGainMin);
    settings.setValue("vibratoGainMax",    reed.vibratoGainMax);
    settings.setValue("nlTypeMin",         reed.nlTypeMin);
    settings.setValue("nlTypeMax",         reed.nlTypeMax);
    settings.setValue("nlAmountMin",       reed.nlAmountMin);
    settings.setValue("nlAmountMax",       reed.nlAmountMax);
    settings.setValue("nlFreqModMin",      reed.nlFreqModMin);
    settings.setValue("nlFreqModMax",      reed.nlFreqModMax);
    settings.setValue("nlAttackMin",       reed.nlAttackMin);
    settings.setValue("nlAttackMax",       reed.nlAttackMax);
    settings.endGroup();

    // Easing
    settings.beginGroup("Easing");
    settings.setValue("overshootDefault", easing.overshootDefault);
    settings.setValue("overshootMin", easing.overshootMin);
    settings.setValue("overshootMax", easing.overshootMax);
    settings.setValue("amplitudeDefault", easing.amplitudeDefault);
    settings.setValue("amplitudeMin", easing.amplitudeMin);
    settings.setValue("amplitudeMax", easing.amplitudeMax);
    settings.setValue("periodDefault", easing.periodDefault);
    settings.setValue("periodMin", easing.periodMin);
    settings.setValue("periodMax", easing.periodMax);
    settings.setValue("frequencyDefault", easing.frequencyDefault);
    settings.setValue("frequencyMin", easing.frequencyMin);
    settings.setValue("frequencyMax", easing.frequencyMax);
    settings.setValue("decayDefault", easing.decayDefault);
    settings.setValue("decayMin", easing.decayMin);
    settings.setValue("decayMax", easing.decayMax);
    settings.endGroup();

    settings.sync();
    qDebug() << "ContainerSettings saved";
    emit settingsChanged();
}

void ContainerSettings::loadSettings()
{
    QSettings settings("Kala", "ContainerSettings");

    // Harmonic Generator
    settings.beginGroup("HarmonicGenerator");
    harmonicGenerator.maxHarmonics = settings.value("maxHarmonics", 128).toInt();
    harmonicGenerator.fundamentalHzMin = settings.value("fundamentalHzMin", 20.0).toDouble();
    harmonicGenerator.fundamentalHzMax = settings.value("fundamentalHzMax", 8000.0).toDouble();
    harmonicGenerator.rolloffPowerMin = settings.value("rolloffPowerMin", 0.2).toDouble();
    harmonicGenerator.rolloffPowerMax = settings.value("rolloffPowerMax", 5.0).toDouble();
    harmonicGenerator.driftMax = settings.value("driftMax", 0.1).toDouble();
    harmonicGenerator.padBandwidthMin = settings.value("padBandwidthMin", 1.0).toDouble();
    harmonicGenerator.padBandwidthMax = settings.value("padBandwidthMax", 200.0).toDouble();
    harmonicGenerator.padBandwidthScaleMin = settings.value("padBandwidthScaleMin", 0.0).toDouble();
    harmonicGenerator.padBandwidthScaleMax = settings.value("padBandwidthScaleMax", 2.0).toDouble();
    settings.endGroup();

    // Rolloff Processor
    settings.beginGroup("RolloffProcessor");
    rolloffProcessor.lowRolloffMin = settings.value("lowRolloffMin", 0.0).toDouble();
    rolloffProcessor.lowRolloffMax = settings.value("lowRolloffMax", 2.0).toDouble();
    rolloffProcessor.highRolloffMin = settings.value("highRolloffMin", 0.0).toDouble();
    rolloffProcessor.highRolloffMax = settings.value("highRolloffMax", 3.0).toDouble();
    rolloffProcessor.crossoverMin = settings.value("crossoverMin", 1.0).toDouble();
    rolloffProcessor.crossoverMax = settings.value("crossoverMax", 32.0).toDouble();
    rolloffProcessor.transitionMin = settings.value("transitionMin", 0.0).toDouble();
    rolloffProcessor.transitionMax = settings.value("transitionMax", 16.0).toDouble();
    settings.endGroup();

    // Spectrum to Signal
    settings.beginGroup("SpectrumToSignal");
    spectrumToSignal.normalizeMin = settings.value("normalizeMin", 0.0).toDouble();
    spectrumToSignal.normalizeMax = settings.value("normalizeMax", 1.0).toDouble();
    settings.endGroup();

    // Formant Body
    settings.beginGroup("FormantBody");
    formantBody.f1FreqMin = settings.value("f1FreqMin", 200.0).toDouble();
    formantBody.f1FreqMax = settings.value("f1FreqMax", 1000.0).toDouble();
    formantBody.f2FreqMin = settings.value("f2FreqMin", 800.0).toDouble();
    formantBody.f2FreqMax = settings.value("f2FreqMax", 3000.0).toDouble();
    formantBody.qMin = settings.value("qMin", 1.0).toDouble();
    formantBody.qMax = settings.value("qMax", 20.0).toDouble();
    formantBody.mixMin = settings.value("mixMin", 0.0).toDouble();
    formantBody.mixMax = settings.value("mixMax", 1.0).toDouble();
    settings.endGroup();

    // Breath Turbulence
    settings.beginGroup("BreathTurbulence");
    breathTurbulence.blendMin = settings.value("blendMin", 0.0).toDouble();
    breathTurbulence.blendMax = settings.value("blendMax", 1.0).toDouble();
    settings.endGroup();

    // Noise Color Filter
    settings.beginGroup("NoiseColorFilter");
    noiseColorFilter.colorMin = settings.value("colorMin", 100.0).toDouble();
    noiseColorFilter.colorMax = settings.value("colorMax", 8000.0).toDouble();
    noiseColorFilter.filterQMin = settings.value("filterQMin", 0.5).toDouble();
    noiseColorFilter.filterQMax = settings.value("filterQMax", 10.0).toDouble();
    settings.endGroup();

    // Physics System
    settings.beginGroup("PhysicsSystem");
    physicsSystem.massMin = settings.value("massMin", 0.0).toDouble();
    physicsSystem.massMax = settings.value("massMax", 10.0).toDouble();
    physicsSystem.springKMin = settings.value("springKMin", 0.0001).toDouble();
    physicsSystem.springKMax = settings.value("springKMax", 1.0).toDouble();
    physicsSystem.dampingMin = settings.value("dampingMin", 0.5).toDouble();
    physicsSystem.dampingMax = settings.value("dampingMax", 0.9999).toDouble();
    physicsSystem.impulseMin = settings.value("impulseMin", 0.0).toDouble();
    physicsSystem.impulseMax = settings.value("impulseMax", 1000.0).toDouble();
    settings.endGroup();

    // Drift Engine
    settings.beginGroup("DriftEngine");
    driftEngine.amountMin = settings.value("amountMin", 0.0).toDouble();
    driftEngine.amountMax = settings.value("amountMax", 0.1).toDouble();
    driftEngine.rateMin = settings.value("rateMin", 0.01).toDouble();
    driftEngine.rateMax = settings.value("rateMax", 10.0).toDouble();
    settings.endGroup();

    // Gate Processor
    settings.beginGroup("GateProcessor");
    gateProcessor.velocityMin = settings.value("velocityMin", 0.0).toDouble();
    gateProcessor.velocityMax = settings.value("velocityMax", 1.0).toDouble();
    gateProcessor.attackTimeMin = settings.value("attackTimeMin", 0.001).toDouble();
    gateProcessor.attackTimeMax = settings.value("attackTimeMax", 1.0).toDouble();
    gateProcessor.releaseTimeMin = settings.value("releaseTimeMin", 0.001).toDouble();
    gateProcessor.releaseTimeMax = settings.value("releaseTimeMax", 2.0).toDouble();
    gateProcessor.velocitySensMin = settings.value("velocitySensMin", 0.0).toDouble();
    gateProcessor.velocitySensMax = settings.value("velocitySensMax", 1.0).toDouble();
    settings.endGroup();

    // Karplus Strong
    settings.beginGroup("KarplusStrong");
    karplusStrong.attackPortionMin = settings.value("attackPortionMin", 0.01).toDouble();
    karplusStrong.attackPortionMax = settings.value("attackPortionMax", 1.0).toDouble();
    karplusStrong.dampingMin = settings.value("dampingMin", 0.0).toDouble();
    karplusStrong.dampingMax = settings.value("dampingMax", 1.0).toDouble();
    karplusStrong.pluckPositionMin = settings.value("pluckPositionMin", 0.0).toDouble();
    karplusStrong.pluckPositionMax = settings.value("pluckPositionMax", 1.0).toDouble();
    karplusStrong.mixMin = settings.value("mixMin", 0.0).toDouble();
    karplusStrong.mixMax = settings.value("mixMax", 1.0).toDouble();
    karplusStrong.brightnessMin = settings.value("brightnessMin", 0.0).toDouble();
    karplusStrong.brightnessMax = settings.value("brightnessMax", 1.0).toDouble();
    karplusStrong.excitationSoftnessMin = settings.value("excitationSoftnessMin", 0.0).toDouble();
    karplusStrong.excitationSoftnessMax = settings.value("excitationSoftnessMax", 1.0).toDouble();
    karplusStrong.pluckHardnessMin = settings.value("pluckHardnessMin", 0.0).toDouble();
    karplusStrong.pluckHardnessMax = settings.value("pluckHardnessMax", 1.0).toDouble();
    karplusStrong.bodyResonanceMin = settings.value("bodyResonanceMin", 0.0).toDouble();
    karplusStrong.bodyResonanceMax = settings.value("bodyResonanceMax", 1.0).toDouble();
    karplusStrong.bodyFreqMin = settings.value("bodyFreqMin", 80.0).toDouble();
    karplusStrong.bodyFreqMax = settings.value("bodyFreqMax", 400.0).toDouble();
    karplusStrong.pickDirectionMin = settings.value("pickDirectionMin", 0.0).toDouble();
    karplusStrong.pickDirectionMax = settings.value("pickDirectionMax", 1.0).toDouble();
    settings.endGroup();

    // Attack
    settings.beginGroup("Attack");
    attack.durationMin = settings.value("durationMin", 0.005).toDouble();
    attack.durationMax = settings.value("durationMax", 5.0).toDouble();
    attack.intensityMin = settings.value("intensityMin", 0.0).toDouble();
    attack.intensityMax = settings.value("intensityMax", 1.0).toDouble();
    attack.mixMin = settings.value("mixMin", 0.0).toDouble();
    attack.mixMax = settings.value("mixMax", 1.0).toDouble();
    attack.noiseAmountMin = settings.value("noiseAmountMin", 0.0).toDouble();
    attack.noiseAmountMax = settings.value("noiseAmountMax", 1.0).toDouble();
    attack.jetRatioMin = settings.value("jetRatioMin", 0.0).toDouble();
    attack.jetRatioMax = settings.value("jetRatioMax", 1.0).toDouble();
    attack.reedStiffnessMin = settings.value("reedStiffnessMin", 0.0).toDouble();
    attack.reedStiffnessMax = settings.value("reedStiffnessMax", 1.0).toDouble();
    attack.reedApertureMin = settings.value("reedApertureMin", 0.0).toDouble();
    attack.reedApertureMax = settings.value("reedApertureMax", 1.0).toDouble();
    attack.blowPositionMin = settings.value("blowPositionMin", 0.0).toDouble();
    attack.blowPositionMax = settings.value("blowPositionMax", 1.0).toDouble();
    attack.lipTensionMin = settings.value("lipTensionMin", 0.0).toDouble();
    attack.lipTensionMax = settings.value("lipTensionMax", 1.0).toDouble();
    attack.buzzQMin = settings.value("buzzQMin", 0.5).toDouble();
    attack.buzzQMax = settings.value("buzzQMax", 50.0).toDouble();
    attack.hardnessMin = settings.value("hardnessMin", 0.0).toDouble();
    attack.hardnessMax = settings.value("hardnessMax", 1.0).toDouble();
    attack.brightnessMin = settings.value("brightnessMin", 0.0).toDouble();
    attack.brightnessMax = settings.value("brightnessMax", 1.0).toDouble();
    attack.tightnessMin = settings.value("tightnessMin", 0.0).toDouble();
    attack.tightnessMax = settings.value("tightnessMax", 1.0).toDouble();
    attack.toneMin = settings.value("toneMin", 0.0).toDouble();
    attack.toneMax = settings.value("toneMax", 1.0).toDouble();
    settings.endGroup();

    // LFO
    settings.beginGroup("LFO");
    lfo.frequencyMin = settings.value("frequencyMin", 0.01).toDouble();
    lfo.frequencyMax = settings.value("frequencyMax", 20.0).toDouble();
    lfo.amplitudeMin = settings.value("amplitudeMin", 0.0).toDouble();
    lfo.amplitudeMax = settings.value("amplitudeMax", 1.0).toDouble();
    settings.endGroup();

    // Wavetable Synth
    settings.beginGroup("WavetableSynth");
    wavetableSynth.positionMin = settings.value("positionMin", 0.0).toDouble();
    wavetableSynth.positionMax = settings.value("positionMax", 1.0).toDouble();
    settings.endGroup();

    // Frequency Mapper
    settings.beginGroup("FrequencyMapper");
    frequencyMapper.lowFreqMin = settings.value("lowFreqMin", 1.0).toDouble();
    frequencyMapper.lowFreqMax = settings.value("lowFreqMax", 500.0).toDouble();
    frequencyMapper.highFreqMin = settings.value("highFreqMin", 500.0).toDouble();
    frequencyMapper.highFreqMax = settings.value("highFreqMax", 20000.0).toDouble();
    frequencyMapper.outputMinMin = settings.value("outputMinMin", -2.0).toDouble();
    frequencyMapper.outputMinMax = settings.value("outputMinMax", 1.0).toDouble();
    frequencyMapper.outputMaxMin = settings.value("outputMaxMin", 0.0).toDouble();
    frequencyMapper.outputMaxMax = settings.value("outputMaxMax", 2.0).toDouble();
    settings.endGroup();

    // Bandpass EQ
    settings.beginGroup("BandpassEQ");
    bandpassEQ.gainMin = settings.value("gainMin", 0.0).toDouble();
    bandpassEQ.gainMax = settings.value("gainMax", 2.0).toDouble();
    bandpassEQ.qMin = settings.value("qMin", 0.5).toDouble();
    bandpassEQ.qMax = settings.value("qMax", 20.0).toDouble();
    settings.endGroup();

    // Comb Filter
    settings.beginGroup("CombFilter");
    combFilter.delayTimeMin = settings.value("delayTimeMin", 0.1).toDouble();
    combFilter.delayTimeMax = settings.value("delayTimeMax", 100.0).toDouble();
    combFilter.feedbackMin = settings.value("feedbackMin", -0.99).toDouble();
    combFilter.feedbackMax = settings.value("feedbackMax", 0.99).toDouble();
    combFilter.dampingMin = settings.value("dampingMin", 0.0).toDouble();
    combFilter.dampingMax = settings.value("dampingMax", 1.0).toDouble();
    settings.endGroup();

    // LP/HP Filter
    settings.beginGroup("LowHighPassFilter");
    lowHighPassFilter.cutoffMin = settings.value("cutoffMin", 20.0).toDouble();
    lowHighPassFilter.cutoffMax = settings.value("cutoffMax", 20000.0).toDouble();
    lowHighPassFilter.resonanceMin = settings.value("resonanceMin", 0.5).toDouble();
    lowHighPassFilter.resonanceMax = settings.value("resonanceMax", 20.0).toDouble();
    settings.endGroup();

    // Recorder
    settings.beginGroup("Recorder");
    recorder.breathPressureMin = settings.value("breathPressureMin", 0.0).toDouble();
    recorder.breathPressureMax = settings.value("breathPressureMax", 1.0).toDouble();
    recorder.jetRatioMin       = settings.value("jetRatioMin",       0.05).toDouble();
    recorder.jetRatioMax       = settings.value("jetRatioMax",       0.60).toDouble();
    recorder.noiseGainMin      = settings.value("noiseGainMin",      0.0).toDouble();
    recorder.noiseGainMax      = settings.value("noiseGainMax",      0.5).toDouble();
    recorder.vibratoFreqMin    = settings.value("vibratoFreqMin",    0.0).toDouble();
    recorder.vibratoFreqMax    = settings.value("vibratoFreqMax",    20.0).toDouble();
    recorder.vibratoGainMin    = settings.value("vibratoGainMin",    0.0).toDouble();
    recorder.vibratoGainMax    = settings.value("vibratoGainMax",    1.0).toDouble();
    recorder.endReflectionMin  = settings.value("endReflectionMin",  -1.0).toDouble();
    recorder.endReflectionMax  = settings.value("endReflectionMax",   1.0).toDouble();
    recorder.jetReflectionMin  = settings.value("jetReflectionMin",  -1.0).toDouble();
    recorder.jetReflectionMax  = settings.value("jetReflectionMax",   1.0).toDouble();
    settings.endGroup();

    // Reed
    settings.beginGroup("Reed");
    reed.breathPressureMin = settings.value("breathPressureMin", 0.0).toDouble();
    reed.breathPressureMax = settings.value("breathPressureMax", 1.0).toDouble();
    reed.reedStiffnessMin  = settings.value("reedStiffnessMin",  0.0).toDouble();
    reed.reedStiffnessMax  = settings.value("reedStiffnessMax",  1.0).toDouble();
    reed.reedApertureMin   = settings.value("reedApertureMin",   0.0).toDouble();
    reed.reedApertureMax   = settings.value("reedApertureMax",   1.0).toDouble();
    reed.blowPositionMin   = settings.value("blowPositionMin",   0.0).toDouble();
    reed.blowPositionMax   = settings.value("blowPositionMax",   1.0).toDouble();
    reed.noiseGainMin      = settings.value("noiseGainMin",      0.0).toDouble();
    reed.noiseGainMax      = settings.value("noiseGainMax",      0.4).toDouble();
    reed.vibratoFreqMin    = settings.value("vibratoFreqMin",    0.0).toDouble();
    reed.vibratoFreqMax    = settings.value("vibratoFreqMax",   12.0).toDouble();
    reed.vibratoGainMin    = settings.value("vibratoGainMin",    0.0).toDouble();
    reed.vibratoGainMax    = settings.value("vibratoGainMax",    0.5).toDouble();
    reed.nlTypeMin         = settings.value("nlTypeMin",          0.0).toDouble();
    reed.nlTypeMax         = settings.value("nlTypeMax",          4.0).toDouble();
    reed.nlAmountMin       = settings.value("nlAmountMin",        0.0).toDouble();
    reed.nlAmountMax       = settings.value("nlAmountMax",        1.0).toDouble();
    reed.nlFreqModMin      = settings.value("nlFreqModMin",       0.0).toDouble();
    reed.nlFreqModMax      = settings.value("nlFreqModMax",     200.0).toDouble();
    reed.nlAttackMin       = settings.value("nlAttackMin",      0.001).toDouble();
    reed.nlAttackMax       = settings.value("nlAttackMax",        2.0).toDouble();
    settings.endGroup();

    // Easing
    settings.beginGroup("Easing");
    easing.overshootDefault = settings.value("overshootDefault", 0.5).toDouble();
    easing.overshootMin = settings.value("overshootMin", 0.0).toDouble();
    easing.overshootMax = settings.value("overshootMax", 2.0).toDouble();
    easing.amplitudeDefault = settings.value("amplitudeDefault", 0.5).toDouble();
    easing.amplitudeMin = settings.value("amplitudeMin", 0.1).toDouble();
    easing.amplitudeMax = settings.value("amplitudeMax", 2.0).toDouble();
    easing.periodDefault = settings.value("periodDefault", 0.4).toDouble();
    easing.periodMin = settings.value("periodMin", 0.1).toDouble();
    easing.periodMax = settings.value("periodMax", 1.0).toDouble();
    easing.frequencyDefault = settings.value("frequencyDefault", 2.0).toDouble();
    easing.frequencyMin = settings.value("frequencyMin", 0.5).toDouble();
    easing.frequencyMax = settings.value("frequencyMax", 5.0).toDouble();
    easing.decayDefault = settings.value("decayDefault", 0.7).toDouble();
    easing.decayMin = settings.value("decayMin", 0.1).toDouble();
    easing.decayMax = settings.value("decayMax", 1.0).toDouble();
    settings.endGroup();

    qDebug() << "ContainerSettings loaded";
}

void ContainerSettings::resetToDefaults()
{
    // Harmonic Generator defaults
    harmonicGenerator.maxHarmonics = 128;
    harmonicGenerator.fundamentalHzMin = 20.0;
    harmonicGenerator.fundamentalHzMax = 8000.0;
    harmonicGenerator.rolloffPowerMin = 0.2;
    harmonicGenerator.rolloffPowerMax = 5.0;
    harmonicGenerator.driftMax = 0.1;
    harmonicGenerator.padBandwidthMin = 1.0;
    harmonicGenerator.padBandwidthMax = 200.0;
    harmonicGenerator.padBandwidthScaleMin = 0.0;
    harmonicGenerator.padBandwidthScaleMax = 2.0;

    // Rolloff Processor defaults
    rolloffProcessor.lowRolloffMin = 0.0;
    rolloffProcessor.lowRolloffMax = 2.0;
    rolloffProcessor.highRolloffMin = 0.0;
    rolloffProcessor.highRolloffMax = 3.0;
    rolloffProcessor.crossoverMin = 1.0;
    rolloffProcessor.crossoverMax = 32.0;
    rolloffProcessor.transitionMin = 0.0;
    rolloffProcessor.transitionMax = 16.0;

    // Spectrum to Signal defaults
    spectrumToSignal.normalizeMin = 0.0;
    spectrumToSignal.normalizeMax = 1.0;

    // Formant Body defaults
    formantBody.f1FreqMin = 200.0;
    formantBody.f1FreqMax = 1000.0;
    formantBody.f2FreqMin = 800.0;
    formantBody.f2FreqMax = 3000.0;
    formantBody.qMin = 1.0;
    formantBody.qMax = 20.0;
    formantBody.mixMin = 0.0;
    formantBody.mixMax = 1.0;

    // Breath Turbulence defaults
    breathTurbulence.blendMin = 0.0;
    breathTurbulence.blendMax = 1.0;

    // Noise Color Filter defaults
    noiseColorFilter.colorMin = 100.0;
    noiseColorFilter.colorMax = 8000.0;
    noiseColorFilter.filterQMin = 0.5;
    noiseColorFilter.filterQMax = 10.0;

    // Physics System defaults
    physicsSystem.massMin = 0.0;
    physicsSystem.massMax = 10.0;
    physicsSystem.springKMin = 0.0001;
    physicsSystem.springKMax = 1.0;
    physicsSystem.dampingMin = 0.5;
    physicsSystem.dampingMax = 0.9999;
    physicsSystem.impulseMin = 0.0;
    physicsSystem.impulseMax = 1000.0;

    // Drift Engine defaults
    driftEngine.amountMin = 0.0;
    driftEngine.amountMax = 0.1;
    driftEngine.rateMin = 0.01;
    driftEngine.rateMax = 10.0;

    // Gate Processor defaults
    gateProcessor.velocityMin = 0.0;
    gateProcessor.velocityMax = 1.0;
    gateProcessor.attackTimeMin = 0.001;
    gateProcessor.attackTimeMax = 1.0;
    gateProcessor.releaseTimeMin = 0.001;
    gateProcessor.releaseTimeMax = 2.0;
    gateProcessor.velocitySensMin = 0.0;
    gateProcessor.velocitySensMax = 1.0;

    // Karplus Strong defaults
    karplusStrong.attackPortionMin = 0.01;
    karplusStrong.attackPortionMax = 1.0;
    karplusStrong.dampingMin = 0.0;
    karplusStrong.dampingMax = 1.0;
    karplusStrong.pluckPositionMin = 0.0;
    karplusStrong.pluckPositionMax = 1.0;
    karplusStrong.mixMin = 0.0;
    karplusStrong.mixMax = 1.0;
    karplusStrong.brightnessMin = 0.0;
    karplusStrong.brightnessMax = 1.0;
    karplusStrong.excitationSoftnessMin = 0.0;
    karplusStrong.excitationSoftnessMax = 1.0;
    karplusStrong.pluckHardnessMin = 0.0;
    karplusStrong.pluckHardnessMax = 1.0;
    karplusStrong.bodyResonanceMin = 0.0;
    karplusStrong.bodyResonanceMax = 1.0;
    karplusStrong.bodyFreqMin = 80.0;
    karplusStrong.bodyFreqMax = 400.0;
    karplusStrong.pickDirectionMin = 0.0;
    karplusStrong.pickDirectionMax = 1.0;

    // Attack defaults
    attack.durationMin = 0.005;
    attack.durationMax = 5.0;
    attack.intensityMin = 0.0;
    attack.intensityMax = 1.0;
    attack.mixMin = 0.0;
    attack.mixMax = 1.0;
    attack.noiseAmountMin = 0.0;
    attack.noiseAmountMax = 1.0;
    attack.jetRatioMin = 0.0;
    attack.jetRatioMax = 1.0;
    attack.reedStiffnessMin = 0.0;
    attack.reedStiffnessMax = 1.0;
    attack.reedApertureMin = 0.0;
    attack.reedApertureMax = 1.0;
    attack.blowPositionMin = 0.0;
    attack.blowPositionMax = 1.0;
    attack.lipTensionMin = 0.0;
    attack.lipTensionMax = 1.0;
    attack.buzzQMin = 0.5;
    attack.buzzQMax = 50.0;
    attack.hardnessMin = 0.0;
    attack.hardnessMax = 1.0;
    attack.brightnessMin = 0.0;
    attack.brightnessMax = 1.0;
    attack.tightnessMin = 0.0;
    attack.tightnessMax = 1.0;
    attack.toneMin = 0.0;
    attack.toneMax = 1.0;

    // LFO defaults
    lfo.frequencyMin = 0.01;
    lfo.frequencyMax = 20.0;
    lfo.amplitudeMin = 0.0;
    lfo.amplitudeMax = 1.0;

    // Wavetable Synth defaults
    wavetableSynth.positionMin = 0.0;
    wavetableSynth.positionMax = 1.0;

    // Frequency Mapper defaults
    frequencyMapper.lowFreqMin = 1.0;
    frequencyMapper.lowFreqMax = 500.0;
    frequencyMapper.highFreqMin = 500.0;
    frequencyMapper.highFreqMax = 20000.0;
    frequencyMapper.outputMinMin = -2.0;
    frequencyMapper.outputMinMax = 1.0;
    frequencyMapper.outputMaxMin = 0.0;
    frequencyMapper.outputMaxMax = 2.0;

    // Bandpass EQ defaults
    bandpassEQ.gainMin = 0.0;
    bandpassEQ.gainMax = 2.0;
    bandpassEQ.qMin = 0.5;
    bandpassEQ.qMax = 20.0;

    // Comb Filter defaults
    combFilter.delayTimeMin = 0.1;
    combFilter.delayTimeMax = 100.0;
    combFilter.feedbackMin = -0.99;
    combFilter.feedbackMax = 0.99;
    combFilter.dampingMin = 0.0;
    combFilter.dampingMax = 1.0;

    // LP/HP Filter defaults
    lowHighPassFilter.cutoffMin = 20.0;
    lowHighPassFilter.cutoffMax = 20000.0;
    lowHighPassFilter.resonanceMin = 0.5;
    lowHighPassFilter.resonanceMax = 20.0;

    // Recorder defaults
    recorder.breathPressureMin = 0.0;
    recorder.breathPressureMax = 1.0;
    recorder.jetRatioMin       = 0.05;
    recorder.jetRatioMax       = 0.60;
    recorder.noiseGainMin      = 0.0;
    recorder.noiseGainMax      = 0.5;
    recorder.vibratoFreqMin    = 0.0;
    recorder.vibratoFreqMax    = 20.0;
    recorder.vibratoGainMin    = 0.0;
    recorder.vibratoGainMax    = 1.0;
    recorder.endReflectionMin  = -1.0;
    recorder.endReflectionMax  =  1.0;
    recorder.jetReflectionMin  = -1.0;
    recorder.jetReflectionMax  =  1.0;

    // Reed defaults
    reed.breathPressureMin = 0.0;
    reed.breathPressureMax = 1.0;
    reed.reedStiffnessMin  = 0.0;
    reed.reedStiffnessMax  = 1.0;
    reed.reedApertureMin   = 0.0;
    reed.reedApertureMax   = 1.0;
    reed.blowPositionMin   = 0.0;
    reed.blowPositionMax   = 1.0;
    reed.noiseGainMin      = 0.0;
    reed.noiseGainMax      = 0.4;
    reed.vibratoFreqMin    = 0.0;
    reed.vibratoFreqMax    = 12.0;
    reed.vibratoGainMin    = 0.0;
    reed.vibratoGainMax    = 0.5;
    reed.nlTypeMin         = 0.0;
    reed.nlTypeMax         = 4.0;
    reed.nlAmountMin       = 0.0;
    reed.nlAmountMax       = 1.0;
    reed.nlFreqModMin      = 0.0;
    reed.nlFreqModMax      = 200.0;
    reed.nlAttackMin       = 0.001;
    reed.nlAttackMax       = 2.0;

    // Easing defaults
    easing.overshootDefault = 0.5;
    easing.overshootMin = 0.0;
    easing.overshootMax = 2.0;
    easing.amplitudeDefault = 0.5;
    easing.amplitudeMin = 0.1;
    easing.amplitudeMax = 2.0;
    easing.periodDefault = 0.4;
    easing.periodMin = 0.1;
    easing.periodMax = 1.0;
    easing.frequencyDefault = 2.0;
    easing.frequencyMin = 0.5;
    easing.frequencyMax = 5.0;
    easing.decayDefault = 0.7;
    easing.decayMin = 0.1;
    easing.decayMax = 1.0;

    saveSettings();
    qDebug() << "ContainerSettings reset to defaults";
}

QJsonObject ContainerSettings::toJson() const
{
    QJsonObject json;

    // Harmonic Generator
    QJsonObject hg;
    hg["maxHarmonics"] = harmonicGenerator.maxHarmonics;
    hg["fundamentalHzMin"] = harmonicGenerator.fundamentalHzMin;
    hg["fundamentalHzMax"] = harmonicGenerator.fundamentalHzMax;
    hg["rolloffPowerMin"] = harmonicGenerator.rolloffPowerMin;
    hg["rolloffPowerMax"] = harmonicGenerator.rolloffPowerMax;
    hg["driftMax"] = harmonicGenerator.driftMax;
    hg["padBandwidthMin"] = harmonicGenerator.padBandwidthMin;
    hg["padBandwidthMax"] = harmonicGenerator.padBandwidthMax;
    hg["padBandwidthScaleMin"] = harmonicGenerator.padBandwidthScaleMin;
    hg["padBandwidthScaleMax"] = harmonicGenerator.padBandwidthScaleMax;
    json["harmonicGenerator"] = hg;

    // Rolloff Processor
    QJsonObject rp;
    rp["lowRolloffMin"] = rolloffProcessor.lowRolloffMin;
    rp["lowRolloffMax"] = rolloffProcessor.lowRolloffMax;
    rp["highRolloffMin"] = rolloffProcessor.highRolloffMin;
    rp["highRolloffMax"] = rolloffProcessor.highRolloffMax;
    rp["crossoverMin"] = rolloffProcessor.crossoverMin;
    rp["crossoverMax"] = rolloffProcessor.crossoverMax;
    rp["transitionMin"] = rolloffProcessor.transitionMin;
    rp["transitionMax"] = rolloffProcessor.transitionMax;
    json["rolloffProcessor"] = rp;

    // Spectrum to Signal
    QJsonObject sts;
    sts["normalizeMin"] = spectrumToSignal.normalizeMin;
    sts["normalizeMax"] = spectrumToSignal.normalizeMax;
    json["spectrumToSignal"] = sts;

    // Formant Body
    QJsonObject fb;
    fb["f1FreqMin"] = formantBody.f1FreqMin;
    fb["f1FreqMax"] = formantBody.f1FreqMax;
    fb["f2FreqMin"] = formantBody.f2FreqMin;
    fb["f2FreqMax"] = formantBody.f2FreqMax;
    fb["qMin"] = formantBody.qMin;
    fb["qMax"] = formantBody.qMax;
    fb["mixMin"] = formantBody.mixMin;
    fb["mixMax"] = formantBody.mixMax;
    json["formantBody"] = fb;

    // Breath Turbulence
    QJsonObject bt;
    bt["blendMin"] = breathTurbulence.blendMin;
    bt["blendMax"] = breathTurbulence.blendMax;
    json["breathTurbulence"] = bt;

    // Noise Color Filter
    QJsonObject ncf;
    ncf["colorMin"] = noiseColorFilter.colorMin;
    ncf["colorMax"] = noiseColorFilter.colorMax;
    ncf["filterQMin"] = noiseColorFilter.filterQMin;
    ncf["filterQMax"] = noiseColorFilter.filterQMax;
    json["noiseColorFilter"] = ncf;

    // Physics System
    QJsonObject ps;
    ps["massMin"] = physicsSystem.massMin;
    ps["massMax"] = physicsSystem.massMax;
    ps["springKMin"] = physicsSystem.springKMin;
    ps["springKMax"] = physicsSystem.springKMax;
    ps["dampingMin"] = physicsSystem.dampingMin;
    ps["dampingMax"] = physicsSystem.dampingMax;
    ps["impulseMin"] = physicsSystem.impulseMin;
    ps["impulseMax"] = physicsSystem.impulseMax;
    json["physicsSystem"] = ps;

    // Drift Engine
    QJsonObject de;
    de["amountMin"] = driftEngine.amountMin;
    de["amountMax"] = driftEngine.amountMax;
    de["rateMin"] = driftEngine.rateMin;
    de["rateMax"] = driftEngine.rateMax;
    json["driftEngine"] = de;

    // Gate Processor
    QJsonObject gp;
    gp["velocityMin"] = gateProcessor.velocityMin;
    gp["velocityMax"] = gateProcessor.velocityMax;
    gp["attackTimeMin"] = gateProcessor.attackTimeMin;
    gp["attackTimeMax"] = gateProcessor.attackTimeMax;
    gp["releaseTimeMin"] = gateProcessor.releaseTimeMin;
    gp["releaseTimeMax"] = gateProcessor.releaseTimeMax;
    gp["velocitySensMin"] = gateProcessor.velocitySensMin;
    gp["velocitySensMax"] = gateProcessor.velocitySensMax;
    json["gateProcessor"] = gp;

    // Karplus Strong
    QJsonObject ks;
    ks["attackPortionMin"] = karplusStrong.attackPortionMin;
    ks["attackPortionMax"] = karplusStrong.attackPortionMax;
    ks["dampingMin"] = karplusStrong.dampingMin;
    ks["dampingMax"] = karplusStrong.dampingMax;
    ks["pluckPositionMin"] = karplusStrong.pluckPositionMin;
    ks["pluckPositionMax"] = karplusStrong.pluckPositionMax;
    ks["mixMin"] = karplusStrong.mixMin;
    ks["mixMax"] = karplusStrong.mixMax;
    ks["brightnessMin"] = karplusStrong.brightnessMin;
    ks["brightnessMax"] = karplusStrong.brightnessMax;
    ks["excitationSoftnessMin"] = karplusStrong.excitationSoftnessMin;
    ks["excitationSoftnessMax"] = karplusStrong.excitationSoftnessMax;
    ks["pluckHardnessMin"] = karplusStrong.pluckHardnessMin;
    ks["pluckHardnessMax"] = karplusStrong.pluckHardnessMax;
    ks["bodyResonanceMin"] = karplusStrong.bodyResonanceMin;
    ks["bodyResonanceMax"] = karplusStrong.bodyResonanceMax;
    ks["bodyFreqMin"] = karplusStrong.bodyFreqMin;
    ks["bodyFreqMax"] = karplusStrong.bodyFreqMax;
    ks["pickDirectionMin"] = karplusStrong.pickDirectionMin;
    ks["pickDirectionMax"] = karplusStrong.pickDirectionMax;
    json["karplusStrong"] = ks;

    // Attack
    QJsonObject atk;
    atk["durationMin"] = attack.durationMin;
    atk["durationMax"] = attack.durationMax;
    atk["intensityMin"] = attack.intensityMin;
    atk["intensityMax"] = attack.intensityMax;
    atk["mixMin"] = attack.mixMin;
    atk["mixMax"] = attack.mixMax;
    atk["noiseAmountMin"] = attack.noiseAmountMin;
    atk["noiseAmountMax"] = attack.noiseAmountMax;
    atk["jetRatioMin"] = attack.jetRatioMin;
    atk["jetRatioMax"] = attack.jetRatioMax;
    atk["reedStiffnessMin"] = attack.reedStiffnessMin;
    atk["reedStiffnessMax"] = attack.reedStiffnessMax;
    atk["reedApertureMin"] = attack.reedApertureMin;
    atk["reedApertureMax"] = attack.reedApertureMax;
    atk["blowPositionMin"] = attack.blowPositionMin;
    atk["blowPositionMax"] = attack.blowPositionMax;
    atk["lipTensionMin"] = attack.lipTensionMin;
    atk["lipTensionMax"] = attack.lipTensionMax;
    atk["buzzQMin"] = attack.buzzQMin;
    atk["buzzQMax"] = attack.buzzQMax;
    atk["hardnessMin"] = attack.hardnessMin;
    atk["hardnessMax"] = attack.hardnessMax;
    atk["brightnessMin"] = attack.brightnessMin;
    atk["brightnessMax"] = attack.brightnessMax;
    atk["tightnessMin"] = attack.tightnessMin;
    atk["tightnessMax"] = attack.tightnessMax;
    atk["toneMin"] = attack.toneMin;
    atk["toneMax"] = attack.toneMax;
    json["attack"] = atk;

    // LFO
    QJsonObject lfoJson;
    lfoJson["frequencyMin"] = lfo.frequencyMin;
    lfoJson["frequencyMax"] = lfo.frequencyMax;
    lfoJson["amplitudeMin"] = lfo.amplitudeMin;
    lfoJson["amplitudeMax"] = lfo.amplitudeMax;
    json["lfo"] = lfoJson;

    // Wavetable Synth
    QJsonObject wts;
    wts["positionMin"] = wavetableSynth.positionMin;
    wts["positionMax"] = wavetableSynth.positionMax;
    json["wavetableSynth"] = wts;

    // Frequency Mapper
    QJsonObject fm;
    fm["lowFreqMin"] = frequencyMapper.lowFreqMin;
    fm["lowFreqMax"] = frequencyMapper.lowFreqMax;
    fm["highFreqMin"] = frequencyMapper.highFreqMin;
    fm["highFreqMax"] = frequencyMapper.highFreqMax;
    fm["outputMinMin"] = frequencyMapper.outputMinMin;
    fm["outputMinMax"] = frequencyMapper.outputMinMax;
    fm["outputMaxMin"] = frequencyMapper.outputMaxMin;
    fm["outputMaxMax"] = frequencyMapper.outputMaxMax;
    json["frequencyMapper"] = fm;

    // Bandpass EQ
    QJsonObject bpeq;
    bpeq["gainMin"] = bandpassEQ.gainMin;
    bpeq["gainMax"] = bandpassEQ.gainMax;
    bpeq["qMin"] = bandpassEQ.qMin;
    bpeq["qMax"] = bandpassEQ.qMax;
    json["bandpassEQ"] = bpeq;

    // Comb Filter
    QJsonObject cf;
    cf["delayTimeMin"] = combFilter.delayTimeMin;
    cf["delayTimeMax"] = combFilter.delayTimeMax;
    cf["feedbackMin"] = combFilter.feedbackMin;
    cf["feedbackMax"] = combFilter.feedbackMax;
    cf["dampingMin"] = combFilter.dampingMin;
    cf["dampingMax"] = combFilter.dampingMax;
    json["combFilter"] = cf;

    // LP/HP Filter
    QJsonObject lhpf;
    lhpf["cutoffMin"] = lowHighPassFilter.cutoffMin;
    lhpf["cutoffMax"] = lowHighPassFilter.cutoffMax;
    lhpf["resonanceMin"] = lowHighPassFilter.resonanceMin;
    lhpf["resonanceMax"] = lowHighPassFilter.resonanceMax;
    json["lowHighPassFilter"] = lhpf;

    // Recorder
    QJsonObject rec;
    rec["breathPressureMin"] = recorder.breathPressureMin;
    rec["breathPressureMax"] = recorder.breathPressureMax;
    rec["jetRatioMin"]       = recorder.jetRatioMin;
    rec["jetRatioMax"]       = recorder.jetRatioMax;
    rec["noiseGainMin"]      = recorder.noiseGainMin;
    rec["noiseGainMax"]      = recorder.noiseGainMax;
    rec["vibratoFreqMin"]    = recorder.vibratoFreqMin;
    rec["vibratoFreqMax"]    = recorder.vibratoFreqMax;
    rec["vibratoGainMin"]    = recorder.vibratoGainMin;
    rec["vibratoGainMax"]    = recorder.vibratoGainMax;
    rec["endReflectionMin"]  = recorder.endReflectionMin;
    rec["endReflectionMax"]  = recorder.endReflectionMax;
    rec["jetReflectionMin"]  = recorder.jetReflectionMin;
    rec["jetReflectionMax"]  = recorder.jetReflectionMax;
    json["recorder"] = rec;

    // Reed
    QJsonObject sax;
    sax["breathPressureMin"] = reed.breathPressureMin;
    sax["breathPressureMax"] = reed.breathPressureMax;
    sax["reedStiffnessMin"]  = reed.reedStiffnessMin;
    sax["reedStiffnessMax"]  = reed.reedStiffnessMax;
    sax["reedApertureMin"]   = reed.reedApertureMin;
    sax["reedApertureMax"]   = reed.reedApertureMax;
    sax["blowPositionMin"]   = reed.blowPositionMin;
    sax["blowPositionMax"]   = reed.blowPositionMax;
    sax["noiseGainMin"]      = reed.noiseGainMin;
    sax["noiseGainMax"]      = reed.noiseGainMax;
    sax["vibratoFreqMin"]    = reed.vibratoFreqMin;
    sax["vibratoFreqMax"]    = reed.vibratoFreqMax;
    sax["vibratoGainMin"]    = reed.vibratoGainMin;
    sax["vibratoGainMax"]    = reed.vibratoGainMax;
    sax["nlTypeMin"]         = reed.nlTypeMin;
    sax["nlTypeMax"]         = reed.nlTypeMax;
    sax["nlAmountMin"]       = reed.nlAmountMin;
    sax["nlAmountMax"]       = reed.nlAmountMax;
    sax["nlFreqModMin"]      = reed.nlFreqModMin;
    sax["nlFreqModMax"]      = reed.nlFreqModMax;
    sax["nlAttackMin"]       = reed.nlAttackMin;
    sax["nlAttackMax"]       = reed.nlAttackMax;
    json["reed"] = sax;

    // Easing
    QJsonObject easingJson;
    easingJson["overshootDefault"] = easing.overshootDefault;
    easingJson["overshootMin"] = easing.overshootMin;
    easingJson["overshootMax"] = easing.overshootMax;
    easingJson["amplitudeDefault"] = easing.amplitudeDefault;
    easingJson["amplitudeMin"] = easing.amplitudeMin;
    easingJson["amplitudeMax"] = easing.amplitudeMax;
    easingJson["periodDefault"] = easing.periodDefault;
    easingJson["periodMin"] = easing.periodMin;
    easingJson["periodMax"] = easing.periodMax;
    easingJson["frequencyDefault"] = easing.frequencyDefault;
    easingJson["frequencyMin"] = easing.frequencyMin;
    easingJson["frequencyMax"] = easing.frequencyMax;
    easingJson["decayDefault"] = easing.decayDefault;
    easingJson["decayMin"] = easing.decayMin;
    easingJson["decayMax"] = easing.decayMax;
    json["easing"] = easingJson;

    return json;
}

void ContainerSettings::fromJson(const QJsonObject &json)
{
    // Harmonic Generator
    if (json.contains("harmonicGenerator")) {
        QJsonObject hg = json["harmonicGenerator"].toObject();
        harmonicGenerator.maxHarmonics = hg["maxHarmonics"].toInt(128);
        harmonicGenerator.fundamentalHzMin = hg["fundamentalHzMin"].toDouble(20.0);
        harmonicGenerator.fundamentalHzMax = hg["fundamentalHzMax"].toDouble(8000.0);
        harmonicGenerator.rolloffPowerMin = hg["rolloffPowerMin"].toDouble(0.2);
        harmonicGenerator.rolloffPowerMax = hg["rolloffPowerMax"].toDouble(5.0);
        harmonicGenerator.driftMax = hg["driftMax"].toDouble(0.1);
        harmonicGenerator.padBandwidthMin = hg["padBandwidthMin"].toDouble(1.0);
        harmonicGenerator.padBandwidthMax = hg["padBandwidthMax"].toDouble(200.0);
        harmonicGenerator.padBandwidthScaleMin = hg["padBandwidthScaleMin"].toDouble(0.0);
        harmonicGenerator.padBandwidthScaleMax = hg["padBandwidthScaleMax"].toDouble(2.0);
    }

    // Rolloff Processor
    if (json.contains("rolloffProcessor")) {
        QJsonObject rp = json["rolloffProcessor"].toObject();
        rolloffProcessor.lowRolloffMin = rp["lowRolloffMin"].toDouble(0.0);
        rolloffProcessor.lowRolloffMax = rp["lowRolloffMax"].toDouble(2.0);
        rolloffProcessor.highRolloffMin = rp["highRolloffMin"].toDouble(0.0);
        rolloffProcessor.highRolloffMax = rp["highRolloffMax"].toDouble(3.0);
        rolloffProcessor.crossoverMin = rp["crossoverMin"].toDouble(1.0);
        rolloffProcessor.crossoverMax = rp["crossoverMax"].toDouble(32.0);
        rolloffProcessor.transitionMin = rp["transitionMin"].toDouble(0.0);
        rolloffProcessor.transitionMax = rp["transitionMax"].toDouble(16.0);
    }

    // Spectrum to Signal
    if (json.contains("spectrumToSignal")) {
        QJsonObject sts = json["spectrumToSignal"].toObject();
        spectrumToSignal.normalizeMin = sts["normalizeMin"].toDouble(0.0);
        spectrumToSignal.normalizeMax = sts["normalizeMax"].toDouble(1.0);
    }

    // Formant Body
    if (json.contains("formantBody")) {
        QJsonObject fb = json["formantBody"].toObject();
        formantBody.f1FreqMin = fb["f1FreqMin"].toDouble(200.0);
        formantBody.f1FreqMax = fb["f1FreqMax"].toDouble(1000.0);
        formantBody.f2FreqMin = fb["f2FreqMin"].toDouble(800.0);
        formantBody.f2FreqMax = fb["f2FreqMax"].toDouble(3000.0);
        formantBody.qMin = fb["qMin"].toDouble(1.0);
        formantBody.qMax = fb["qMax"].toDouble(20.0);
        formantBody.mixMin = fb["mixMin"].toDouble(0.0);
        formantBody.mixMax = fb["mixMax"].toDouble(1.0);
    }

    // Breath Turbulence
    if (json.contains("breathTurbulence")) {
        QJsonObject bt = json["breathTurbulence"].toObject();
        breathTurbulence.blendMin = bt["blendMin"].toDouble(0.0);
        breathTurbulence.blendMax = bt["blendMax"].toDouble(1.0);
    }

    // Noise Color Filter
    if (json.contains("noiseColorFilter")) {
        QJsonObject ncf = json["noiseColorFilter"].toObject();
        noiseColorFilter.colorMin = ncf["colorMin"].toDouble(100.0);
        noiseColorFilter.colorMax = ncf["colorMax"].toDouble(8000.0);
        noiseColorFilter.filterQMin = ncf["filterQMin"].toDouble(0.5);
        noiseColorFilter.filterQMax = ncf["filterQMax"].toDouble(10.0);
    }

    // Physics System
    if (json.contains("physicsSystem")) {
        QJsonObject ps = json["physicsSystem"].toObject();
        physicsSystem.massMin = ps["massMin"].toDouble(0.0);
        physicsSystem.massMax = ps["massMax"].toDouble(10.0);
        physicsSystem.springKMin = ps["springKMin"].toDouble(0.0001);
        physicsSystem.springKMax = ps["springKMax"].toDouble(1.0);
        physicsSystem.dampingMin = ps["dampingMin"].toDouble(0.5);
        physicsSystem.dampingMax = ps["dampingMax"].toDouble(0.9999);
        physicsSystem.impulseMin = ps["impulseMin"].toDouble(0.0);
        physicsSystem.impulseMax = ps["impulseMax"].toDouble(1000.0);
    }

    // Drift Engine
    if (json.contains("driftEngine")) {
        QJsonObject de = json["driftEngine"].toObject();
        driftEngine.amountMin = de["amountMin"].toDouble(0.0);
        driftEngine.amountMax = de["amountMax"].toDouble(0.1);
        driftEngine.rateMin = de["rateMin"].toDouble(0.01);
        driftEngine.rateMax = de["rateMax"].toDouble(10.0);
    }

    // Gate Processor
    if (json.contains("gateProcessor")) {
        QJsonObject gp = json["gateProcessor"].toObject();
        gateProcessor.velocityMin = gp["velocityMin"].toDouble(0.0);
        gateProcessor.velocityMax = gp["velocityMax"].toDouble(1.0);
        gateProcessor.attackTimeMin = gp["attackTimeMin"].toDouble(0.001);
        gateProcessor.attackTimeMax = gp["attackTimeMax"].toDouble(1.0);
        gateProcessor.releaseTimeMin = gp["releaseTimeMin"].toDouble(0.001);
        gateProcessor.releaseTimeMax = gp["releaseTimeMax"].toDouble(2.0);
        gateProcessor.velocitySensMin = gp["velocitySensMin"].toDouble(0.0);
        gateProcessor.velocitySensMax = gp["velocitySensMax"].toDouble(1.0);
    }

    // Karplus Strong
    if (json.contains("karplusStrong")) {
        QJsonObject ks = json["karplusStrong"].toObject();
        karplusStrong.attackPortionMin = ks["attackPortionMin"].toDouble(0.01);
        karplusStrong.attackPortionMax = ks["attackPortionMax"].toDouble(1.0);
        karplusStrong.dampingMin = ks["dampingMin"].toDouble(0.0);
        karplusStrong.dampingMax = ks["dampingMax"].toDouble(1.0);
        karplusStrong.pluckPositionMin = ks["pluckPositionMin"].toDouble(0.0);
        karplusStrong.pluckPositionMax = ks["pluckPositionMax"].toDouble(1.0);
        karplusStrong.mixMin = ks["mixMin"].toDouble(0.0);
        karplusStrong.mixMax = ks["mixMax"].toDouble(1.0);
        karplusStrong.brightnessMin = ks["brightnessMin"].toDouble(0.0);
        karplusStrong.brightnessMax = ks["brightnessMax"].toDouble(1.0);
        karplusStrong.excitationSoftnessMin = ks["excitationSoftnessMin"].toDouble(0.0);
        karplusStrong.excitationSoftnessMax = ks["excitationSoftnessMax"].toDouble(1.0);
        karplusStrong.pluckHardnessMin = ks["pluckHardnessMin"].toDouble(0.0);
        karplusStrong.pluckHardnessMax = ks["pluckHardnessMax"].toDouble(1.0);
        karplusStrong.bodyResonanceMin = ks["bodyResonanceMin"].toDouble(0.0);
        karplusStrong.bodyResonanceMax = ks["bodyResonanceMax"].toDouble(1.0);
        karplusStrong.bodyFreqMin = ks["bodyFreqMin"].toDouble(80.0);
        karplusStrong.bodyFreqMax = ks["bodyFreqMax"].toDouble(400.0);
        karplusStrong.pickDirectionMin = ks["pickDirectionMin"].toDouble(0.0);
        karplusStrong.pickDirectionMax = ks["pickDirectionMax"].toDouble(1.0);
    }

    // Attack
    if (json.contains("attack")) {
        QJsonObject atk = json["attack"].toObject();
        attack.durationMin = atk["durationMin"].toDouble(0.005);
        attack.durationMax = atk["durationMax"].toDouble(5.0);
        attack.intensityMin = atk["intensityMin"].toDouble(0.0);
        attack.intensityMax = atk["intensityMax"].toDouble(1.0);
        attack.mixMin = atk["mixMin"].toDouble(0.0);
        attack.mixMax = atk["mixMax"].toDouble(1.0);
        attack.noiseAmountMin = atk["noiseAmountMin"].toDouble(0.0);
        attack.noiseAmountMax = atk["noiseAmountMax"].toDouble(1.0);
        attack.jetRatioMin = atk["jetRatioMin"].toDouble(0.0);
        attack.jetRatioMax = atk["jetRatioMax"].toDouble(1.0);
        attack.reedStiffnessMin = atk["reedStiffnessMin"].toDouble(0.0);
        attack.reedStiffnessMax = atk["reedStiffnessMax"].toDouble(1.0);
        attack.reedApertureMin = atk["reedApertureMin"].toDouble(0.0);
        attack.reedApertureMax = atk["reedApertureMax"].toDouble(1.0);
        attack.blowPositionMin = atk["blowPositionMin"].toDouble(0.0);
        attack.blowPositionMax = atk["blowPositionMax"].toDouble(1.0);
        attack.lipTensionMin = atk["lipTensionMin"].toDouble(0.0);
        attack.lipTensionMax = atk["lipTensionMax"].toDouble(1.0);
        attack.buzzQMin = atk["buzzQMin"].toDouble(0.5);
        attack.buzzQMax = atk["buzzQMax"].toDouble(50.0);
        attack.hardnessMin = atk["hardnessMin"].toDouble(0.0);
        attack.hardnessMax = atk["hardnessMax"].toDouble(1.0);
        attack.brightnessMin = atk["brightnessMin"].toDouble(0.0);
        attack.brightnessMax = atk["brightnessMax"].toDouble(1.0);
        attack.tightnessMin = atk["tightnessMin"].toDouble(0.0);
        attack.tightnessMax = atk["tightnessMax"].toDouble(1.0);
        attack.toneMin = atk["toneMin"].toDouble(0.0);
        attack.toneMax = atk["toneMax"].toDouble(1.0);
    }

    // LFO
    if (json.contains("lfo")) {
        QJsonObject lfoJson = json["lfo"].toObject();
        lfo.frequencyMin = lfoJson["frequencyMin"].toDouble(0.01);
        lfo.frequencyMax = lfoJson["frequencyMax"].toDouble(20.0);
        lfo.amplitudeMin = lfoJson["amplitudeMin"].toDouble(0.0);
        lfo.amplitudeMax = lfoJson["amplitudeMax"].toDouble(1.0);
    }

    // Wavetable Synth
    if (json.contains("wavetableSynth")) {
        QJsonObject wts = json["wavetableSynth"].toObject();
        wavetableSynth.positionMin = wts["positionMin"].toDouble(0.0);
        wavetableSynth.positionMax = wts["positionMax"].toDouble(1.0);
    }

    // Frequency Mapper
    if (json.contains("frequencyMapper")) {
        QJsonObject fm = json["frequencyMapper"].toObject();
        frequencyMapper.lowFreqMin = fm["lowFreqMin"].toDouble(1.0);
        frequencyMapper.lowFreqMax = fm["lowFreqMax"].toDouble(500.0);
        frequencyMapper.highFreqMin = fm["highFreqMin"].toDouble(500.0);
        frequencyMapper.highFreqMax = fm["highFreqMax"].toDouble(20000.0);
        frequencyMapper.outputMinMin = fm["outputMinMin"].toDouble(-2.0);
        frequencyMapper.outputMinMax = fm["outputMinMax"].toDouble(1.0);
        frequencyMapper.outputMaxMin = fm["outputMaxMin"].toDouble(0.0);
        frequencyMapper.outputMaxMax = fm["outputMaxMax"].toDouble(2.0);
    }

    // Bandpass EQ
    if (json.contains("bandpassEQ")) {
        QJsonObject bpeq = json["bandpassEQ"].toObject();
        bandpassEQ.gainMin = bpeq["gainMin"].toDouble(0.0);
        bandpassEQ.gainMax = bpeq["gainMax"].toDouble(2.0);
        bandpassEQ.qMin = bpeq["qMin"].toDouble(0.5);
        bandpassEQ.qMax = bpeq["qMax"].toDouble(20.0);
    }

    // Comb Filter
    if (json.contains("combFilter")) {
        QJsonObject cf = json["combFilter"].toObject();
        combFilter.delayTimeMin = cf["delayTimeMin"].toDouble(0.1);
        combFilter.delayTimeMax = cf["delayTimeMax"].toDouble(100.0);
        combFilter.feedbackMin = cf["feedbackMin"].toDouble(-0.99);
        combFilter.feedbackMax = cf["feedbackMax"].toDouble(0.99);
        combFilter.dampingMin = cf["dampingMin"].toDouble(0.0);
        combFilter.dampingMax = cf["dampingMax"].toDouble(1.0);
    }

    // LP/HP Filter
    if (json.contains("lowHighPassFilter")) {
        QJsonObject lhpf = json["lowHighPassFilter"].toObject();
        lowHighPassFilter.cutoffMin = lhpf["cutoffMin"].toDouble(20.0);
        lowHighPassFilter.cutoffMax = lhpf["cutoffMax"].toDouble(20000.0);
        lowHighPassFilter.resonanceMin = lhpf["resonanceMin"].toDouble(0.5);
        lowHighPassFilter.resonanceMax = lhpf["resonanceMax"].toDouble(20.0);
    }

    // Recorder
    if (json.contains("recorder")) {
        QJsonObject rec = json["recorder"].toObject();
        recorder.breathPressureMin = rec["breathPressureMin"].toDouble(0.0);
        recorder.breathPressureMax = rec["breathPressureMax"].toDouble(1.0);
        recorder.jetRatioMin       = rec["jetRatioMin"]      .toDouble(0.05);
        recorder.jetRatioMax       = rec["jetRatioMax"]      .toDouble(0.60);
        recorder.noiseGainMin      = rec["noiseGainMin"]     .toDouble(0.0);
        recorder.noiseGainMax      = rec["noiseGainMax"]     .toDouble(0.5);
        recorder.vibratoFreqMin    = rec["vibratoFreqMin"]   .toDouble(0.0);
        recorder.vibratoFreqMax    = rec["vibratoFreqMax"]   .toDouble(20.0);
        recorder.vibratoGainMin    = rec["vibratoGainMin"]   .toDouble(0.0);
        recorder.vibratoGainMax    = rec["vibratoGainMax"]   .toDouble(1.0);
        recorder.endReflectionMin  = rec["endReflectionMin"] .toDouble(-1.0);
        recorder.endReflectionMax  = rec["endReflectionMax"] .toDouble( 1.0);
        recorder.jetReflectionMin  = rec["jetReflectionMin"] .toDouble(-1.0);
        recorder.jetReflectionMax  = rec["jetReflectionMax"] .toDouble( 1.0);
    }

    // Reed
    if (json.contains("reed") || json.contains("saxophone")) {
        QJsonObject sax = (json.contains("reed") ? json["reed"] : json["saxophone"]).toObject();
        reed.breathPressureMin = sax["breathPressureMin"].toDouble(0.0);
        reed.breathPressureMax = sax["breathPressureMax"].toDouble(1.0);
        reed.reedStiffnessMin  = sax["reedStiffnessMin"] .toDouble(0.0);
        reed.reedStiffnessMax  = sax["reedStiffnessMax"] .toDouble(1.0);
        reed.reedApertureMin   = sax["reedApertureMin"]  .toDouble(0.0);
        reed.reedApertureMax   = sax["reedApertureMax"]  .toDouble(1.0);
        reed.blowPositionMin   = sax["blowPositionMin"]  .toDouble(0.0);
        reed.blowPositionMax   = sax["blowPositionMax"]  .toDouble(1.0);
        reed.noiseGainMin      = sax["noiseGainMin"]     .toDouble(0.0);
        reed.noiseGainMax      = sax["noiseGainMax"]     .toDouble(0.4);
        reed.vibratoFreqMin    = sax["vibratoFreqMin"]   .toDouble(0.0);
        reed.vibratoFreqMax    = sax["vibratoFreqMax"]   .toDouble(12.0);
        reed.vibratoGainMin    = sax["vibratoGainMin"]   .toDouble(0.0);
        reed.vibratoGainMax    = sax["vibratoGainMax"]   .toDouble(0.5);
        reed.nlTypeMin         = sax["nlTypeMin"]         .toDouble(0.0);
        reed.nlTypeMax         = sax["nlTypeMax"]         .toDouble(4.0);
        reed.nlAmountMin       = sax["nlAmountMin"]       .toDouble(0.0);
        reed.nlAmountMax       = sax["nlAmountMax"]       .toDouble(1.0);
        reed.nlFreqModMin      = sax["nlFreqModMin"]      .toDouble(0.0);
        reed.nlFreqModMax      = sax["nlFreqModMax"]      .toDouble(200.0);
        reed.nlAttackMin       = sax["nlAttackMin"]       .toDouble(0.001);
        reed.nlAttackMax       = sax["nlAttackMax"]       .toDouble(2.0);
    }

    // Easing
    if (json.contains("easing")) {
        QJsonObject easingJson = json["easing"].toObject();
        easing.overshootDefault = easingJson["overshootDefault"].toDouble(0.5);
        easing.overshootMin = easingJson["overshootMin"].toDouble(0.0);
        easing.overshootMax = easingJson["overshootMax"].toDouble(2.0);
        easing.amplitudeDefault = easingJson["amplitudeDefault"].toDouble(0.5);
        easing.amplitudeMin = easingJson["amplitudeMin"].toDouble(0.1);
        easing.amplitudeMax = easingJson["amplitudeMax"].toDouble(2.0);
        easing.periodDefault = easingJson["periodDefault"].toDouble(0.4);
        easing.periodMin = easingJson["periodMin"].toDouble(0.1);
        easing.periodMax = easingJson["periodMax"].toDouble(1.0);
        easing.frequencyDefault = easingJson["frequencyDefault"].toDouble(2.0);
        easing.frequencyMin = easingJson["frequencyMin"].toDouble(0.5);
        easing.frequencyMax = easingJson["frequencyMax"].toDouble(5.0);
        easing.decayDefault = easingJson["decayDefault"].toDouble(0.7);
        easing.decayMin = easingJson["decayMin"].toDouble(0.1);
        easing.decayMax = easingJson["decayMax"].toDouble(1.0);
    }

    // Also save to QSettings so changes persist
    saveSettings();
    qDebug() << "ContainerSettings loaded from project JSON";
}
