//
// Created by NBT22 on 2/13/25.
//

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <luna/luna.h>
#include <luna/lunaDevice.h>
#include <luna/lunaTypes.h>
#include <stdexcept>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Device.hpp"
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

static constexpr VmaAllocationCreateInfo ALLOCATION_CREATE_INFO = {
    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    .usage = VMA_MEMORY_USAGE_AUTO,
};

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
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    CHECK_RESULT_THROW(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));
    switch (VK_API_VERSION_MINOR(apiVersion))
    {
        case 1:
            vulkan11Features_ = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
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
            break;
        default:
            assert(1 <= VK_API_VERSION_MINOR(apiVersion) && VK_API_VERSION_MINOR(apiVersion) <= 4);
    }
    rayTracingPipelineFeatures_ = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .pNext = &vulkan11Features_,
    };
    accelerationStructureFeatures_ = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .pNext = &rayTracingPipelineFeatures_,
    };
    features_ = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &accelerationStructureFeatures_,
    };
    for (const VkPhysicalDevice physicalDevice: physicalDevices)
    {
        vkGetPhysicalDeviceFeatures2(physicalDevice, &features_);
        // checkUsability can throw an error, but it will be caught by the function calling this constructor
        if (!checkFeatureSupport_(creationInfo.requiredFeatures) ||
            !checkUsability_(physicalDevice, creationInfo.surface))
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

    std::vector<float> priorities;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    CHECK_RESULT_THROW(initQueueFamilies_(physicalDevice_, creationInfo.surface, priorities, queueCreateInfos));
    for (float &priority: priorities)
    {
        priority = 1.0f;
    }
    for (VkDeviceQueueCreateInfo &createInfo: queueCreateInfos)
    {
        createInfo.pQueuePriorities = priorities.data();
    }

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
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = creationInfo.extensionCount,
        .ppEnabledExtensionNames = creationInfo.extensionNames,
        .pEnabledFeatures = &creationInfo.requiredFeatures.features,
    };
    CHECK_RESULT_THROW(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_));
    volkLoadDevice(logicalDevice_);

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
        .flags = creationInfo.allocatorCreateFlags == 0
                         ? static_cast<VmaAllocatorCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT)
                         : creationInfo.allocatorCreateFlags,
        .physicalDevice = physicalDevice_,
        .device = logicalDevice_,
        .pVulkanFunctions = &vmaVulkanFunctions,
        .instance = instance,
        .vulkanApiVersion = apiVersion,
    };
    CHECK_RESULT_THROW(vmaCreateAllocator(&allocationCreateInfo, &allocator_));

    std::vector<uint32_t> queueFamilyIndices;
    queueFamilyIndices.reserve(queueFamilies_.size());
    for (uint32_t i = 0; i < queueFamilies_.size(); i++)
    {
        queueFamilyIndices.emplace_back(i);
    }
    const LunaBufferCreationInfo bufferCreationInfo = {
        .size = 1 << 16,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
        .queueFamilyIndices = queueFamilyIndices.data(),
        .allocationCreateInfo = &ALLOCATION_CREATE_INFO,
    };
    LunaBuffer stagingBufferHandle = LUNA_NULL_HANDLE;
    CHECK_RESULT_THROW(BufferRegion::createBufferRegion(*this, bufferCreationInfo, &stagingBufferHandle));
    stagingBuffer_ = helpers::fromHandle<BufferRegionIndex>(stagingBufferHandle);

    isDestroyed_ = false;
}

