#ifndef EDNA_ENGINE_VECTORANIMATION_HPP
#define EDNA_ENGINE_VECTORANIMATION_HPP

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <library/animation/animations/Animation.hpp>

namespace Animation
{
    template <typename T>
    class VectorAnimation : public Animation<T>
    {
    public:
        using Animation<T>::Animation;

    protected:
        T interpolate(float t) const override;
    };

    using Vec2Animation = VectorAnimation<glm::vec2>;
    using Vec3Animation = VectorAnimation<glm::vec3>;
    using Vec4Animation = VectorAnimation<glm::vec4>;
}

#endif //EDNA_ENGINE_VECTORANIMATION_HPP
