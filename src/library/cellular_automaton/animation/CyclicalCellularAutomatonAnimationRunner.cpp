#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationRunner.hpp>

#include <algorithm>
#include <utility>

#include <library/animation/animations/FloatAnimation.hpp>
#include <library/cellular_automaton/colors/ColorPaletteAnimation.hpp>

namespace cellular_automaton
{
    CyclicalCellularAutomatonAnimationRunner::CyclicalCellularAutomatonAnimationRunner(
        CyclicalCellularAutomatonAnimationGenerator generator,
        float initial_mutation_chance,
        ColorPalette initial_palette)
        : generator_(std::move(generator)),
          current_mutation_chance_(initial_mutation_chance),
          current_palette_(std::move(initial_palette))
    {
        regenerate();
    }

    void CyclicalCellularAutomatonAnimationRunner::update()
    {
        for (auto& animation : animations_) animation->step();

        if (allFinished()) {
            animations_.clear();
            regenerate();
        }
    }

    bool CyclicalCellularAutomatonAnimationRunner::allFinished() const
    {
        if (animations_.empty()) return true;
        return std::all_of(animations_.begin(), animations_.end(),
                           [](const auto& animation) { return animation->finished(); });
    }

    void CyclicalCellularAutomatonAnimationRunner::regenerate()
    {
        auto mutation = generator_.generateMutationChanceAnimation(current_mutation_chance_);
        current_mutation_chance_ = mutation.target;
        // unique_ptr<Animation<T>> converts to shared_ptr<IAnimation> via move.
        animations_.push_back(std::move(mutation.animation));

        auto palette = generator_.generatePaletteAnimation(current_palette_);
        current_palette_ = std::move(palette.target);
        animations_.push_back(std::move(palette.animation));
    }
}
