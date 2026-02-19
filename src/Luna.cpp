//
// Created by NBT22 on 3/1/25.
//

#include <array>
#include <cassert>
#include <cstdint>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "DescriptorSetLayout.hpp"
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "Semaphore.hpp"

#define VMA_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <volk.h>

namespace luna::helpers
{
static VkResult recreateSwapchain(const VkSurfaceCapabilitiesKHR &capabilities)
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
        .imageSharingMode = device.sharingMode(),
        .queueFamilyIndexCount = device.familyCount(),
        .pQueueFamilyIndices = device.queueFamilyIndices(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = swapchain.compositeAlpha,
        .presentMode = swapchain.presentMode,
        .clipped = VK_TRUE,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(luna::device, &createInfo, nullptr, &luna::swapchain.swapchain));

    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(luna::device,
                                                luna::swapchain.swapchain,
                                                &luna::swapchain.imageCount,
                                                nullptr));

    swapchain.images.resize(swapchain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(luna::device,
                                                luna::swapchain.swapchain,
                                                &luna::swapchain.imageCount,
                                                luna::swapchain.images.data()));

    swapchain.imageViews.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(luna::device,
                                            luna::swapchain.images[i],
                                            luna::swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &luna::swapchain.imageViews[i]));
    }
    assert(capabilities.minImageCount <= luna::swapchain.imageCount &&
           luna::swapchain.imageCount <= capabilities.maxImageCount);
    CHECK_RESULT_RETURN(luna::device.createSemaphores(luna::swapchain.imageCount));

    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    CHECK_RESULT_RETURN(
            luna::device.commandPools().graphics->commandBuffer().resizeArray(luna::device,
                                                                              *luna::device.commandPools().graphics,
                                                                              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                              nullptr,
                                                                              &semaphoreCreateInfo,
                                                                              luna::swapchain.imageCount));

    swapchain.imageIndex = -1u;
    return VK_SUCCESS;
}
} // namespace luna::helpers

VkResult lunaResizeSwapchain(const uint32_t renderPassResizeInfoCount,
                             const LunaRenderPassResizeInfo *renderPassResizeInfos,
                             const VkExtent2D *targetExtent,
                             VkExtent2D *newSwapchainExtent)
{
    using namespace luna;

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, swapchain.surface, &capabilities));
    // TODO: Check platform compatability because this is only known to work on Wayland
    capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;
    swapchain.safeToUse.wait(false);
    swapchain.safeToUse = false;
    if (targetExtent != nullptr)
    {
        assert(capabilities.minImageExtent.width <= targetExtent->width &&
               targetExtent->width <= capabilities.maxImageExtent.width);
        assert(capabilities.minImageExtent.height <= targetExtent->height &&
               targetExtent->height <= capabilities.maxImageExtent.height);
        swapchain.extent = *targetExtent;
    } else
    {
        swapchain.extent = capabilities.currentExtent;
    }
    assert(capabilities.minImageExtent.width <= swapchain.extent.width &&
           swapchain.extent.width <= capabilities.maxImageExtent.width);
    assert(capabilities.minImageExtent.height <= swapchain.extent.height &&
           swapchain.extent.height <= capabilities.maxImageExtent.height);

    CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(commandBuffer.waitForAllFences(device));
    CHECK_RESULT_RETURN(commandBuffer.recreateSemaphores(device));
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        vkDestroyImageView(device, swapchain.imageViews.at(i), nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);

    CHECK_RESULT_RETURN(luna::helpers::recreateSwapchain(capabilities));
    for (uint32_t i = 0; i < renderPassResizeInfoCount; i++)
    {
        const LunaRenderPassResizeInfo &renderPassResizeInfo = renderPassResizeInfos[i];
        const uint32_t width = renderPassResizeInfo.width == LUNA_RENDER_PASS_WIDTH_SWAPCHAIN_WIDTH
                                       ? swapchain.extent.width
                                       : renderPassResizeInfo.width;
        const uint32_t height = renderPassResizeInfo.height == LUNA_RENDER_PASS_HEIGHT_SWAPCHAIN_HEIGHT
                                        ? swapchain.extent.height
                                        : renderPassResizeInfo.height;
        CHECK_RESULT_RETURN(helpers::fromHandle<RenderPass>(renderPassResizeInfo.renderPass)
                                    ->recreateFramebuffer(device, swapchain, width, height));
    }
    swapchain.safeToUse = true;
    swapchain.safeToUse.notify_all();

    if (newSwapchainExtent != nullptr)
    {
        *newSwapchainExtent = swapchain.extent;
    }

    return VK_SUCCESS;
}

