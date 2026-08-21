//
// Created by NBT22 on 3/1/25.
//

#define VMA_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <volk.h>
#undef VMA_IMPLEMENTATION
#undef VOLK_IMPLEMENTATION

#include <array>
#include <cassert>
#include <cstdint>
#include <list>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna::helpers
{
static VkResult recreateSwapchain(const VkDevice device,
                                  const VkSurfaceCapabilitiesKHR &capabilities,
                                  const uint32_t queueFamilyIndexCount,
                                  const uint32_t *queueFamilyIndices)
{
    const VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = swapchain.surface,
        .minImageCount = capabilities.minImageCount,
        .imageFormat = swapchain.format.format,
        .imageColorSpace = swapchain.format.colorSpace,
        .imageExtent = swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = swapchain.imageUsage,
        .imageSharingMode = queueFamilyIndexCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = queueFamilyIndexCount,
        .pQueueFamilyIndices = queueFamilyIndices,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = swapchain.compositeAlpha,
        .presentMode = swapchain.presentMode,
        .clipped = VK_TRUE,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(device, &createInfo, nullptr, &luna::swapchain.swapchain));

    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(device,
                                                luna::swapchain.swapchain,
                                                &luna::swapchain.imageCount,
                                                nullptr));

    swapchain.images.resize(swapchain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(device,
                                                luna::swapchain.swapchain,
                                                &luna::swapchain.imageCount,
                                                luna::swapchain.images.data()));

    swapchain.imageViews.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(device,
                                            luna::swapchain.images.at(i),
                                            luna::swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &luna::swapchain.imageViews.at(i)));
    }
    assert(capabilities.minImageCount <= luna::swapchain.imageCount &&
           luna::swapchain.imageCount <= capabilities.maxImageCount);

    for (Semaphore &semaphore: swapchain.renderSemaphores)
    {
        semaphore.destroy(device);
    }
    swapchain.renderSemaphores.clear();
    swapchain.renderSemaphores.reserve(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        swapchain.renderSemaphores.emplace_back(static_cast<VkDevice>(device), semaphoreCreateInfo);
    }

    return VK_SUCCESS;
}

