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
    core::swapChain.extent = capabilities.currentExtent;
    assert(capabilities.minImageExtent.width <= core::swapChain.extent.width &&
           core::swapChain.extent.width <= capabilities.maxImageExtent.width);
    assert(capabilities.minImageExtent.height <= core::swapChain.extent.height &&
           core::swapChain.extent.height <= capabilities.maxImageExtent.height);

    const VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = core::swapChain.surface,
        .minImageCount = core::swapChain.imageCount,
        .imageFormat = core::swapChain.format.format,
        .imageColorSpace = core::swapChain.format.colorSpace,
        .imageExtent = core::swapChain.extent,
        .imageArrayLayers = 1,
        .imageUsage = core::swapChain.imageUsage,
        .imageSharingMode = core::device.sharingMode(),
        .queueFamilyIndexCount = core::device.familyCount(),
        .pQueueFamilyIndices = core::device.queueFamilyIndices(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = core::swapChain.compositeAlpha,
        .presentMode = core::swapChain.presentMode,
        .clipped = VK_TRUE,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(core::device, &createInfo, nullptr, &core::swapChain.swapChain));

    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(core::device,
                                                core::swapChain.swapChain,
                                                &core::swapChain.imageCount,
                                                nullptr));

    core::swapChain.images.resize(core::swapChain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(core::device,
                                                core::swapChain.swapChain,
                                                &core::swapChain.imageCount,
                                                core::swapChain.images.data()));

    core::swapChain.imageViews.resize(core::swapChain.imageCount);
    for (uint32_t i = 0; i < core::swapChain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(core::device,
                                            core::swapChain.images[i],
                                            core::swapChain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &core::swapChain.imageViews[i]));
    }
    core::swapChain.imageIndex = -1u;
    return VK_SUCCESS;
}
} // namespace luna::helpers

