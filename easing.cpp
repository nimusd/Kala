#include "easing.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// Constructors
// ============================================================================

Easing::Easing()
    : type(Type::Linear)
{
    setNameFromType();
}

Easing::Easing(Type type)
    : type(type)
{
    setNameFromType();
}

void Easing::setNameFromType()
{
    switch (type) {
        case Type::Linear:       name = "Linear"; break;
        case Type::QuadIn:       name = "Quad In"; power = 2.0; break;
        case Type::QuadOut:      name = "Quad Out"; power = 2.0; break;
        case Type::QuadInOut:    name = "Quad In-Out"; power = 2.0; break;
        case Type::CubicIn:      name = "Cubic In"; power = 3.0; break;
        case Type::CubicOut:     name = "Cubic Out"; power = 3.0; break;
        case Type::CubicInOut:   name = "Cubic In-Out"; power = 3.0; break;
        case Type::QuarticIn:    name = "Quartic In"; power = 4.0; break;
        case Type::QuarticOut:   name = "Quartic Out"; power = 4.0; break;
        case Type::QuarticInOut: name = "Quartic In-Out"; power = 4.0; break;
        case Type::SineIn:       name = "Sine In"; break;
        case Type::SineOut:      name = "Sine Out"; break;
        case Type::SineInOut:    name = "Sine In-Out"; break;
        case Type::ExpoIn:       name = "Expo In"; break;
        case Type::ExpoOut:      name = "Expo Out"; break;
        case Type::ExpoInOut:    name = "Expo In-Out"; break;
        case Type::CircIn:       name = "Circ In"; break;
        case Type::CircOut:      name = "Circ Out"; break;
        case Type::CircInOut:    name = "Circ In-Out"; break;
        case Type::BackIn:       name = "Back In"; break;
        case Type::BackOut:      name = "Back Out"; break;
        case Type::BackInOut:    name = "Back In-Out"; break;
        case Type::ElasticIn:    name = "Elastic In"; break;
        case Type::ElasticOut:   name = "Elastic Out"; break;
        case Type::ElasticInOut: name = "Elastic In-Out"; break;
        case Type::BounceIn:     name = "Bounce In"; break;
        case Type::BounceOut:    name = "Bounce Out"; break;
        case Type::BounceInOut:  name = "Bounce In-Out"; break;
        case Type::Wobble:       name = "Wobble"; break;
        default:                 name = "Linear"; break;
    }
}

// ============================================================================
// Core calculation
// ============================================================================

double Easing::calculate(double t) const
{
    // Clamp input to 0-1 (output may exceed this for overshooting easings)
    t = std::clamp(t, 0.0, 1.0);

    switch (type) {
        case Type::Linear:       return calcLinear(t);
        case Type::QuadIn:       return calcQuadIn(t);
        case Type::QuadOut:      return calcQuadOut(t);
        case Type::QuadInOut:    return calcQuadInOut(t);
        case Type::CubicIn:      return calcCubicIn(t);
        case Type::CubicOut:     return calcCubicOut(t);
        case Type::CubicInOut:   return calcCubicInOut(t);
        case Type::QuarticIn:    return calcQuarticIn(t);
        case Type::QuarticOut:   return calcQuarticOut(t);
        case Type::QuarticInOut: return calcQuarticInOut(t);
        case Type::SineIn:       return calcSineIn(t);
        case Type::SineOut:      return calcSineOut(t);
        case Type::SineInOut:    return calcSineInOut(t);
        case Type::ExpoIn:       return calcExpoIn(t);
        case Type::ExpoOut:      return calcExpoOut(t);
        case Type::ExpoInOut:    return calcExpoInOut(t);
        case Type::CircIn:       return calcCircIn(t);
        case Type::CircOut:      return calcCircOut(t);
        case Type::CircInOut:    return calcCircInOut(t);
        case Type::BackIn:       return calcBackIn(t);
        case Type::BackOut:      return calcBackOut(t);
        case Type::BackInOut:    return calcBackInOut(t);
        case Type::ElasticIn:    return calcElasticIn(t);
        case Type::ElasticOut:   return calcElasticOut(t);
        case Type::ElasticInOut: return calcElasticInOut(t);
        case Type::BounceIn:     return calcBounceIn(t);
        case Type::BounceOut:    return calcBounceOut(t);
        case Type::BounceInOut:  return calcBounceInOut(t);
        case Type::Wobble:       return calcWobble(t);
        default:                 return t;
    }
}

