#include <library/animation/runners/InterpolatedAnimationTrack.hpp>
#include <library/rave_visualizer/PaletteAnimationRunner.hpp>
#include <memory>
#include <utility>

namespace RaveVisualizer
{
    PaletteAnimationRunner::PaletteAnimationRunner(::color::ColorPalette initial_palette)
        : generator_([this](const ::color::ColorPalette& p) { broadcast(p); })
    {
        auto track = std::make_unique<::Animation::InterpolatedAnimationTrack<::color::ColorPalette>>(
            std::move(initial_palette),
            [this](const ::color::ColorPalette& current) -> std::shared_ptr<::Animation::Animation<::color::ColorPalette>>
            { return std::move(generator_.generatePaletteAnimation(current).animation); },
            COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS);
        runner_.addTrack(std::move(track));
    }

    void PaletteAnimationRunner::addListener(Listener listener)
    {
        if (!listener)
        {
            return;
        }
        listeners_.push_back(std::move(listener));
    }

    void PaletteAnimationRunner::broadcast(const ::color::ColorPalette& palette)
    {
        for (const auto& listener : listeners_)
        {
            listener(palette);
        }
    }
} // namespace RaveVisualizer