VkResult lunaResizeSwapchain(const uint32_t renderPassResizeInfoCount,
                             const LunaRenderPassResizeInfo *renderPassResizeInfos,
                             VkExtent2D *newSwapchainExtent)
{
    using namespace luna::core;

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, swapChain.surface, &capabilities));
    if (capabilities.currentExtent.width == UINT32_MAX || capabilities.currentExtent.height == UINT32_MAX)
    {
        return VK_SUCCESS;
    }

    const CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer();
    const auto &commandBufferArray = dynamic_cast<const commandBuffer::CommandBufferArray<4> &>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferArray.waitForAllFences(device));
    // vkDestroySemaphore(device, device.renderFinishedSemaphore(swapChain.lastImageIndex), nullptr);
    for (uint32_t i = 0; i < swapChain.imageCount; i++)
    {
        vkDestroyImageView(device, swapChain.imageViews.at(i), nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain.swapChain, nullptr);


    CHECK_RESULT_RETURN(luna::helpers::recreateSwapchain(capabilities));
    for (uint32_t i = 0; i < renderPassResizeInfoCount; i++)
    {
        const LunaRenderPassResizeInfo &renderPassResizeInfo = renderPassResizeInfos[i];
        const uint32_t width = renderPassResizeInfo.width == -1u ? swapChain.extent.width : renderPassResizeInfo.width;
        const uint32_t height = renderPassResizeInfo.height == -1u ? swapChain.extent.height
                                                                   : renderPassResizeInfo.height;
        CHECK_RESULT_RETURN(renderPass(renderPassResizeInfo.renderPass)
                                    .recreateFramebuffer(device, swapChain, width, height));
    }

    *newSwapchainExtent = swapChain.extent;

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
        .pSignalSemaphores = &device.renderFinishedSemaphore(swapChain.imageIndex),
    };
    CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(device.familyQueues().graphics, queueSubmitInfo));

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &device.renderFinishedSemaphore(swapChain.imageIndex),
        .swapchainCount = 1,
        .pSwapchains = &swapChain.swapChain,
        .pImageIndices = &swapChain.imageIndex,
    };
    // TODO: This...
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

    swapChain.imageIndex = -1u;
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
    descriptorPools.reserve(descriptorPools.size() + 1);
    const std::vector<VkDescriptorPool>::iterator poolIterator = std::find(descriptorPools.begin(),
                                                                           descriptorPools.end(),
                                                                           VK_NULL_HANDLE);
    descriptorPools.emplace(poolIterator);
    const VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = creationInfo->flags,
        .maxSets = creationInfo->maxSets,
        .poolSizeCount = creationInfo->poolSizeCount,
        .pPoolSizes = creationInfo->poolSizes,
    };
    CHECK_RESULT_RETURN(vkCreateDescriptorPool(device, &createInfo, nullptr, poolIterator.base()));
    descriptorPoolIndices.emplace_back(poolIterator - descriptorPools.begin());
    if (descriptorPool != nullptr)
    {
        *descriptorPool = &descriptorPoolIndices.back();
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
        uint32_t slotsFound = 0;
        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(allocationInfo->descriptorSetCount);
        const auto *poolIndex = static_cast<const DescriptorPoolIndex *>(allocationInfo->descriptorPool);
        for (uint32_t i = 0; i < allocationInfo->descriptorSetCount; i++)
        {
            const VkDescriptorSetLayout layout = descriptorSetLayout(allocationInfo->setLayouts[i]);
            const auto *layoutIndex = static_cast<const DescriptorSetLayoutIndex *>(allocationInfo->setLayouts[i]);
            layouts.emplace_back(layout);

            if (slotsFound == i)
            {
                const std::vector<VkDescriptorSet>::iterator descriptorSetBegin = luna::core::descriptorSets.begin();
                const std::vector<VkDescriptorSet>::iterator descriptorSetEnd = luna::core::descriptorSets.end();
                const std::vector<VkDescriptorSet>::iterator descriptorSetIterator = std::find(descriptorSetBegin,
                                                                                               descriptorSetEnd,
                                                                                               VK_NULL_HANDLE);
                if (descriptorSetIterator != descriptorSetEnd)
                {
                    descriptorSetIndices.emplace_back(descriptorSetIterator - descriptorSetBegin,
                                                      layoutIndex,
                                                      poolIndex);
                    descriptorSets[i] = &descriptorSetIndices.back();

                    const VkDescriptorSetAllocateInfo allocateInfo = {
                        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                        .descriptorPool = descriptorPool(allocationInfo->descriptorPool),
                        .descriptorSetCount = 1,
                        .pSetLayouts = &layout,
                    };
                    CHECK_RESULT_RETURN(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSetIterator.base()));
                    slotsFound++;
                    continue;
                }
            }
            descriptorSetIndices.emplace_back(luna::core::descriptorSets.size() + i - slotsFound,
                                              layoutIndex,
                                              poolIndex);
            descriptorSets[i] = &descriptorSetIndices.back();
        }
        const VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool(allocationInfo->descriptorPool),
            .descriptorSetCount = allocationInfo->descriptorSetCount - slotsFound,
            .pSetLayouts = layouts.data() + slotsFound,
        };
        const size_t oldSize = luna::core::descriptorSets.size();
        luna::core::descriptorSets.resize(oldSize + allocationInfo->descriptorSetCount - slotsFound);
        CHECK_RESULT_RETURN(vkAllocateDescriptorSets(device,
                                                     &allocateInfo,
                                                     luna::core::descriptorSets.data() + oldSize));
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
        const auto &[descriptorSetIndex,
                     bindingName,
                     descriptorArrayElement,
                     descriptorCount,
                     imageInfo,
                     bufferInfo] = descriptorWrites[i];
        DescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        luna::core::descriptorSet(descriptorSetIndex, nullptr, &descriptorSetLayout, &descriptorSet);
        const DescriptorSetLayout::Binding &binding = descriptorSetLayout.binding(bindingName);
        if (imageInfo != nullptr)
        {
            Image &image = images.at(static_cast<const ImageIndex *>(imageInfo->image)->index);
            descriptorImageInfo = {
                .sampler = image.sampler(imageInfo->sampler),
                .imageView = image.imageView(),
                .imageLayout = imageInfo->imageLayout,
            };
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                descriptorSet,
                                binding.index,
                                descriptorArrayElement,
                                descriptorCount,
                                binding.type,
                                &descriptorImageInfo,
                                nullptr,
                                nullptr);
        } else if (bufferInfo != nullptr)
        {
            const buffer::BufferRegion &bufferRegion = luna::core::bufferRegion(bufferInfo->buffer);
            const auto *bufferRegionIndex = static_cast<const buffer::BufferRegionIndex *>(bufferInfo->buffer);
            const VkDescriptorBufferInfo descriptorBufferInfo = {
                .buffer = buffers.at(bufferRegionIndex->bufferIndex),
                .offset = bufferInfo->offset + bufferRegion.offset(bufferRegionIndex->subRegion),
                .range = bufferInfo->range == 0 ? bufferRegion.size() : bufferInfo->range,
            };
            writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                nullptr,
                                descriptorSet,
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