QString Easing::getCategory() const
{
    switch (type) {
        case Type::Linear:
            return "Linear";
        case Type::QuadIn:
        case Type::QuadOut:
        case Type::QuadInOut:
            return "Quadratic";
        case Type::CubicIn:
        case Type::CubicOut:
        case Type::CubicInOut:
            return "Cubic";
        case Type::QuarticIn:
        case Type::QuarticOut:
        case Type::QuarticInOut:
            return "Quartic";
        case Type::SineIn:
        case Type::SineOut:
        case Type::SineInOut:
            return "Sine";
        case Type::ExpoIn:
        case Type::ExpoOut:
        case Type::ExpoInOut:
            return "Exponential";
        case Type::CircIn:
        case Type::CircOut:
        case Type::CircInOut:
            return "Circular";
        case Type::BackIn:
        case Type::BackOut:
        case Type::BackInOut:
            return "Back";
        case Type::ElasticIn:
        case Type::ElasticOut:
        case Type::ElasticInOut:
            return "Elastic";
        case Type::BounceIn:
        case Type::BounceOut:
        case Type::BounceInOut:
            return "Bounce";
        case Type::Wobble:
            return "Wobble";
        default:
            return "Unknown";
    }
}

bool Easing::canOvershoot() const
{
    switch (type) {
        case Type::BackIn:
        case Type::BackOut:
        case Type::BackInOut:
        case Type::ElasticIn:
        case Type::ElasticOut:
        case Type::ElasticInOut:
            return true;
        default:
            return false;
    }
}

bool Easing::hasParameters() const
{
    switch (type) {
        case Type::QuadIn:
        case Type::QuadOut:
        case Type::QuadInOut:
        case Type::CubicIn:
        case Type::CubicOut:
        case Type::CubicInOut:
        case Type::QuarticIn:
        case Type::QuarticOut:
        case Type::QuarticInOut:
        case Type::SineIn:
        case Type::SineOut:
        case Type::SineInOut:
        case Type::ExpoIn:
        case Type::ExpoOut:
        case Type::ExpoInOut:
        case Type::CircIn:
        case Type::CircOut:
        case Type::CircInOut:
        case Type::BackIn:
        case Type::BackOut:
        case Type::BackInOut:
        case Type::ElasticIn:
        case Type::ElasticOut:
        case Type::ElasticInOut:
        case Type::BounceIn:
        case Type::BounceOut:
        case Type::BounceInOut:
        case Type::Wobble:
            return true;
        default:
            return false;
    }
}

void Easing::setOvershoot(double value)
{
    overshoot = std::clamp(value, 0.0, 5.0);
}

void Easing::setAmplitude(double value)
{
    amplitude = std::clamp(value, 0.1, 3.0);
}

void Easing::setPeriod(double value)
{
    period = std::clamp(value, 0.1, 2.0);
}

void Easing::setFrequency(double value)
{
    frequency = std::clamp(value, 1.0, 10.0);
}

void Easing::setDecay(double value)
{
    decay = std::clamp(value, 0.1, 2.0);
}

void Easing::setSineAmplitude(double value)
{
    sineAmplitude = std::clamp(value, 0.1, 3.0);
}

void Easing::setSineCycles(double value)
{
    sineCycles = std::clamp(value, 0.25, 4.0);
}

void Easing::setPower(double value)
{
    power = std::clamp(value, 0.5, 8.0);
}

void Easing::setExpoStrength(double value)
{
    expoStrength = std::clamp(value, 1.0, 20.0);
}

void Easing::setCircStrength(double value)
{
    circStrength = std::clamp(value, 0.1, 3.0);
}

void Easing::setBounceCount(double value)
{
    bounceCount = std::clamp(value, 1.0, 8.0);
}

