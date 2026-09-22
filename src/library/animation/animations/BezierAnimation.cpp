#include <glm/glm.hpp>
#include <library/animation/animations/BezierAnimation.hpp>

namespace Animation
{
    template <typename T> T BezierAnimation<T>::interpolate(float t) const
    {
        // De Casteljau evaluation of a cubic Bezier at parameter t.
        const T& p0 = this->start;
        const T& p3 = this->target;
        const T q0 = glm::mix(p0, c1_, t);
        const T q1 = glm::mix(c1_, c2_, t);
        const T q2 = glm::mix(c2_, p3, t);
        const T r0 = glm::mix(q0, q1, t);
        const T r1 = glm::mix(q1, q2, t);
        return glm::mix(r0, r1, t);
    }

    template class BezierAnimation<glm::vec2>;
    template class BezierAnimation<glm::vec3>;
    template class BezierAnimation<glm::vec4>;
} // namespace Animation
