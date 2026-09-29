#include <library/animation/runners/DiscreteAnimationTrack.hpp>
#include <library/animation/runners/InterpolatedAnimationTrack.hpp>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationRunner.hpp>
#include <memory>
#include <utility>

namespace cellular_automaton
{
    CyclicalCellularAutomatonAnimationRunner::CyclicalCellularAutomatonAnimationRunner(
        std::function<void(float)> set_mutation_chance,
        std::function<void(const std::vector<glm::ivec2>&)> set_neighborhood,
        std::function<void(uint32_t)> set_threshold, float initial_mutation_chance)
        : generator_(std::move(set_mutation_chance), std::move(set_neighborhood), std::move(set_threshold))
    {
        runner_.addTrack(std::make_unique<::Animation::InterpolatedAnimationTrack<float>>(
            initial_mutation_chance,
            [this](const float& current) -> std::shared_ptr<::Animation::Animation<float>>
            { return std::move(generator_.generateMutationChanceAnimation(current).animation); },
            COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS));

        runner_.addTrack(std::make_unique<::Animation::DiscreteAnimationTrack>(
            [this]() { generator_.applyRandomNeighborhood(); }, COOLDOWN_MIN_SECONDS, COOLDOWN_MAX_SECONDS));
    }
} // namespace cellular_automaton
