#ifndef EASING_H
#define EASING_H

#include <QString>
#include <QVector>
#include <QJsonObject>

/**
 * Easing - Pure mathematical easing functions
 *
 * Standalone easing function library that can be used by:
 * - Sound Engine (EasingApplicator container)
 * - Score (note/group timing modification)
 *
 * Each easing transforms a normalized time value t (0-1) to an eased value.
 * Some easings (Back, Elastic) may return values outside 0-1 range
 * (overshoot/anticipation effects).
 */
class Easing
{
public:
    enum class Type {
        // Linear (1)
        Linear = 0,

        // Quadratic (3)
        QuadIn,
        QuadOut,
        QuadInOut,

        // Cubic (3)
        CubicIn,
        CubicOut,
        CubicInOut,

        // Quartic (3)
        QuarticIn,
        QuarticOut,
        QuarticInOut,

        // Sine (3)
        SineIn,
        SineOut,
        SineInOut,

        // Exponential (3)
        ExpoIn,
        ExpoOut,
        ExpoInOut,

        // Circular (3)
        CircIn,
        CircOut,
        CircInOut,

        // Back - overshoots (3)
        BackIn,
        BackOut,
        BackInOut,

        // Elastic - spring-like (3)
        ElasticIn,
        ElasticOut,
        ElasticInOut,

        // Bounce - bouncing ball (3)
        BounceIn,
        BounceOut,
        BounceInOut,

        // Wobble - decaying oscillation (1)
        Wobble,

        Count  // Total count marker
    };

    // Default constructor (Linear)
    Easing();

    // Constructor with type
    explicit Easing(Type type);

    // Core function: calculate eased value for input t (0-1)
    // Output may be outside 0-1 for overshooting easings
    double calculate(double t) const;

    // Getters
    Type getType() const { return type; }
    int getId() const { return static_cast<int>(type); }
    QString getName() const { return name; }
    QString getCategory() const;

    // Does this easing potentially go outside 0-1 range?
    bool canOvershoot() const;

    // Does this easing type support custom parameters?
    bool hasParameters() const;

    // Parameter access (only meaningful for Back, Elastic, Bounce)
    // Back: overshoot controls how far it goes past the target
    double getOvershoot() const { return overshoot; }
    void setOvershoot(double value);

    // Elastic: amplitude and period control the spring behavior
    double getAmplitude() const { return amplitude; }
    void setAmplitude(double value);
    double getPeriod() const { return period; }
    void setPeriod(double value);

    // Wobble: frequency and decay control the oscillation
    double getFrequency() const { return frequency; }
    void setFrequency(double value);
    double getDecay() const { return decay; }
    void setDecay(double value);

    // Sine: amplitude and cycles control the wave
    double getSineAmplitude() const { return sineAmplitude; }
    void setSineAmplitude(double value);
    double getSineCycles() const { return sineCycles; }
    void setSineCycles(double value);

    // Power (Quad/Cubic/Quartic): exponent control
    double getPower() const { return power; }
    void setPower(double value);

    // Expo: strength control
    double getExpoStrength() const { return expoStrength; }
    void setExpoStrength(double value);

    // Circ: curvature control
    double getCircStrength() const { return circStrength; }
    void setCircStrength(double value);

    // Bounce: count and strength control
    double getBounceCount() const { return bounceCount; }
    void setBounceCount(double value);
    double getBounceStrength() const { return bounceStrength; }
    void setBounceStrength(double value);

    // Static factory methods - Standard easings
    static Easing linear();

    static Easing quadIn();
    static Easing quadOut();
    static Easing quadInOut();

    static Easing cubicIn();
    static Easing cubicOut();
    static Easing cubicInOut();

    static Easing quarticIn();
    static Easing quarticOut();
    static Easing quarticInOut();

    static Easing sineIn();
    static Easing sineOut();
    static Easing sineInOut();

