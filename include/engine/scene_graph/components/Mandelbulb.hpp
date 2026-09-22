#ifndef EDNA_ENGINE_MANDELBULB_HPP
#define EDNA_ENGINE_MANDELBULB_HPP
#include "Component.hpp"
#include "SwapchainManager.hpp"

#include <FrameGate.hpp>
#include <glm/glm.hpp>
#include <library/mandelbulb/animation/MandelbulbAnimationRunner.hpp>
#include <memory>
#include <string>

namespace RtEngine
{
    class MandelbulbRenderer;
    class Camera;

    class Mandelbulb : public Component
    {
    public:
        Mandelbulb() = default;
        Mandelbulb(const std::shared_ptr<EngineContext>& context, const std::shared_ptr<Node>& node)
            : Component(context, node)
        {
        }

        static inline const std::string COMPONENT_NAME = "Mandelbulb";

        void OnStart() override;
        void OnRender(DrawContext&) override
        {
        }
        void OnUpdate() override;
        void OnDestroy() override;

        void initProperties(const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& update_flags) override;

        std::shared_ptr<ImageConnector> getOutputConnector() const override;

        std::shared_ptr<MandelbulbRenderer> getRenderer() const
        {
            return renderer;
        }

    private:
        static constexpr uint32_t DEFAULT_MAX_ITERATIONS = 8u;
        static constexpr uint32_t MIN_MAX_ITERATIONS = 2u;
        static constexpr uint32_t MAX_MAX_ITERATIONS = 32u;
        static constexpr float DEFAULT_POWER = 8.0f;
        static constexpr float MIN_POWER = 2.0f;
        static constexpr float MAX_POWER = 16.0f;
        static constexpr float ROTATION_SPEED_BOUND = 6.283185f;
        static constexpr float ANGLE_OFFSET_BOUND = 3.14159265f;
        static constexpr float STEP_ROTATION_ANGLE_BOUND = 0.78539816f; // pi/4
        static inline const std::string DEFAULT_PALETTE_NAME = "Sunburn";

        glm::vec3 initial = glm::vec3(0.0f);
        float power = DEFAULT_POWER;
        float theta_offset = 0.0f;
        glm::vec3 step_rotation_axis = glm::vec3(0.0f, 1.0f, 0.0f);
        float step_rotation_angle = 0.0f;
        uint32_t max_iterations = DEFAULT_MAX_ITERATIONS;
        float rotation_speed = 0.5f;
        float diffuse_weight = 0.85f;
        float rim_weight = 0.18f;
        bool animate = false;
        std::string palette_name = DEFAULT_PALETTE_NAME;
        std::string applied_palette_name;
        std::string coloring_mode = "OrbitTrap";

        float rotation_angle = 0.0f;

        std::shared_ptr<MandelbulbRenderer> renderer;
        std::weak_ptr<Camera> camera;
        std::unique_ptr<::mandelbulb::MandelbulbAnimationRunner> animation_runner;

        FrameGate update_gate{60.0f};
        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;
    };
} // namespace RtEngine

#endif // EDNA_ENGINE_MANDELBULB_HPP