void Easing::setBounceStrength(double value)
{
    bounceStrength = std::clamp(value, 1.0, 20.0);
}

// ============================================================================
// Easing calculations - Linear
// ============================================================================

double Easing::calcLinear(double t) const
{
    return t;
}

// ============================================================================
// Easing calculations - Quadratic
// Uses 'power' parameter (default 2.0 for quadratic)
// ============================================================================

double Easing::calcQuadIn(double t) const
{
    return std::pow(t, power);
}

double Easing::calcQuadOut(double t) const
{
    return 1.0 - std::pow(1.0 - t, power);
}

double Easing::calcQuadInOut(double t) const
{
    if (t < 0.5) {
        return std::pow(2.0, power - 1.0) * std::pow(t, power);
    } else {
        return 1.0 - std::pow(-2.0 * t + 2.0, power) / 2.0;
    }
}

// ============================================================================
// Easing calculations - Cubic
// Uses 'power' parameter (default 3.0 for cubic)
// ============================================================================

double Easing::calcCubicIn(double t) const
{
    return std::pow(t, power);
}

double Easing::calcCubicOut(double t) const
{
    return 1.0 - std::pow(1.0 - t, power);
}

double Easing::calcCubicInOut(double t) const
{
    if (t < 0.5) {
        return std::pow(2.0, power - 1.0) * std::pow(t, power);
    } else {
        return 1.0 - std::pow(-2.0 * t + 2.0, power) / 2.0;
    }
}

// ============================================================================
// Easing calculations - Quartic
// Uses 'power' parameter (default 4.0 for quartic)
// ============================================================================

double Easing::calcQuarticIn(double t) const
{
    return std::pow(t, power);
}

double Easing::calcQuarticOut(double t) const
{
    return 1.0 - std::pow(1.0 - t, power);
}

double Easing::calcQuarticInOut(double t) const
{
    if (t < 0.5) {
        return std::pow(2.0, power - 1.0) * std::pow(t, power);
    } else {
        return 1.0 - std::pow(-2.0 * t + 2.0, power) / 2.0;
    }
}

// ============================================================================
// Easing calculations - Sine
// Uses 'sineAmplitude' and 'sineCycles' parameters
// sineCycles: 0.5 = quarter wave (default ease), 1.0 = half wave, 2.0 = full wave, etc.
// sineAmplitude: intensity of the easing effect (1.0 = normal)
// ============================================================================

double Easing::calcSineIn(double t) const
{
    // Base sine-in with configurable cycles
    double angle = t * M_PI * sineCycles;
    double sineValue = 1.0 - std::cos(angle);

    // Normalize so it ends at 1.0 when t=1
    double normalizedEnd = 1.0 - std::cos(M_PI * sineCycles);
    if (std::abs(normalizedEnd) > 0.0001) {
        sineValue /= normalizedEnd;
    }

    // Apply amplitude (blend between linear and sine)
    return t + (sineValue - t) * sineAmplitude;
}

double Easing::calcSineOut(double t) const
{
    // Base sine-out with configurable cycles
    double angle = t * M_PI * sineCycles;
    double sineValue = std::sin(angle);

    // Normalize so it ends at 1.0 when t=1
    double normalizedEnd = std::sin(M_PI * sineCycles);
    if (std::abs(normalizedEnd) > 0.0001) {
        sineValue /= normalizedEnd;
    }

    // Apply amplitude (blend between linear and sine)
    return t + (sineValue - t) * sineAmplitude;
}

double Easing::calcSineInOut(double t) const
{
    // Base sine-in-out with configurable cycles
    double angle = t * M_PI * sineCycles * 2.0;
    double sineValue = -(std::cos(angle) - 1.0) / 2.0;

    // Normalize so it ends at 1.0 when t=1
    double normalizedEnd = -(std::cos(M_PI * sineCycles * 2.0) - 1.0) / 2.0;
    if (std::abs(normalizedEnd) > 0.0001) {
        sineValue /= normalizedEnd;
    }

    // Apply amplitude (blend between linear and sine)
    return t + (sineValue - t) * sineAmplitude;
}

