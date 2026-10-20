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
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"

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
                                        LunaDescriptorSet **descriptorSets);
        VkResult createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo,
                                        LunaGraphicsPipeline *pipeline,
                                        VkRenderPass renderPass,
                                        uint32_t subpassIndex);
        VkResult createComputePipeline(VkDevice device,
                                       const LunaComputePipelineCreationInfo &creationInfo,
                                       LunaComputePipeline *pipeline);
        VkResult createBufferRegion(const LunaBufferCreationInfo &creationInfo, LunaBuffer *&outBuffer);
        VkResult createSampler(const LunaSamplerCreationInfo &creationInfo, LunaSampler *sampler);
        VkResult createImage(CommandBuffer &commandBuffer,
                             const LunaImageCreationInfo &creationInfo,
                             uint32_t depth,
                             uint32_t arrayLayers,
                             VkImageViewType viewType,
                             LunaImage *image);
        VkResult createSemaphore(const LunaSemaphoreCreationInfo &creationInfo, LunaSemaphore *semaphore);

        void destroySampler(const LunaSampler &sampler);
        // TODO: Some form of scheduling so that this doesn't destroy images which are currently in use
        void destroyImage(const LunaImage &image);
        void destroySemaphore(const LunaSemaphore &semaphore);
        void destroyGraphicsPipeline(const LunaGraphicsPipeline &graphicsPipeline);
        void destroyComputePipeline(const LunaComputePipeline &computePipeline);

        [[nodiscard]] uint32_t findQueueFamilyIndex(const LunaQueueFamilyProperties &requiredProperties) const;
        [[nodiscard]] const BufferRegion *writeDataToStagingBuffer(const void *data, VkDeviceSize size);
        void clearStagingBuffer() const;

        [[nodiscard]] bool isDestroyed() const noexcept;
        [[nodiscard]] const LunaQueueFamilyProperties *queueFamilies() const noexcept;
        [[nodiscard]] VmaAllocator allocator() const noexcept;

    private:
        VkResult initQueueFamilies_(VkPhysicalDevice physicalDevice,
                                    VkSurfaceKHR surface,
                                    std::vector<float> &priorities,
                                    std::vector<VkDeviceQueueCreateInfo> &creationInfos);
        [[nodiscard]] static bool checkFeatureSupport_(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                                                       const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures);
        [[nodiscard]] static bool checkFeatureSupport_(const VkPhysicalDeviceFeatures *requiredFeatures,
                                                       const VkPhysicalDeviceFeatures *supportedFeatures);
        template<typename T> [[nodiscard]] static bool checkFeatureSupport_(const T *requiredFeatures,
                                                                            const T *supportedFeatures,
                                                                            uint32_t memberCount);
        [[nodiscard]] static bool checkFeatureSupport_(const VkBool32 *required,
                                                       const VkBool32 *supported,
                                                       uint32_t memberCount);
        [[nodiscard]] bool checkUsability_(VkPhysicalDevice device, VkSurfaceKHR surface);

        bool isDestroyed_{true};
        VkPhysicalDevice physicalDevice_{};
        VkDevice logicalDevice_{};
        VkPhysicalDeviceProperties properties_{};
        VkPhysicalDeviceMemoryProperties memoryProperties_{};
        VmaAllocator allocator_{};
        std::vector<LunaQueueFamilyProperties> queueFamilies_{};
        std::list<CommandPool> commandPools_;
        std::list<ShaderModule> shaderModules_;
        std::list<RenderPass> renderPasses_;
        std::list<DescriptorSetLayout> descriptorSetLayouts_;
        std::list<VkDescriptorPool> descriptorPools_{};
        std::list<VkDescriptorSet> descriptorSets_{};
        std::list<DescriptorSetIndex> descriptorSetIndices_;
        std::list<GraphicsPipeline> graphicsPipelines_;
        std::list<ComputePipeline> computePipelines_;
        std::list<BufferPool> bufferPools_;
        std::list<VkSampler> samplers_{};
        std::list<Image> images_;
        std::list<Semaphore> semaphores_;
        BufferPool *stagingBuffer_{};
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

