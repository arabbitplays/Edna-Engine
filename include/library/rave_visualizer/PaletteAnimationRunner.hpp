#ifndef EDNA_ENGINE_RAVE_VISUALIZER_PALETTEANIMATIONRUNNER_HPP
#define EDNA_ENGINE_RAVE_VISUALIZER_PALETTEANIMATIONRUNNER_HPP

#include <functional>
#include <library/animation/runners/AnimationRunner.hpp>
#include <library/animation/runners/InterpolatedAnimationTrack.hpp>
#include <library/color/ColorPalette.hpp>
#include <library/color/ColorPaletteAnimationGenerator.hpp>
#include <vector>

namespace RaveVisualizer
{
    class PaletteAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 10.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 30.0f;

        using Listener = std::function<void(const ::color::ColorPalette&)>;

        explicit PaletteAnimationRunner(::color::ColorPalette initial_palette);

        void addListener(Listener listener);

        void update()
        {
            runner_.update();
        }

    private:
        void broadcast(const ::color::ColorPalette& palette);

        std::vector<Listener> listeners_;
        ::color::ColorPaletteAnimationGenerator generator_;
        ::Animation::AnimationRunner runner_;
    };
} // namespace RaveVisualizer

#endif // EDNA_ENGINE_RAVE_VISUALIZER_PALETTEANIMATIONRUNNER_HPP
