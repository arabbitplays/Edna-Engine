#include "RenderTarget.hpp"

#include <RandomUtil.hpp>

namespace RtEngine
{
    namespace
    {
        std::vector<uint8_t> generateRngPixels(VkExtent2D extent)
        {
            const size_t uint_count = static_cast<size_t>(extent.width) * extent.height * 4;
            std::vector<uint8_t> pixels(uint_count * sizeof(uint32_t));
            auto *ints = reinterpret_cast<uint32_t *>(pixels.data());
            for (size_t i = 0; i < uint_count; i++) {
                ints[i] = RandomUtil::generateInt();
            }
            return pixels;
        }
    }

    RenderTarget::RenderTarget(const std::shared_ptr<ResourceBuilder>& resource_builder, VkExtent2D image_extent, uint32_t max_frames_in_flight,
                               std::shared_ptr<ImageConnector> render_target_connector)
        : owns_render_target_connector(render_target_connector == nullptr),
          render_target_connector(render_target_connector
              ? std::move(render_target_connector)
              : std::make_shared<ImageConnector>(
                  resource_builder, image_extent, max_frames_in_flight,
                  VK_FORMAT_R32G32B32A32_SFLOAT,
                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                  VK_IMAGE_ASPECT_COLOR_BIT)),
          rng_connector(std::make_shared<ImageConnector>(
              resource_builder, image_extent, max_frames_in_flight, VK_FORMAT_R32G32B32A32_UINT,
              VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, &generateRngPixels))
    {
    }

    void RenderTarget::recreate(const VkExtent2D new_image_extent)
    {
        if (owns_render_target_connector) {
            render_target_connector->recreate(new_image_extent);
        }
        rng_connector->recreate(new_image_extent);
        resetAccumulatedFrames();
    }


    AllocatedImage RenderTarget::getCurrentTargetImage() const
    {
        return render_target_connector->getImageAt(current_image);
    }

    AllocatedImage RenderTarget::getLastTargetImage() const {
        const uint32_t count = render_target_connector->getImageCount();
        const uint32_t idx = current_image != 0 ? current_image - 1 : count - 1;
        return render_target_connector->getImageAt(idx);
    }


    AllocatedImage RenderTarget::getCurrentRngImage() const
    {
        return rng_connector->getImageAt(current_image);
    }

    void RenderTarget::nextImage()
    {
        current_image = (current_image + 1) % render_target_connector->getImageCount();
    }

    VkExtent2D RenderTarget::getExtent() const {
        return render_target_connector->getExtent();
    }

    uint32_t RenderTarget::getAccumulatedFrameCount() const {
        return accumulated_frame_count;
    }

    void RenderTarget::resetAccumulatedFrames() {
        accumulated_frame_count = 0;
    }

    void RenderTarget::incrementAccumulatedFrameCount() {
        accumulated_frame_count++;
    }

    uint32_t RenderTarget::getTotalSampleCount() const {
        return accumulated_frame_count * samples_per_frame;
    }

    uint32_t RenderTarget::getSamplesPerFrame() const {
        return samples_per_frame;
    }

    void RenderTarget::setSamplesPerFrame(uint32_t new_samples_per_frame) {
        samples_per_frame = new_samples_per_frame;
    }

    void RenderTarget::destroy() const {
        if (owns_render_target_connector) {
            render_target_connector->destroy();
        }
        rng_connector->destroy();
    }

    std::shared_ptr<ImageConnector> RenderTarget::getRenderTargetConnector() const {
        return render_target_connector;
    }
}
