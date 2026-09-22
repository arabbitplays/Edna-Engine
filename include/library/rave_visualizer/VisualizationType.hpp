#ifndef EDNA_ENGINE_VISUALIZATIONTYPE_HPP
#define EDNA_ENGINE_VISUALIZATIONTYPE_HPP

#include <array>
#include <cstddef>
#include <string>

namespace RaveVisualizer
{
    enum class VisualizationType
    {
        CCA,
        MANDELBROT,
        MANDELBULB,
    };

    constexpr std::size_t VISUALIZATION_TYPE_COUNT = 3;

    inline std::size_t visualizationTypeIndex(VisualizationType type)
    {
        return static_cast<std::size_t>(type);
    }

    inline VisualizationType visualizationTypeFromIndex(std::size_t index)
    {
        return static_cast<VisualizationType>(index);
    }

    inline const std::array<std::string, VISUALIZATION_TYPE_COUNT>& visualizationTypeNames()
    {
        static const std::array<std::string, VISUALIZATION_TYPE_COUNT> names = {"CCA", "Mandelbrot", "Mandelbulb"};
        return names;
    }
} // namespace RaveVisualizer

#endif // EDNA_ENGINE_VISUALIZATIONTYPE_HPP
