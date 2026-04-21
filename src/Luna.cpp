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
    if (vkCmdPipelineBarrier2 == nullptr)
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
            .bufferMemoryBarrierCount = dependencyInfo.bufferMemoryBarrierCount,
            .imageMemoryBarrierCount = dependencyInfo.imageMemoryBarrierCount,
        };
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferMemoryBarriers;
        std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;

        memoryBarriers.reserve(dependencyInfo.memoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.memoryBarrierCount; i++)
        {
            const LunaMemoryBarrier &memoryBarrier = dependencyInfo.memoryBarriers[i];
            memoryBarriers.emplace_back(VK_STRUCTURE_TYPE_MEMORY_BARRIER,
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
            bufferMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                              nullptr,
                                              bufferMemoryBarrier.sourceStageMask,
                                              bufferMemoryBarrier.sourceAccessMask,
                                              bufferMemoryBarrier.destinationStageMask,
                                              bufferMemoryBarrier.destinationAccessMask,
                                              bufferMemoryBarrier.srcQueueFamilyIndex,
                                              bufferMemoryBarrier.dstQueueFamilyIndex,
                                              bufferRegionIndex.buffer(),
                                              bufferRegionIndex.offset() + bufferMemoryBarrier.offset,
                                              bufferMemoryBarrier.size);
        }

        imageMemoryBarriers.reserve(dependencyInfo.imageMemoryBarrierCount);
        for (uint32_t i = 0; i < dependencyInfo.imageMemoryBarrierCount; i++)
        {
            const LunaImageMemoryBarrier &imageMemoryBarrier = dependencyInfo.imageMemoryBarriers[i];
            assert(imageMemoryBarrier.image);
            imageMemoryBarriers.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
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
        assert(dependencyInfo.bufferMemoryBarrierCount == bufferMemoryBarriers.size());
        assert(dependencyInfo.imageMemoryBarrierCount == imageMemoryBarriers.size());
        vkDependencyInfo.pMemoryBarriers = memoryBarriers.data();
        vkDependencyInfo.pBufferMemoryBarriers = bufferMemoryBarriers.data();
        vkDependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();
        vkCmdPipelineBarrier2(commandBuffer, &vkDependencyInfo);
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
                                    LunaDescriptorSet *descriptorSets)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(allocationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->allocateDescriptorSets(*allocationInfo,
                                                                                                descriptorSets));
    return VK_SUCCESS;
}

VkResult lunaWriteDescriptorSets(const LunaDevice device,
                                 const uint32_t descriptorWriteCount,
                                 const LunaWriteDescriptorSet *descriptorWrites)
{
    using namespace luna;
    std::list<VkDescriptorImageInfo> descriptorImageInfos;
    std::list<VkDescriptorBufferInfo> descriptorBufferInfos;
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(descriptorWriteCount);
    for (uint32_t i = 0; i < descriptorWriteCount; i++)
    {
        const LunaWriteDescriptorSet &descriptorWrite = descriptorWrites[i];
        const LunaDescriptorSet descriptorSet = descriptorWrite.descriptorSet;
        const DescriptorSetIndex *descriptorSetIndex = luna::helpers::fromHandle<DescriptorSetIndex>(descriptorSet);
        const DescriptorSetLayout::Binding &binding = descriptorSetIndex->layout->binding(descriptorWrite.bindingName);
        if (descriptorWrite.imageInfo != nullptr)
        {
            const Image *image = luna::helpers::fromHandle<Image>(descriptorWrite.imageInfo->image);
            descriptorImageInfos.emplace_back(image->sampler(descriptorWrite.imageInfo->sampler),
                                              image->imageView(),
                                              descriptorWrite.imageInfo->imageLayout);
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                descriptorWrite.descriptorCount == 0 ? 1 : descriptorWrite.descriptorCount,
                                binding.type,
                                &descriptorImageInfos.back(),
                                nullptr,
                                nullptr);
        }
        if (descriptorWrite.bufferInfo != nullptr)
        {
            const LunaBuffer buffer = descriptorWrite.bufferInfo->buffer;
            const BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(buffer);
            assert(bufferRegionIndex != nullptr);
            assert(descriptorWrite.bufferInfo->offset < bufferRegionIndex->size());
            descriptorBufferInfos.emplace_back(bufferRegionIndex->buffer(),
                                               bufferRegionIndex->offset() + descriptorWrite.bufferInfo->offset,
                                               descriptorWrite.bufferInfo->range == 0
                                                       ? bufferRegionIndex->size() - descriptorWrite.bufferInfo->offset
                                                       : descriptorWrite.bufferInfo->range);
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                descriptorWrite.descriptorCount == 0 ? 1 : descriptorWrite.descriptorCount,
                                binding.type,
                                nullptr,
                                &descriptorBufferInfos.back(),
                                nullptr);
        }
        if (descriptorWrite.texelBufferView != LUNA_NULL_HANDLE)
        {
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                descriptorWrite.descriptorCount == 0 ? 1 : descriptorWrite.descriptorCount,
                                binding.type,
                                nullptr,
                                nullptr,
                                helpers::fromHandle<VkBufferView>(descriptorWrite.texelBufferView));
        }
    }
    vkUpdateDescriptorSets(static_cast<VkDevice>(*luna::helpers::fromHandle<Device>(device)),
                           writes.size(),
                           writes.data(),
                           0,
                           nullptr);
    return VK_SUCCESS;
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
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    const luna::BufferRegionIndex *countBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->countBuffer);
    vkCmdDrawIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                           drawParameterBufferRegionIndex->buffer(),
                           drawParameterBufferRegionIndex->offset(),
                           countBufferRegionIndex->buffer(),
                           countBufferRegionIndex->offset(),
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
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::GraphicsPipeline::bind(device,
                                                     commandBuffer,
                                                     drawInfo->pipeline,
                                                     drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    const luna::BufferRegionIndex *countBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->countBuffer);
    vkCmdDrawIndexedIndirectCount(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                  drawParameterBufferRegionIndex->buffer(),
                                  drawParameterBufferRegionIndex->offset(),
                                  countBufferRegionIndex->buffer(),
                                  countBufferRegionIndex->offset(),
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
    assert(capabilities.minImageExtent.width <= luna::swapchain.extent.width &&
           luna::swapchain.extent.width <= capabilities.maxImageExtent.width);
    assert(capabilities.minImageExtent.height <= luna::swapchain.extent.height &&
           luna::swapchain.extent.height <= capabilities.maxImageExtent.height);

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

VkResult lunaBeginFrame(const LunaDevice device,
                        const LunaCommandBuffer commandBuffer,
                        const bool allowSuboptimalSwapchain)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);

    const VkDevice vkDevice = static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device));
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    // TODO: If this fails it blocks the render thread, which is unacceptable, so there should be handling
    CHECK_RESULT_RETURN(commandBufferObject.waitForFence(vkDevice));
    CHECK_RESULT_RETURN(commandBufferObject.resetFence(vkDevice));
    const VkResult acquireImageResult =
            vkAcquireNextImageKHR(vkDevice,
                                  luna::swapchain.swapchain,
                                  UINT64_MAX,
                                  luna::swapchain.presentSemaphores.at(luna::swapchain.frameIndex),
                                  VK_NULL_HANDLE,
                                  &luna::swapchain.imageIndex);
    switch (acquireImageResult)
    {
        case VK_SUCCESS:
            break;
        case VK_SUBOPTIMAL_KHR:
            if (allowSuboptimalSwapchain)
            {
                break;
            }
            return acquireImageResult;
        case VK_ERROR_OUT_OF_DATE_KHR:
            return acquireImageResult;
        default:
            assert(acquireImageResult != VK_SUCCESS);
            return acquireImageResult;
    }

    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(vkDevice));

    return VK_SUCCESS;
}

