#include "AccelerationStructure.hpp"

#include <cassert>
#include <stdexcept>

namespace RtEngine
{
    void getAccelerationStructureBuildSizesKhr(VkDevice device, VkAccelerationStructureBuildTypeKHR build_type,
        const VkAccelerationStructureBuildGeometryInfoKHR* p_build_info, const uint32_t* p_max_primitive_counts,
        VkAccelerationStructureBuildSizesInfoKHR* p_size_info)
    {
        auto func = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(
            device, "vkGetAccelerationStructureBuildSizesKHR");
        if (func != nullptr)
        {
            func(device, build_type, p_build_info, p_max_primitive_counts, p_size_info);
            return;
        }
    }

    VkResult createAccelerationStructureKhr(VkDevice device, const VkAccelerationStructureCreateInfoKHR* p_create_info,
        const VkAllocationCallbacks* p_allocator, VkAccelerationStructureKHR* p_acceleration_structure)
    {
        auto func =
            (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
        if (func != nullptr)
        {
            return func(device, p_create_info, p_allocator, p_acceleration_structure);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void cmdBuildAccelerationStructuresKhr(VkDevice device, VkCommandBuffer command_buffer, uint32_t info_count,
        const VkAccelerationStructureBuildGeometryInfoKHR* p_infos,
        const VkAccelerationStructureBuildRangeInfoKHR* const* pp_build_range_infos)
    {

        auto func =
            (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
        if (func != nullptr)
        {
            func(command_buffer, info_count, p_infos, pp_build_range_infos);
            return;
        }
    }

    void destroyAccelerationStructureKhr(
        VkDevice device, VkAccelerationStructureKHR acceleration_structure, const VkAllocationCallbacks* p_allocator)
    {
        auto func =
            (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
        if (func != nullptr)
        {
            func(device, acceleration_structure, p_allocator);
            return;
        }
    }

    VkDeviceAddress getAccelerationStructureDeviceAddressKhr(
        VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* p_info)
    {
        auto func = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(
            device, "vkGetAccelerationStructureDeviceAddressKHR");
        if (func != nullptr)
        {
            return func(device, p_info);
        }
        return 0;
    }

    // -----------------------------------------------------------------------------------------------------------------------

    void AccelerationStructure::addTriangleGeometry(const AllocatedBuffer& vertex_buffer,
        const AllocatedBuffer& index_buffer, uint32_t max_vertex, uint32_t triangle_count, uint32_t vertex_stride,
        uint32_t vertex_offset, uint32_t index_offset)
    {
        assert(type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

        VkAccelerationStructureGeometryKHR acceleration_structure_geometry{};
        acceleration_structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        acceleration_structure_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        acceleration_structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        acceleration_structure_geometry.geometry.triangles.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        acceleration_structure_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        acceleration_structure_geometry.geometry.triangles.vertexData.deviceAddress =
            vertex_buffer.deviceAddress + (vertex_offset * vertex_stride);
        acceleration_structure_geometry.geometry.triangles.maxVertex = max_vertex;
        acceleration_structure_geometry.geometry.triangles.vertexStride = vertex_stride;
        acceleration_structure_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        acceleration_structure_geometry.geometry.triangles.indexData.deviceAddress =
            index_buffer.deviceAddress + (index_offset * sizeof(uint32_t));

        Geometry geometry{};
        geometry.handle = acceleration_structure_geometry;
        geometry.primitiveCount = triangle_count;
        geometries.push_back(geometry);
    }

    VkTransformMatrixKHR convertToVkTransform(const glm::mat4& mat)
    {
        VkTransformMatrixKHR transform{};

        // Copy first three rows from glm::mat4 to VkTransformMatrixKHR
        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                transform.matrix[row][col] = mat[col][row]; // Column-major order
            }
        }

        return transform;
    }

    void AccelerationStructure::addInstance(
        std::shared_ptr<AccelerationStructure>& instance, glm::mat4 transform_matrix, uint32_t instance_id)
    {
        VkAccelerationStructureInstanceKHR acceleration_structure_instance{};
        acceleration_structure_instance.transform = convertToVkTransform(transform_matrix);
        acceleration_structure_instance.instanceCustomIndex = instance_id;
        acceleration_structure_instance.mask = 0xFF;
        acceleration_structure_instance.instanceShaderBindingTableRecordOffset = 0;
        // accelerationStructureInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        acceleration_structure_instance.accelerationStructureReference = instance->getDeviceAddress();
        instances.push_back(acceleration_structure_instance);
    }

    void AccelerationStructure::fillInstanceBuffer()
    {
        uint32_t instance_data_size = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);

        if (instance_buffer.handle == VK_NULL_HANDLE)
        {
            instance_buffer = ressource_builder.createBuffer(instance_data_size,
                VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        instance_buffer.update(device, instances.data(), instance_data_size);
    }

    void AccelerationStructure::addInstanceGeometry()
    {
        assert(type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);

        fillInstanceBuffer();

        VkDeviceOrHostAddressConstKHR instance_data_device_address{};
        instance_data_device_address.deviceAddress = instance_buffer.deviceAddress;

        VkAccelerationStructureGeometryKHR acceleration_structure_geometry{};
        acceleration_structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        acceleration_structure_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        acceleration_structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        acceleration_structure_geometry.geometry.instances.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        acceleration_structure_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
        acceleration_structure_geometry.geometry.instances.data = instance_data_device_address;

        Geometry geometry{};
        geometry.handle = acceleration_structure_geometry;
        geometry.primitiveCount = static_cast<uint32_t>(instances.size());
        geometries.push_back(geometry);

        instances.clear();
    }

    void AccelerationStructure::update_instance_geometry(uint32_t index)
    {
        assert(type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
        fillInstanceBuffer();

        VkDeviceOrHostAddressConstKHR instance_data_device_address{};
        instance_data_device_address.deviceAddress = instance_buffer.deviceAddress;

        VkAccelerationStructureGeometryKHR acceleration_structure_geometry = geometries[index].handle;
        acceleration_structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        acceleration_structure_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        acceleration_structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        acceleration_structure_geometry.geometry.instances.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        acceleration_structure_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
        acceleration_structure_geometry.geometry.instances.data = instance_data_device_address;

        geometries[index].primitiveCount = static_cast<uint32_t>(instances.size());
        geometries[index].updated = true;

        instances.clear();
    }

    void AccelerationStructure::build(
        VkBuildAccelerationStructureFlagsKHR flags, VkBuildAccelerationStructureModeKHR mode)
    {
        assert(!geometries.empty());

        std::vector<VkAccelerationStructureGeometryKHR> acceleration_structure_geometries;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR> acceleration_structure_build_range_infos;
        std::vector<uint32_t> primitive_counts;
        for (auto geometry : geometries)
        {
            if (mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && !geometry.updated)
            {
                continue;
            }

            acceleration_structure_geometries.push_back(geometry.handle);

            VkAccelerationStructureBuildRangeInfoKHR acceleration_build_range_info{};
            acceleration_build_range_info.primitiveCount = geometry.primitiveCount;
            acceleration_build_range_info.primitiveOffset = 0;
            acceleration_build_range_info.firstVertex = 0;
            acceleration_build_range_info.transformOffset = 0;
            acceleration_structure_build_range_infos.push_back(acceleration_build_range_info);

            primitive_counts.push_back(geometry.primitiveCount);
            geometry.updated = false;
        }

        VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{};
        build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        build_geometry_info.type = type;
        build_geometry_info.flags = flags;
        build_geometry_info.mode = mode;
        if (mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && handle != VK_NULL_HANDLE)
        {
            build_geometry_info.srcAccelerationStructure = handle;
            build_geometry_info.dstAccelerationStructure = handle;
        }
        build_geometry_info.geometryCount = static_cast<uint32_t>(acceleration_structure_geometries.size());
        build_geometry_info.pGeometries = acceleration_structure_geometries.data();

        VkAccelerationStructureBuildSizesInfoKHR build_sizes_info{};
        build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        getAccelerationStructureBuildSizesKhr(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &build_geometry_info, &geometries[0].primitiveCount, &build_sizes_info);

        if (buffer.handle == VK_NULL_HANDLE || buffer.size != build_sizes_info.accelerationStructureSize)
        {
            buffer = ressource_builder.createBuffer(build_sizes_info.accelerationStructureSize,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info{};
            acceleration_structure_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            acceleration_structure_create_info.buffer = buffer.handle;
            acceleration_structure_create_info.size = build_sizes_info.accelerationStructureSize;
            acceleration_structure_create_info.type = type;
            if (createAccelerationStructureKhr(device, &acceleration_structure_create_info, nullptr, &handle) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("failed to create bl acceleration structure!");
            }
        }

        AllocatedBuffer scratch_buffer = ressource_builder.createBuffer(
            build_sizes_info.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        build_geometry_info.dstAccelerationStructure = handle;
        build_geometry_info.scratchData.deviceAddress = scratch_buffer.deviceAddress;

        VkCommandBuffer cmd_buffer = command_manager.beginSingleTimeCommands();
        auto* as_build_range_infos = &*acceleration_structure_build_range_infos.data();
        cmdBuildAccelerationStructuresKhr(device, cmd_buffer, 1, &build_geometry_info, &as_build_range_infos);
        command_manager.endSingleTimeCommand(cmd_buffer);

        ressource_builder.destroyBuffer(scratch_buffer);

        VkAccelerationStructureDeviceAddressInfoKHR acceleration_structure_device_address_info{};
        acceleration_structure_device_address_info.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        acceleration_structure_device_address_info.accelerationStructure = handle;
        device_address = getAccelerationStructureDeviceAddressKhr(device, &acceleration_structure_device_address_info);
    }

    void AccelerationStructure::destroy()
    {
        if (buffer.handle != VK_NULL_HANDLE)
        {
            ressource_builder.destroyBuffer(buffer);
        }

        if (instance_buffer.handle != VK_NULL_HANDLE)
        {
            ressource_builder.destroyBuffer(instance_buffer);
        }

        if (handle != VK_NULL_HANDLE)
        {
            destroyAccelerationStructureKhr(device, handle, nullptr);
        }
    }

    VkAccelerationStructureKHR AccelerationStructure::getHandle() const
    {
        return handle;
    }

    uint64_t AccelerationStructure::getDeviceAddress() const
    {
        return device_address;
    }
} // namespace RtEngine
