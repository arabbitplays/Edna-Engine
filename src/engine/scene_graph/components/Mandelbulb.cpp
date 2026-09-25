#include "Mandelbulb.hpp"

#include "Camera.hpp"
#include "compute/MandelbulbRenderer.hpp"
#include "Scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <library/color/ColorPaletteFactory.hpp>
#include <library/color/ColorPaletteName.hpp>
#include <library/mandelbulb/animation/MandelbulbAnimationGenerator.hpp>
#include <logging/LogManager.hpp>

using namespace color;

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<Mandelbulb>();
            return instance;
        }

        std::vector<glm::vec4> loadPaletteColors(const std::string& name)
        {
            const auto palette_name = ColorPaletteName::fromString(name, ColorPaletteName::Sunburn);
            return ColorPaletteFactory::create(palette_name).colors;
        }

        const std::vector<std::string>& coloringModeNames()
        {
            static const std::vector<std::string> names = {"Iterations", "OrbitTrap", "Radius"};
            return names;
        }

        MandelbulbRenderer::ColoringMode parseColoringMode(const std::string& name)
        {
            if (name == "Iterations") return MandelbulbRenderer::ColoringMode::Iterations;
            if (name == "Radius") return MandelbulbRenderer::ColoringMode::Radius;
            return MandelbulbRenderer::ColoringMode::OrbitTrap;
        }
    } // namespace

    void Mandelbulb::OnStart()
    {
        const std::shared_ptr<RenderingManager> rendering_manager = context->rendering_manager;
        const std::shared_ptr<VulkanContext> vulkan_context = rendering_manager->getVulkanContext();
        const VkExtent2D extent = vulkan_context->swapchain->extent;

        const std::vector<glm::vec4> colors = loadPaletteColors(palette_name);
        applied_palette_name = palette_name;

        renderer = std::make_shared<MandelbulbRenderer>(
            vulkan_context, extent, colors, initial, power, max_iterations, parseColoringMode(coloring_mode));
        renderer->init();

        rendering_manager->addComputeRenderer(renderer, renderer->getOutputConnector());

        // Runner writes to the same fields the manual controls set, so
        // toggling `animate` just decides who owns the values.
        animation_runner = std::make_unique<::mandelbulb::MandelbulbAnimationRunner>(
            [this](float v) { power = v; },
            [this](float v) { theta_offset = v; },
            [this](float v) { step_rotation_angle = v; },
            [this](const glm::vec3& v) { step_rotation_axis = v; },
            power, theta_offset, step_rotation_angle, step_rotation_axis);

        resize_callback_handle = context->swapchain_manager->addRecreateCallback(
            [this](uint32_t width, uint32_t height) { renderer->handleResize(VkExtent2D{width, height}); });
    }

    void Mandelbulb::OnDestroy()
    {
        if (resize_callback_handle != 0 && context && context->swapchain_manager)
        {
            context->swapchain_manager->removeRecreateCallback(resize_callback_handle);
            resize_callback_handle = 0;
        }
    }

    void Mandelbulb::OnUpdate()
    {
        if (!renderer)
        {
            return;
        }
        if (!update_gate.tick())
        {
            return;
        }

        if (palette_name != applied_palette_name)
        {
            renderer->setPalette(loadPaletteColors(palette_name));
            applied_palette_name = palette_name;
        }

        if (camera.expired() && context && context->scene_manager)
        {
            camera = context->scene_manager->getComponent<Camera>();
        }

        if (animate && animation_runner)
        {
            animation_runner->update();
        }

        rotation_angle += rotation_speed * FIXED_DELTA_TIME;
        const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));

        const glm::vec3 axis = glm::length(step_rotation_axis) > 1e-4f
            ? glm::normalize(step_rotation_axis)
            : glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::mat4 step_rotation =
            glm::rotate(glm::mat4(1.0f), step_rotation_angle, axis);

        if (const auto cam = camera.lock())
        {
            renderer->setCamera(cam->getInverseView(), cam->getInverseProjection());
        }
        renderer->setRotation(rotation);
        renderer->setStepRotation(step_rotation);
        renderer->setInitial(initial);
        renderer->setPower(power);
        renderer->setThetaOffset(theta_offset);
        renderer->setDiffuseWeight(diffuse_weight);
        renderer->setRimWeight(rim_weight);
        renderer->setMaxIterations(max_iterations);
        renderer->setColoringMode(parseColoringMode(coloring_mode));
    }

    std::shared_ptr<ImageConnector> Mandelbulb::getOutputConnector() const
    {
        return renderer ? renderer->getOutputConnector() : nullptr;
    }

    void Mandelbulb::initProperties(
        const std::shared_ptr<IProperties>& config, const UpdateFlagsHandle& /*update_flags*/)
    {
        if (config->startChild(COMPONENT_NAME))
        {
            config->addVector("initial", &initial);
            config->addFloat("power", &power, MIN_POWER, MAX_POWER);
            config->addFloat("theta_offset", &theta_offset, -ANGLE_OFFSET_BOUND, ANGLE_OFFSET_BOUND);
            config->addVector("step_rotation_axis", &step_rotation_axis);
            config->addFloat("step_rotation_angle", &step_rotation_angle, -STEP_ROTATION_ANGLE_BOUND,
                STEP_ROTATION_ANGLE_BOUND);
            config->addUint("max_iterations", &max_iterations, MIN_MAX_ITERATIONS, MAX_MAX_ITERATIONS);
            config->addFloat("rotation_speed", &rotation_speed, -ROTATION_SPEED_BOUND, ROTATION_SPEED_BOUND);
            config->addFloat("diffuse_weight", &diffuse_weight, 0.0f, 2.0f);
            config->addFloat("rim_weight", &rim_weight, 0.0f, 1.0f);
            config->addBool("animate", &animate);
            config->addSelection("palette", &palette_name, ::color::ColorPaletteName::getAllNames());
            config->addSelection("coloring", &coloring_mode, coloringModeNames());
            config->endChild();
        }
    }
} // namespace RtEngine