// ============================================================================
// Easing calculations - Exponential
// Uses 'expoStrength' parameter (default 10.0)
// ============================================================================

double Easing::calcExpoIn(double t) const
{
    if (t == 0.0) return 0.0;
    return std::pow(2.0, expoStrength * (t - 1.0));
}

double Easing::calcExpoOut(double t) const
{
    if (t == 1.0) return 1.0;
    return 1.0 - std::pow(2.0, -expoStrength * t);
}

double Easing::calcExpoInOut(double t) const
{
    if (t == 0.0) return 0.0;
    if (t == 1.0) return 1.0;
    if (t < 0.5) {
        return std::pow(2.0, 2.0 * expoStrength * t - expoStrength) / 2.0;
    } else {
        return (2.0 - std::pow(2.0, -2.0 * expoStrength * t + expoStrength)) / 2.0;
    }
}

// ============================================================================
// Easing calculations - Circular
// Uses 'circStrength' parameter (default 1.0)
// Strength controls the curvature intensity
// ============================================================================

double Easing::calcCircIn(double t) const
{
    double circValue = 1.0 - std::sqrt(1.0 - t * t);
    // Blend between linear and circular based on strength
    return t + (circValue - t) * circStrength;
}

double Easing::calcCircOut(double t) const
{
    double circValue = std::sqrt(1.0 - std::pow(t - 1.0, 2.0));
    // Blend between linear and circular based on strength
    return t + (circValue - t) * circStrength;
}

double Easing::calcCircInOut(double t) const
{
    double circValue;
    if (t < 0.5) {
        circValue = (1.0 - std::sqrt(1.0 - std::pow(2.0 * t, 2.0))) / 2.0;
    } else {
        circValue = (std::sqrt(1.0 - std::pow(-2.0 * t + 2.0, 2.0)) + 1.0) / 2.0;
    }
    // Blend between linear and circular based on strength
    return t + (circValue - t) * circStrength;
}

// ============================================================================
// Easing calculations - Back (overshoots)
// Uses 'overshoot' parameter (default 1.70158)
// ============================================================================

double Easing::calcBackIn(double t) const
{
    const double c1 = overshoot;
    const double c3 = c1 + 1.0;
    return c3 * t * t * t - c1 * t * t;
}

double Easing::calcBackOut(double t) const
{
    const double c1 = overshoot;
    const double c3 = c1 + 1.0;
    return 1.0 + c3 * std::pow(t - 1.0, 3.0) + c1 * std::pow(t - 1.0, 2.0);
}

double Easing::calcBackInOut(double t) const
{
    const double c1 = overshoot;
    const double c2 = c1 * 1.525;
    if (t < 0.5) {
        return (std::pow(2.0 * t, 2.0) * ((c2 + 1.0) * 2.0 * t - c2)) / 2.0;
    } else {
        return (std::pow(2.0 * t - 2.0, 2.0) * ((c2 + 1.0) * (t * 2.0 - 2.0) + c2) + 2.0) / 2.0;
    }
}

// ============================================================================
// Easing calculations - Elastic (spring-like)
// Uses 'amplitude' and 'period' parameters
// ============================================================================

double Easing::calcElasticIn(double t) const
{
    if (t == 0.0) return 0.0;
    if (t == 1.0) return 1.0;
    const double c4 = (2.0 * M_PI) / period;
    return -amplitude * std::pow(2.0, 10.0 * t - 10.0) * std::sin((t * 10.0 - 10.75) * c4 / (2.0 * M_PI) * period);
}

double Easing::calcElasticOut(double t) const
{
    if (t == 0.0) return 0.0;
    if (t == 1.0) return 1.0;
    const double c4 = (2.0 * M_PI) / period;
    return amplitude * std::pow(2.0, -10.0 * t) * std::sin((t * 10.0 - 0.75) * c4 / (2.0 * M_PI) * period) + 1.0;
}

