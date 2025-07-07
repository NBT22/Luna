//
// Created by NBT22 on 3/1/25.
//

#include <array>
#include <luna/core/Instance.hpp>
#include <luna/core/Luna.hpp>
#include <luna/luna.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace luna::helpers
{
static VkResult recreateSwapchain(const VkSurfaceCapabilitiesKHR &capabilities)
{
    const VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = core::swapchain.surface,
        .minImageCount = capabilities.minImageCount,
        .imageFormat = core::swapchain.format.format,
        .imageColorSpace = core::swapchain.format.colorSpace,
        .imageExtent = core::swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = core::swapchain.imageUsage,
        .imageSharingMode = core::device.sharingMode(),
        .queueFamilyIndexCount = core::device.familyCount(),
        .pQueueFamilyIndices = core::device.queueFamilyIndices(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = core::swapchain.compositeAlpha,
        .presentMode = core::swapchain.presentMode,
        .clipped = VK_TRUE,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(core::device, &createInfo, nullptr, &core::swapchain.swapchain));

    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(core::device,
                                                core::swapchain.swapchain,
                                                &core::swapchain.imageCount,
                                                nullptr));

    core::swapchain.images.resize(core::swapchain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(core::device,
                                                core::swapchain.swapchain,
                                                &core::swapchain.imageCount,
                                                core::swapchain.images.data()));

    core::swapchain.imageViews.resize(core::swapchain.imageCount);
    for (uint32_t i = 0; i < core::swapchain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(core::device,
                                            core::swapchain.images[i],
                                            core::swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &core::swapchain.imageViews[i]));
    }
    assert(capabilities.minImageCount <= core::swapchain.imageCount &&
           core::swapchain.imageCount <= capabilities.maxImageCount);
    CHECK_RESULT_RETURN(core::device.createSemaphores(core::swapchain.imageCount));
    core::swapchain.imageIndex = -1u;
    return VK_SUCCESS;
}
} // namespace luna::helpers

VkResult lunaResizeSwapchain(const uint32_t renderPassResizeInfoCount,
                             const LunaRenderPassResizeInfo *renderPassResizeInfos,
                             const VkExtent2D *targetExtent,
                             VkExtent2D *newSwapchainExtent)
{
    using namespace luna::core;

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, swapchain.surface, &capabilities));
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

    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    const auto &commandBufferArray = dynamic_cast<const commandBuffer::CommandBufferArray<5> &>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferArray.waitForAllFences(device));
    CHECK_RESULT_RETURN(const_cast<commandBuffer::CommandBufferArray<5> &>(commandBufferArray)
                                .recreateSemaphores(device));
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
        CHECK_RESULT_RETURN(renderPass(renderPassResizeInfo.renderPass)
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
    using namespace luna::core;
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
    boundVertexBuffer = VK_NULL_HANDLE;
    boundIndexBuffer = VK_NULL_HANDLE;
    return presentationResult;
}
VkResult lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo,
                                  LunaDescriptorPool *descriptorPool)
{
    using namespace luna::core;
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
        *descriptorPool = &descriptorPools.back();
    }
    return VK_SUCCESS;
}
VkResult lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
                                    LunaDescriptorSet *descriptorSets)
{
    using namespace luna::core;
    assert(allocationInfo);
    if (allocationInfo->descriptorSetCount != 0)
    {
        assert(allocationInfo->setLayouts);
        const VkDescriptorPool *pool = descriptorPool(allocationInfo->descriptorPool);
        for (uint32_t i = 0; i < allocationInfo->descriptorSetCount; i++)
        {
            const DescriptorSetLayout *layout = descriptorSetLayout(allocationInfo->setLayouts[i]);
            const VkDescriptorSetLayout vkLayout = *layout;

            luna::core::descriptorSets.emplace_back();
            VkDescriptorSet *descriptorSet = &luna::core::descriptorSets.back();
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
    using namespace luna::core;
    VkDescriptorImageInfo descriptorImageInfo;
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(writeCount);
    for (uint32_t i = 0; i < writeCount; i++)
    {
        const auto &[descriptorSet,
                     bindingName,
                     descriptorArrayElement,
                     descriptorCount,
                     imageInfo,
                     bufferInfo] = descriptorWrites[i];
        const DescriptorSetIndex *descriptorSetIndex = static_cast<const DescriptorSetIndex *>(descriptorSet);
        const DescriptorSetLayout::Binding &binding = descriptorSetIndex->layout->binding(bindingName);
        if (imageInfo != nullptr)
        {
            Image *image = const_cast<Image *>(static_cast<const Image *>(imageInfo->image));
            descriptorImageInfo = {
                .sampler = image->sampler(imageInfo->sampler),
                .imageView = image->imageView(),
                .imageLayout = imageInfo->imageLayout,
            };
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorArrayElement,
                                descriptorCount,
                                binding.type,
                                &descriptorImageInfo,
                                nullptr,
                                nullptr);
        } else if (bufferInfo != nullptr)
        {
            const auto *bufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(bufferInfo->buffer);
            const VkDescriptorBufferInfo descriptorBufferInfo = {
                .buffer = *bufferRegionIndex->buffer(),
                .offset = bufferInfo->offset + bufferRegionIndex->offset(),
                .range = bufferInfo->range == 0 ? bufferRegionIndex->size() : bufferInfo->range,
            };
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                *descriptorSetIndex->set,
                                binding.index,
                                descriptorArrayElement,
                                descriptorCount,
                                binding.type,
                                nullptr,
                                &descriptorBufferInfo,
                                nullptr);
        }
    }
    vkUpdateDescriptorSets(device, writeCount, writes.data(), 0, nullptr);
}
