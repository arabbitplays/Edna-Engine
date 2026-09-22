#ifndef EDNA_ENGINE_MANDELBROT_STATE_HPP
#define EDNA_ENGINE_MANDELBROT_STATE_HPP

#include <glm/vec2.hpp>

namespace mandelbrot
{
    struct MandelbrotState
    {
        glm::vec2 offset;  // screen widths from world origin
        float step_size;   // complex-plane units per pixel
        glm::vec2 initial; // z0 (mandelbrot) or c (julia)
        bool julia_mode;
    };
} // namespace mandelbrot

#endif // EDNA_ENGINE_MANDELBROT_STATE_HPP
