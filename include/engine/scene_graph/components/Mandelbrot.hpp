#ifndef EDNA_ENGINE_MANDELBROT_HPP
#define EDNA_ENGINE_MANDELBROT_HPP
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <library/mandelbrot/animation/MandelbrotAnimationRunner.hpp>

#include "Component.hpp"
#include "SwapchainManager.hpp"

namespace RtEngine {
    class MandelbrotRenderer;

    class Mandelbrot : public Component {
    public:
        Mandelbrot() = default;
        Mandelbrot(const std::shared_ptr<EngineContext>& context,
                   const std::shared_ptr<Node>& node)
            : Component(context, node) {}

        static inline const std::string COMPONENT_NAME = "Mandelbrot";

        void OnStart() override;
        void OnRender(DrawContext&) override {}
        void OnUpdate() override;
        void OnDestroy() override;

        void initProperties(const std::shared_ptr<IProperties>& config,
                            const UpdateFlagsHandle& update_flags) override;

    private:
        static constexpr float    DEFAULT_STEP_SIZE = 0.0001f;
        static constexpr uint32_t DEFAULT_MAX_ITERATIONS = 128u;
        static constexpr uint32_t MIN_MAX_ITERATIONS = 16u;
        static constexpr uint32_t MAX_MAX_ITERATIONS = 1024u;
        static constexpr float    ORIGIN_BOUND = 5.0f;
        static constexpr float    OFFSET_BOUND = 5.0f;
        static constexpr float    INITIAL_BOUND = 2.0f;
        static inline const std::string DEFAULT_PALETTE_NAME = "Sunburn";

        glm::vec2 origin = glm::vec2(-1.0, .0f);
        glm::vec2 offset = glm::vec2(0.0f);
        float    step_size = DEFAULT_STEP_SIZE;
        uint32_t max_iterations = DEFAULT_MAX_ITERATIONS;
        glm::vec2 initial_number = glm::vec2(0.0f);
        bool     julia_mode = false;
        bool     animate = false;
        std::string palette_name = DEFAULT_PALETTE_NAME;
        std::string applied_palette_name;

        std::shared_ptr<MandelbrotRenderer> renderer;
        std::unique_ptr<::mandelbrot::MandelbrotAnimationRunner> animation_runner;

        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;
    };
} // RtEngine

#endif //EDNA_ENGINE_MANDELBROT_HPP
