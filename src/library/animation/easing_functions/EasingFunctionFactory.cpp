#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>

namespace Animation
{
    std::shared_ptr<EasingFunction> makeEasingFunction(EasingCurve curve, EasingDirection direction)
    {
        switch (curve)
        {
        case EasingCurve::Linear:
            return std::make_shared<Linear>();

        case EasingCurve::Cubic:
            switch (direction)
            {
            case EasingDirection::In:
                return std::make_shared<EaseInCubic>();
            case EasingDirection::Out:
                return std::make_shared<EaseOutCubic>();
            case EasingDirection::InOut:
                return std::make_shared<EaseInOutCubic>();
            }
            break;

        case EasingCurve::Elastic:
            switch (direction)
            {
            case EasingDirection::In:
                return std::make_shared<EaseInElastic>();
            case EasingDirection::Out:
                return std::make_shared<EaseOutElastic>();
            case EasingDirection::InOut:
                return std::make_shared<EaseInOutElastic>();
            }
            break;

        case EasingCurve::Bounce:
            switch (direction)
            {
            case EasingDirection::In:
                return std::make_shared<EaseInBounce>();
            case EasingDirection::Out:
                return std::make_shared<EaseOutBounce>();
            case EasingDirection::InOut:
                return std::make_shared<EaseInOutBounce>();
            }
            break;
        }

        return std::make_shared<Linear>();
    }
} // namespace Animation