VkResult lunaEndFrame(const LunaDevice device,
                      const LunaCommandBuffer commandBuffer,
                      const VkPresentInfoKHR *presentInfo,
                      const LunaCommandBufferSubmitInfo *submitInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(presentInfo);
    assert(submitInfo && submitInfo->queue != VK_NULL_HANDLE);

    static constexpr VkPipelineStageFlags2 STAGE_MASK = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    const LunaSemaphore presentSemaphore =
            luna::helpers::toHandle(luna::swapchain.presentSemaphores.at(luna::swapchain.frameIndex));
    const LunaSemaphore renderSemaphore =
            luna::helpers::toHandle(luna::swapchain.renderSemaphores.at(luna::swapchain.imageIndex));

    std::vector<LunaSemaphore> submissionWaitSemaphores{presentSemaphore};
    std::vector<VkPipelineStageFlags2> submissionWaitDstStageMasks{STAGE_MASK};
    if (submitInfo->waitSemaphoreCount > 0)
    {
        submissionWaitSemaphores.insert(submissionWaitSemaphores.end(),
                                        submitInfo->waitSemaphores,
                                        submitInfo->waitSemaphores + submitInfo->waitSemaphoreCount);
        submissionWaitDstStageMasks.insert(submissionWaitDstStageMasks.end(),
                                           submitInfo->waitDstStageMasks,
                                           submitInfo->waitDstStageMasks + submitInfo->waitSemaphoreCount);
    }
    std::vector<LunaSemaphore> signalSemaphores{renderSemaphore};
    if (submitInfo->signalSemaphoreCount > 0)
    {
        signalSemaphores.insert(signalSemaphores.end(),
                                submitInfo->signalSemaphores,
                                submitInfo->signalSemaphores + submitInfo->signalSemaphoreCount);
    }

    const LunaCommandBufferSubmitInfo finalSubmitInfo = {
        .queue = submitInfo->queue,
        .stageMask = submitInfo->stageMask,
        .waitSemaphoreCount = static_cast<uint32_t>(submissionWaitSemaphores.size()),
        .waitSemaphores = submissionWaitSemaphores.data(),
        .waitDstStageMasks = submissionWaitDstStageMasks.data(),
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .signalSemaphores = signalSemaphores.data(),
    };
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer)
                                ->endAndSubmit(static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
                                               finalSubmitInfo));

    std::vector<VkSemaphore> presentationWaitSemaphores{
        luna::swapchain.renderSemaphores.at(luna::swapchain.imageIndex)};
    if (presentInfo->waitSemaphoreCount > 0)
    {
        presentationWaitSemaphores.insert(presentationWaitSemaphores.end(),
                                          presentInfo->pWaitSemaphores,
                                          presentInfo->pWaitSemaphores + presentInfo->waitSemaphoreCount);
    }

    ++luna::swapchain.frameIndex;
    luna::swapchain.frameIndex %= luna::Swapchain::FRAMES_IN_FLIGHT;

    const VkPresentInfoKHR finalPresentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = presentInfo->pNext,
        .waitSemaphoreCount = static_cast<uint32_t>(presentationWaitSemaphores.size()),
        .pWaitSemaphores = presentationWaitSemaphores.data(),
        .swapchainCount = presentInfo->swapchainCount,
        .pSwapchains = presentInfo->pSwapchains,
        .pImageIndices = presentInfo->pImageIndices,
        .pResults = presentInfo->pResults,
    };
    // TODO: Handling the result like this doesn't ever call the CHECK_RESULT_RETURN macro
    return vkQueuePresentKHR(submitInfo->queue, &finalPresentInfo);
}