void pipelineBarrier(const VkCommandBuffer commandBuffer, const LunaDependencyInfo &dependencyInfo)
{
    if (vkCmdPipelineBarrier2 == nullptr && vkCmdPipelineBarrier2KHR == nullptr)
    {
        VkPipelineStageFlags sourceStageMask{};
        VkPipelineStageFlags destinationStageMask{};

        std::vector<VkMemoryBarrier> memoryBarriers;
        std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
        std::vector<VkImageMemoryBarrier> imageMemoryBarriers;

        memoryBarriers.reserve(dependencyInfo.memoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.memoryBarrierCount; i++)
        {
            const LunaMemoryBarrier &memoryBarrier = dependencyInfo.memoryBarriers[i];
            sourceStageMask |= memoryBarrier.sourceStageMask;
            destinationStageMask |= memoryBarrier.destinationStageMask;
            memoryBarriers.emplace_back(VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                                        nullptr,
                                        memoryBarrier.sourceAccessMask,
                                        memoryBarrier.destinationAccessMask);
        }

        bufferMemoryBarriers.reserve(dependencyInfo.bufferMemoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.bufferMemoryBarrierCount; i++)
        {
            const LunaBufferMemoryBarrier &bufferMemoryBarrier = dependencyInfo.bufferMemoryBarriers[i];
            sourceStageMask |= bufferMemoryBarrier.sourceStageMask;
            destinationStageMask |= bufferMemoryBarrier.destinationStageMask;
            assert(bufferMemoryBarrier.buffer);
            const BufferRegionIndex &bufferRegionIndex =
                    *luna::helpers::fromHandle<BufferRegionIndex>(bufferMemoryBarrier.buffer);
            bufferMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                              nullptr,
                                              bufferMemoryBarrier.sourceAccessMask,
                                              bufferMemoryBarrier.destinationAccessMask,
                                              bufferMemoryBarrier.srcQueueFamilyIndex,
                                              bufferMemoryBarrier.dstQueueFamilyIndex,
                                              bufferRegionIndex.buffer(),
                                              bufferRegionIndex.offset() + bufferMemoryBarrier.offset,
                                              bufferMemoryBarrier.size == 0 ? bufferRegionIndex.size()
                                                                            : bufferMemoryBarrier.size);
        }

        imageMemoryBarriers.reserve(dependencyInfo.imageMemoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.imageMemoryBarrierCount; i++)
        {
            const LunaImageMemoryBarrier &imageMemoryBarrier = dependencyInfo.imageMemoryBarriers[i];
            sourceStageMask |= imageMemoryBarrier.sourceStageMask;
            destinationStageMask |= imageMemoryBarrier.destinationStageMask;
            assert(imageMemoryBarrier.image);
            imageMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                             nullptr,
                                             imageMemoryBarrier.sourceAccessMask,
                                             imageMemoryBarrier.destinationAccessMask,
                                             imageMemoryBarrier.oldLayout,
                                             imageMemoryBarrier.newLayout,
                                             imageMemoryBarrier.srcQueueFamilyIndex,
                                             imageMemoryBarrier.dstQueueFamilyIndex,
                                             luna::helpers::fromHandle<Image>(imageMemoryBarrier.image)->image(),
                                             imageMemoryBarrier.subresourceRange);
        }
        assert(dependencyInfo.memoryBarrierCount == memoryBarriers.size());
        assert(dependencyInfo.bufferMemoryBarrierCount == bufferMemoryBarriers.size());
        assert(dependencyInfo.imageMemoryBarrierCount == imageMemoryBarriers.size());
        vkCmdPipelineBarrier(commandBuffer,
                             sourceStageMask,
                             destinationStageMask,
                             dependencyInfo.flags,
                             memoryBarriers.size(),
                             memoryBarriers.data(),
                             bufferMemoryBarriers.size(),
                             bufferMemoryBarriers.data(),
                             imageMemoryBarriers.size(),
                             imageMemoryBarriers.data());
    } else
    {
        VkDependencyInfo vkDependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = dependencyInfo.flags,
            .memoryBarrierCount = dependencyInfo.memoryBarrierCount,
            .imageMemoryBarrierCount = dependencyInfo.imageMemoryBarrierCount,
        };
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferMemoryBarriers;
        std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;

        memoryBarriers.reserve(dependencyInfo.memoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.memoryBarrierCount; i++)
        {
            const LunaMemoryBarrier &memoryBarrier = dependencyInfo.memoryBarriers[i];
            memoryBarriers.emplace_back(VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
                                        nullptr,
                                        memoryBarrier.sourceStageMask,
                                        memoryBarrier.sourceAccessMask,
                                        memoryBarrier.destinationStageMask,
                                        memoryBarrier.destinationAccessMask);
        }

        bufferMemoryBarriers.reserve(dependencyInfo.bufferMemoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.bufferMemoryBarrierCount; i++)
        {
            const LunaBufferMemoryBarrier &bufferMemoryBarrier = dependencyInfo.bufferMemoryBarriers[i];
            assert(bufferMemoryBarrier.buffer);
            const BufferRegionIndex &bufferRegionIndex =
                    *luna::helpers::fromHandle<BufferRegionIndex>(bufferMemoryBarrier.buffer);
            bufferMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                                              nullptr,
                                              bufferMemoryBarrier.sourceStageMask,
                                              bufferMemoryBarrier.sourceAccessMask,
                                              bufferMemoryBarrier.destinationStageMask,
                                              bufferMemoryBarrier.destinationAccessMask,
                                              bufferMemoryBarrier.srcQueueFamilyIndex,
                                              bufferMemoryBarrier.dstQueueFamilyIndex,
                                              bufferRegionIndex.buffer(),
                                              bufferRegionIndex.offset() + bufferMemoryBarrier.offset,
                                              bufferMemoryBarrier.size == 0 ? bufferRegionIndex.size()
                                                                            : bufferMemoryBarrier.size);
        }

        for (uint32_t i = 0; i < dependencyInfo.multiBufferMemoryBarrierCount; i++)
        {
            const LunaMultiBufferMemoryBarrier &bufferMemoryBarrier = dependencyInfo.multiBufferMemoryBarriers[i];
            for (uint32_t j = 0; j < bufferMemoryBarrier.bufferCount; j++)
            {
                assert(bufferMemoryBarrier.buffers[j] != LUNA_NULL_HANDLE);
                const BufferRegionIndex &bufferRegionIndex =
                        *luna::helpers::fromHandle<BufferRegionIndex>(bufferMemoryBarrier.buffers[j]);
                if (bufferRegionIndex.size() == 0)
                {
                    continue;
                }
                bufferMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                                                  nullptr,
                                                  bufferMemoryBarrier.sourceStageMask,
                                                  bufferMemoryBarrier.sourceAccessMask,
                                                  bufferMemoryBarrier.destinationStageMask,
                                                  bufferMemoryBarrier.destinationAccessMask,
                                                  bufferMemoryBarrier.srcQueueFamilyIndex,
                                                  bufferMemoryBarrier.dstQueueFamilyIndex,
                                                  bufferRegionIndex.buffer(),
                                                  bufferRegionIndex.offset(),
                                                  bufferMemoryBarrier.size == 0 ? bufferRegionIndex.size()
                                                                                : bufferMemoryBarrier.size);
            }
        }

        imageMemoryBarriers.reserve(dependencyInfo.imageMemoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.imageMemoryBarrierCount; i++)
        {
            const LunaImageMemoryBarrier &imageMemoryBarrier = dependencyInfo.imageMemoryBarriers[i];
            assert(imageMemoryBarrier.image);
            imageMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                             nullptr,
                                             imageMemoryBarrier.sourceStageMask,
                                             imageMemoryBarrier.sourceAccessMask,
                                             imageMemoryBarrier.destinationStageMask,
                                             imageMemoryBarrier.destinationAccessMask,
                                             imageMemoryBarrier.oldLayout,
                                             imageMemoryBarrier.newLayout,
                                             imageMemoryBarrier.srcQueueFamilyIndex,
                                             imageMemoryBarrier.dstQueueFamilyIndex,
                                             luna::helpers::fromHandle<Image>(imageMemoryBarrier.image)->image(),
                                             imageMemoryBarrier.subresourceRange);
        }
        assert(dependencyInfo.memoryBarrierCount == memoryBarriers.size());
        assert(dependencyInfo.imageMemoryBarrierCount == imageMemoryBarriers.size());
        vkDependencyInfo.pMemoryBarriers = memoryBarriers.data();
        vkDependencyInfo.bufferMemoryBarrierCount = bufferMemoryBarriers.size();
        vkDependencyInfo.pBufferMemoryBarriers = bufferMemoryBarriers.data();
        vkDependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();
        if (vkCmdPipelineBarrier2 == nullptr)
        {
            vkCmdPipelineBarrier2KHR(commandBuffer, &vkDependencyInfo);
        } else
        {
            vkCmdPipelineBarrier2(commandBuffer, &vkDependencyInfo);
        }
    }
}
} // namespace luna::helpers

