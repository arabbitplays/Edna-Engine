#ifndef EDNA_ENGINE_FRAMEGATE_HPP
#define EDNA_ENGINE_FRAMEGATE_HPP
#include <chrono>

namespace RtEngine {
    // tick() returns true at most once per 1/target_fps seconds; the first
    // call after construction always returns true so callers do their setup
    // work on the first frame.
    class FrameGate {
    public:
        explicit FrameGate(float target_fps)
            : interval_seconds(1.0f / target_fps) {}

        bool tick() {
            const auto now = std::chrono::steady_clock::now();
            if (first) {
                first = false;
                last_tick = now;
                return true;
            }
            const float elapsed = std::chrono::duration<float>(now - last_tick).count();
            if (elapsed < interval_seconds) return false;
            last_tick = now;
            return true;
        }

        void setTargetFps(float fps) { interval_seconds = 1.0f / fps; }

    private:
        float interval_seconds;
        std::chrono::steady_clock::time_point last_tick;
        bool first = true;
    };
}

#endif //EDNA_ENGINE_FRAMEGATE_HPP
