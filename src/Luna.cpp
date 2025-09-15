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
    CHECK_RESULT_RETURN(luna::device.commandPools()
                                .graphics.commandBuffer()
                                .resizeArray(luna::device,
                                             luna::device.commandPools().graphics,
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

    CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
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
        const uint32_t width = renderPassResizeInfo.width == -1u ? swapchain.extent.width : renderPassResizeInfo.width;
        const uint32_t height = renderPassResizeInfo.height == -1u ? swapchain.extent.height
                                                                   : renderPassResizeInfo.height;
        // NOLINTNEXTLINE(*-pro-type-const-cast)
        CHECK_RESULT_RETURN(const_cast<RenderPass *>(renderPass(renderPassResizeInfo.renderPass))
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
VkResult lunaPresentSwapchain()
{
    using namespace luna;
    CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    assert(commandBuffer.isRecording());

    const Semaphore &secondaryGraphicsSemaphore = device.commandPools().graphics.commandBuffer(1).semaphore();
    const std::array<VkSemaphore, 2> waitSemaphores = {commandBuffer.semaphore(), secondaryGraphicsSemaphore};
    const std::array<VkPipelineStageFlags, 2> waitStageMasks = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                                secondaryGraphicsSemaphore.stageMask()};
    const VkSubmitInfo queueSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = device.commandPools().graphics.commandBuffer(1).getAndSetIsSignaled(false) ? 2u : 1u,
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
        *descriptorPool = static_cast<LunaDescriptorPool>(&descriptorPools.back());
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
        const VkDescriptorPool *pool = descriptorPool(allocationInfo->descriptorPool);
        for (uint32_t i = 0; i < allocationInfo->descriptorSetCount; i++)
        {
            const DescriptorSetLayout *layout = descriptorSetLayout(allocationInfo->setLayouts[i]);
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
            descriptorSets[i] = &descriptorSetIndices.back();
        }
    }
    return VK_SUCCESS;
}
void lunaWriteDescriptorSets(const uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites)
{
    using namespace luna;
    VkDescriptorImageInfo descriptorImageInfo;
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(writeCount);
    for (uint32_t i = 0; i < writeCount; i++)
    {
        const LunaWriteDescriptorSet &descriptorWrite = descriptorWrites[i];
        const LunaDescriptorSet descriptorSet = descriptorWrite.descriptorSet;
        const DescriptorSetIndex *descriptorSetIndex = static_cast<const DescriptorSetIndex *>(descriptorSet);
        const DescriptorSetLayout::Binding &binding = descriptorSetIndex->layout->binding(descriptorWrite.bindingName);
        if (descriptorWrite.imageInfo != nullptr)
        {
            const Image *image = static_cast<const Image *>(descriptorWrite.imageInfo->image);
            descriptorImageInfo = {
                .sampler = image->sampler(descriptorWrite.imageInfo->sampler),
                .imageView = image->imageView(),
                .imageLayout = descriptorWrite.imageInfo->imageLayout,
            };
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                descriptorWrite.descriptorCount,
                                binding.type,
                                &descriptorImageInfo,
                                nullptr,
                                nullptr);
        } else if (descriptorWrite.bufferInfo != nullptr)
        {
            const LunaBuffer buffer = descriptorWrite.bufferInfo->buffer;
            const BufferRegionIndex *bufferRegionIndex = static_cast<const BufferRegionIndex *>(buffer);
            const VkDescriptorBufferInfo descriptorBufferInfo = {
                .buffer = *bufferRegionIndex->buffer(),
                .offset = descriptorWrite.bufferInfo->offset + bufferRegionIndex->offset(),
                .range = descriptorWrite.bufferInfo->range == 0 ? bufferRegionIndex->size()
                                                                : descriptorWrite.bufferInfo->range,
            };
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorWrite.descriptorArrayElement,
                                descriptorWrite.descriptorCount,
                                binding.type,
                                nullptr,
                                &descriptorBufferInfo,
                                nullptr);
        }
    }
    vkUpdateDescriptorSets(device, writeCount, writes.data(), 0, nullptr);
}
void lunaDestroyDescriptorSet(LunaDescriptorSet descriptorSet)
{
    (void)descriptorSet;
}
