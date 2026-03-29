//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "CommandPool.hpp"
#include "helpers/Handle.hpp"
#include "Semaphore.hpp"
#include "ShaderModule.hpp"

namespace luna
{
template<typename T> struct FamilyValues
{
        T graphics{};
        T compute{};
        T presentation{};
};

class Device
{
    public:
        Device() = default;
        explicit Device(const LunaDeviceCreationInfo2 &creationInfo);

        operator const VkPhysicalDevice &() const;
        operator const VkDevice &() const;

        void destroy();

        VkResult addShaderModule(const LunaShaderModuleCreationInfo &creationInfo, LunaShaderModule *shaderModule);
        VkResult createSemaphores(uint32_t imageCount);
        VkResult createInternalCommandPools();
        VkResult addApplicationCommandPool(const LunaCommandPoolCreationInfo &creationInfo,
                                           LunaCommandPool *commandPool);

        [[nodiscard]] bool isDestroyed() const noexcept;
        [[nodiscard]] VkSharingMode sharingMode() const noexcept;
        /// A getter for the @c familyCount_ value
        /// @return The total count of unique families
        [[nodiscard]] uint32_t familyCount() const noexcept;
        [[nodiscard]] const uint32_t *queueFamilyIndices() const noexcept;
        [[nodiscard]] VmaAllocator allocator() const noexcept;
        [[nodiscard]] const FamilyValues<VkQueue> &familyQueues() const noexcept;
        [[nodiscard]] FamilyValues<CommandPool *> &commandPools() noexcept;
        [[nodiscard]] const FamilyValues<CommandPool *> &commandPools() const noexcept;
        [[nodiscard]] Semaphore &renderFinishedSemaphore(uint32_t imageIndex);
        [[nodiscard]] VkPhysicalDeviceVulkan13Features vulkan13Features() const noexcept;

    private:
        VkResult findQueueFamilyIndices_(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        void initQueueFamilyIndices_();
        [[nodiscard]] bool checkFeatureSupport_(const VkPhysicalDeviceFeatures2 &requiredFeatures) const;
        [[nodiscard]] bool checkFeatureSupport_(const VkBool32 *requiredFeatures) const;
        [[nodiscard]] bool checkUsability_(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool isDestroyed_{true};
        VkPhysicalDevice physicalDevice_{};
        VkDevice logicalDevice_{};
        VkPhysicalDeviceVulkan14Features vulkan14Features_{};
        VkPhysicalDeviceVulkan13Features vulkan13Features_{};
        VkPhysicalDeviceVulkan12Features vulkan12Features_{};
        VkPhysicalDeviceVulkan11Features vulkan11Features_{};
        VkPhysicalDeviceFeatures2 features_{};
        VkPhysicalDeviceProperties properties_{};
        VkPhysicalDeviceMemoryProperties memoryProperties_{};
        VmaAllocator allocator_{};
        std::vector<uint32_t> queueFamilyIndices_{};
        FamilyValues<bool> hasFamily_{};
        FamilyValues<VkQueue> familyQueues_{};
        FamilyValues<uint32_t> familyIndices_{};
        std::list<CommandPool> commandPools_{};
        FamilyValues<CommandPool *> internalCommandPools_{};
        std::vector<Semaphore> renderFinishedSemaphores_{};

        std::list<ShaderModule> shaderModules_{};
};
} // namespace luna

#pragma region Implementation

#include <cassert>
#include <cstring>
#include <volk.h>
#include "Luna.hpp"

namespace luna
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
    shaderModules_.clear();
    if (internalCommandPools_.graphics != nullptr)
    {
        internalCommandPools_.graphics->destroy();
    }
    if (internalCommandPools_.compute != nullptr)
    {
        internalCommandPools_.compute->destroy();
    }
    if (internalCommandPools_.presentation != nullptr)
    {
        internalCommandPools_.presentation->destroy();
    }
    renderFinishedSemaphores_.clear();
    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(logicalDevice_, nullptr);

    queueFamilyIndices_.clear();
    queueFamilyIndices_.shrink_to_fit();
    isDestroyed_ = true;
}

inline VkResult Device::addShaderModule(const LunaShaderModuleCreationInfo &creationInfo,
                                        LunaShaderModule *shaderModule)
{
    TRY_CATCH_RESULT(shaderModules_.emplace_back(creationInfo));
    if (shaderModule != nullptr)
    {
        *shaderModule = helpers::toHandle(&shaderModules_.back());
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
inline VkResult Device::createInternalCommandPools()
{
    const VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = familyIndices_.graphics,
    };
    TRY_CATCH_RESULT(commandPools_.emplace_back(logicalDevice_, &graphicsCommandPoolCreateInfo));
    internalCommandPools_.graphics = &commandPools_.back();

    CHECK_RESULT_RETURN(internalCommandPools_.graphics->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    CHECK_RESULT_RETURN(internalCommandPools_.graphics->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));

    const VkCommandPoolCreateInfo computeCommandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = familyIndices_.compute,
    };
    TRY_CATCH_RESULT(commandPools_.emplace_back(logicalDevice_, &computeCommandPoolCreateInfo));
    internalCommandPools_.compute = &commandPools_.back();

    CHECK_RESULT_RETURN(internalCommandPools_.compute->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));

    return VK_SUCCESS;
}
inline VkResult Device::addApplicationCommandPool(const LunaCommandPoolCreationInfo &creationInfo,
                                                  LunaCommandPool *commandPool)
{
    TRY_CATCH_RESULT(commandPools_.emplace_back(creationInfo));
    if (commandPool != nullptr)
    {
        *commandPool = helpers::toHandle(&commandPools_.back());
    }
    return VK_SUCCESS;
}

