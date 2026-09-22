#include <library/mandelbrot/animation/MandelbrotProbe.hpp>

#include <algorithm>
#include <cmath>

namespace mandelbrot
{
    ProbeResult probeInterest(const double center_re, const double center_im,
                              const double span,
                              const double initial_re, const double initial_im,
                              const bool julia_mode,
                              const std::uint32_t max_iter,
                              const std::uint32_t grid_size)
    {
        if (grid_size == 0u || max_iter == 0u) return {0.0f, 0.0f, {}, 0u};

        const double half_span   = 0.5 * span;
        const double step        = span / static_cast<double>(grid_size);
        const double top_left_re = center_re - half_span + 0.5 * step;
        const double top_left_im = center_im - half_span + 0.5 * step;

        // Matches shader bailout so smoothed values align with the GPU.
        constexpr double ESCAPE_SQ = 65536.0;

        const std::uint32_t pixel_count = grid_size * grid_size;
        std::vector<float> mu_norm(pixel_count, 0.0f);
        std::uint32_t inside_count = 0u;

        for (std::uint32_t j = 0; j < grid_size; ++j) {
            const double py = top_left_im + static_cast<double>(j) * step;
            for (std::uint32_t i = 0; i < grid_size; ++i) {
                const double px = top_left_re + static_cast<double>(i) * step;

                double cx, cy, zx, zy;
                if (julia_mode) {
                    cx = initial_re; cy = initial_im;
                    zx = px;         zy = py;
                } else {
                    cx = px;         cy = py;
                    zx = initial_re; zy = initial_im;
                }

                std::uint32_t iter = 0u;
                double zx2 = zx * zx;
                double zy2 = zy * zy;
                while (iter < max_iter) {
                    zx2 = zx * zx;
                    zy2 = zy * zy;
                    if (zx2 + zy2 > ESCAPE_SQ) break;
                    const double new_zx = zx2 - zy2 + cx;
                    zy = 2.0 * zx * zy + cy;
                    zx = new_zx;
                    ++iter;
                }

                const std::uint32_t idx = j * grid_size + i;
                if (iter >= max_iter) {
                    mu_norm[idx] = PROBE_IN_SET;
                    ++inside_count;
                    continue;
                }

                const double log_mod_sq = std::log(zx2 + zy2);
                const double nu         = std::log(0.5 * log_mod_sq / std::log(2.0)) / std::log(2.0);
                const double mu         = static_cast<double>(iter) + 1.0 - nu;
                mu_norm[idx] = static_cast<float>(
                    std::clamp(mu / static_cast<double>(max_iter), 0.0, 1.0));
            }
        }

        const float inside_fraction =
            static_cast<float>(inside_count) / static_cast<float>(pixel_count);

        // Boundary indicator per neighbour pair: 0 if both in-set (dead
        // interior), 1 if they straddle the set boundary, 1 if both escaped
        // but their normalised mu delta exceeds the smooth-gradient threshold.
        // Smooth gradients contribute nothing.
        auto is_boundary = [&](std::uint32_t a_idx, std::uint32_t b_idx) -> bool {
            const float a = mu_norm[a_idx];
            const float b = mu_norm[b_idx];
            const bool a_in = (a == PROBE_IN_SET);
            const bool b_in = (b == PROBE_IN_SET);
            if (a_in && b_in) return false;
            if (a_in != b_in) return true;
            return std::fabs(a - b) > BOUNDARY_DELTA_THRESHOLD;
        };

        std::uint32_t boundary_pairs = 0u;
        std::uint32_t total_pairs    = 0u;
        for (std::uint32_t j = 0; j < grid_size; ++j) {
            for (std::uint32_t i = 0; i < grid_size; ++i) {
                const std::uint32_t idx = j * grid_size + i;
                if (i + 1u < grid_size) {
                    if (is_boundary(idx, idx + 1u)) ++boundary_pairs;
                    ++total_pairs;
                }
                if (j + 1u < grid_size) {
                    if (is_boundary(idx, idx + grid_size)) ++boundary_pairs;
                    ++total_pairs;
                }
            }
        }

        const float edge_score = total_pairs == 0u
            ? 0.0f
            : static_cast<float>(boundary_pairs) / static_cast<float>(total_pairs);

        return {edge_score, inside_fraction, std::move(mu_norm), grid_size};
    }

    float cellInterest(const ProbeResult& probe, const std::uint32_t x, const std::uint32_t y)
    {
        const std::uint32_t g = probe.grid_size;
        if (g == 0u || x >= g || y >= g) return 0.0f;

        auto is_boundary = [&](std::uint32_t a_idx, std::uint32_t b_idx) -> bool {
            const float a = probe.mu_norm[a_idx];
            const float b = probe.mu_norm[b_idx];
            const bool a_in = (a == PROBE_IN_SET);
            const bool b_in = (b == PROBE_IN_SET);
            if (a_in && b_in) return false;
            if (a_in != b_in) return true;
            return std::fabs(a - b) > BOUNDARY_DELTA_THRESHOLD;
        };

        const std::uint32_t idx = y * g + x;
        std::uint32_t boundary = 0u, total = 0u;
        if (x + 1u < g) { if (is_boundary(idx, idx + 1u))     ++boundary; ++total; }
        if (x >= 1u)    { if (is_boundary(idx, idx - 1u))     ++boundary; ++total; }
        if (y + 1u < g) { if (is_boundary(idx, idx + g))      ++boundary; ++total; }
        if (y >= 1u)    { if (is_boundary(idx, idx - g))      ++boundary; ++total; }
        return total == 0u ? 0.0f : static_cast<float>(boundary) / static_cast<float>(total);
    }
}
