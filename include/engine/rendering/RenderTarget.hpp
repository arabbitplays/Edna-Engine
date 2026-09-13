#ifndef RENDERTARGET_HPP
#define RENDERTARGET_HPP
#include <memory>
#include <ImageConnector.hpp>
#include <ResourceBuilder.hpp>
#include <Texture.hpp>

namespace RtEngine
{
    class RenderTarget {
    public:
        RenderTarget(const std::shared_ptr<ResourceBuilder>& resource_builder, VkExtent2D image_extent, uint32_t max_frames_in_flight,
                     std::shared_ptr<ImageConnector> render_target_connector = nullptr);

        AllocatedImage getCurrentTargetImage() const;

        AllocatedImage getLastTargetImage() const;

        AllocatedImage getCurrentRngImage() const;
        void nextImage();

        VkExtent2D getExtent() const;

        uint32_t getAccumulatedFrameCount() const;
        void resetAccumulatedFrames();
        void incrementAccumulatedFrameCount();

        uint32_t getTotalSampleCount() const;

        uint32_t getSamplesPerFrame() const;
        void setSamplesPerFrame(uint32_t new_samples_per_frame);

        void recreate(VkExtent2D new_image_extent);

        void destroy() const;

        std::shared_ptr<ImageConnector> getRenderTargetConnector() const;
    private:
        bool owns_render_target_connector;
        std::shared_ptr<ImageConnector> render_target_connector;
        std::shared_ptr<ImageConnector> rng_connector;

        uint32_t current_image = 0;

        uint32_t accumulated_frame_count = 0;
        uint32_t samples_per_frame = 8;
    };
}



#endif //RENDERTARGET_HPP
