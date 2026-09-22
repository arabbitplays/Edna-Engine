#ifndef EDNA_ENGINE_PALETTEANIMATIONRUNNER_HPP
#define EDNA_ENGINE_PALETTEANIMATIONRUNNER_HPP

#include <chrono>
#include <functional>
#include <library/animation/animations/IAnimation.hpp>
#include <library/color/ColorPalette.hpp>
#include <vector>

namespace color
{
    // Cycles through random palettes with an eased interpolation, then a
    // random cooldown. Callers register listeners that receive every
    // interpolated palette (via `step()` inside the animation) as well as the
    // final target on completion.
    class PaletteAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 10.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 30.0f;

        using Listener = std::function<void(const ColorPalette&)>;

        explicit PaletteAnimationRunner(ColorPalette initial_palette);

        // The listener is invoked once immediately with the current palette so
        // late-registered listeners start in sync.
        void addListener(Listener listener);

        void update();

        const ColorPalette& currentPalette() const
        {
            return current_palette_;
        }

    private:
        void broadcast(const ColorPalette& palette);
        void startAnimation();

        std::vector<Listener> listeners_;

        ColorPalette current_palette_;
        ::Animation::AnimationHandle animation_;
        float cooldown_seconds_ = 0.0f;

        std::chrono::steady_clock::time_point last_tick_;
    };
} // namespace color

#endif // EDNA_ENGINE_PALETTEANIMATIONRUNNER_HPP
