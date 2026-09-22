#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <glm/glm.hpp>
#include <library/animation/animations/BezierAnimation.hpp>
#include <library/animation/easing_functions/EasingCurve.hpp>
#include <library/animation/easing_functions/EasingDirection.hpp>
#include <library/animation/easing_functions/EasingFunction.hpp>
#include <library/animation/easing_functions/EasingFunctionFactory.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <library/mandelbrot/animation/MandelbrotAnimationGenerator.hpp>
#include <library/mandelbrot/animation/MandelbrotProbe.hpp>
#include <limits>
#include <logging/LogManager.hpp>
#include <memory>
#include <util/RandomUtil.hpp>
#include <utility>

namespace mandelbrot
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<MandelbrotAnimationGenerator>();
            return instance;
        }

        float randomFloat(float min, float max)
        {
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return min + (t * (max - min));
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
            constexpr std::array curves = {::Animation::EasingCurve::Cubic};
            const std::size_t idx = RtEngine::RandomUtil::generateInt() % curves.size();
            return ::Animation::makeEasingFunction(curves[idx], ::Animation::EasingDirection::InOut);
        }

        // Probe a candidate view. Offset is treated as a position in the
        // canonical reference frame (center = offset * PROBE_REFERENCE_SPAN),
        // so scoring is consistent across candidates regardless of the
        // component's origin field.
        ProbeResult probeCandidate(const glm::vec2& offset, const glm::vec2& initial, bool julia_mode)
        {
            using Gen = MandelbrotAnimationGenerator;
            const auto span = static_cast<double>(Gen::PROBE_REFERENCE_SPAN);
            const double cx = static_cast<double>(offset.x) * span;
            const double cy = static_cast<double>(offset.y) * span;
            return probeInterest(cx, cy, span, static_cast<double>(initial.x), static_cast<double>(initial.y),
                julia_mode, Gen::PROBE_MAX_ITER, Gen::PROBE_GRID_SIZE);
        }

        float scoreProbe(const ProbeResult& p)
        {
            using Gen = MandelbrotAnimationGenerator;
            if (p.inside_fraction > Gen::PROBE_MAX_INSIDE_FRACT)
            {
                return 0.0F;
            }
            return p.edge_score;
        }

        glm::vec2 cubicBezier(
            const glm::vec2& p0, const glm::vec2& c1, const glm::vec2& c2, const glm::vec2& p3, float t)
        {
            const float u = 1.0F - t;
            return u * u * u * p0 + 3.0F * u * u * t * c1 + 3.0F * u * t * t * c2 + t * t * t * p3;
        }

        // Sample N waypoints along an animation and return the minimum
        // probe score. Uses min because a good target reached through a
        // boring corridor is not what we want.
        template <typename Sample> float scorePath(bool julia_mode, Sample&& sample_at)
        {
            using Gen = MandelbrotAnimationGenerator;
            float worst = 1.0F;
            const std::uint32_t n = Gen::PATH_WAYPOINT_COUNT;
            for (std::uint32_t k = 0; k < n; ++k)
            {
                const float t = static_cast<float>(k) / static_cast<float>(n - 1U);
                const auto [offset, initial] = sample_at(t);
                const ProbeResult probe = probeCandidate(offset, initial, julia_mode);
                worst = std::min(worst, scoreProbe(probe));
            }
            return worst;
        }

        // Probe a wide search area at fine resolution, return the cell centre
        // with the highest local boundary density (in offset units, in
        // [-OFFSET_MAX, OFFSET_MAX]). Beats dart-throwing because it examines
        // every cell instead of hoping N random rolls hit one.
        glm::vec2 pickDirectedOffset(const glm::vec2& initial, bool julia_mode)
        {
            using Gen = MandelbrotAnimationGenerator;
            const auto span = static_cast<double>(Gen::DIRECTED_SEARCH_SPAN);
            const ProbeResult probe = probeInterest(0.0, 0.0, span, static_cast<double>(initial.x),
                static_cast<double>(initial.y), julia_mode, Gen::PROBE_MAX_ITER, Gen::DIRECTED_GRID_SIZE);

            const std::uint32_t g = probe.grid_size;
            if (g == 0U)
            {
                return randomVec2(Gen::OFFSET_MIN, Gen::OFFSET_MAX);
            }

            std::uint32_t best_x = 0u;
            std::uint32_t best_y = 0u;
            float best_score = -1.0F;
            for (std::uint32_t y = 0; y < g; ++y)
            {
                for (std::uint32_t x = 0; x < g; ++x)
                {
                    const float s = cellInterest(probe, x, y);
                    if (s > best_score)
                    {
                        best_score = s;
                        best_x = x;
                        best_y = y;
                    }
                }
            }

            const double step = span / static_cast<double>(g);
            const double half_span = 0.5 * span;
            const double centre_re = -half_span + ((static_cast<double>(best_x) + 0.5) * step);
            const double centre_im = -half_span + ((static_cast<double>(best_y) + 0.5) * step);
            const float reference = Gen::PROBE_REFERENCE_SPAN;
            logger()->info(std::format("directed offset cell=({},{}) score={:.3f}", best_x, best_y, best_score));
            return {std::clamp(static_cast<float>(centre_re) / reference, Gen::OFFSET_MIN, Gen::OFFSET_MAX),
                std::clamp(static_cast<float>(centre_im) / reference, Gen::OFFSET_MIN, Gen::OFFSET_MAX)};
        }

        // Roll up to PROBE_MAX_ATTEMPTS candidates. Return the first one
        // whose score clears the gate; otherwise the highest-scoring one
        // seen. Never blocks.
        template <typename Candidate, typename Roll, typename Score>
        Candidate rejectionSample(Roll&& roll, Score&& score_of)
        {
            using Gen = MandelbrotAnimationGenerator;
            Candidate best{};
            float best_score = -1.0F;
            for (std::uint32_t attempt = 0; attempt < Gen::PROBE_MAX_ATTEMPTS; ++attempt)
            {
                Candidate candidate = roll();
                const float score = score_of(candidate);
                if (score >= Gen::PROBE_ACCEPT_EDGE)
                {
                    logger()->info(
                        std::format("candidate accepted after {} attempt(s), score={:.3f}", attempt + 1U, score));
                    return candidate;
                }
                if (score > best_score)
                {
                    best_score = score;
                    best = candidate;
                }
            }
            logger()->info(
                std::format("exhausted {} attempts, best score={:.3f}", Gen::PROBE_MAX_ATTEMPTS, best_score));
            return best;
        }
    } // namespace

    MandelbrotAnimationGenerator::MandelbrotAnimationGenerator(std::function<void(const glm::vec2&)> set_offset,
        std::function<void(float)> set_step_size, std::function<void(const glm::vec2&)> set_initial,
        std::function<void(const ::color::ColorPalette&)> set_palette)
        : set_offset_(std::move(set_offset)), set_step_size_(std::move(set_step_size)),
          set_initial_(std::move(set_initial)), set_palette_(std::move(set_palette))
    {
    }

    MandelbrotAnimationGenerator::Vec2AnimationResult MandelbrotAnimationGenerator::generateOffsetAnimation(
        const MandelbrotState& current)
    {
        const glm::vec2 target = pickDirectedOffset(current.initial, current.julia_mode);

        // Target is fixed; rejection-sample the Bezier control points so the
        // arc between current and target stays interesting.
        using ControlPoints = std::pair<glm::vec2, glm::vec2>;
        const auto [c1, c2] = rejectionSample<ControlPoints>([]()
            { return ControlPoints{randomVec2(OFFSET_MIN, OFFSET_MAX), randomVec2(OFFSET_MIN, OFFSET_MAX)}; },
            [&](const ControlPoints& cp)
            {
                return scorePath(current.julia_mode,
                    [&](float t)
                    {
                        return std::pair{cubicBezier(current.offset, cp.first, cp.second, target, t), current.initial};
                    });
            });
        logger()->info(std::format("offset target=({:.3f},{:.3f})", target.x, target.y));

        auto set = set_offset_;
        auto animation = std::make_unique<::Animation::Vec2BezierAnimation>(
            current.offset, c1, c2, target, randomStepCount(),
            [set](const glm::vec2& v)
            {
                if (set)
                {
                    set(v);
                }
            },
            randomInOutEasing());

        return {.animation = std::move(animation), .target = target};
    }

    MandelbrotAnimationGenerator::FloatAnimationResult MandelbrotAnimationGenerator::generateStepSizeAnimation(
        const MandelbrotState& current, const float current_view_edge_score)
    {
        // Bias log-delta by current view density: high edge score pushes the
        // target downward (zoom in on detail), low pushes upward (zoom out to
        // find something). Saturation matches the runner's speed threshold.
        const float density = std::clamp(current_view_edge_score / ZOOM_EDGE_SATURATION, 0.0F, 1.0F);
        const float bias = ZOOM_BIAS_LOG * (1.0F - 2.0F * density);
        const float log_delta = bias + randomFloat(-ZOOM_NOISE_LOG, ZOOM_NOISE_LOG);
        const float log_current = std::log10(std::max(current.step_size, 1e-9F));
        const float log_target = std::clamp(log_current + log_delta, LOG_STEP_SIZE_MIN, LOG_STEP_SIZE_MAX);
        const float target = std::pow(10.0F, log_target);
        logger()->info(
            std::format("step_size target={:.6f} (log_delta={:.3f} density={:.2f})", target, log_delta, density));

        auto set = set_step_size_;
        auto animation = std::make_unique<::Animation::FloatAnimation>(
            log_current, log_target, randomStepCount(),
            [set](const float& log_v)
            {
                if (set)
                {
                    set(std::pow(10.0F, log_v));
                }
            },
            randomInOutEasing());

        return {.animation = std::move(animation), .target = target};
    }

    MandelbrotAnimationGenerator::Vec2AnimationResult MandelbrotAnimationGenerator::generateInitialAnimation(
        const MandelbrotState& current)
    {
        const auto target = rejectionSample<glm::vec2>([]() { return randomVec2(INITIAL_MIN, INITIAL_MAX); },
            [&](const glm::vec2& candidate)
            {
                return scorePath(current.julia_mode,
                    [&](float t) { return std::pair{current.offset, glm::mix(current.initial, candidate, t)}; });
            });

        logger()->info(std::format("initial target=({:.3f},{:.3f})", target.x, target.y));

        auto set = set_initial_;
        auto animation = std::make_unique<::Animation::Vec2Animation>(
            current.initial, target, randomStepCount(),
            [set](const glm::vec2& v)
            {
                if (set)
                {
                    set(v);
                }
            },
            randomInOutEasing());

        return {.animation = std::move(animation), .target = target};
    }

    MandelbrotAnimationGenerator::PaletteAnimationResult MandelbrotAnimationGenerator::generatePaletteAnimation(
        const ::color::ColorPalette& current)
    {
        const auto all_names = ::color::ColorPaletteName::getAllNames();
        const std::size_t idx = RtEngine::RandomUtil::generateInt() % all_names.size();
        const auto& picked_name = all_names[idx];
        logger()->info(std::format("palette target={}", picked_name));

        ::color::ColorPalette target = ::color::ColorPaletteFactory::create(
            ::color::ColorPaletteName::fromString(picked_name, ::color::ColorPaletteName::Fire));

        auto set = set_palette_;
        auto animation = std::make_unique<::color::ColorPaletteAnimation>(
            current, target, randomStepCount(),
            [set](const ::color::ColorPalette& p)
            {
                if (set)
                {
                    set(p);
                }
            },
            randomInOutEasing());

        return {.animation = std::move(animation), .target = std::move(target)};
    }
} // namespace mandelbrot
