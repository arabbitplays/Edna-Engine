#include <library/mandelbrot/animation/MandelbrotAnimationGenerator.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include <spdlog/spdlog.h>

#include <library/animation/animations/BezierAnimation.hpp>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <library/mandelbrot/animation/MandelbrotProbe.hpp>
#include <util/RandomUtil.hpp>

namespace mandelbrot
{
    namespace
    {
        float randomFloat(float min, float max)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min + t * (max - min);
        }

        glm::vec2 randomVec2(float min, float max)
        {
            return {randomFloat(min, max), randomFloat(min, max)};
        }

        int randomStepCount()
        {
            using Gen = MandelbrotAnimationGenerator;
            const auto range = static_cast<uint32_t>(Gen::STEP_COUNT_MAX - Gen::STEP_COUNT_MIN + 1);
            return Gen::STEP_COUNT_MIN + static_cast<int>(RtEngine::RandomUtil::generateInt() % range);
        }

        std::shared_ptr<::Animation::EasingFunction> randomInOutEasing()
        {
            constexpr std::array curves = {
                ::Animation::EasingCurve::Cubic
            };
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % curves.size();
            return ::Animation::makeEasingFunction(curves[idx], ::Animation::EasingDirection::InOut);
        }

        // Probe a candidate view. Offset is treated as a position in the
        // canonical reference frame (center = offset * PROBE_REFERENCE_SPAN),
        // so scoring is consistent across candidates regardless of the
        // component's origin field.
        ProbeResult probeCandidate(const glm::vec2& offset,
                                   const glm::vec2& initial,
                                   bool julia_mode)
        {
            using Gen = MandelbrotAnimationGenerator;
            const double span   = static_cast<double>(Gen::PROBE_REFERENCE_SPAN);
            const double cx     = static_cast<double>(offset.x) * span;
            const double cy     = static_cast<double>(offset.y) * span;
            return probeInterest(cx, cy, span,
                                 static_cast<double>(initial.x),
                                 static_cast<double>(initial.y),
                                 julia_mode,
                                 Gen::PROBE_MAX_ITER,
                                 Gen::PROBE_GRID_SIZE);
        }

        float scoreProbe(const ProbeResult& p)
        {
            using Gen = MandelbrotAnimationGenerator;
            if (p.inside_fraction > Gen::PROBE_MAX_INSIDE_FRACT) return 0.0f;
            return p.edge_score;
        }

        // Probe a wide search area at fine resolution, return the cell centre
        // with the highest local boundary density (in offset units, in
        // [-OFFSET_MAX, OFFSET_MAX]). Beats dart-throwing because it examines
        // every cell instead of hoping N random rolls hit one.
        glm::vec2 pickDirectedOffset(const glm::vec2& initial, bool julia_mode)
        {
            using Gen = MandelbrotAnimationGenerator;
            const double span = static_cast<double>(Gen::DIRECTED_SEARCH_SPAN);
            const ProbeResult probe = probeInterest(
                0.0, 0.0, span,
                static_cast<double>(initial.x),
                static_cast<double>(initial.y),
                julia_mode,
                Gen::PROBE_MAX_ITER,
                Gen::DIRECTED_GRID_SIZE);

            const std::uint32_t g = probe.grid_size;
            if (g == 0u) return randomVec2(Gen::OFFSET_MIN, Gen::OFFSET_MAX);

            std::uint32_t best_x = 0u, best_y = 0u;
            float best_score = -1.0f;
            for (std::uint32_t y = 0; y < g; ++y) {
                for (std::uint32_t x = 0; x < g; ++x) {
                    const float s = cellInterest(probe, x, y);
                    if (s > best_score) { best_score = s; best_x = x; best_y = y; }
                }
            }

            const double step        = span / static_cast<double>(g);
            const double half_span   = 0.5 * span;
            const double centre_re   = -half_span + (static_cast<double>(best_x) + 0.5) * step;
            const double centre_im   = -half_span + (static_cast<double>(best_y) + 0.5) * step;
            const float  reference   = Gen::PROBE_REFERENCE_SPAN;
            spdlog::info("Mandelbrot anim: directed offset cell=({},{}) score={:.3f}",
                         best_x, best_y, best_score);
            return glm::vec2(
                std::clamp(static_cast<float>(centre_re) / reference, Gen::OFFSET_MIN, Gen::OFFSET_MAX),
                std::clamp(static_cast<float>(centre_im) / reference, Gen::OFFSET_MIN, Gen::OFFSET_MAX));
        }

