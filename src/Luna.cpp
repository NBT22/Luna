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
#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "GraphicsPipeline.hpp"
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"
#include "Semaphore.hpp"

namespace luna::helpers
{
static VkResult recreateSwapchain(Device &device, const VkSurfaceCapabilitiesKHR &capabilities)
{
    const VkDevice vkDevice = static_cast<VkDevice>(device);
    const VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = swapchain.surface,
        .minImageCount = capabilities.minImageCount,
        .imageFormat = swapchain.format.format,
        .imageColorSpace = swapchain.format.colorSpace,
        .imageExtent = swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = swapchain.imageUsage,
        .imageSharingMode = device.sharingMode(),
        .queueFamilyIndexCount = device.familyCount(),
        .pQueueFamilyIndices = device.queueFamilyIndices(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = swapchain.compositeAlpha,
        .presentMode = swapchain.presentMode,
        .clipped = VK_TRUE,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &luna::swapchain.swapchain));

    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(vkDevice,
                                                luna::swapchain.swapchain,
                                                &luna::swapchain.imageCount,
                                                nullptr));

    swapchain.images.resize(swapchain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(vkDevice,
                                                luna::swapchain.swapchain,
                                                &luna::swapchain.imageCount,
                                                luna::swapchain.images.data()));

    swapchain.imageViews.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(vkDevice,
                                            luna::swapchain.images.at(i),
                                            luna::swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &luna::swapchain.imageViews.at(i)));
    }
    assert(capabilities.minImageCount <= luna::swapchain.imageCount &&
           luna::swapchain.imageCount <= capabilities.maxImageCount);
    CHECK_RESULT_RETURN(device.createSemaphores(luna::swapchain.imageCount));

    CHECK_RESULT_RETURN(device.commandPools().graphics->commandBuffer().resizeArray(vkDevice,
                                                                                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                                    luna::swapchain.imageCount));

    swapchain.imageIndex = -1u;
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
                                              VK_QUEUE_FAMILY_IGNORED,
                                              VK_QUEUE_FAMILY_IGNORED,
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
                                             VK_QUEUE_FAMILY_IGNORED,
                                             VK_QUEUE_FAMILY_IGNORED,
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
                                              VK_QUEUE_FAMILY_IGNORED,
                                              VK_QUEUE_FAMILY_IGNORED,
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
                                             VK_QUEUE_FAMILY_IGNORED,
                                             VK_QUEUE_FAMILY_IGNORED,
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
                                                                                    luna::Device>(device)),
                                                              true));
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

