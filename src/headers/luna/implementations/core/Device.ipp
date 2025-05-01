//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <cassert>
#include <cstring>

namespace luna::core
{
inline void Device::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    for (const VkShaderModule shaderModule: shaderModules_)
    {
        vkDestroyShaderModule(logicalDevice_, shaderModule, nullptr);
    }
    // commandBuffers_.graphics.destroy(logicalDevice_);
    // commandBuffers_.transfer.destroy(logicalDevice_);
    // commandBuffers_.presentation.destroy(logicalDevice_);
    vkDestroySemaphore(logicalDevice_, imageAvailableSemaphore_, nullptr);
    vkDestroySemaphore(logicalDevice_, renderFinishedSemaphore_, nullptr);
    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(logicalDevice_, nullptr);

    shaderModules_.clear();
    shaderModules_.shrink_to_fit();
    queueFamilyIndices_.clear();
    queueFamilyIndices_.shrink_to_fit();
    isDestroyed_ = true;
}

inline VkResult Device::addShaderModule(const VkShaderModuleCreateInfo *creationInfo, VkShaderModule *shaderModule)
{
    shaderModules_.emplace_back();
    CHECK_RESULT_RETURN(vkCreateShaderModule(logicalDevice_, creationInfo, nullptr, &shaderModules_.back()));
    if (shaderModule != nullptr)
    {
        *shaderModule = shaderModules_.back();
    }
    return VK_SUCCESS;
}

inline VkPhysicalDevice Device::physicalDevice() const
{
    return physicalDevice_;
}
inline VkDevice Device::logicalDevice() const
{
    return logicalDevice_;
}
inline VkSharingMode Device::sharingMode() const
{
    return familyCount_ == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
}
inline uint32_t Device::familyCount() const
{
    return familyCount_;
}
inline const uint32_t *Device::queueFamilyIndices() const
{
    return queueFamilyIndices_.data();
}
inline VmaAllocator Device::allocator() const
{
    return allocator_;
}
inline const FamilyValues<VkQueue> &Device::familyQueues() const
{
    return familyQueues_;
}
inline FamilyValues<CommandPool> &Device::commandPools()
{
    return commandPools_;
}
inline const FamilyValues<CommandPool> &Device::commandPools() const
{
    return commandPools_;
}
inline const VkSemaphore &Device::imageAvailableSemaphore() const
{
    return imageAvailableSemaphore_;
}
inline VkSemaphore Device::renderFinishedSemaphore() const
{
    return renderFinishedSemaphore_;
}

