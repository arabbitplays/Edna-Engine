#ifndef VULKANCONTEXT_HPP
#define VULKANCONTEXT_HPP

#include <engine/Window.hpp>

#include <engine/rendering/CommandManager.hpp>
#include <engine/rendering/DescriptorAllocator.hpp>
#include <engine/rendering/context/DeviceManager.hpp>
#include <engine/rendering/builders/MeshAssetBuilder.hpp>
#include <engine/rendering/builders/ResourceBuilder.hpp>
#include <engine/rendering/Swapchain.hpp>

namespace RtEngine
{
    struct VulkanContext
    {
        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<Window> window;
        std::shared_ptr<Swapchain> swapchain;

        std::shared_ptr<ResourceBuilder> resource_builder;
        std::shared_ptr<DescriptorAllocator> descriptor_allocator;
        std::shared_ptr<CommandManager> command_manager;
    };

} // namespace RtEngine
#endif // VULKANCONTEXT_HPP
