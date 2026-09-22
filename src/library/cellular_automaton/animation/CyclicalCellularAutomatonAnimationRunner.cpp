#include <cstdint>
#include <library/cellular_automaton/animation/CyclicalCellularAutomatonAnimationRunner.hpp>
#include <limits>
#include <util/RandomUtil.hpp>
#include <utility>

namespace cellular_automaton
{
    namespace
    {
        float randomCooldown()
        {
            using Runner = CyclicalCellularAutomatonAnimationRunner;
            const float t = static_cast<float>(RtEngine::RandomUtil::generateInt()) /
                            static_cast<float>(std::numeric_limits<uint32_t>::max());
            return Runner::COOLDOWN_MIN_SECONDS + (t * (Runner::COOLDOWN_MAX_SECONDS - Runner::COOLDOWN_MIN_SECONDS));
        }
    } // namespace

    CyclicalCellularAutomatonAnimationRunner::CyclicalCellularAutomatonAnimationRunner(
        CyclicalCellularAutomatonAnimationGenerator generator, float initial_mutation_chance,
        ::color::ColorPalette initial_palette)
        : generator_(std::move(generator)), mutation_current_(initial_mutation_chance),
          palette_current_(std::move(initial_palette)), last_tick_(std::chrono::steady_clock::now())
    {
        startMutation();
        startPalette();
        generator_.applyRandomNeighborhood();
        neighborhood_cooldown_seconds_ = randomCooldown();
    }

    void CyclicalCellularAutomatonAnimationRunner::update()
    {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - last_tick_).count();
        last_tick_ = now;

        tick(mutation_track_, dt, &CyclicalCellularAutomatonAnimationRunner::startMutation);
        tick(palette_track_, dt, &CyclicalCellularAutomatonAnimationRunner::startPalette);

        neighborhood_cooldown_seconds_ -= dt;
        if (neighborhood_cooldown_seconds_ <= 0.0F)
        {
            generator_.applyRandomNeighborhood();
            neighborhood_cooldown_seconds_ = randomCooldown();
        }
    }

    void CyclicalCellularAutomatonAnimationRunner::tick(
        Track& track, float dt, void (CyclicalCellularAutomatonAnimationRunner::*start)())
    {
        if (track.animation)
        {
            track.animation->step();
            if (track.animation->finished())
            {
                track.animation.reset();
                track.cooldown_seconds = randomCooldown();
            }
            return;
        }

        track.cooldown_seconds -= dt;
        if (track.cooldown_seconds <= 0.0F)
        {
            track.cooldown_seconds = 0.0F;
            (this->*start)();
        }
    }

    void CyclicalCellularAutomatonAnimationRunner::startMutation()
    {
        auto mutation = generator_.generateMutationChanceAnimation(mutation_current_);
        mutation_current_ = mutation.target;
        mutation_track_.animation = std::move(mutation.animation);
    }

    void CyclicalCellularAutomatonAnimationRunner::startPalette()
    {
        auto palette = generator_.generatePaletteAnimation(palette_current_);
        palette_current_ = std::move(palette.target);
        palette_track_.animation = std::move(palette.animation);
    }
} // namespace cellular_automaton
