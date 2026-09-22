#ifndef EDNA_ENGINE_MANDELBROT_PROBE_HPP
#define EDNA_ENGINE_MANDELBROT_PROBE_HPP

#include <cstdint>
#include <vector>

namespace mandelbrot
{
    struct ProbeResult
    {
        // Fraction of neighbour pairs that cross a real boundary — either the
        // in-set/escaping frontier, or an escape-time discontinuity above
        // BOUNDARY_DELTA_THRESHOLD. Smooth gradients score 0.
        float edge_score;
        // Fraction of grid cells that never escaped. Used to reject views
        // dominated by dead-black interior.
        float inside_fraction;

        // Per-cell smoothed iteration count in [0, 1], or IN_SET sentinel.
        // Row-major, grid_size×grid_size. Empty if grid_size == 0.
        std::vector<float> mu_norm;
        std::uint32_t grid_size = 0;
    };

    // Sentinel written into mu_norm for cells that never escaped.
    constexpr float PROBE_IN_SET = -1.0f;

    // A neighbour pair with normalised mu delta above this counts as a
    // boundary transition even when both cells escaped. Below this we treat
    // the delta as a smooth gradient and score 0.
    constexpr float BOUNDARY_DELTA_THRESHOLD = 0.05f;

    // CPU-side scoring of a candidate view. Uses the shader's smooth-iter
    // formula so scores align with what will render.
    ProbeResult probeInterest(double center_re, double center_im,
                              double span,
                              double initial_re, double initial_im,
                              bool julia_mode,
                              std::uint32_t max_iter,
                              std::uint32_t grid_size = 16u);

    // Fraction of the cell's up-to-4 neighbours that cross a boundary. Used
    // to rank cells in a directed-search probe. Returns 0 for out-of-range
    // coordinates or empty probes.
    float cellInterest(const ProbeResult& probe, std::uint32_t x, std::uint32_t y);
}

#endif //EDNA_ENGINE_MANDELBROT_PROBE_HPP