VkResult lunaCreateDescriptorPool(const LunaDevice device,
                                  const LunaDescriptorPoolCreationInfo *creationInfo,
                                  LunaDescriptorPool *descriptorPool)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createDescriptorPool(*creationInfo,
                                                                                              descriptorPool));
    return VK_SUCCESS;
}

VkResult lunaAllocateDescriptorSets(const LunaDevice device,
                                    const LunaDescriptorSetAllocationInfo *allocationInfo,
                                    LunaDescriptorSet **descriptorSets)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(allocationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->allocateDescriptorSets(*allocationInfo,
                                                                                                descriptorSets));
    return VK_SUCCESS;
}

void lunaWriteDescriptorSets(const LunaDevice device,
                             const uint32_t descriptorWriteCount,
                             const LunaWriteDescriptorSet *descriptorWrites)
{
    using namespace luna;
    std::list<std::vector<VkDescriptorImageInfo>> descriptorImageInfos;
    std::list<std::vector<VkDescriptorBufferInfo>> descriptorBufferInfos;
    std::list<std::vector<VkBufferView>> texelBufferViews;
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(descriptorWriteCount);
    for (uint32_t i = 0; i < descriptorWriteCount; i++)
    {
        const LunaWriteDescriptorSet &descriptorWrite = descriptorWrites[i];
        const LunaDescriptorSet descriptorSet = descriptorWrite.descriptorSet;
        const DescriptorSetIndex *descriptorSetIndex = luna::helpers::fromHandle<DescriptorSetIndex>(descriptorSet);
        const DescriptorSetLayout::Binding &binding = descriptorSetIndex->layout->binding(descriptorWrite.bindingName);
        const uint32_t descriptorCount = descriptorWrite.descriptorCount == 0 ? 1 : descriptorWrite.descriptorCount;
        if (descriptorWrite.imageInfos != nullptr)
        {
            std::vector<VkDescriptorImageInfo> imageInfos{};
            for (uint32_t j = 0; j < descriptorCount; j++)
            {
                const Image *image = luna::helpers::fromHandle<Image>(descriptorWrite.imageInfos[j].image);
                imageInfos.emplace_back(image->sampler(descriptorWrite.imageInfos[j].sampler),
                                        image->imageView(),
                                        descriptorWrite.imageInfos[j].imageLayout);
            }
            descriptorImageInfos.push_back(imageInfos);
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                imageInfos.size(),
                                binding.type,
                                descriptorImageInfos.back().data(),
                                nullptr,
                                nullptr);
        }
        if (descriptorWrite.bufferInfos != nullptr)
        {
            std::vector<VkDescriptorBufferInfo> bufferInfos{};
            for (uint32_t j = 0; j < descriptorCount; j++)
            {
                assert(descriptorWrite.bufferInfos[j].buffer != LUNA_NULL_HANDLE);
                const LunaBuffer buffer = descriptorWrite.bufferInfos[j].buffer;
                const BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<BufferRegionIndex>(buffer);
                if (bufferRegionIndex.size() == 0)
                {
                    continue;
                }
                assert(descriptorWrite.bufferInfos[j].offset < bufferRegionIndex.size());
                bufferInfos.emplace_back(bufferRegionIndex.buffer(),
                                         bufferRegionIndex.offset() + descriptorWrite.bufferInfos[j].offset,
                                         descriptorWrite.bufferInfos[j].range == 0
                                                 ? bufferRegionIndex.size() - descriptorWrite.bufferInfos[j].offset
                                                 : descriptorWrite.bufferInfos[j].range);
            }
            descriptorBufferInfos.push_back(bufferInfos);

            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                bufferInfos.size(),
                                binding.type,
                                nullptr,
                                descriptorBufferInfos.back().data(),
                                nullptr);
        }
        if (descriptorWrite.texelBufferViews != nullptr)
        {
            std::vector<VkBufferView> bufferViews{};
            for (uint32_t j = 0; j < descriptorCount; j++)
            {
                assert(descriptorWrite.texelBufferViews[j] != LUNA_NULL_HANDLE);
                bufferViews.emplace_back(*helpers::fromHandle<VkBufferView>(descriptorWrite.texelBufferViews[j]));
            }
            texelBufferViews.push_back(bufferViews);
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                bufferViews.size(),
                                binding.type,
                                nullptr,
                                nullptr,
                                texelBufferViews.back().data());
        }
    }
    vkUpdateDescriptorSets(static_cast<VkDevice>(*luna::helpers::fromHandle<Device>(device)),
                           writes.size(),
                           writes.data(),
                           0,
                           nullptr);
}

