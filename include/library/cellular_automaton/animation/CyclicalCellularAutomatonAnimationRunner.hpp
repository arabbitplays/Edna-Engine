#ifndef EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
#define EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP

#include <cstdint>
#include <functional>
#include <glm/vec2.hpp>
#include <library/animation/runners/AnimationRunner.hpp>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationGenerator.hpp>
#include <vector>

namespace cellular_automaton
{
    class CyclicalCellularAutomatonAnimationRunner
    {
    public:
        static constexpr float COOLDOWN_MIN_SECONDS = 10.0f;
        static constexpr float COOLDOWN_MAX_SECONDS = 30.0f;

        CyclicalCellularAutomatonAnimationRunner(std::function<void(float)> set_mutation_chance,
            std::function<void(const std::vector<glm::ivec2>&)> set_neighborhood,
            std::function<void(uint32_t)> set_threshold, float initial_mutation_chance);

        void update()
        {
            runner_.update();
        }

    private:
        CyclicalCellularAutomatonAnimationGenerator generator_;
        ::Animation::AnimationRunner runner_;
    };
} // namespace cellular_automaton

#endif // EDNA_ENGINE_CCA_ANIMATIONRUNNER_HPP