VkResult lunaResizeSwapchain(const LunaDevice device,
                             const LunaCommandBuffer commandBuffer,
                             const uint32_t renderPassResizeInfoCount,
                             const LunaRenderPassResizeInfo *renderPassResizeInfos,
                             const VkExtent2D *targetExtent,
                             VkExtent2D *newSwapchainExtent)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);

    luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    const VkDevice vkDevice = static_cast<VkDevice>(deviceObject);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(static_cast<VkPhysicalDevice>(deviceObject),
                                                                  luna::swapchain.surface,
                                                                  &capabilities));
    capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;
    luna::swapchain.safeToUse.wait(false);
    luna::swapchain.safeToUse = false;
    if (targetExtent != nullptr)
    {
        assert(capabilities.minImageExtent.width <= targetExtent->width &&
               targetExtent->width <= capabilities.maxImageExtent.width);
        assert(capabilities.minImageExtent.height <= targetExtent->height &&
               targetExtent->height <= capabilities.maxImageExtent.height);
        luna::swapchain.extent = *targetExtent;
    } else
    {
        luna::swapchain.extent = capabilities.currentExtent;
    }
    assert(capabilities.minImageExtent.width <= luna::swapchain.extent.width &&
           luna::swapchain.extent.width <= capabilities.maxImageExtent.width);
    assert(capabilities.minImageExtent.height <= luna::swapchain.extent.height &&
           luna::swapchain.extent.height <= capabilities.maxImageExtent.height);

    CHECK_RESULT_RETURN(commandBufferObject.waitForAllFences(vkDevice));
    CHECK_RESULT_RETURN(commandBufferObject.recreateSemaphores(vkDevice));
    for (uint32_t i = 0; i < luna::swapchain.imageCount; i++)
    {
        vkDestroyImageView(vkDevice, luna::swapchain.imageViews.at(i), nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, luna::swapchain.swapchain, nullptr);

    CHECK_RESULT_RETURN(luna::helpers::recreateSwapchain(deviceObject, capabilities));
    for (uint32_t i = 0; i < renderPassResizeInfoCount; i++)
    {
        const LunaRenderPassResizeInfo &renderPassResizeInfo = renderPassResizeInfos[i];
        const uint32_t width = renderPassResizeInfo.width == LUNA_RENDER_PASS_WIDTH_SWAPCHAIN_WIDTH
                                       ? luna::swapchain.extent.width
                                       : renderPassResizeInfo.width;
        const uint32_t height = renderPassResizeInfo.height == LUNA_RENDER_PASS_HEIGHT_SWAPCHAIN_HEIGHT
                                        ? luna::swapchain.extent.height
                                        : renderPassResizeInfo.height;
        CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::RenderPass>(renderPassResizeInfo.renderPass)
                                    ->recreateFramebuffer(vkDevice,
                                                          deviceObject.familyCount(),
                                                          deviceObject.queueFamilyIndices(),
                                                          deviceObject.allocator(),
                                                          width,
                                                          height));
    }
    luna::swapchain.safeToUse = true;
    luna::swapchain.safeToUse.notify_all();

    if (newSwapchainExtent != nullptr)
    {
        *newSwapchainExtent = luna::swapchain.extent;
    }

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
    const VkResult acquireImageResult = vkAcquireNextImageKHR(vkDevice,
                                                              luna::swapchain.swapchain,
                                                              UINT64_MAX,
                                                              commandBufferObject.semaphore(),
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

    CHECK_RESULT_RETURN(commandBufferObject.beginSingleUseCommandBuffer());

    return VK_SUCCESS;
}

VkResult lunaEndFrame(const LunaDevice device)
{
    assert(device != LUNA_NULL_HANDLE);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::CommandBuffer &commandBuffer = deviceObject.commandPools().graphics->commandBuffer();
    assert(commandBuffer.isRecording());

    const luna::Semaphore &secondaryGraphicsSemaphore =
            deviceObject.commandPools().graphics->commandBuffer(1).semaphore();
    const std::array<VkSemaphore, 2> waitSemaphores = {commandBuffer.semaphore(), secondaryGraphicsSemaphore};
    const std::array<VkPipelineStageFlags, 2> waitStageMasks = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                                secondaryGraphicsSemaphore.stageMask()};
    const VkSubmitInfo queueSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = deviceObject.commandPools().graphics->commandBuffer(1).getAndSetIsSignaled(false) ? 2u
                                                                                                                : 1u,
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStageMasks.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &static_cast<const VkCommandBuffer &>(commandBuffer),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &deviceObject.renderFinishedSemaphore(luna::swapchain.imageIndex),
    };
    CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(deviceObject.familyQueues().graphics, queueSubmitInfo));

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &deviceObject.renderFinishedSemaphore(luna::swapchain.imageIndex),
        .swapchainCount = 1,
        .pSwapchains = &luna::swapchain.swapchain,
        .pImageIndices = &luna::swapchain.imageIndex,
    };
    const VkResult presentationResult = vkQueuePresentKHR(deviceObject.familyQueues().presentation, &presentInfo);
    switch (presentationResult)
    {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
            break;
        default:
            return presentationResult;
    }

    luna::swapchain.imageIndex = -1u;
    return presentationResult;
}