inline const BufferRegion *Device::writeDataToStagingBuffer(const void *data, const VkDeviceSize size)
{
    const LunaBufferCreationInfo stagingBufferRegionCreationInfo = {
        .size = size,
        .alignment = 4,
        .memoryType = LUNA_MEMORY_TYPE_HOST_VISIBLE,
    };
    // TODO: This gets leaked
    const BufferRegion *stagingBufferRegion =
            helpers::fromHandle<BufferRegion>(stagingBuffer_->createRegion(stagingBufferRegionCreationInfo));
    if (stagingBufferRegion == nullptr)
    {
        return nullptr;
    }
    std::copy_n(static_cast<const char *>(data), size, stagingBufferRegion->data());
    return stagingBufferRegion;
}
inline void Device::clearStagingBuffer() const
{
    stagingBuffer_->clearAllRegions();
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
inline bool Device::checkFeatureSupport_(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                                         const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures)
{
    if (!checkFeatureSupport_(&requiredFeatures.features, &supportedFeatures.features))
    {
        return false;
    }
    if (!checkFeatureSupport_(vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(requiredFeatures.ptr()),
                              vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(supportedFeatures.ptr()),
                              12))
    {
        return false;
    }
    if (!checkFeatureSupport_(vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(requiredFeatures.ptr()),
                              vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(supportedFeatures.ptr()),
                              47))
    {
        return false;
    }
    if (!checkFeatureSupport_(vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(requiredFeatures.ptr()),
                              vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(supportedFeatures.ptr()),
                              15))
    {
        return false;
    }
    if (!checkFeatureSupport_(vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(requiredFeatures.ptr()),
                              vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(supportedFeatures.ptr()),
                              21))
    {
        return false;
    }
    if (!checkFeatureSupport_(vku::FindStructInPNextChain<VkPhysicalDeviceRayQueryFeaturesKHR>(requiredFeatures.ptr()),
                              vku::FindStructInPNextChain<VkPhysicalDeviceRayQueryFeaturesKHR>(supportedFeatures.ptr()),
                              1))
    {
        return false;
    }
    if (!checkFeatureSupport_(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(requiredFeatures.ptr()),
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(supportedFeatures.ptr()),
                5))
    {
        return false;
    }
    if (!checkFeatureSupport_(
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(requiredFeatures.ptr()),
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(supportedFeatures.ptr()),
                5))
    {
        return false;
    }

    return true;
}
template<typename T> bool Device::checkFeatureSupport_(const T *requiredFeatures,
                                                       const T *supportedFeatures,
                                                       const uint32_t memberCount)
{
    if (requiredFeatures == nullptr || supportedFeatures == nullptr)
    {
        assert(requiredFeatures == nullptr && supportedFeatures == nullptr); // Internal state check
        return true;
    }
    return checkFeatureSupport_(reinterpret_cast<const VkBool32 *>(reinterpret_cast<const char *>(requiredFeatures) +
                                                                   sizeof(VkBaseInStructure)),
                                reinterpret_cast<const VkBool32 *>(reinterpret_cast<const char *>(supportedFeatures) +
                                                                   sizeof(VkBaseInStructure)),
                                memberCount);
}
inline bool Device::checkFeatureSupport_(const VkPhysicalDeviceFeatures *requiredFeatures,
                                         const VkPhysicalDeviceFeatures *supportedFeatures)
{
    if (requiredFeatures == nullptr || supportedFeatures == nullptr)
    {
        assert(requiredFeatures == nullptr && supportedFeatures == nullptr); // Internal state check
        return true;
    }
    return checkFeatureSupport_(reinterpret_cast<const VkBool32 *>(requiredFeatures),
                                reinterpret_cast<const VkBool32 *>(supportedFeatures),
                                sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
}
inline bool Device::checkFeatureSupport_(const VkBool32 *required,
                                         const VkBool32 *supported,
                                         const uint32_t memberCount)
{
    for (uint32_t i = 0; i < memberCount; i++)
    {
        if (required[i] != VK_FALSE && supported[i] == VK_FALSE)
        {
            return false;
        }
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