    static Easing expoIn();
    static Easing expoOut();
    static Easing expoInOut();

    static Easing circIn();
    static Easing circOut();
    static Easing circInOut();

    static Easing backIn();
    static Easing backOut();
    static Easing backInOut();

    static Easing elasticIn();
    static Easing elasticOut();
    static Easing elasticInOut();

    static Easing bounceIn();
    static Easing bounceOut();
    static Easing bounceInOut();

    static Easing wobble();

    // Get all available easings
    static QVector<Easing> getAllEasings();

    // Get easing by ID
    static Easing getById(int id);

    // Get easing by name
    static Easing getByName(const QString &name);

    // Serialization (includes parameters)
    QJsonObject toJson() const;
    static Easing fromJson(const QJsonObject &json);

    // Get default parameter values for reference
    static double getDefaultOvershoot() { return 1.70158; }
    static double getDefaultAmplitude() { return 1.0; }
    static double getDefaultPeriod() { return 0.3; }
    static double getDefaultFrequency() { return 3.0; }
    static double getDefaultDecay() { return 0.8; }
    static double getDefaultSineAmplitude() { return 1.0; }
    static double getDefaultSineCycles() { return 0.5; }
    static double getDefaultPower() { return 2.0; }  // Quad default
    static double getDefaultExpoStrength() { return 10.0; }
    static double getDefaultCircStrength() { return 1.0; }
    static double getDefaultBounceCount() { return 4.0; }
    static double getDefaultBounceStrength() { return 7.5625; }

private:
    Type type;
    QString name;

    // Parameters for customizable easings
    double overshoot = 1.70158;  // Back: controls overshoot amount (default ~1.7)
    double amplitude = 1.0;      // Elastic: controls oscillation amplitude
    double period = 0.3;         // Elastic: controls oscillation period
    double frequency = 3.0;      // Wobble: oscillation frequency (default 3.0)
    double decay = 0.8;          // Wobble: oscillation decay rate (default 0.8)
    double sineAmplitude = 1.0;  // Sine: wave amplitude/intensity (default 1.0)
    double sineCycles = 0.5;     // Sine: number of wave cycles (default 0.5 = quarter wave)
    double power = 2.0;          // Quad/Cubic/Quartic: exponent (2=quad, 3=cubic, 4=quartic)
    double expoStrength = 10.0;  // Expo: exponential steepness (default 10)
    double circStrength = 1.0;   // Circ: circular curve intensity (default 1.0)
    double bounceCount = 4.0;    // Bounce: number of bounces (default 4)
    double bounceStrength = 7.5625; // Bounce: bounce intensity (default 7.5625)

    // Internal calculation methods
    double calcLinear(double t) const;
    double calcQuadIn(double t) const;
    double calcQuadOut(double t) const;
    double calcQuadInOut(double t) const;
    double calcCubicIn(double t) const;
    double calcCubicOut(double t) const;
    double calcCubicInOut(double t) const;
    double calcQuarticIn(double t) const;
    double calcQuarticOut(double t) const;
    double calcQuarticInOut(double t) const;
    double calcSineIn(double t) const;
    double calcSineOut(double t) const;
    double calcSineInOut(double t) const;
    double calcExpoIn(double t) const;
    double calcExpoOut(double t) const;
    double calcExpoInOut(double t) const;
    double calcCircIn(double t) const;
    double calcCircOut(double t) const;
    double calcCircInOut(double t) const;
    double calcBackIn(double t) const;
    double calcBackOut(double t) const;
    double calcBackInOut(double t) const;
    double calcElasticIn(double t) const;
    double calcElasticOut(double t) const;
    double calcElasticInOut(double t) const;
    double calcBounceIn(double t) const;
    double calcBounceOut(double t) const;
    double calcBounceInOut(double t) const;
    double calcWobble(double t) const;

    // Helper for setting name based on type
    void setNameFromType();
};

#endif // EASING_H