double Easing::calcElasticInOut(double t) const
{
    if (t == 0.0) return 0.0;
    if (t == 1.0) return 1.0;
    const double c5 = (2.0 * M_PI) / (period * 1.5);
    if (t < 0.5) {
        return -amplitude * (std::pow(2.0, 20.0 * t - 10.0) * std::sin((20.0 * t - 11.125) * c5 / (2.0 * M_PI) * period * 1.5)) / 2.0;
    } else {
        return amplitude * (std::pow(2.0, -20.0 * t + 10.0) * std::sin((20.0 * t - 11.125) * c5 / (2.0 * M_PI) * period * 1.5)) / 2.0 + 1.0;
    }
}

// ============================================================================
// Easing calculations - Bounce
// Uses 'bounceStrength' (default 7.5625) and 'bounceCount' (default 4) parameters
// ============================================================================

double Easing::calcBounceOut(double t) const
{
    const double n1 = bounceStrength;
    // Calculate divisor based on bounce count
    // bounceCount of 4 gives d1 = 2.75 (original), 2 gives ~1.5, 8 gives ~5.5
    const double d1 = 0.25 + bounceCount * 0.625;

    if (t < 1.0 / d1) {
        return n1 * t * t;
    } else if (t < 2.0 / d1) {
        double t2 = t - 1.5 / d1;
        return n1 * t2 * t2 + 0.75;
    } else if (t < 2.5 / d1) {
        double t2 = t - 2.25 / d1;
        return n1 * t2 * t2 + 0.9375;
    } else {
        double t2 = t - 2.625 / d1;
        return n1 * t2 * t2 + 0.984375;
    }
}

double Easing::calcBounceIn(double t) const
{
    return 1.0 - calcBounceOut(1.0 - t);
}

double Easing::calcBounceInOut(double t) const
{
    if (t < 0.5) {
        return (1.0 - calcBounceOut(1.0 - 2.0 * t)) / 2.0;
    } else {
        return (1.0 + calcBounceOut(2.0 * t - 1.0)) / 2.0;
    }
}

// ============================================================================
// Easing calculations - Wobble (decaying oscillation)
// Uses 'frequency' and 'decay' parameters
// ============================================================================

double Easing::calcWobble(double t) const
{
    // Base ease-out motion (cubic)
    double baseEase = 1.0 - std::pow(1.0 - t, 3.0);

    // Add decaying sine wave oscillation
    double oscillation = std::sin(frequency * M_PI * t) * std::exp(-decay * t);

    // Combine base motion with wobble (oscillation fades as t approaches 1)
    return baseEase + oscillation * 0.15 * (1.0 - t);
}

// ============================================================================
// Static factory methods
// ============================================================================

Easing Easing::linear()       { return Easing(Type::Linear); }

Easing Easing::quadIn()       { return Easing(Type::QuadIn); }
Easing Easing::quadOut()      { return Easing(Type::QuadOut); }
Easing Easing::quadInOut()    { return Easing(Type::QuadInOut); }

Easing Easing::cubicIn()      { return Easing(Type::CubicIn); }
Easing Easing::cubicOut()     { return Easing(Type::CubicOut); }
Easing Easing::cubicInOut()   { return Easing(Type::CubicInOut); }

Easing Easing::quarticIn()    { return Easing(Type::QuarticIn); }
Easing Easing::quarticOut()   { return Easing(Type::QuarticOut); }
Easing Easing::quarticInOut() { return Easing(Type::QuarticInOut); }

Easing Easing::sineIn()       { return Easing(Type::SineIn); }
Easing Easing::sineOut()      { return Easing(Type::SineOut); }
Easing Easing::sineInOut()    { return Easing(Type::SineInOut); }

Easing Easing::expoIn()       { return Easing(Type::ExpoIn); }
Easing Easing::expoOut()      { return Easing(Type::ExpoOut); }
Easing Easing::expoInOut()    { return Easing(Type::ExpoInOut); }

Easing Easing::circIn()       { return Easing(Type::CircIn); }
Easing Easing::circOut()      { return Easing(Type::CircOut); }
Easing Easing::circInOut()    { return Easing(Type::CircInOut); }

Easing Easing::backIn()       { return Easing(Type::BackIn); }
Easing Easing::backOut()      { return Easing(Type::BackOut); }
Easing Easing::backInOut()    { return Easing(Type::BackInOut); }

