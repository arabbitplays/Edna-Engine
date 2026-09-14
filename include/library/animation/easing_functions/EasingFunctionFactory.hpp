#ifndef EDNA_ENGINE_EASINGFUNCTIONFACTORY_HPP
#define EDNA_ENGINE_EASINGFUNCTIONFACTORY_HPP

#include <memory>

#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>

namespace Animation
{
    std::shared_ptr<EasingFunction> makeEasingFunction(EasingCurve curve,
                                                      EasingDirection direction = EasingDirection::InOut);
}

#endif //EDNA_ENGINE_EASINGFUNCTIONFACTORY_HPP
