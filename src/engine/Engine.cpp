#include "../../include/engine/Engine.hpp"

#include <logging/LogManager.hpp>

#include "BenchmarkRunner.hpp"
#include "CommandLineParser.hpp"
#include "ComputeRunner.hpp"
#include "HierarchyWindow.hpp"
#include "InspectorWindow.hpp"
#include "ReferenceRunner.hpp"
#include "SceneReader.hpp"
#include "YamlLoadProperties.hpp"

namespace RtEngine {
    namespace {
        Logging::LoggerHandle& logger() {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<Engine>();
            return instance;
        }
    }

    void Engine::run(CliArguments cli_args) {
        parseCliArguments(cli_args);

        init();
        mainLoop();
        cleanup();
    }

    void Engine::init() {
        config_properties = std::make_shared<YamlLoadProperties>(options->config_file);

        createWindow();
        createRenderer();
        createEngineContext();
        createGuiManager();
        createRunner();

        setupGui();
    }

    void Engine::createWindow() {
        window = std::make_shared<Window>(1920, 1040);
    }

    void Engine::createRenderer() {
        const bool enable_raytracing = options->runner_type != COMPUTE_ONLY;
        rendering_manager = std::make_shared<RenderingManager>(window, options->resources_dir, true, enable_raytracing);

        auto update_flags = std::make_shared<UpdateFlags>();
        rendering_manager->initRendererProperties(config_properties, update_flags);
    }

    void Engine::createGuiManager() {
        gui_manager = std::make_shared<GuiManager>(scene_manager, rendering_manager->getGuiRenderer());
        gui_manager->addCallbackToAll([&] (const UpdateFlagsHandle& update_flags) {
            runner->setUpdateFlags(update_flags);
        });
    }

    void Engine::createEngineContext() {
        engine_context = std::make_shared<EngineContext>();
        engine_context->window = window;
        engine_context->rendering_manager = rendering_manager;
        engine_context->texture_repository = rendering_manager->getTextureRepository();
        engine_context->mesh_repository = rendering_manager->getMeshRepository();
        auto scene_reader = std::make_shared<SceneReader>(engine_context);
        scene_manager = std::make_shared<SceneManager>(options->resources_dir, rendering_manager->getVulkanContext()->device_manager, scene_reader); // non-interfaced version
        engine_context->scene_manager = scene_manager; // interfaced version for components providing scene information
        engine_context->input_manager = std::make_shared<InputManager>(window);
        engine_context->swapchain_manager = rendering_manager->getSwapchainManager();
        engine_context->sync_manager = rendering_manager->getSyncManager();
    }

    void Engine::createRunner() {
        if (options->runner_type == OFFLINE) {
            runner = std::make_shared<Runner>(engine_context, scene_manager);
            logger()->info("Offline runner created");
        } else if (options->runner_type == REALTIME) {
            //vulkan_renderer = std::make_shared<RealtimeRunner>();
        } else if (options->runner_type == REFERENCE) {
            runner = std::make_shared<ReferenceRunner>(engine_context, scene_manager);
            logger()->info("Reference runner created");
        } else if (options->runner_type == BENCHMARK) {
            runner = std::make_shared<BenchmarkRunner>(engine_context, scene_manager);
            logger()->info("Benchmark runner created");
        } else if (options->runner_type == COMPUTE_ONLY) {
            runner = std::make_shared<ComputeRunner>(engine_context, scene_manager);
            logger()->info("Compute runner created");
        } else {
            logger()->error("No runner created");
            return;
        }

        auto update_flags = std::make_shared<UpdateFlags>();
        runner->initProperties(config_properties, update_flags);
        runner->setUpdateFlags(update_flags);
    }

    void Engine::setupGui() const {
        gui_manager->options_window->addSerializable(runner);
        if (rendering_manager->raytracingEnabled()) {
            gui_manager->options_window->addSerializable(rendering_manager->getRaytracingRenderer());
        }
    }

    void Engine::mainLoop() {
        while (window->is_open() && runner->isRunning()) {
            window->pollEvents();

            runner->renderScene();
            finishFrame();
        }
    }

    void Engine::finishFrame() {
        engine_context->input_manager->reset();
    }

    void Engine::cleanup() {
        rendering_manager->getVulkanContext()->device_manager->waitForIdle();
        scene_manager->destroy();
        rendering_manager->destroy();

        window->destroy();
    }

    void Engine::parseCliArguments(CliArguments cli_args) {
        options = std::make_shared<EngineOptions>();

        CommandLineParser cli_parser = CommandLineParser();

        bool help = false;
        bool reference = false;
        bool benchmark = false;
        bool realtime = false;
        bool compute = false;

        cli_parser.addFlag("--help", &help, "Show this message.");
        cli_parser.addString("--resources", &options->resources_dir,
                             "The path to the directory where all resource files can be found.");
        cli_parser.addString("--config", &options->config_file, "Path to the cofnig file.");
        cli_parser.addFlag("--ref", &reference, "Render a reference image.");
        cli_parser.addFlag("--benchmark", &benchmark, "Render an image and benchmark it against a reference.");
        cli_parser.addFlag("--realtime", &realtime, "Render an image in realtime.");
        cli_parser.addFlag("--compute", &compute, "Run a stack of compute renderers with no scene or raytracing.");
        cli_parser.addFlag("-v", &options->verbose, "Display debug messages.");
        cli_parser.parse(cli_args.argc, cli_args.argv);

        if (help) {
            cli_parser.printHelp();
            return;
        }

        if (benchmark) {
            options->runner_type = BENCHMARK;
        } else if (reference) {
            options->runner_type = REFERENCE;
        } else if (realtime) {
            options->runner_type = REALTIME;
        } else if (compute) {
            options->runner_type = COMPUTE_ONLY;
        } else {
            options->runner_type = OFFLINE;
        }
    }
} // RtEngine
