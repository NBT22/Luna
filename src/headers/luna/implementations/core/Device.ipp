//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

namespace luna::core
{
inline Device::operator const VkPhysicalDevice &() const
{
    return physicalDevice_;
}
inline Device::operator const VkDevice &() const
{
    return logicalDevice_;
}

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
    internalCommandPools_.graphics.destroy(logicalDevice_);
    internalCommandPools_.transfer.destroy(logicalDevice_);
    internalCommandPools_.presentation.destroy(logicalDevice_);
    for (const Semaphore &renderFinishedSemaphore: renderFinishedSemaphores_)
    {
        vkDestroySemaphore(logicalDevice_, renderFinishedSemaphore, nullptr);
    }
    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(logicalDevice_, nullptr);

    shaderModules_.clear();
    shaderModules_.shrink_to_fit();
    queueFamilyIndices_.clear();
    queueFamilyIndices_.shrink_to_fit();
    isDestroyed_ = true;
}

inline VkResult Device::addShaderModule(const VkShaderModuleCreateInfo *creationInfo, LunaShaderModule *shaderModule)
{
    const std::vector<VkShaderModule>::iterator shaderModuleIterator = std::find(shaderModules_.begin(),
                                                                                 shaderModules_.end(),
                                                                                 VK_NULL_HANDLE);
    if (shaderModuleIterator == shaderModules_.end())
    {
        shaderModuleIndices_.emplace_back(shaderModules_.size());
        shaderModules_.emplace_back();
        CHECK_RESULT_RETURN(vkCreateShaderModule(logicalDevice_, creationInfo, nullptr, &shaderModules_.back()));
    } else
    {
        const uint32_t index = shaderModuleIterator - shaderModules_.begin();
        shaderModuleIndices_.emplace_back(index);
        CHECK_RESULT_RETURN(vkCreateShaderModule(logicalDevice_, creationInfo, nullptr, &shaderModules_.at(index)));
    }
    if (shaderModule != nullptr)
    {
        *shaderModule = &shaderModuleIndices_.back();
    }
    return VK_SUCCESS;
}
inline VkResult Device::createSemaphores(const uint32_t imageCount)
{
    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    const uint32_t oldSize = renderFinishedSemaphores_.size();
    renderFinishedSemaphores_.resize(imageCount);
    for (uint32_t i = oldSize; i < imageCount; i++)
    {
        Semaphore &semaphore = renderFinishedSemaphores_.at(i);
        CHECK_RESULT_RETURN(vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &semaphore));
    }
    return VK_SUCCESS;
}
inline VkResult Device::addApplicationCommandPool(const LunaCommandPoolCreationInfo &creationInfo,
                                                  LunaCommandPool *commandPool)
{
    const std::vector<CommandPool>::iterator &commandPoolIterator = std::find_if(applicationCommandPools_.begin(),
                                                                                 applicationCommandPools_.end(),
                                                                                 CommandPool::isDestroyed);
    if (commandPoolIterator == applicationCommandPools_.end())
    {
        applicationCommandPoolIndices_.emplace_back(applicationCommandPools_.size());
        TRY_CATCH_RESULT(applicationCommandPools_.emplace_back(logicalDevice_, creationInfo));
    } else
    {
        const uint32_t index = commandPoolIterator - applicationCommandPools_.begin();
        applicationCommandPoolIndices_.emplace_back(index);
        CHECK_RESULT_RETURN(applicationCommandPools_.at(index).allocate(logicalDevice_, creationInfo));
    }
    if (commandPool != nullptr)
    {
        *commandPool = &applicationCommandPoolIndices_.back();
    }
    return VK_SUCCESS;
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
    return internalCommandPools_;
}
inline const FamilyValues<CommandPool> &Device::commandPools() const
{
    return internalCommandPools_;
}
inline Semaphore &Device::renderFinishedSemaphore(const uint32_t imageIndex)
{
    return renderFinishedSemaphores_.at(imageIndex);
}
inline VkShaderModule Device::shaderModule(const LunaShaderModule shaderModule) const
{
    return shaderModules_.at(*static_cast<const uint32_t *>(shaderModule));
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
    std::vector<VkQueueFamilyProperties> families(familyCount);
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
    const VkBool32 *requiredFeatureArray = reinterpret_cast<const VkBool32 *>(&requiredFeatures.features);
    constexpr int featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&features_.features);
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
    static_assert(alignof(void *) == alignof(VkPhysicalDeviceVulkan11Features) &&
                  alignof(void *) == alignof(VkPhysicalDeviceVulkan12Features) &&
                  alignof(void *) == alignof(VkPhysicalDeviceVulkan13Features) &&
                  alignof(void *) == alignof(VkPhysicalDeviceVulkan14Features));
    constexpr size_t offset = 2 * alignof(void *) / sizeof(VkBool32);
    const VkBool32 *requiredFeatureArray = requiredFeatures + offset;
    switch (*reinterpret_cast<const VkStructureType *>(requiredFeatures))
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
            [[maybe_unused]] const VkStructureType &structureType = static_cast<VkStructureType>(*requiredFeatures);
            assert(structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES ||
                   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES ||
                   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES ||
                   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES);
    }

    const void *pNext = reinterpret_cast<const void *>(*reinterpret_cast<const uint64_t *>(requiredFeatures + 1));
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
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
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
    CHECK_RESULT_RETURN(internalCommandPools_.graphics.allocate(logicalDevice_, graphicsCommandPoolCreateInfo));

    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    // TODO: The count should be dynamic, which means this shouldn't use templates but instead just function args
    CHECK_RESULT_RETURN(internalCommandPools_.graphics.allocateCommandBuffer<5>(logicalDevice_,
                                                                                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                                nullptr,
                                                                                &semaphoreCreateInfo));
    CHECK_RESULT_RETURN(internalCommandPools_.graphics.allocateCommandBuffer(logicalDevice_,
                                                                             VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                             nullptr,
                                                                             &semaphoreCreateInfo));

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
} // namespace luna::core