VkResult lunaBeginFrame(const bool allowSuboptimalSwapchain)
{
    luna::CommandBuffer &commandBuffer = luna::device.commandPools().graphics->commandBuffer();
    // TODO: If this fails it blocks the render thread, which is unacceptable, so there should be handling
    CHECK_RESULT_RETURN(commandBuffer.waitForFence(luna::device));
    CHECK_RESULT_RETURN(commandBuffer.resetFence(luna::device));
    const VkResult acquireImageResult = vkAcquireNextImageKHR(luna::device,
                                                              luna::swapchain.swapchain,
                                                              UINT64_MAX,
                                                              commandBuffer.semaphore(),
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

    CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());

    return VK_SUCCESS;
}

VkResult lunaEndFrame()
{
    using namespace luna;
    CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer();
    assert(commandBuffer.isRecording());

    const Semaphore &secondaryGraphicsSemaphore = device.commandPools().graphics->commandBuffer(1).semaphore();
    const std::array<VkSemaphore, 2> waitSemaphores = {commandBuffer.semaphore(), secondaryGraphicsSemaphore};
    const std::array<VkPipelineStageFlags, 2> waitStageMasks = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                                secondaryGraphicsSemaphore.stageMask()};
    const VkSubmitInfo queueSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = device.commandPools().graphics->commandBuffer(1).getAndSetIsSignaled(false) ? 2u : 1u,
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStageMasks.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &device.renderFinishedSemaphore(swapchain.imageIndex),
    };
    CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(device.familyQueues().graphics, queueSubmitInfo));

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &device.renderFinishedSemaphore(swapchain.imageIndex),
        .swapchainCount = 1,
        .pSwapchains = &swapchain.swapchain,
        .pImageIndices = &swapchain.imageIndex,
    };
    const VkResult presentationResult = vkQueuePresentKHR(device.familyQueues().presentation, &presentInfo);
    switch (presentationResult)
    {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
            break;
        default:
            return presentationResult;
    }

    swapchain.imageIndex = -1u;
    boundPipeline = VK_NULL_HANDLE;
    boundVertexBuffer = LUNA_NULL_HANDLE;
    boundIndexBuffer = LUNA_NULL_HANDLE;
    return presentationResult;
}

VkResult lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo,
                                  LunaDescriptorPool *descriptorPool)
{
    using namespace luna;
    assert(creationInfo);
    descriptorPools.emplace_back();
    const VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = creationInfo->flags,
        .maxSets = creationInfo->maxSets,
        .poolSizeCount = creationInfo->poolSizeCount,
        .pPoolSizes = creationInfo->poolSizes,
    };
    CHECK_RESULT_RETURN(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPools.back()));
    if (descriptorPool != nullptr)
    {
        *descriptorPool = helpers::toHandle(&descriptorPools.back());
    }
    return VK_SUCCESS;
}

VkResult lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
                                    LunaDescriptorSet *descriptorSets)
{
    using namespace luna;
    assert(allocationInfo);
    if (allocationInfo->descriptorSetCount != 0)
    {
        assert(allocationInfo->setLayouts);
        const VkDescriptorPool *pool = helpers::fromHandle<VkDescriptorPool>(allocationInfo->descriptorPool);
        for (uint32_t i = 0; i < allocationInfo->descriptorSetCount; i++)
        {
            const DescriptorSetLayout *layout = helpers::fromHandle<DescriptorSetLayout>(allocationInfo->setLayouts[i]);
            const VkDescriptorSetLayout vkLayout = *layout;

            luna::descriptorSets.emplace_back();
            VkDescriptorSet *descriptorSet = &luna::descriptorSets.back();
            const VkDescriptorSetAllocateInfo allocateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = *pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &vkLayout,
            };
            CHECK_RESULT_RETURN(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSet));
            descriptorSetIndices.emplace_back(pool, layout, descriptorSet);
            descriptorSets[i] = helpers::toHandle(&descriptorSetIndices.back());
        }
    }
    return VK_SUCCESS;
}

void lunaWriteDescriptorSets(const uint32_t descriptorWriteCount, const LunaWriteDescriptorSet *descriptorWrites)
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
                                descriptorWrite.descriptorCount,
                                binding.type,
                                &descriptorImageInfos.back(),
                                nullptr,
                                nullptr);
        } else if (descriptorWrite.bufferInfo != nullptr)
        {
            const LunaBuffer buffer = descriptorWrite.bufferInfo->buffer;
            const BufferRegionIndex *bufferRegionIndex = luna::helpers::fromHandle<BufferRegionIndex>(buffer);
            descriptorBufferInfos.emplace_back(bufferRegionIndex->buffer(),
                                               descriptorWrite.bufferInfo->offset + bufferRegionIndex->offset(),
                                               descriptorWrite.bufferInfo->range == 0
                                                       ? bufferRegionIndex->size()
                                                       : descriptorWrite.bufferInfo->range);
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                descriptorWrite.descriptorCount,
                                binding.type,
                                nullptr,
                                &descriptorBufferInfos.back(),
                                nullptr);
        }
    }
    vkUpdateDescriptorSets(device, descriptorWriteCount, writes.data(), 0, nullptr);
}

