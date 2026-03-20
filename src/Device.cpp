//
// Created by NBT22 on 2/13/25.
//

#include <array>
#include <cassert>
#include <cstdint>
#include <luna/luna.h>
#include <luna/lunaDevice.h>
#include <luna/lunaTypes.h>
#include <stdexcept>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "Device.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna
{
Device::Device(const LunaDeviceCreationInfo2 &creationInfo)
{
    assert(isDestroyed_);
    uint32_t deviceCount = 0;
    CHECK_RESULT_THROW(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    CHECK_RESULT_THROW(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
    switch (VK_API_VERSION_MINOR(apiVersion))
    {
        case 1:
            vulkan11Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            };
            features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vulkan11Features_,
            };
            break;
        case 2:
            vulkan12Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            };
            vulkan11Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = &vulkan12Features_,
            };
            features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vulkan11Features_,
            };
            break;
        case 3:
            vulkan13Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            };
            vulkan12Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = &vulkan13Features_,
            };
            vulkan11Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = &vulkan12Features_,
            };
            features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vulkan11Features_,
            };
            break;
        case 4:
            vulkan14Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            };
            vulkan13Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
                .pNext = &vulkan14Features_,
            };
            vulkan12Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = &vulkan13Features_,
            };
            vulkan11Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = &vulkan12Features_,
            };
            features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vulkan11Features_,
            };
            break;
        default:
            assert(1 <= VK_API_VERSION_MINOR(apiVersion) && VK_API_VERSION_MINOR(apiVersion) <= 4);
    }
    for (const VkPhysicalDevice physicalDevice: devices)
    {
        vkGetPhysicalDeviceFeatures2(physicalDevice, &features_);
        // checkUsability can throw an error, but it will be caught by the function calling this constructor
        if (!checkFeatureSupport(creationInfo.requiredFeatures) ||
            !checkUsability(physicalDevice, creationInfo.surface))
        {
            continue;
        }
        physicalDevice_ = physicalDevice;
        if (creationInfo.physicalDevicePreferenceDefinition != nullptr &&
            properties_.deviceType == creationInfo.physicalDevicePreferenceDefinition->preferredDeviceType)
        {
            break;
        }
    }
    if (physicalDevice_ == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU for Vulkan!");
    }

    constexpr float queuePriority = 1;
    std::array<VkDeviceQueueCreateInfo, 2> queuesCreateInfo{
        VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = familyIndices_.graphics,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        },
        VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = familyIndices_.compute,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        },
    };
    initQueueFamilyIndices();

    const VkPhysicalDeviceSynchronization2Features synchronization2Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .pNext = creationInfo.requiredFeatures.pNext,
        .synchronization2 = vulkan13Features_.synchronization2,
    };
    const void *pNext = VK_API_VERSION_MINOR(apiVersion) >= 3 ? &synchronization2Features
                                                              : creationInfo.requiredFeatures.pNext;

    const VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = pNext,
        .queueCreateInfoCount = hasFamily_.compute ? 2u : 1u,
        .pQueueCreateInfos = queuesCreateInfo.data(),
        .enabledExtensionCount = creationInfo.extensionCount,
        .ppEnabledExtensionNames = creationInfo.extensionNames,
        .pEnabledFeatures = &creationInfo.requiredFeatures.features,
    };
    CHECK_RESULT_THROW(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_));
    volkLoadDevice(logicalDevice_);

    vkGetDeviceQueue(logicalDevice_, familyIndices_.graphics, 0, &familyQueues_.graphics);
    vkGetDeviceQueue(logicalDevice_, familyIndices_.compute, 0, &familyQueues_.compute);
    vkGetDeviceQueue(logicalDevice_, familyIndices_.presentation, 0, &familyQueues_.presentation);

    const VmaVulkanFunctions vmaVulkanFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR,
        .vkBindBufferMemory2KHR = vkBindBufferMemory2KHR,
        .vkBindImageMemory2KHR = vkBindImageMemory2KHR,
        .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR,
        .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
        .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
        // .vkGetMemoryWin32HandleKHR = vkGetMemoryWin32HandleKHR,
    };
    const VmaAllocatorCreateInfo allocationCreateInfo = {
        .flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
        .physicalDevice = physicalDevice_,
        .device = logicalDevice_,
        .pVulkanFunctions = &vmaVulkanFunctions,
        .instance = instance,
        .vulkanApiVersion = apiVersion,
    };
    CHECK_RESULT_THROW(vmaCreateAllocator(&allocationCreateInfo, &allocator_));

    isDestroyed_ = false;

    CHECK_RESULT_THROW(createCommandPools());
}
} // namespace luna

VkResult lunaCreateDevice(const LunaDeviceCreationInfo *creationInfo)
{
    assert(creationInfo);
    const VkPhysicalDeviceFeatures2 requiredFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features = creationInfo->requiredFeatures,
    };
    const LunaDeviceCreationInfo2 creationInfo2 = {
        .extensionCount = creationInfo->extensionCount,
        .extensionNames = creationInfo->extensionNames,
        .requiredFeatures = requiredFeatures2,
        .surface = creationInfo->surface,
        .physicalDevicePreferenceDefinition = creationInfo->physicalDevicePreferenceDefinition,
    };
    TRY_CATCH_RESULT(luna::device = luna::Device(creationInfo2));
    constexpr VmaAllocationCreateInfo allocationCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    const LunaBufferCreationInfo bufferCreationInfo = {
        .size = 1 << 16,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .allocationCreateInfo = &allocationCreateInfo,
    };
    LunaBuffer stagingBufferHandle = luna::helpers::toHandle(luna::stagingBuffer);
    CHECK_RESULT_RETURN(luna::BufferRegion::createBufferRegion(bufferCreationInfo, &stagingBufferHandle));
    luna::stagingBuffer = luna::helpers::fromHandle<luna::BufferRegionIndex>(stagingBufferHandle);
    return VK_SUCCESS;
}

VkResult lunaCreateDevice2(const LunaDeviceCreationInfo2 *creationInfo)
{
    assert(creationInfo);
    TRY_CATCH_RESULT(luna::device = luna::Device(*creationInfo));
    constexpr VmaAllocationCreateInfo allocationCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    const LunaBufferCreationInfo bufferCreationInfo = {
        .size = 1 << 16,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .allocationCreateInfo = &allocationCreateInfo,
    };
    LunaBuffer stagingBufferHandle = luna::helpers::toHandle(luna::stagingBuffer);
    CHECK_RESULT_RETURN(luna::BufferRegion::createBufferRegion(bufferCreationInfo, &stagingBufferHandle));
    luna::stagingBuffer = luna::helpers::fromHandle<luna::BufferRegionIndex>(stagingBufferHandle);
    return VK_SUCCESS;
}

VkResult lunaDeviceWaitIdle()
{
    CHECK_RESULT_RETURN(vkDeviceWaitIdle(luna::device));
    return VK_SUCCESS;
}

void lunaGetPhysicalDeviceProperties(VkPhysicalDeviceProperties *properties)
{
    vkGetPhysicalDeviceProperties(luna::device, properties);
}

void lunaGetPhysicalDeviceProperties2(VkPhysicalDeviceProperties2 *properties)
{
    vkGetPhysicalDeviceProperties2(luna::device, properties);
}

VkResult lunaCreateShaderModule(const LunaShaderModuleCreationInfo *creationInfo, LunaShaderModule *shaderModule)
{
    assert(creationInfo);

    CHECK_RESULT_RETURN(luna::device.addShaderModule(*creationInfo, shaderModule));
    return VK_SUCCESS;
}