// TODO: Better family finding logic to allow for
//  1. The application to tell Luna which families it would prefer to have be shared or prefer to be alone
//  2. Ensuring that the most optimal layout is found, regardless of what order the implementation provides the families
inline VkResult Device::findQueueFamilyIndices(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
    assert(physicalDevice != VK_NULL_HANDLE);
    familyCount_ = 0;
    hasFamily_.graphics = false;
    hasFamily_.transfer = false;
    hasFamily_.presentation = false;

    bool presentationFound = false;
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families;
    families.reserve(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());
    for (uint32_t index = 0; index < familyCount; index++)
    {
        VkBool32 supportsPresentation = VK_FALSE;
        CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                                                                 index,
                                                                 surface,
                                                                 &supportsPresentation));
        if (!hasFamily_.graphics && (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            familyIndices_.graphics = index;
            familyCount_++;
            hasFamily_.graphics = true;

            if (supportsPresentation != 0)
            {
                familyIndices_.presentation = index;
                presentationFound = true;
            }
        } else if (!presentationFound && supportsPresentation != 0)
        {
            familyIndices_.presentation = index;
            familyCount_++;
            hasFamily_.presentation = true;
            presentationFound = true;

            if (!hasFamily_.transfer && (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
            {
                familyIndices_.transfer = index;
                hasFamily_.transfer = true;
            }
        } else if (!hasFamily_.transfer && (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
        {
            familyIndices_.transfer = index;
            familyCount_++;
            hasFamily_.transfer = true;
        }

        if (hasFamily_.graphics && hasFamily_.transfer && presentationFound)
        {
            return VK_SUCCESS;
        }
    }
    if (!presentationFound || !hasFamily_.graphics)
    {
        familyCount_ = 0;
        return VK_ERROR_UNKNOWN;
    }
    if (!hasFamily_.transfer)
    {
        familyIndices_.transfer = familyIndices_.graphics;
    }
    return VK_SUCCESS;
}
inline void Device::initQueueFamilyIndices()
{
    assert(queueFamilyIndices_.empty());
    queueFamilyIndices_.reserve(familyCount_);
    queueFamilyIndices_.emplace_back(familyIndices_.graphics);
    switch (familyCount_)
    {
        case 2:
            queueFamilyIndices_.emplace_back(hasFamily_.transfer ? familyIndices_.transfer
                                                                 : familyIndices_.presentation);
            break;
        case 3:
            queueFamilyIndices_.emplace_back(familyIndices_.presentation);
            queueFamilyIndices_.emplace_back(familyIndices_.transfer);
            break;
        default:
            assert(familyCount_ == 1 || familyCount_ == 2 || familyCount_ == 3);
    }
}
inline bool Device::checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const
{
    const VkBool32 *requiredFeatureArray = std::bit_cast<const VkBool32 *,
                                                         const VkPhysicalDeviceFeatures2 *>(&requiredFeatures);
    constexpr int featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    const VkBool32 *supportedFeatureArray = std::bit_cast<const VkBool32 *,
                                                          const VkPhysicalDeviceFeatures *>(&features_.features);
    for (int i = 0; i < featureCount; i++)
    {
        if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
        {
            return false;
        }
    }

    if (requiredFeatures.pNext != nullptr)
    {
        return checkFeatureSupport(static_cast<const VkBool32 *>(requiredFeatures.pNext));
    }

    return true;
}
inline bool Device::checkFeatureSupport(const VkBool32 *requiredFeatures) const
{
    assert(requiredFeatures);
    const VkBool32 *requiredFeatureArray = requiredFeatures + 2;
    switch (*std::bit_cast<const VkStructureType *, const VkBool32 *>(requiredFeatures))
    {
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
        {
            constexpr int vulkan11FeatureCount = (sizeof(VkPhysicalDeviceVulkan11Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan11Features_.storageBuffer16BitAccess;
            for (int i = 0; i < vulkan11FeatureCount; i++)
            {
                if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
                {
                    return false;
                }
            }
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
        {
            constexpr int vulkan12FeatureCount = (sizeof(VkPhysicalDeviceVulkan12Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan12Features_.samplerMirrorClampToEdge;
            for (int i = 0; i < vulkan12FeatureCount; i++)
            {
                if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
                {
                    return false;
                }
            }
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
        {
            constexpr int vulkan13FeatureCount = (sizeof(VkPhysicalDeviceVulkan13Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan13Features_.robustImageAccess;
            for (int i = 0; i < vulkan13FeatureCount; i++)
            {
                if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
                {
                    return false;
                }
            }
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
        {
            constexpr int vulkan14FeatureCount = (sizeof(VkPhysicalDeviceVulkan14Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan14Features_.globalPriorityQuery;
            for (int i = 0; i < vulkan14FeatureCount; i++)
            {
                if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
                {
                    return false;
                }
            }
            break;
        }
        default:
            [[maybe_unused]] const VkStructureType
                    structureType = *std::bit_cast<const VkStructureType *, const VkBool32 *>(requiredFeatures);
            assert(structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES ||
                   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES ||
                   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES ||
                   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES);
    }

    const void *pNext = *std::bit_cast<const void **, const VkBool32 *>(requiredFeatures + 1);
    if (pNext != nullptr)
    {
        return checkFeatureSupport(static_cast<const VkBool32 *>(pNext));
    }
    return true;
}
inline bool Device::checkUsability(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    uint32_t count;
    CHECK_RESULT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr));
    if (count == 0)
    {
        return false;
    }
    CHECK_RESULT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
    if (count == 0)
    {
        return false;
    }

    CHECK_RESULT_THROW(findQueueFamilyIndices(device, surface));

    vkGetPhysicalDeviceProperties(device, &properties_);
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties_);

    uint32_t extensionCount;
    CHECK_RESULT_THROW(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
    if (extensionCount == 0)
    {
        return false;
    }
    std::vector<VkExtensionProperties> availableExtensions;
    availableExtensions.reserve(extensionCount);
    CHECK_RESULT_THROW(vkEnumerateDeviceExtensionProperties(device,
                                                            nullptr,
                                                            &extensionCount,
                                                            availableExtensions.data()));
    for (uint32_t j = 0; j < extensionCount; j++)
    {
        if (std::strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            return true;
        }
    }
    return false;
}
inline VkResult Device::createCommandPools()
{
    const VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = familyIndices_.graphics,
    };
    CHECK_RESULT_RETURN(commandPools_.graphics.allocate(logicalDevice_, graphicsCommandPoolCreateInfo));
    CHECK_RESULT_RETURN(commandPools_.graphics.allocateCommandBuffer(logicalDevice_,
                                                                     VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                     nullptr));
    // CHECK_RESULT_RETURN(commandPools_.graphics.allocateCommandBuffer(logicalDevice_,
    //                                                                  VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    //                                                                  nullptr));

    // TODO: Both the transfer and presentation families are currently unused
    // const VkCommandPoolCreateInfo transferCommandPoolCreateInfo = {
    //     .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    //     .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    //     .queueFamilyIndex = familyIndices_.transfer,
    // };
    // CHECK_RESULT_RETURN(commandPools_.transfer.allocate(logicalDevice_, transferCommandPoolCreateInfo));
    // CHECK_RESULT_RETURN(commandPools_.transfer.allocateCommandBuffer(logicalDevice_,
    //                                                                  VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    //                                                                  nullptr));

    // if (hasFamily_.presentation)
    // {
    //     const VkCommandPoolCreateInfo presentationCommandPoolCreateInfo = {
    //         .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    //         .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    //         .queueFamilyIndex = familyIndices_.presentation,
    //     };
    //     CHECK_RESULT_RETURN(commandPools_.presentation.allocate(logicalDevice_, presentationCommandPoolCreateInfo));
    //     CHECK_RESULT_RETURN(commandPools_.presentation.allocateCommandBuffer(logicalDevice_,
    //                                                                          VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    //                                                                          nullptr));
    // }
    return VK_SUCCESS;
}
inline VkResult Device::createSemaphores()
{
    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    CHECK_RESULT_RETURN(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore_));
    CHECK_RESULT_RETURN(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore_));
    return VK_SUCCESS;
}
} // namespace luna::core