void Device::destroy()
{
    if (isDestroyed_)
    {
        return;
    }

    for (const VkSampler sampler: samplers_)
    {
        vkDestroySampler(logicalDevice_, sampler, nullptr);
    }
    samplers_.clear();
    for (Image &image: images_)
    {
        image.destroy(logicalDevice_, allocator_);
    }
    images_.clear();

    for (GraphicsPipeline &pipeline: graphicsPipelines_)
    {
        pipeline.destroy(logicalDevice_);
    }
    graphicsPipelines_.clear();
    for (RenderPass &renderPass: renderPasses_)
    {
        renderPass.destroy(logicalDevice_, allocator_);
    }
    renderPasses_.clear();

    for (ComputePipeline &pipeline: computePipelines_)
    {
        pipeline.destroy(logicalDevice_);
    }
    computePipelines_.clear();

    for (const VkDescriptorPool descriptorPool: descriptorPools_)
    {
        vkDestroyDescriptorPool(logicalDevice_, descriptorPool, nullptr);
    }
    descriptorPools_.clear();
    for (DescriptorSetLayout &descriptorSetLayout: descriptorSetLayouts_)
    {
        descriptorSetLayout.destroy(logicalDevice_);
    }
    descriptorSetLayouts_.clear();
    descriptorSetIndices_.clear();
    descriptorSets_.clear();

    for (BufferRegionIndex &bufferRegionIndex: bufferRegionIndices_)
    {
        bufferRegionIndex.destroy(*this);
    }
    bufferRegionIndices_.clear();

    for (Buffer &buffer: buffers_)
    {
        buffer.destroy(logicalDevice_, allocator_);
    }
    buffers_.clear();

    for (ShaderModule &shaderModule: shaderModules_)
    {
        shaderModule.destroy(logicalDevice_);
    }
    shaderModules_.clear();

    for (CommandPool &commandPool: commandPools_)
    {
        commandPool.destroy(logicalDevice_);
    }
    commandPools_.clear();
    for (Semaphore &semaphore: semaphores_)
    {
        semaphore.destroy(logicalDevice_);
    }
    semaphores_.clear();
    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(logicalDevice_, nullptr);

    queueFamilies_.clear();
    queueFamilies_.shrink_to_fit();
    isDestroyed_ = true;
}

VkResult Device::createCommandPool(const LunaCommandPoolCreationInfo &creationInfo, LunaCommandPool *commandPool)
{
    TRY_CATCH_RESULT(commandPools_.emplace_back(logicalDevice_, creationInfo));
    if (commandPool != nullptr)
    {
        *commandPool = helpers::toHandle(&commandPools_.back());
    }
    return VK_SUCCESS;
}