        // Roll up to PROBE_MAX_ATTEMPTS candidates. Return the first one
        // whose probe clears the gate; otherwise the highest-scoring one
        // seen. Never blocks.
        template <typename Candidate, typename Roll, typename Probe>
        Candidate rejectionSample(Roll&& roll, Probe&& probe_of)
        {
            using Gen = MandelbrotAnimationGenerator;
            Candidate best{};
            float best_score = -1.0f;
            ProbeResult best_probe{};
            for (std::uint32_t attempt = 0; attempt < Gen::PROBE_MAX_ATTEMPTS; ++attempt) {
                Candidate candidate = roll();
                const ProbeResult probe = probe_of(candidate);
                const float score = scoreProbe(probe);
                if (score >= Gen::PROBE_ACCEPT_EDGE) {
                    spdlog::info("Mandelbrot anim: probe accepted after {} attempt(s), edge={:.3f} inside={:.2f}",
                                 attempt + 1u, probe.edge_score, probe.inside_fraction);
                    return candidate;
                }
                if (score > best_score) {
                    best_score = score;
                    best_probe = probe;
                    best = candidate;
                }
            }
            spdlog::info("Mandelbrot anim: probe exhausted {} attempts, best edge={:.3f} inside={:.2f}",
                         Gen::PROBE_MAX_ATTEMPTS, best_probe.edge_score, best_probe.inside_fraction);
            return best;
        }
    }

    MandelbrotAnimationGenerator::MandelbrotAnimationGenerator(
        std::function<void(const glm::vec2&)>             set_offset,
        std::function<void(float)>                        set_step_size,
        std::function<void(const glm::vec2&)>             set_initial,
        std::function<void(const ::color::ColorPalette&)> set_palette)
        : set_offset_(std::move(set_offset)),
          set_step_size_(std::move(set_step_size)),
          set_initial_(std::move(set_initial)),
          set_palette_(std::move(set_palette))
    {
    }

    MandelbrotAnimationGenerator::Vec2AnimationResult
    MandelbrotAnimationGenerator::generateOffsetAnimation(const MandelbrotState& current)
    {
        const glm::vec2 target = pickDirectedOffset(current.initial, current.julia_mode);

        const glm::vec2 c1 = randomVec2(OFFSET_MIN, OFFSET_MAX);
        const glm::vec2 c2 = randomVec2(OFFSET_MIN, OFFSET_MAX);
        spdlog::info("Mandelbrot anim: offset target=({:.3f},{:.3f})", target.x, target.y);

        auto set = set_offset_;
        auto animation = std::make_unique<::Animation::Vec2BezierAnimation>(
            current.offset, c1, c2, target, randomStepCount(),
            [set](const glm::vec2& v) { if (set) set(v); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    MandelbrotAnimationGenerator::FloatAnimationResult
    MandelbrotAnimationGenerator::generateStepSizeAnimation(const MandelbrotState& current)
    {
        // Uniform random in log space. Coupled-zoom targeting (a later
        // commit) refines this.
        const float log_target  = randomFloat(LOG_STEP_SIZE_MIN, LOG_STEP_SIZE_MAX);
        const float target      = std::pow(10.0f, log_target);
        const float log_current = std::log10(std::max(current.step_size, 1e-9f));
        spdlog::info("Mandelbrot anim: step_size target={:.6f} (log={:.3f})", target, log_target);

        auto set = set_step_size_;
        auto animation = std::make_unique<::Animation::FloatAnimation>(
            log_current, log_target, randomStepCount(),
            [set](const float& log_v) { if (set) set(std::pow(10.0f, log_v)); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    MandelbrotAnimationGenerator::Vec2AnimationResult
    MandelbrotAnimationGenerator::generateInitialAnimation(const MandelbrotState& current)
    {
        const glm::vec2 target = rejectionSample<glm::vec2>(
            []() { return randomVec2(INITIAL_MIN, INITIAL_MAX); },
            [&](const glm::vec2& candidate) {
                return probeCandidate(current.offset, candidate, current.julia_mode);
            });

        spdlog::info("Mandelbrot anim: initial target=({:.3f},{:.3f})", target.x, target.y);

        auto set = set_initial_;
        auto animation = std::make_unique<::Animation::Vec2Animation>(
            current.initial, target, randomStepCount(),
            [set](const glm::vec2& v) { if (set) set(v); },
            randomInOutEasing());

        return {std::move(animation), target};
    }

    MandelbrotAnimationGenerator::PaletteAnimationResult
    MandelbrotAnimationGenerator::generatePaletteAnimation(const ::color::ColorPalette& current)
    {
        const auto all_names = ::color::ColorPaletteName::getAllNames();
        const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
        const auto& picked_name = all_names[idx];
        spdlog::info("Mandelbrot anim: palette target={}", picked_name);

        ::color::ColorPalette target = ::color::ColorPaletteFactory::create(
            ::color::ColorPaletteName::fromString(picked_name, ::color::ColorPaletteName::Fire));

        auto set = set_palette_;
        auto animation = std::make_unique<::color::ColorPaletteAnimation>(
            current, target, randomStepCount(),
            [set](const ::color::ColorPalette& p) { if (set) set(p); },
            randomInOutEasing());

        return {std::move(animation), std::move(target)};
    }
}
