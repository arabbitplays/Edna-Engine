#include <library/animation/animations/VectorAnimation.hpp>

#include <glm/glm.hpp>

namespace Animation
{
    template <typename T>
    T VectorAnimation<T>::interpolate(float t) const
    {
        return glm::mix(this->start, this->target, t);
    }

    template class Animation<glm::vec2>;
    template class Animation<glm::vec3>;
    template class Animation<glm::vec4>;

    template class VectorAnimation<glm::vec2>;
    template class VectorAnimation<glm::vec3>;
    template class VectorAnimation<glm::vec4>;
}
