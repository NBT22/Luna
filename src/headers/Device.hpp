//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "ComputePipeline.hpp"
#include "DescriptorSetLayout.hpp"
#include "GraphicsPipeline.hpp"
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "RenderPass.hpp"
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
        Device() = delete;
        explicit Device(const LunaDeviceCreationInfo2 &creationInfo);

        explicit operator const VkPhysicalDevice &() const;
        explicit operator const VkDevice &() const;

        void destroy();

        VkResult createCommandPool(const LunaCommandPoolCreationInfo &creationInfo, LunaCommandPool *commandPool);
        VkResult createShaderModule(const LunaShaderModuleCreationInfo &creationInfo, LunaShaderModule *shaderModule);
        VkResult createRenderPass(const LunaRenderPassCreationInfo &creationInfo, LunaRenderPass *renderPass);
        VkResult createRenderPass(const LunaRenderPassCreationInfo2 &creationInfo, LunaRenderPass *renderPass);
        VkResult createDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo,
                                           LunaDescriptorSetLayout *descriptorSetLayout);
        VkResult createDescriptorPool(const LunaDescriptorPoolCreationInfo &creationInfo,
                                      LunaDescriptorPool *descriptorPool);
        VkResult allocateDescriptorSets(const LunaDescriptorSetAllocationInfo &allocationInfo,
                                        LunaDescriptorSet *descriptorSets);
        VkResult createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo,
                                        LunaGraphicsPipeline *pipeline);
        VkResult createComputePipeline(VkDevice device,
                                       const LunaComputePipelineCreationInfo &creationInfo,
                                       LunaComputePipeline *pipeline);
        VkResult createBuffer(const VkBufferCreateInfo &bufferCreateInfo,
                              const VmaAllocationCreateInfo &allocationCreateInfo,
                              Buffer *&outBuffer);
        VkResult createBuffer(const VkBufferCreateInfo &bufferCreateInfo,
                              const VmaAllocationCreateInfo &allocationCreateInfo,
                              VkDeviceSize alignment,
                              Buffer *&outBuffer);
        VkResult createBufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, LunaBuffer *outBuffer);
        VkResult createSampler(const LunaSamplerCreationInfo &creationInfo, LunaSampler *sampler);
        VkResult createImage(CommandBuffer &commandBuffer,
                             const LunaImageCreationInfo &creationInfo,
                             uint32_t depth,
                             uint32_t arrayLayers,
                             LunaImage *image);

        /// Removes a buffer region index from the list. Calling this function with an invalid or null pointer will have no effect
        void destroyBufferRegionIndex(BufferRegionIndex *&bufferRegionIndex);
        void destroySampler(const LunaSampler &sampler);
        // TODO: Some form of scheduling so that this doesn't destroy images which are currently in use
        void destroyImage(const LunaImage &image);

        [[nodiscard]] uint32_t findQueueFamilyIndex(const LunaQueueFamilyProperties &requiredProperties) const;

        [[nodiscard]] bool isDestroyed() const noexcept;
        [[nodiscard]] const LunaQueueFamilyProperties *queueFamilies() const noexcept;
        [[nodiscard]] VmaAllocator allocator() const noexcept;
        [[nodiscard]] std::list<Buffer> &buffers() noexcept;
        [[nodiscard]] BufferRegionIndex *&stagingBuffer() noexcept;

    private:
        VkResult initQueueFamilies_(VkPhysicalDevice physicalDevice,
                                    VkSurfaceKHR surface,
                                    std::vector<float> &priorities,
                                    std::vector<VkDeviceQueueCreateInfo> &creationInfos);
        [[nodiscard]] bool checkFeatureSupport_(const VkPhysicalDeviceFeatures2 &requiredFeatures) const;
        [[nodiscard]] bool checkFeatureSupport_(const VkBool32 *requiredFeatures) const;
        [[nodiscard]] bool checkUsability_(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool isDestroyed_{true};
        VkPhysicalDevice physicalDevice_{};
        VkDevice logicalDevice_{};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures_{};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures_{};
        VkPhysicalDeviceVulkan14Features vulkan14Features_{};
        VkPhysicalDeviceVulkan13Features vulkan13Features_{};
        VkPhysicalDeviceVulkan12Features vulkan12Features_{};
        VkPhysicalDeviceVulkan11Features vulkan11Features_{};
        VkPhysicalDeviceFeatures2 features_{};
        VkPhysicalDeviceProperties properties_{};
        VkPhysicalDeviceMemoryProperties memoryProperties_{};
        VmaAllocator allocator_{};
        std::vector<LunaQueueFamilyProperties> queueFamilies_{};
        std::list<CommandPool> commandPools_{};
        std::list<ShaderModule> shaderModules_{};
        std::list<RenderPass> renderPasses_{};
        std::list<DescriptorSetLayout> descriptorSetLayouts_{};
        std::list<VkDescriptorPool> descriptorPools_{};
        std::list<VkDescriptorSet> descriptorSets_{};
        std::list<DescriptorSetIndex> descriptorSetIndices_{};
        std::list<GraphicsPipeline> graphicsPipelines_{};
        std::list<ComputePipeline> computePipelines_{};
        std::list<Buffer> buffers_{};
        std::list<BufferRegionIndex> bufferRegionIndices_{};
        std::list<VkSampler> samplers_{};
        std::list<Image> images_{};
        // TODO (0.3.0): This should be able to have a minimum size (and just always resize to `std::max(newSize, minSize)`),
        //  that way it can shrink if it grows to an absurd size
        BufferRegionIndex *stagingBuffer_{};
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

inline bool Device::isDestroyed() const noexcept
{
    return isDestroyed_;
}
inline const LunaQueueFamilyProperties *Device::queueFamilies() const noexcept
{
    return queueFamilies_.data();
}
inline VmaAllocator Device::allocator() const noexcept
{
    return allocator_;
}
inline std::list<Buffer> &Device::buffers() noexcept
{
    return buffers_;
}
inline BufferRegionIndex *&Device::stagingBuffer() noexcept
{
    return stagingBuffer_;
}

inline VkResult Device::initQueueFamilies_(const VkPhysicalDevice physicalDevice,
                                           const VkSurfaceKHR surface,
                                           std::vector<float> &priorities,
                                           std::vector<VkDeviceQueueCreateInfo> &creationInfos)
{
    assert(physicalDevice != VK_NULL_HANDLE);
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());
    queueFamilies_.reserve(familyCount);
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
        queueFamilies_.emplace_back(families.at(index), supportsPresentation == VK_TRUE);
        priorities.resize(std::max(static_cast<uint32_t>(priorities.size()), families.at(index).queueCount));
        creationInfos.emplace_back(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                   nullptr,
                                   0,
                                   index,
                                   families.at(index).queueCount,
                                   nullptr);
    }
    return VK_SUCCESS;
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
    constexpr size_t featuresOffset = 2 * alignof(void *) / sizeof(VkBool32);
    const VkBool32 *requiredFeatureArray = requiredFeatures + featuresOffset;
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
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
        {
            constexpr size_t rayTracingPipelineFeatureCount = (sizeof(VkPhysicalDeviceRayTracingPipelineFeaturesKHR) -
                                                               16) /
                                                              sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &rayTracingPipelineFeatures_.rayTracingPipeline;
            for (size_t i = 0; i < rayTracingPipelineFeatureCount; i++)
            {
                if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
                {
                    return false;
                }
            }
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
        {
            constexpr size_t accelerationStructureFeatureCount =
                    (sizeof(VkPhysicalDeviceAccelerationStructureFeaturesKHR) - 16) / sizeof(VkBool32);
            const VkBool32 *supportedFeatureArray = &accelerationStructureFeatures_.accelerationStructure;
            for (size_t i = 0; i < accelerationStructureFeatureCount; i++)
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

    constexpr size_t pNextOffset = alignof(void *) / sizeof(VkBool32);
    const void *pNext = *reinterpret_cast<const void *const *>(requiredFeatures + pNextOffset);
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
