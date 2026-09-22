#include <algorithm>
#include <glm/glm.hpp>
#include <library/color/ColorPaletteAnimation.hpp>

namespace color
{
    ColorPalette ColorPaletteAnimation::interpolate(float t) const
    {
        const auto& s = start.colors;
        const auto& e = target.colors;
        const size_t n = std::min(s.size(), e.size());

        ColorPalette result;
        result.colors.reserve(n);
        for (size_t i = 0; i < n; ++i)
        {
            result.colors.emplace_back(glm::mix(s[i], e[i], t));
        }

        return result;
    }
} // namespace color