Easing Easing::elasticIn()    { return Easing(Type::ElasticIn); }
Easing Easing::elasticOut()   { return Easing(Type::ElasticOut); }
Easing Easing::elasticInOut() { return Easing(Type::ElasticInOut); }

Easing Easing::bounceIn()     { return Easing(Type::BounceIn); }
Easing Easing::bounceOut()    { return Easing(Type::BounceOut); }
Easing Easing::bounceInOut()  { return Easing(Type::BounceInOut); }

Easing Easing::wobble()       { return Easing(Type::Wobble); }

// ============================================================================
// Collection methods
// ============================================================================

QVector<Easing> Easing::getAllEasings()
{
    QVector<Easing> easings;
    easings.reserve(static_cast<int>(Type::Count));

    for (int i = 0; i < static_cast<int>(Type::Count); ++i) {
        easings.append(Easing(static_cast<Type>(i)));
    }

    return easings;
}

Easing Easing::getById(int id)
{
    if (id >= 0 && id < static_cast<int>(Type::Count)) {
        return Easing(static_cast<Type>(id));
    }
    return Easing(Type::Linear);
}

Easing Easing::getByName(const QString &name)
{
    auto allEasings = getAllEasings();
    for (const auto &easing : allEasings) {
        if (easing.getName().compare(name, Qt::CaseInsensitive) == 0) {
            return easing;
        }
    }
    return Easing(Type::Linear);
}

// ============================================================================
// Serialization
// ============================================================================

QJsonObject Easing::toJson() const
{
    QJsonObject json;
    json["id"] = getId();
    json["name"] = name;

    // Include parameters if this easing type uses them
    if (hasParameters()) {
        json["overshoot"] = overshoot;
        json["amplitude"] = amplitude;
        json["period"] = period;
        json["frequency"] = frequency;
        json["decay"] = decay;
        json["sineAmplitude"] = sineAmplitude;
        json["sineCycles"] = sineCycles;
        json["power"] = power;
        json["expoStrength"] = expoStrength;
        json["circStrength"] = circStrength;
        json["bounceCount"] = bounceCount;
        json["bounceStrength"] = bounceStrength;
    }

    return json;
}

Easing Easing::fromJson(const QJsonObject &json)
{
    int id = json["id"].toInt(0);
    Easing easing = getById(id);

    // Load parameters if present
    if (json.contains("overshoot")) {
        easing.setOvershoot(json["overshoot"].toDouble(getDefaultOvershoot()));
    }
    if (json.contains("amplitude")) {
        easing.setAmplitude(json["amplitude"].toDouble(getDefaultAmplitude()));
    }
    if (json.contains("period")) {
        easing.setPeriod(json["period"].toDouble(getDefaultPeriod()));
    }
    if (json.contains("frequency")) {
        easing.setFrequency(json["frequency"].toDouble(getDefaultFrequency()));
    }
    if (json.contains("decay")) {
        easing.setDecay(json["decay"].toDouble(getDefaultDecay()));
    }
    if (json.contains("sineAmplitude")) {
        easing.setSineAmplitude(json["sineAmplitude"].toDouble(getDefaultSineAmplitude()));
    }
    if (json.contains("sineCycles")) {
        easing.setSineCycles(json["sineCycles"].toDouble(getDefaultSineCycles()));
    }
    if (json.contains("power")) {
        easing.setPower(json["power"].toDouble(getDefaultPower()));
    }
    if (json.contains("expoStrength")) {
        easing.setExpoStrength(json["expoStrength"].toDouble(getDefaultExpoStrength()));
    }
    if (json.contains("circStrength")) {
        easing.setCircStrength(json["circStrength"].toDouble(getDefaultCircStrength()));
    }
    if (json.contains("bounceCount")) {
        easing.setBounceCount(json["bounceCount"].toDouble(getDefaultBounceCount()));
    }
    if (json.contains("bounceStrength")) {
        easing.setBounceStrength(json["bounceStrength"].toDouble(getDefaultBounceStrength()));
    }

    return easing;
}
