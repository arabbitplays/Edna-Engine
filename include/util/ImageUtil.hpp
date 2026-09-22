//
// Created by oschdi on 22.01.26.
//

#ifndef VULKAN_RAYTRACING_IMAGEUTIL_HPP
#define VULKAN_RAYTRACING_IMAGEUTIL_HPP
#include <stb_image.h>
#include <stb_image_write.h>
#include <format>
#include <string>
#include <logging/LogManager.hpp>

class ImageUtil {
public:
    ImageUtil() = delete;
    ~ImageUtil() = delete;

    static void writePNG(std::string path, uint8_t *data, uint32_t width, uint32_t height) {
        Logging::LoggerHandle logger = Logging::LogManager::getClassLogger<ImageUtil>();
        if (stbi_write_png(path.c_str(), width, height, 4, data, width * 4)) {
            logger->info(std::format("Saved rendered image to {}!", path));
        } else {
            logger->error(std::format("failed to save output image to {}!", path));
        }
    }

    static uint8_t* loadPNG(std::string path, int32_t* width, int32_t* height) {
        int32_t channels;

        uint8_t* data = stbi_load(path.c_str(), width, height, &channels, 0);

        if (!data)
            throw std::runtime_error("Failed to load PNG " + path);
        assert(channels == 4);

        return data;
    }
};

#endif //VULKAN_RAYTRACING_IMAGEUTIL_HPP
