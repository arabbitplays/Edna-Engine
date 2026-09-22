#include "ReferenceRunner.hpp"

#include "ImageUtil.hpp"
#include "PathUtil.hpp"

#include <cmath>
#include <filesystem>
#include <format>
#include <logging/LogManager.hpp>
#include <utility>

namespace RtEngine
{
    namespace
    {
        Logging::LoggerHandle& logger()
        {
            static Logging::LoggerHandle instance = Logging::LogManager::getClassLogger<ReferenceRunner>();
            return instance;
        }
    } // namespace

    constexpr std::string sample_count_option_name = "Sample_Count";

    ReferenceRunner::ReferenceRunner(
        const std::shared_ptr<EngineContext>& engine_context, const std::shared_ptr<SceneManager>& scene_manager)
        : Runner(engine_context, scene_manager)
    {
        final_image_count = final_sample_count / samples_per_image;
        if (std::filesystem::create_directories(OUT_FOLDER))
        {
            logger()->info(std::format("Created directory {}", OUT_FOLDER));
        }
    }

    void ReferenceRunner::loadScene(const std::string& scene_path)
    {
        Runner::loadScene(scene_path);

        scene_manager->getCurrentScene()->update();
        draw_context = createMainDrawContext();

        assert(draw_context->targets.size() == 1);
        std::shared_ptr<RenderTarget> target = draw_context->targets[0];
        target->setSamplesPerFrame(8);

        stopwatch_start = std::chrono::steady_clock::now();
    }

    void ReferenceRunner::renderScene()
    {
        if (update_flags->checkFlag(SCENE_UPDATE))
        {
            loadScene(scene_manager->getScenePath(scene_name));
        }
        std::shared_ptr<RenderTarget> target = draw_context->targets[0];

        if (std::cmp_equal(samples_per_image, target->getTotalSampleCount()))
        {
            waitForIdle();

            float* data = raytracing_renderer->downloadRenderTarget(target);
            done_images.push_back(data);

            if (done_images.size() == final_image_count)
            {
                running = false;
                mergeImages(target->getExtent().width, target->getExtent().height);
            }
            else
            {
                target->resetAccumulatedFrames();
                present_sample_count = 8;
            }
            return;
        }

        drawFrame(draw_context);
    }

    void ReferenceRunner::drawFrame(const std::shared_ptr<DrawContext>& draw_context)
    {
        sync_manager->waitForNextFrameStart();

        std::shared_ptr<RenderTarget> target = draw_context->targets[0];

        uint32_t curr_sample_count = target->getTotalSampleCount();
        bool present_image = present_sample_count <= curr_sample_count;

        int32_t swapchain_image_idx = 0;
        if (present_image)
        {
            swapchain_image_idx = present_stage->acquireNextSwapchainImage();
            if (swapchain_image_idx < 0)
            {
                handleResize();
                return;
            }
            present_sample_count *= 2;
        }

        const uint32_t frame_idx = sync_manager->currentFrameInFlight();

        prepareFrame(draw_context, frame_idx);

        raytracing_renderer->writeRenderTarget(target);

        renderFrame(frame_idx, static_cast<uint32_t>(swapchain_image_idx), present_image);
        finishFrame(draw_context);

        if (curr_sample_count % (1 << 10) == 0)
        {
            double elapsed_time =
                std::chrono::duration<double>(std::chrono::steady_clock::now() - stopwatch_start).count();
            uint32_t collected_sample_count = (done_images.size() * samples_per_image) + curr_sample_count;

            uint32_t samples_left = final_sample_count - collected_sample_count;
            double time_left = elapsed_time / collected_sample_count * samples_left;
            int hours = static_cast<int>(time_left) / 3600;
            int minutes = (static_cast<int>(time_left) % 3600) / 60;
            int sec = static_cast<int>(time_left) % 60;
            uint32_t progress = std::round(
                static_cast<float>(collected_sample_count) / static_cast<float>(final_sample_count) * 100.0F);
            logger()->info(
                std::format("Collected sample count: {}, progress: {}%, estimated time remaining: {}h {}m {}s",
                    collected_sample_count, progress, hours, minutes, sec));
        }
    }

    void ReferenceRunner::prepareFrame(const std::shared_ptr<DrawContext>& draw_context, uint32_t frame_idx)
    {
        raytracing_renderer->writeResources(draw_context, update_flags, frame_idx);
        update_flags->resetFlags();
    }

    void ReferenceRunner::mergeImages(const uint32_t width, const uint32_t height)
    {
        std::vector<float*> merged_images{};
        while (done_images.size() != 1)
        {
            assert(done_images.size() % 2 == 0);
            for (uint32_t i = 0; i < done_images.size(); i += 2)
            {
                merged_images.push_back(calculateMean(done_images[i], done_images[i + 1], width * height * 4));
            }
            done_images = merged_images;
            merged_images.clear();
        }

        uint8_t* fixed_data = raytracing_renderer->fixImageFormatForStorage(
            done_images[0], width * height, VK_FORMAT_R32G32B32A32_SFLOAT);
        ImageUtil::writePNG(getOutputImagePath(final_sample_count), fixed_data, width, height);

        delete[] fixed_data;
        done_images.clear(); // every pointer is now invalid anyway
    }

    std::string ReferenceRunner::getTmpImagePath(uint32_t image_idx, uint32_t samples)
    {
        std::string scene_name = PathUtil::getFileName(scene_manager->getCurrentScene()->path);
        return std::format("{}/{}_{}_{}.png", TMP_FOLDER, samples, scene_name, image_idx);
    }

    std::string ReferenceRunner::getOutputImagePath(uint32_t samples)
    {
        std::string scene_name = PathUtil::getFileName(scene_manager->getCurrentScene()->path);
        return std::format("{}/{}_{}.png", OUT_FOLDER, samples, scene_name);
    }

    float* ReferenceRunner::calculateMean(float* img_a, const float* img_b, uint32_t size)
    {
        for (int i = 0; std::cmp_less(i, size); ++i)
        {
            img_a[i] = (img_a[i] + img_b[i]) / 2.0F;
        }

        delete[] img_b;
        return img_a;
    }

    /*void ReferenceRunner::initProperties() {
        RaytracingRenderer::initProperties();
        renderer_properties->addInt(SAMPLE_COUNT_OPTION_NAME, &sample_count);
    }*/
} // namespace RtEngine
