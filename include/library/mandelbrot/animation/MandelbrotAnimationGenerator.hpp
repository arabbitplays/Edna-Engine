#ifndef EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP
#define EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP

#include <cstdint>
#include <functional>
#include <glm/vec2.hpp>
#include <library/animation/animations/FloatAnimation.hpp>
#include <library/animation/animations/VectorAnimation.hpp>
#include <library/mandelbrot/MandelbrotState.hpp>
#include <memory>

namespace mandelbrot
{
    class MandelbrotAnimationGenerator
    {
    public:
        static constexpr int STEP_COUNT_MIN = 500;
        static constexpr int STEP_COUNT_MAX = 2500;

        static constexpr float OFFSET_MIN = -1.0f;
        static constexpr float OFFSET_MAX = 1.0f;

        // step_size animated in log10 space so linear interpolation covers
        // orders of magnitude evenly.
        static constexpr float LOG_STEP_SIZE_MIN = -6.0f;
        static constexpr float LOG_STEP_SIZE_MAX = -3.0f;

        static constexpr float INITIAL_MIN = -1.2f;
        static constexpr float INITIAL_MAX = 1.2f;

        // Reject candidates whose probe scores below PROBE_ACCEPT_EDGE or
        // whose inside_fraction exceeds PROBE_MAX_INSIDE_FRACT. If no
        // attempt clears the gate, the best-scoring candidate wins so
        // generation never blocks.
        static constexpr float PROBE_ACCEPT_EDGE = 0.05f;
        static constexpr float PROBE_MAX_INSIDE_FRACT = 0.60f;
        static constexpr std::uint32_t PROBE_MAX_ATTEMPTS = 8u;
        static constexpr std::uint32_t PROBE_GRID_SIZE = 16u;
        static constexpr std::uint32_t PROBE_MAX_ITER = 256u;

        // Canonical span used when the generator can't see the actual
        // on-screen span. Targets that score well here are the
        // "structurally interesting" ones.
        static constexpr float PROBE_REFERENCE_SPAN = 3.0f;

        // Directed offset selection scans a probe grid covering the full
        // offset range and picks the highest-interest cell as the target.
        // Wider span + finer grid than a per-candidate probe.
        static constexpr float DIRECTED_SEARCH_SPAN = 6.0f; // 2 * REFERENCE_SPAN
        static constexpr std::uint32_t DIRECTED_GRID_SIZE = 32u;

        // Number of waypoints sampled along a candidate animation for
        // path-aware acceptance. Acceptance uses min-along-path so the
        // interpolation itself has to stay interesting, not just the target.
        static constexpr std::uint32_t PATH_WAYPOINT_COUNT = 5u;

        // Zoom coupling: step_size targets are biased in log space by the
        // current view's edge score. Dense view -> zoom in aggressively;
        // flat -> zoom back out at a gentler pace so we spend more time
        // exploring interesting regions than fleeing dead ones.
        // ZOOM_EDGE_SATURATION mirrors the runner's speed cap threshold.
        static constexpr float ZOOM_IN_BIAS_LOG = 1.4f;
        static constexpr float ZOOM_OUT_BIAS_LOG = 0.6f;
        static constexpr float ZOOM_NOISE_LOG = 0.4f;
        static constexpr float ZOOM_EDGE_SATURATION = 0.15f;

        MandelbrotAnimationGenerator(std::function<void(const glm::vec2&)> set_offset,
            std::function<void(float)> set_step_size, std::function<void(const glm::vec2&)> set_initial);

        struct Vec2AnimationResult
        {
            std::unique_ptr<::Animation::Animation<glm::vec2>> animation;
            glm::vec2 target;
        };

        struct FloatAnimationResult
        {
            std::unique_ptr<::Animation::FloatAnimation> animation;
            float target;
        };

        // Candidates are scored against `current` for the fields the method
        // does not roll, so target selection sees the actual fractal being
        // rendered (in particular the current initial/c and julia_mode).
        Vec2AnimationResult generateOffsetAnimation(const MandelbrotState& current);
        FloatAnimationResult generateStepSizeAnimation(const MandelbrotState& current, float current_view_edge_score);
        Vec2AnimationResult generateInitialAnimation(const MandelbrotState& current);

    private:
        std::function<void(const glm::vec2&)> set_offset_;
        std::function<void(float)> set_step_size_;
        std::function<void(const glm::vec2&)> set_initial_;
    };
} // namespace mandelbrot

#endif // EDNA_ENGINE_MANDELBROT_ANIMATIONGENERATOR_HPP
