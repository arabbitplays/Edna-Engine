#include "GuiRenderer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <RenderPassBuilder.hpp>
#include <stdexcept>

// only used as imgui callback
namespace RtEngine
{
    void checkVulkanResult(VkResult err)
    {
        if (err == 0)
        {
            return;
        }
        throw std::runtime_error("Error: VkResult = " + err);
    }

    GuiRenderer::GuiRenderer(const std::shared_ptr<VulkanContext>& context) : context(context)
    {
        VkDevice device = context->device_manager->getDevice();
        createRenderPass(device, context->swapchain->imageFormat);
        createFrameBuffers(device, context->swapchain);
        createDescriptorPool(device);

        deletion_queue.pushFunction(
            [&]()
            {
                vkDestroyRenderPass(this->context->device_manager->getDevice(), render_pass, nullptr);
                vkDestroyDescriptorPool(this->context->device_manager->getDevice(), descriptor_pool, nullptr);
            });

        initImGui(context->device_manager, context->window->getHandle(), context->swapchain);
    }

    void GuiRenderer::createRenderPass(VkDevice device, VkFormat image_format)
    {
        RenderPassBuilder render_pass_builder;
        render_pass_builder.setColorAttachmentFormat(image_format);
        render_pass = render_pass_builder.createRenderPass(device);
    }

    void GuiRenderer::createFrameBuffers(VkDevice device, const std::shared_ptr<Swapchain>& swapchain)
    {
        frame_buffers.resize(swapchain->imageViews.size());
        for (size_t i = 0; i < frame_buffers.size(); i++)
        {
            std::array<VkImageView, 1> attachments{
                swapchain->imageViews[i],
            };

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = render_pass;
            framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_info.pAttachments = attachments.data();
            framebuffer_info.width = swapchain->extent.width;
            framebuffer_info.height = swapchain->extent.height;
            framebuffer_info.layers = 1;

            if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &frame_buffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void GuiRenderer::createDescriptorPool(VkDevice device)
    {
        std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };
        descriptor_pool =
            DescriptorAllocator::createPool(device, pool_sizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    }

    void GuiRenderer::initImGui(const std::shared_ptr<DeviceManager>& device_manager, GLFWwindow* window,
        const std::shared_ptr<Swapchain>& swapchain)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device_manager->getInstance();
        init_info.PhysicalDevice = device_manager->getPhysicalDevice();
        init_info.Device = device_manager->getDevice();
        init_info.QueueFamily = device_manager->getQueueIndices().graphicsAndComputeFamily.value();
        init_info.Queue = device_manager->getQueue(GRAPHICS);
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptor_pool;
        init_info.RenderPass = render_pass;
        init_info.Subpass = 0;
        init_info.MinImageCount = minImageCount;
        init_info.ImageCount = static_cast<uint32_t>(swapchain->images.size());
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = checkVulkanResult;
        ImGui_ImplVulkan_Init(&init_info);
    }

    void GuiRenderer::addWindow(const std::shared_ptr<GuiWindow>& window)
    {
        gui_windows.push_back(window);
    }

    void GuiRenderer::recreateFramebuffer()
    {
        for (auto* framebuffer : frame_buffers)
        {
            vkDestroyFramebuffer(context->device_manager->getDevice(), framebuffer, nullptr);
        }

        createFrameBuffers(context->device_manager->getDevice(), context->swapchain);
    }

    void GuiRenderer::recordGuiCommands(VkCommandBuffer command_buffer, uint32_t image_index)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (auto& window : gui_windows)
        {
            window->createFrame();
        }

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        std::array<VkClearValue, 1> clear_values = {};
        clear_values[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};

        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = render_pass;
        info.framebuffer = frame_buffers[image_index];
        info.renderArea.extent = context->swapchain->extent;
        info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

        vkCmdEndRenderPass(command_buffer);
    }

    void GuiRenderer::cleanup()
    {
        shutdownImGui();

        for (auto* framebuffer : frame_buffers)
        {
            vkDestroyFramebuffer(context->device_manager->getDevice(), framebuffer, nullptr);
        }
        deletion_queue.flush();
    }

    void GuiRenderer::shutdownImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace RtEngine
