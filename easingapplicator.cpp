#include "easingapplicator.h"

EasingApplicator::EasingApplicator()
    : easingType(EasingType::Linear)
    , easingMode(EasingMode::InOut)
    , easing(Easing::linear())
{
}

void EasingApplicator::setEasingType(EasingType type)
{
    easingType = type;
    updateEasing();
}

void EasingApplicator::setEasingSelect(int index)
{
    // Use the new Easing class directly - handles all 28 easing types
    easing = Easing::getById(index);

    // Update legacy fields for backward compatibility
    // Map new easing types to legacy enum where possible
    switch (static_cast<Easing::Type>(index)) {
        case Easing::Type::Linear:
            easingType = EasingType::Linear;
            easingMode = EasingMode::InOut;
            break;
        case Easing::Type::QuadIn:
            easingType = EasingType::QuadIn;
            easingMode = EasingMode::In;
            break;
        case Easing::Type::QuadOut:
            easingType = EasingType::QuadOut;
            easingMode = EasingMode::Out;
            break;
        case Easing::Type::QuadInOut:
            easingType = EasingType::QuadInOut;
            easingMode = EasingMode::InOut;
            break;
        case Easing::Type::CubicIn:
            easingType = EasingType::CubicIn;
            easingMode = EasingMode::In;
            break;
        case Easing::Type::CubicOut:
            easingType = EasingType::CubicOut;
            easingMode = EasingMode::Out;
            break;
        case Easing::Type::CubicInOut:
            easingType = EasingType::CubicInOut;
            easingMode = EasingMode::InOut;
            break;
        case Easing::Type::SineIn:
            easingType = EasingType::SineIn;
            easingMode = EasingMode::In;
            break;
        case Easing::Type::SineOut:
            easingType = EasingType::SineOut;
            easingMode = EasingMode::Out;
            break;
        case Easing::Type::SineInOut:
            easingType = EasingType::SineInOut;
            easingMode = EasingMode::InOut;
            break;
        case Easing::Type::ElasticIn:
            easingType = EasingType::Elastic;
            easingMode = EasingMode::In;
            break;
        case Easing::Type::ElasticOut:
            easingType = EasingType::Elastic;
            easingMode = EasingMode::Out;
            break;
        case Easing::Type::ElasticInOut:
            easingType = EasingType::Elastic;
            easingMode = EasingMode::InOut;
            break;
        case Easing::Type::BounceIn:
            easingType = EasingType::Bounce;
            easingMode = EasingMode::In;
            break;
        case Easing::Type::BounceOut:
            easingType = EasingType::Bounce;
            easingMode = EasingMode::Out;
            break;
        case Easing::Type::BounceInOut:
            easingType = EasingType::Bounce;
            easingMode = EasingMode::InOut;
            break;
        case Easing::Type::BackIn:
            easingType = EasingType::Back;
            easingMode = EasingMode::In;
            break;
        case Easing::Type::BackOut:
            easingType = EasingType::Back;
            easingMode = EasingMode::Out;
            break;
        case Easing::Type::BackInOut:
            easingType = EasingType::Back;
            easingMode = EasingMode::InOut;
            break;
        default:
            // New easing types (Quartic, Expo, Circ) - no legacy enum equivalent
            easingType = EasingType::Linear;  // Fallback for legacy getters
            easingMode = EasingMode::InOut;
            break;
    }
}

void EasingApplicator::setEasingMode(EasingMode mode)
{
    easingMode = mode;
    updateEasing();
}

void EasingApplicator::setEasing(const Easing &newEasing)
{
    easing = newEasing;
}

void EasingApplicator::updateEasing()
{
    // Map legacy EasingType + EasingMode to new Easing::Type
    switch (easingType) {
        case EasingType::Linear:
            easing = Easing::linear();
            break;

        case EasingType::QuadIn:
            easing = Easing::quadIn();
            break;
        case EasingType::QuadOut:
            easing = Easing::quadOut();
            break;
        case EasingType::QuadInOut:
            easing = Easing::quadInOut();
            break;

        case EasingType::CubicIn:
            easing = Easing::cubicIn();
            break;
        case EasingType::CubicOut:
            easing = Easing::cubicOut();
            break;
        case EasingType::CubicInOut:
            easing = Easing::cubicInOut();
            break;

        case EasingType::SineIn:
            easing = Easing::sineIn();
            break;
        case EasingType::SineOut:
            easing = Easing::sineOut();
            break;
        case EasingType::SineInOut:
            easing = Easing::sineInOut();
            break;

        case EasingType::Elastic:
            switch (easingMode) {
                case EasingMode::In:    easing = Easing::elasticIn(); break;
                case EasingMode::Out:   easing = Easing::elasticOut(); break;
                case EasingMode::InOut: easing = Easing::elasticInOut(); break;
            }
            break;

        case EasingType::Bounce:
            switch (easingMode) {
                case EasingMode::In:    easing = Easing::bounceIn(); break;
                case EasingMode::Out:   easing = Easing::bounceOut(); break;
                case EasingMode::InOut: easing = Easing::bounceInOut(); break;
            }
            break;

        case EasingType::Back:
            switch (easingMode) {
                case EasingMode::In:    easing = Easing::backIn(); break;
                case EasingMode::Out:   easing = Easing::backOut(); break;
                case EasingMode::InOut: easing = Easing::backInOut(); break;
            }
            break;

        default:
            easing = Easing::linear();
            break;
    }
}

double EasingApplicator::process(double startValue, double endValue, double progress)
{
    // Apply easing function to progress
    double easedProgress = easing.calculate(progress);

    // Interpolate between start and end
    return startValue + (endValue - startValue) * easedProgress;
}