inline bool Device::isDestroyed() const noexcept
{
    return isDestroyed_;
}
inline VkSharingMode Device::sharingMode() const noexcept
{
    return hasFamily_.compute ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
}
inline uint32_t Device::familyCount() const noexcept
{
    return hasFamily_.compute ? 2u : 1u;
}
inline const uint32_t *Device::queueFamilyIndices() const noexcept
{
    return queueFamilyIndices_.data();
}
inline VmaAllocator Device::allocator() const noexcept
{
    return allocator_;
}
inline const FamilyValues<VkQueue> &Device::familyQueues() const noexcept
{
    return familyQueues_;
}
inline FamilyValues<CommandPool *> &Device::commandPools() noexcept
{
    return internalCommandPools_;
}
inline const FamilyValues<CommandPool *> &Device::commandPools() const noexcept
{
    return internalCommandPools_;
}
inline Semaphore &Device::renderFinishedSemaphore(const uint32_t imageIndex)
{
    return renderFinishedSemaphores_.at(imageIndex);
}
inline VkPhysicalDeviceVulkan13Features Device::vulkan13Features() const noexcept
{
    return vulkan13Features_;
}

// TODO: Better family finding logic to allow for
//  1. The application to tell Luna which families it would prefer to have be shared or prefer to be alone
//  2. Ensuring that the most optimal layout is found, regardless of what order the implementation provides the families
inline VkResult Device::findQueueFamilyIndices_(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
    assert(physicalDevice != VK_NULL_HANDLE);
    hasFamily_.graphics = false;
    hasFamily_.compute = false;
    hasFamily_.presentation = false;

    bool computeFound = false;
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());
    for (uint32_t index = 0; index < familyCount; index++)
    {
        VkBool32 supportsPresentation = VK_FALSE;
        if (surface != VK_NULL_HANDLE)
        {
            CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                                                                     index,
                                                                     surface,
                                                                     &supportsPresentation));
        }
        if (!hasFamily_.graphics && (families.at(index).queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            hasFamily_.graphics = true;
            familyIndices_.graphics = index;

            assert(surface == VK_NULL_HANDLE ||
                   supportsPresentation); // This *should* be fine to assume, but technically is not in the spec
            hasFamily_.presentation = true;
            familyIndices_.presentation = index;

            if (!computeFound && (families.at(index).queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
            {
                familyIndices_.compute = index;
                computeFound = true;
            }
        } else if (!hasFamily_.compute && (families.at(index).queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
        {
            computeFound = true;
            hasFamily_.compute = true;
            familyIndices_.compute = index;
        }

        if (hasFamily_.graphics && hasFamily_.compute && hasFamily_.presentation)
        {
            return VK_SUCCESS;
        }
    }

    if (!hasFamily_.compute && computeFound && hasFamily_.graphics && hasFamily_.presentation)
    {
        return VK_SUCCESS;
    }

    // TODO: Allow for not having graphics/compute queues
    return VK_ERROR_UNKNOWN;
}
inline void Device::initQueueFamilyIndices_()
{
    assert(queueFamilyIndices_.empty());
    queueFamilyIndices_.emplace_back(familyIndices_.graphics);
    if (hasFamily_.compute)
    {
        queueFamilyIndices_.emplace_back(familyIndices_.compute);
    }
}
inline bool Device::checkFeatureSupport_(const VkPhysicalDeviceFeatures2 &requiredFeatures) const
{
    const VkBool32 *requiredFeatureArray = reinterpret_cast<const VkBool32 *>(&requiredFeatures.features);
    constexpr size_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&features_.features);
    for (size_t i = 0; i < featureCount; i++)
    {
        if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
        {
            return false;
        }
    }

    if (requiredFeatures.pNext != nullptr)
    {
        return checkFeatureSupport_(static_cast<const VkBool32 *>(requiredFeatures.pNext));
    }

    return true;
}
inline bool Device::checkFeatureSupport_(const VkBool32 *requiredFeatures) const
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
            constexpr size_t vulkan11FeatureCount = (sizeof(VkPhysicalDeviceVulkan11Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan11Features_.storageBuffer16BitAccess;
            for (size_t i = 0; i < vulkan11FeatureCount; i++)
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
            constexpr size_t vulkan12FeatureCount = (sizeof(VkPhysicalDeviceVulkan12Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan12Features_.samplerMirrorClampToEdge;
            for (size_t i = 0; i < vulkan12FeatureCount; i++)
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
            constexpr size_t vulkan13FeatureCount = (sizeof(VkPhysicalDeviceVulkan13Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan13Features_.robustImageAccess;
            for (size_t i = 0; i < vulkan13FeatureCount; i++)
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
            constexpr size_t vulkan14FeatureCount = (sizeof(VkPhysicalDeviceVulkan14Features) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &vulkan14Features_.globalPriorityQuery;
            for (size_t i = 0; i < vulkan14FeatureCount; i++)
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

    const void *pNext = *reinterpret_cast<const void *const *>(requiredFeatures + 1);
    if (pNext != nullptr)
    {
        return checkFeatureSupport_(static_cast<const VkBool32 *>(pNext));
    }
    return true;
}
inline bool Device::checkUsability_(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    if (surface != VK_NULL_HANDLE)
    {
        uint32_t count = 0;
        CHECK_RESULT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr));
        if (count == 0)
        {
            return false;
        }
        count = 0;
        CHECK_RESULT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
        if (count == 0)
        {
            return false;
        }
    }

    CHECK_RESULT_THROW(findQueueFamilyIndices_(device, surface));

    vkGetPhysicalDeviceProperties(device, &properties_);
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties_);

    uint32_t extensionCount = 0;
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
        if (std::strcmp(availableExtensions.at(j).extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            return true;
        }
    }
    return false;
}
} // namespace luna

#pragma endregion Implementation