VkResult lunaPipelineBarrier(const LunaDevice device,
                             const LunaCommandBuffer commandBuffer,
                             const LunaDependencyInfo *dependencyInfo)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(dependencyInfo);

    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(static_cast<VkDevice>(*luna::helpers::fromHandle<
                                                                                    luna::Device>(device))));
    luna::helpers::pipelineBarrier(commandBufferObject, *dependencyInfo);
    return VK_SUCCESS;
}

VkResult lunaDraw(const LunaDevice device, const LunaCommandBuffer commandBuffer, const LunaDrawInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    vkCmdDraw(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
              drawInfo->vertexCount,
              drawInfo->instanceCount,
              drawInfo->firstVertex,
              drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawIndirect(const LunaDevice device,
                          const LunaCommandBuffer commandBuffer,
                          const LunaDrawIndirectInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *bufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndirect(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                      bufferRegionIndex->buffer(),
                      bufferRegionIndex->offset(),
                      drawInfo->drawCount,
                      drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawIndirectCount(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaDrawIndirectCountInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                           drawParameterBufferRegionIndex->buffer(),
                           drawParameterBufferRegionIndex->offset() + sizeof(uint32_t),
                           drawParameterBufferRegionIndex->buffer(),
                           drawParameterBufferRegionIndex->offset(),
                           drawInfo->maxDrawCount,
                           drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawIndexed(const LunaDevice device,
                         const LunaCommandBuffer commandBuffer,
                         const LunaDrawIndexedInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    vkCmdDrawIndexed(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                     drawInfo->indexCount,
                     drawInfo->instanceCount,
                     drawInfo->firstIndex,
                     drawInfo->vertexOffset,
                     drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawIndexedIndirect(const LunaDevice device,
                                 const LunaCommandBuffer commandBuffer,
                                 const LunaDrawIndexedIndirectInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *bufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndexedIndirect(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                             bufferRegionIndex->buffer(),
                             bufferRegionIndex->offset(),
                             drawInfo->drawCount,
                             drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawIndexedIndirectCount(const LunaDevice device,
                                      const LunaCommandBuffer commandBuffer,
                                      const LunaDrawIndexedIndirectCountInfo *drawInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndexedIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                  drawParameterBufferRegionIndex->buffer(),
                                  drawParameterBufferRegionIndex->offset() + sizeof(uint32_t),
                                  drawParameterBufferRegionIndex->buffer(),
                                  drawParameterBufferRegionIndex->offset(),
                                  drawInfo->maxDrawCount,
                                  drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaResizeSwapchain(const LunaDevice device, const LunaSwapchainResizeInfo *resizeInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(resizeInfo);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    const VkDevice vkDevice = static_cast<VkDevice>(deviceObject);

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(static_cast<VkPhysicalDevice>(deviceObject),
                                                                  luna::swapchain.surface,
                                                                  &capabilities));
    capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;
    luna::swapchain.safeToUse.wait(false);
    luna::swapchain.safeToUse = false;
    luna::swapchain.extent = resizeInfo->newSize;
    luna::swapchain.extent.width = std::clamp(luna::swapchain.extent.width,
                                              capabilities.minImageExtent.width,
                                              capabilities.maxImageExtent.width);
    luna::swapchain.extent.height = std::clamp(luna::swapchain.extent.height,
                                               capabilities.minImageExtent.height,
                                               capabilities.maxImageExtent.height);

    for (uint32_t i = 0; i < luna::swapchain.imageCount; i++)
    {
        vkDestroyImageView(vkDevice, luna::swapchain.imageViews.at(i), nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, luna::swapchain.swapchain, nullptr);

    CHECK_RESULT_RETURN(luna::helpers::recreateSwapchain(vkDevice,
                                                         capabilities,
                                                         resizeInfo->queueFamilyIndexCount,
                                                         resizeInfo->queueFamilyIndices));
    for (uint32_t i = 0; i < resizeInfo->renderPassCount; i++)
    {
        CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::RenderPass>(resizeInfo->renderPasses[i])
                                    ->recreateFramebuffer(vkDevice,
                                                          resizeInfo->queueFamilyIndexCount,
                                                          resizeInfo->queueFamilyIndices,
                                                          deviceObject.allocator(),
                                                          luna::swapchain.extent.width,
                                                          luna::swapchain.extent.height));
    }
    luna::swapchain.safeToUse = true;
    luna::swapchain.safeToUse.notify_all();

    return VK_SUCCESS;
}

VkResult lunaBeginFrame(const LunaDevice device, const LunaCommandBuffer commandBuffer)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);

    const VkDevice vkDevice = lunaGetVkDevice(device);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.waitForFence(vkDevice));
    CHECK_RESULT_RETURN(commandBufferObject.resetFence(vkDevice));
    luna::Semaphore &semaphore = luna::swapchain.imageReadySemaphores.at(luna::swapchain.frameIndex);
    assert(!semaphore.isSignaled());
    const VkResult acquireImageResult = vkAcquireNextImageKHR(vkDevice,
                                                              luna::swapchain.swapchain,
                                                              UINT64_MAX,
                                                              semaphore,
                                                              VK_NULL_HANDLE,
                                                              &luna::swapchain.imageIndex);
    if (acquireImageResult != VK_SUCCESS && acquireImageResult != VK_SUBOPTIMAL_KHR)
    {
        CHECK_RESULT_RETURN(acquireImageResult);
    }

    semaphore.setIsSignaled(true);

    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(vkDevice));

    return acquireImageResult;
}

VkResult lunaEndFrame(const LunaDevice device,
                      const LunaCommandBuffer commandBuffer,
                      const LunaPresentInfo *presentInfo,
                      const LunaCommandBufferSubmitInfo *submitInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(presentInfo);
    assert(submitInfo && submitInfo->queue != VK_NULL_HANDLE);

    luna::Semaphore &imageReadySemaphore = luna::swapchain.imageReadySemaphores.at(luna::swapchain.frameIndex);
    luna::Semaphore &renderSemaphore = luna::swapchain.renderSemaphores.at(luna::swapchain.imageIndex);

    std::vector<LunaSemaphore> submissionWaitSemaphores{luna::helpers::toHandle(imageReadySemaphore)};
    std::vector<VkPipelineStageFlags2> submissionWaitDstStageMasks{VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};
    if (submitInfo->waitSemaphoreCount > 0)
    {
        submissionWaitSemaphores.insert(submissionWaitSemaphores.end(),
                                        submitInfo->waitSemaphores,
                                        submitInfo->waitSemaphores + submitInfo->waitSemaphoreCount);
        submissionWaitDstStageMasks.insert(submissionWaitDstStageMasks.end(),
                                           submitInfo->waitDstStageMasks,
                                           submitInfo->waitDstStageMasks + submitInfo->waitSemaphoreCount);
    }
    std::vector<LunaSemaphore> signalSemaphores{luna::helpers::toHandle(renderSemaphore)};
    if (submitInfo->signalSemaphoreCount > 0)
    {
        signalSemaphores.insert(signalSemaphores.end(),
                                submitInfo->signalSemaphores,
                                submitInfo->signalSemaphores + submitInfo->signalSemaphoreCount);
    }

    const LunaCommandBufferSubmitInfo finalSubmitInfo = {
        .queue = submitInfo->queue,
        .waitSemaphoreCount = static_cast<uint32_t>(submissionWaitSemaphores.size()),
        .waitSemaphores = submissionWaitSemaphores.data(),
        .waitDstStageMasks = submissionWaitDstStageMasks.data(),
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .signalSemaphores = signalSemaphores.data(),
    };
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer)
                                ->endAndSubmit(lunaGetVkDevice(device), finalSubmitInfo));

    std::vector<VkSemaphore> presentationWaitSemaphores{renderSemaphore};
    for (uint32_t i = 0; i < presentInfo->waitSemaphoreCount; i++)
    {
        const luna::Semaphore *semaphore = luna::helpers::fromHandle<luna::Semaphore>(presentInfo->waitSemaphores[i]);
        if (semaphore != nullptr && semaphore->isSignaled())
        {
            presentationWaitSemaphores.emplace_back(*semaphore);
        }
    }

    ++luna::swapchain.frameIndex;
    luna::swapchain.frameIndex %= luna::Swapchain::FRAMES_IN_FLIGHT;

    const VkPresentInfoKHR finalPresentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = presentInfo->pNext,
        .waitSemaphoreCount = static_cast<uint32_t>(presentationWaitSemaphores.size()),
        .pWaitSemaphores = presentationWaitSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &luna::swapchain.swapchain,
        .pImageIndices = &luna::swapchain.imageIndex,
    };
    const VkResult presentResult = vkQueuePresentKHR(submitInfo->queue, &finalPresentInfo);
    if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR)
    {
        CHECK_RESULT_RETURN(presentResult);
    }
    return presentResult;
}
