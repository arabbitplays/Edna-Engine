//
// Created by oschdi on 4/27/25.
//

#include <cassert>
#include <cstring>
#include <DeviceManager.hpp>
#include <set>
#include <Swapchain.hpp>
#include <VulkanUtil.hpp>

namespace RtEngine
{
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
        //	"VK_LAYER_PROFILER_unified",
    };

    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    };

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR DeviceManager::RAYTRACING_PROPERTIES{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};

    VkResult createDebugUtilsMessengerExt(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
        const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger)
    {
        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, p_create_info, p_allocator, p_debug_messenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void destroyDebugUtilsMessengerExt(
        VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator)
    {
        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debug_messenger, p_allocator);
        }
    }

    // --------------------------------------------------------------------------------------------------------------------

    DeviceManager::DeviceManager(GLFWwindow* window, bool enable_validation_layers)
    {
        createInstance(enable_validation_layers);
        if (enable_validation_layers)
        {
            setupDebugMessenger();
        }
        createSurface(window);
        pickPhysicalDevice();
        createLogicalDevice(enable_validation_layers);
    }

    VkPhysicalDevice DeviceManager::getPhysicalDevice() const
    {
        return physicalDevice;
    }

    VkDevice DeviceManager::getDevice() const
    {
        return device;
    }

    VkSurfaceKHR DeviceManager::getSurface() const
    {
        return surface;
    }

    VkInstance DeviceManager::getInstance() const
    {
        return instance;
    }

    QueueFamilyIndices DeviceManager::getQueueIndices() const
    {
        return queue_indices;
    }

    VkQueue DeviceManager::getQueue(QueueType type) const
    {
        switch (type)
        {
        case QueueType::GRAPHICS:
            return graphics_queue;
        case QueueType::PRESENT:
            return present_queue;
        case QueueType::COMPUTE:
            return compute_queue;
        }

        throw std::runtime_error("Unknown QueueType");
    }

    void DeviceManager::createInstance(bool enable_validation_layers)
    {
        if (enable_validation_layers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        std::vector<const char*> glfw_extensions = getRequiredExtensions(enable_validation_layers);
        create_info.enabledExtensionCount = static_cast<uint32_t>(glfw_extensions.size());
        create_info.ppEnabledExtensionNames = glfw_extensions.data();

        // Add an extra debug messenger for the create and destroy instance calls
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populateDebugMessengerCreateInfo(debug_create_info);
            create_info.pNext = (&debug_create_info);
        }
        else
        {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }

        deletion_queue.pushFunction([&]() { vkDestroyInstance(instance, nullptr); });
    }

    bool DeviceManager::checkValidationLayerSupport()
    {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char* layer_name : validation_layers)
        {
            bool layer_found = false;
            for (const auto& layer_properties : available_layers)
            {
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> DeviceManager::getRequiredExtensions(bool enable_validation_layers)
    {
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        if (enable_validation_layers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void DeviceManager::setupDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info;
        populateDebugMessengerCreateInfo(create_info);

        if (createDebugUtilsMessengerExt(instance, &create_info, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }

        deletion_queue.pushFunction([&]() { destroyDebugUtilsMessengerExt(instance, debugMessenger, nullptr); });
    }

    void DeviceManager::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info)
    {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#ifdef VERBOSE
        createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debugCallback;
        create_info.pUserData = nullptr; // Optional data that is passed via the pUserData parameter to the callback
    }

    void DeviceManager::createSurface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }

        deletion_queue.pushFunction([&]() { vkDestroySurfaceKHR(instance, surface, nullptr); });
    }

    void DeviceManager::pickPhysicalDevice()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        if (device_count == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
        for (auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find suitable GPU!");
        }

        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &device_properties);
        VkPhysicalDeviceProperties2 physical_device_properties;
        physical_device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physical_device_properties.pNext = &RAYTRACING_PROPERTIES;
        vkGetPhysicalDeviceProperties2(physicalDevice, &physical_device_properties);
    }

    bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_pipeline_features{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
        VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES};
        raytracing_pipeline_features.pNext = &acceleration_structure_features;
        acceleration_structure_features.pNext = &timeline_semaphore_features;
        VkPhysicalDeviceFeatures2 device_features2;
        device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        device_features2.pNext = &raytracing_pipeline_features;
        vkGetPhysicalDeviceFeatures2(device, &device_features2);

        // implement device checks here

        bool extensions_supported = checkDeviceExtensionSupport(device);
        QueueFamilyIndices indices = VulkanUtil::findQueueFamilies(device, surface);

        bool swap_chain_adequate = false;
        if (extensions_supported)
        {
            SwapChainSupportDetails swap_chain_support = Swapchain::querySwapChainSupport(device, surface);
            swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.presentModes.empty();
        }

        return extensions_supported && indices.isComplete() && swap_chain_adequate &&
               (device_features.samplerAnisotropy != 0u) && (device_features.shaderInt64 != 0u) &&
               (device_features.shaderFloat64 != 0u) && (raytracing_pipeline_features.rayTracingPipeline != 0u) &&
               (acceleration_structure_features.accelerationStructure != 0u) &&
               (timeline_semaphore_features.timelineSemaphore != 0u);
    }

    bool DeviceManager::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extension_count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

        std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
        for (const auto& extension : available_extensions)
        {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }

    void DeviceManager::createLogicalDevice(bool enable_validation_layers)
    {
        queue_indices = VulkanUtil::findQueueFamilies(physicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        // so no family is created multiple times if it covers multiple types
        std::set<uint32_t> unique_queue_families = {
            queue_indices.graphicsAndComputeFamily.value(), queue_indices.presentFamily.value()};

        float queue_priority = 1.0F;
        for (uint32_t queue_family : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;
        device_features.shaderInt64 = VK_TRUE;
        device_features.shaderFloat64 = VK_TRUE;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};
        acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        acceleration_structure_features.accelerationStructure = VK_TRUE;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features{};
        ray_tracing_pipeline_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        ray_tracing_pipeline_features.rayTracingPipeline = VK_TRUE;
        ray_tracing_pipeline_features.pNext = &acceleration_structure_features;

        VkPhysicalDeviceBufferDeviceAddressFeatures device_address_features{};
        device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        device_address_features.bufferDeviceAddress = VK_TRUE;
        device_address_features.pNext = &ray_tracing_pipeline_features;

        VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features{};
        timeline_semaphore_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timeline_semaphore_features.timelineSemaphore = VK_TRUE;
        timeline_semaphore_features.pNext = &device_address_features;

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();
        if (enable_validation_layers)
        { // device validation alyers are deprecated, only set for compatibility
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        }
        else
        {
            create_info.enabledLayerCount = 0;
        }
        create_info.pNext = &timeline_semaphore_features;

        if (vkCreateDevice(physicalDevice, &create_info, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        deletion_queue.pushFunction([&]() { vkDestroyDevice(device, nullptr); });

        vkGetDeviceQueue(device, queue_indices.graphicsAndComputeFamily.value(), 0, &graphics_queue);
        vkGetDeviceQueue(device, queue_indices.presentFamily.value(), 0, &present_queue);
        vkGetDeviceQueue(device, queue_indices.graphicsAndComputeFamily.value(), 0, &compute_queue);

        // The renderer stack submits per-renderer to getQueue(renderer->queueType()) and relies on the
        // timeline semaphore for ordering. If GRAPHICS and COMPUTE ever resolve to different queue families,
        // storage-image handoff between renderers needs an explicit ownership transfer or CONCURRENT sharing.
        assert(graphics_queue == compute_queue && "GRAPHICS and COMPUTE queues must be the same queue");
    }

    void DeviceManager::waitForIdle() const
    {
        vkDeviceWaitIdle(device);
    }

    void DeviceManager::destroy()
    {
        deletion_queue.flush();
    }

} // namespace RtEngine