void lunaWriteFramebufferToDescriptor(const LunaDescriptorSet descriptorSet)
{
    vkDeviceWaitIdle(luna::device);

    const VkDescriptorImageInfo imageInfo = {
        .imageView = luna::swapchain.imageViews.at(luna::swapchain.imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };
    const VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = *luna::helpers::fromHandle<luna::DescriptorSetIndex>(descriptorSet)->set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imageInfo,
    };
    vkUpdateDescriptorSets(luna::device, 1, &write, 0, nullptr);
}

VkResult lunaDraw(const LunaDrawInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(luna::device.commandPools().graphics->commandBuffer().isRecording()); // Internal state check
    vkCmdDraw(luna::device.commandPools().graphics->commandBuffer(),
              drawInfo->vertexCount,
              drawInfo->instanceCount,
              drawInfo->firstVertex,
              drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawIndirect(const LunaDrawIndirectInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(luna::device.commandPools().graphics->commandBuffer().isRecording()); // Internal state check
    const luna::BufferRegionIndex *bufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndirect(luna::device.commandPools().graphics->commandBuffer(),
                      bufferRegionIndex->buffer(),
                      bufferRegionIndex->offset(),
                      drawInfo->drawCount,
                      drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawIndirectCount(const LunaDrawIndirectCountInfo *drawInfo)
{
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    const luna::BufferRegionIndex *countBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->countBuffer);
    assert(luna::device.commandPools().graphics->commandBuffer().isRecording()); // Internal state check
    vkCmdDrawIndirectCount(luna::device.commandPools().graphics->commandBuffer(),
                           drawParameterBufferRegionIndex->buffer(),
                           drawParameterBufferRegionIndex->offset(),
                           countBufferRegionIndex->buffer(),
                           countBufferRegionIndex->offset(),
                           drawInfo->maxDrawCount,
                           drawInfo->stride == 0 ? sizeof(VkDrawIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawIndexed(const LunaDrawIndexedInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(luna::device.commandPools().graphics->commandBuffer().isRecording()); // Internal state check
    vkCmdDrawIndexed(luna::device.commandPools().graphics->commandBuffer(),
                     drawInfo->indexCount,
                     drawInfo->instanceCount,
                     drawInfo->firstIndex,
                     drawInfo->vertexOffset,
                     drawInfo->firstInstance);
    return VK_SUCCESS;
}

VkResult lunaDrawIndexedIndirect(const LunaDrawIndexedIndirectInfo *drawInfo)
{
    assert(drawInfo && drawInfo->pipeline != LUNA_NULL_HANDLE && drawInfo->buffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(luna::device.commandPools().graphics->commandBuffer().isRecording()); // Internal state check
    const luna::BufferRegionIndex *bufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    vkCmdDrawIndexedIndirect(luna::device.commandPools().graphics->commandBuffer(),
                             bufferRegionIndex->buffer(),
                             bufferRegionIndex->offset(),
                             drawInfo->drawCount,
                             drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}

VkResult lunaDrawIndexedIndirectCount(const LunaDrawIndexedIndirectCountInfo *drawInfo)
{
    assert(drawInfo &&
           drawInfo->pipeline != LUNA_NULL_HANDLE &&
           drawInfo->buffer != LUNA_NULL_HANDLE &&
           drawInfo->countBuffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(drawInfo->pipeline)
                                ->bind(drawInfo->pipelineBindInfo == nullptr ? LunaGraphicsPipelineBindInfo{}
                                                                             : *drawInfo->pipelineBindInfo));
    assert(luna::device.commandPools().graphics->commandBuffer().isRecording()); // Internal state check
    const luna::BufferRegionIndex *drawParameterBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->buffer);
    const luna::BufferRegionIndex *countBufferRegionIndex =
            luna::helpers::fromHandle<luna::BufferRegionIndex>(drawInfo->countBuffer);
    vkCmdDrawIndexedIndirectCount(luna::device.commandPools().graphics->commandBuffer(),
                                  drawParameterBufferRegionIndex->buffer(),
                                  drawParameterBufferRegionIndex->offset(),
                                  countBufferRegionIndex->buffer(),
                                  countBufferRegionIndex->offset(),
                                  drawInfo->maxDrawCount,
                                  drawInfo->stride == 0 ? sizeof(VkDrawIndexedIndirectCommand) : drawInfo->stride);
    return VK_SUCCESS;
}