VkResult Device::createShaderModule(const LunaShaderModuleCreationInfo &creationInfo, LunaShaderModule *shaderModule)
{
    TRY_CATCH_RESULT(shaderModules_.emplace_back(logicalDevice_, creationInfo));
    if (shaderModule != nullptr)
    {
        *shaderModule = helpers::toHandle(&shaderModules_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createRenderPass(const LunaRenderPassCreationInfo &creationInfo, LunaRenderPass *renderPass)
{
    TRY_CATCH_RESULT(renderPasses_.emplace_back(logicalDevice_, allocator_, creationInfo));
    if (renderPass != nullptr)
    {
        *renderPass = helpers::toHandle(&renderPasses_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createRenderPass(const LunaRenderPassCreationInfo2 &creationInfo, LunaRenderPass *renderPass)
{
    TRY_CATCH_RESULT(renderPasses_.emplace_back(logicalDevice_, allocator_, creationInfo));
    if (renderPass != nullptr)
    {
        *renderPass = helpers::toHandle(&renderPasses_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo,
                                           LunaDescriptorSetLayout *descriptorSetLayout)
{
    TRY_CATCH_RESULT(descriptorSetLayouts_.emplace_back(logicalDevice_, creationInfo));
    if (descriptorSetLayout != nullptr)
    {
        *descriptorSetLayout = helpers::toHandle(&descriptorSetLayouts_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createDescriptorPool(const LunaDescriptorPoolCreationInfo &creationInfo,
                                      LunaDescriptorPool *descriptorPool)
{
    VkDescriptorPool &vkDescriptorPool = descriptorPools_.emplace_back();
    const VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = creationInfo.flags,
        .maxSets = creationInfo.maxSets,
        .poolSizeCount = creationInfo.poolSizeCount,
        .pPoolSizes = creationInfo.poolSizes,
    };
    CHECK_RESULT_RETURN(vkCreateDescriptorPool(logicalDevice_, &createInfo, nullptr, &vkDescriptorPool));
    if (descriptorPool != nullptr)
    {
        *descriptorPool = helpers::toHandle(vkDescriptorPool);
    }
    return VK_SUCCESS;
}
VkResult Device::allocateDescriptorSets(const LunaDescriptorSetAllocationInfo &allocationInfo,
                                        LunaDescriptorSet *descriptorSets)
{
    if (allocationInfo.setLayoutCount != 0)
    {
        assert(allocationInfo.setLayouts);
        const VkDescriptorPool *pool = helpers::fromHandle<VkDescriptorPool>(allocationInfo.descriptorPool);
        for (uint32_t i = 0; i < allocationInfo.setLayoutCount; i++)
        {
            const DescriptorSetLayout *layout = helpers::fromHandle<DescriptorSetLayout>(allocationInfo.setLayouts[i]);
            const VkDescriptorSetLayout vkLayout = *layout;

            VkDescriptorSet *descriptorSet = &descriptorSets_.emplace_back();
            const VkDescriptorSetAllocateInfo allocateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = *pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &vkLayout,
            };
            CHECK_RESULT_RETURN(vkAllocateDescriptorSets(logicalDevice_, &allocateInfo, descriptorSet));
            descriptorSetIndices_.emplace_back(pool, layout, descriptorSet);
            descriptorSets[i] = helpers::toHandle(&descriptorSetIndices_.back());
        }
    }
    return VK_SUCCESS;
}
VkResult Device::createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo,
                                        LunaGraphicsPipeline *pipeline,
                                        const VkRenderPass renderPass,
                                        const uint32_t subpassIndex)
{
    TRY_CATCH_RESULT(graphicsPipelines_.emplace_back(logicalDevice_, creationInfo, renderPass, subpassIndex));
    if (pipeline != nullptr)
    {
        *pipeline = helpers::toHandle(&graphicsPipelines_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createComputePipeline(const VkDevice device,
                                       const LunaComputePipelineCreationInfo &creationInfo,
                                       LunaComputePipeline *pipeline)
{
    TRY_CATCH_RESULT(computePipelines_.emplace_back(device, creationInfo));
    if (pipeline != nullptr)
    {
        *pipeline = helpers::toHandle(&computePipelines_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createBuffer(const VkBufferCreateInfo &bufferCreateInfo,
                              const VmaAllocationCreateInfo &allocationCreateInfo,
                              Buffer *&outBuffer)
{
    TRY_CATCH_RESULT(buffers_.emplace_back(allocator_, bufferCreateInfo, allocationCreateInfo));
    outBuffer = &buffers_.back();
    return VK_SUCCESS;
}
VkResult Device::createBuffer(const VkBufferCreateInfo &bufferCreateInfo,
                              const VmaAllocationCreateInfo &allocationCreateInfo,
                              VkDeviceSize alignment,
                              Buffer *&outBuffer)
{
    TRY_CATCH_RESULT(buffers_.emplace_back(allocator_, bufferCreateInfo, allocationCreateInfo, alignment));
    outBuffer = &buffers_.back();
    return VK_SUCCESS;
}
VkResult Device::createBufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, LunaBuffer *outBuffer)
{
    TRY_CATCH_RESULT(bufferRegionIndices_.emplace_back(buffer, bufferRegion));
    if (outBuffer != nullptr)
    {
        *outBuffer = helpers::toHandle(&bufferRegionIndices_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createSampler(const LunaSamplerCreationInfo &creationInfo, LunaSampler *sampler)
{
    VkSampler &vkSampler = samplers_.emplace_back();
    const VkSamplerCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .flags = creationInfo.flags,
        .magFilter = creationInfo.magFilter,
        .minFilter = creationInfo.minFilter,
        .mipmapMode = creationInfo.mipmapMode,
        .addressModeU = creationInfo.addressModeU,
        .addressModeV = creationInfo.addressModeV,
        .addressModeW = creationInfo.addressModeW,
        .mipLodBias = creationInfo.mipmapLodBias,
        .anisotropyEnable = static_cast<VkBool32>(creationInfo.anisotropyEnable),
        .maxAnisotropy = creationInfo.maxAnisotropy,
        .compareEnable = static_cast<VkBool32>(creationInfo.compareEnable),
        .compareOp = creationInfo.compareOp,
        .minLod = creationInfo.minLod,
        .maxLod = creationInfo.maxLod,
        .borderColor = creationInfo.borderColor,
        .unnormalizedCoordinates = static_cast<VkBool32>(creationInfo.unnormalizedCoordinates),
    };
    CHECK_RESULT_RETURN(vkCreateSampler(logicalDevice_, &createInfo, nullptr, &vkSampler));
    if (sampler != nullptr)
    {
        *sampler = helpers::toHandle(&vkSampler);
    }
    return VK_SUCCESS;
}
VkResult Device::createImage(CommandBuffer &commandBuffer,
                             const LunaImageCreationInfo &creationInfo,
                             uint32_t depth,
                             uint32_t arrayLayers,
                             LunaImage *image)
{
    assert(creationInfo.queueFamilyIndexCount != 0);
    TRY_CATCH_RESULT(images_.emplace_back(*this, commandBuffer, creationInfo, depth, arrayLayers));
    if (creationInfo.writeInfo.descriptorSet != LUNA_NULL_HANDLE)
    {
        images_.back().updateDescriptorBinding(logicalDevice_,
                                               creationInfo.writeInfo.descriptorSet,
                                               creationInfo.writeInfo.descriptorLayoutBindingName,
                                               creationInfo.writeInfo.descriptorArrayElement);
    }

    if (image != nullptr)
    {
        *image = helpers::toHandle(&images_.back());
    }
    return VK_SUCCESS;
}
VkResult Device::createSemaphore(const LunaSemaphoreCreationInfo &creationInfo, LunaSemaphore *semaphore)
{
    const VkSemaphoreTypeCreateInfo typeCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = creationInfo.type,
        .initialValue = creationInfo.initialValue,
    };
    const VkSemaphoreCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &typeCreateInfo,
    };
    TRY_CATCH_RESULT(semaphores_.emplace_back(logicalDevice_, createInfo));
    if (semaphore != nullptr)
    {
        *semaphore = helpers::toHandle(&semaphores_.back());
    }
    return VK_SUCCESS;
}

void Device::destroyBufferRegionIndex(BufferRegionIndex *&bufferRegionIndex)
{
    if (bufferRegionIndex == nullptr)
    {
        return;
    }
    bufferRegionIndex->destroy(*this);
    bufferRegionIndices_.remove(*bufferRegionIndex);
    bufferRegionIndex = nullptr;
}
void Device::destroyBuffer(Buffer *&buffer)
{
    if (buffer == nullptr)
    {
        return;
    }
    buffer->destroy(logicalDevice_, allocator_);
    buffers_.remove(*buffer);
    buffer = nullptr;
}
void Device::destroySampler(const LunaSampler &sampler)
{
    if (sampler == LUNA_NULL_HANDLE)
    {
        return;
    }
    const VkSampler vkSampler = *helpers::fromHandle<VkSampler>(sampler);
    samplers_.remove(vkSampler);
    vkDestroySampler(logicalDevice_, vkSampler, nullptr);
}
void Device::destroyImage(const LunaImage &image)
{
    if (image == LUNA_NULL_HANDLE)
    {
        return;
    }
    Image &imageObject = *helpers::fromHandle<Image>(image);
    imageObject.destroy(logicalDevice_, allocator_);
    images_.remove(imageObject);
}
void Device::destroySemaphore(const LunaSemaphore &semaphore)
{
    if (semaphore == LUNA_NULL_HANDLE)
    {
        return;
    }
    Semaphore &semaphoreObject = *helpers::fromHandle<Semaphore>(semaphore);
    semaphoreObject.destroy(logicalDevice_);
    semaphores_.remove(semaphoreObject);
}
void Device::destroyGraphicsPipeline(const LunaGraphicsPipeline &graphicsPipeline)
{
    if (graphicsPipeline == LUNA_NULL_HANDLE)
    {
        return;
    }
    GraphicsPipeline &pipeline = *helpers::fromHandle<GraphicsPipeline>(graphicsPipeline);
    pipeline.destroy(logicalDevice_);
    graphicsPipelines_.remove_if([&pipeline](const GraphicsPipeline &graphicsPipeline) -> bool {
        return &pipeline == &graphicsPipeline;
    });
}

uint32_t Device::findQueueFamilyIndex(const LunaQueueFamilyProperties &requiredProperties) const
{
    for (uint32_t i = 0; i < queueFamilies_.size(); i++)
    {
        const LunaQueueFamilyProperties &properties = queueFamilies_.at(i);
        if (requiredProperties.presentationSupport && !properties.presentationSupport)
        {
            continue;
        }
        if ((properties.queueFamilyProperties.queueFlags & requiredProperties.queueFamilyProperties.queueFlags) !=
            requiredProperties.queueFamilyProperties.queueFlags)
        {
            continue;
        }
        if (properties.queueFamilyProperties.queueCount < requiredProperties.queueFamilyProperties.queueCount)
        {
            continue;
        }
        if (properties.queueFamilyProperties.timestampValidBits <
            requiredProperties.queueFamilyProperties.timestampValidBits)
        {
            continue;
        }
        const VkExtent3D minImageTransferGranularity = {
            .width = std::max(requiredProperties.queueFamilyProperties.minImageTransferGranularity.width, 1u),
            .height = std::max(requiredProperties.queueFamilyProperties.minImageTransferGranularity.height, 1u),
            .depth = std::max(requiredProperties.queueFamilyProperties.minImageTransferGranularity.depth, 1u),
        };
        if (minImageTransferGranularity.width < properties.queueFamilyProperties.minImageTransferGranularity.width)
        {
            continue;
        }
        if (minImageTransferGranularity.height < properties.queueFamilyProperties.minImageTransferGranularity.height)
        {
            continue;
        }
        if (minImageTransferGranularity.depth < properties.queueFamilyProperties.minImageTransferGranularity.depth)
        {
            continue;
        }
        return i;
    }
    return static_cast<uint32_t>(-1);
}
} // namespace luna

VkResult lunaCreateDevice(const LunaDeviceCreationInfo *creationInfo, LunaDevice *device)
{
    assert(creationInfo);
    assert(device);

    const VkPhysicalDeviceFeatures2 requiredFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features = creationInfo->requiredFeatures,
    };
    const LunaDeviceCreationInfo2 creationInfo2 = {
        .extensionCount = creationInfo->extensionCount,
        .extensionNames = creationInfo->extensionNames,
        .requiredFeatures = requiredFeatures2,
        .surface = creationInfo->surface,
        .requiredQueueFamiliesCount = creationInfo->requiredQueueFamiliesCount,
        .requiredQueueFamilies = creationInfo->requiredQueueFamilies,
        .physicalDevicePreferenceDefinition = creationInfo->physicalDevicePreferenceDefinition,
    };
    TRY_CATCH_RESULT(luna::devices.emplace_back(creationInfo2));
    luna::Device &deviceObject = luna::devices.back();
    *device = luna::helpers::toHandle(deviceObject);
    return VK_SUCCESS;
}

VkResult lunaCreateDevice2(const LunaDeviceCreationInfo2 *creationInfo, LunaDevice *device)
{
    assert(creationInfo);
    assert(device);

    TRY_CATCH_RESULT(luna::devices.emplace_back(*creationInfo));
    luna::Device &deviceObject = luna::devices.back();
    *device = luna::helpers::toHandle(deviceObject);
    return VK_SUCCESS;
}

VkDevice lunaGetVkDevice(const LunaDevice device)
{
    assert(device != LUNA_NULL_HANDLE);
    return static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device));
}

VkPhysicalDevice lunaGetPhysicalDevice(const LunaDevice device)
{
    assert(device != LUNA_NULL_HANDLE);
    return static_cast<VkPhysicalDevice>(*luna::helpers::fromHandle<luna::Device>(device));
}

VkResult lunaDeviceWaitIdle(const LunaDevice device)
{
    CHECK_RESULT_RETURN(vkDeviceWaitIdle(lunaGetVkDevice(device)));
    return VK_SUCCESS;
}

void lunaGetPhysicalDeviceProperties(const LunaDevice device, VkPhysicalDeviceProperties *properties)
{
    vkGetPhysicalDeviceProperties(lunaGetPhysicalDevice(device), properties);
}

void lunaGetPhysicalDeviceProperties2(const LunaDevice device, VkPhysicalDeviceProperties2 *properties)
{
    vkGetPhysicalDeviceProperties2(lunaGetPhysicalDevice(device), properties);
}

const LunaQueueFamilyProperties *lunaGetDeviceQueueFamilies(const LunaDevice device)
{
    assert(device != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::Device>(device)->queueFamilies();
}

uint32_t lunaGetDeviceQueueFamilyIndex(const LunaDevice device, const LunaQueueFamilyProperties *requiredProperties)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(requiredProperties);
    return luna::helpers::fromHandle<luna::Device>(device)->findQueueFamilyIndex(*requiredProperties);
}

VkQueue lunaGetDeviceQueue(const LunaDevice device, const uint32_t queueFamilyIndex, const uint32_t queueIndex)
{
    assert(device != LUNA_NULL_HANDLE);
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(lunaGetVkDevice(device), queueFamilyIndex, queueIndex, &queue);
    return queue;
}

VkResult lunaCreateShaderModule(const LunaDevice device,
                                const LunaShaderModuleCreationInfo *creationInfo,
                                LunaShaderModule *shaderModule)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createShaderModule(*creationInfo,
                                                                                            shaderModule));
    return VK_SUCCESS;
}
