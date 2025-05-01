//
// Created by NBT22 on 3/1/25.
//

#include <luna/core/Instance.hpp>
#include <luna/core/Luna.hpp>
#include <luna/luna.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VkResult lunaCreateShaderModule(const uint32_t *spirv, const size_t bytes, VkShaderModule *shaderModule)
{
    const VkShaderModuleCreateInfo creationInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = bytes,
        .pCode = spirv,
    };
    CHECK_RESULT_RETURN(luna::core::device.addShaderModule(&creationInfo, shaderModule));
    return VK_SUCCESS;
}
VkResult lunaPresentSwapChain()
{
    using namespace luna::core;
    CommandBuffer &commandBuffer = device.commandBuffers().graphics;
    assert(commandBuffer.isRecording());

    const VkSemaphore &renderFinishedSemaphore = device.renderFinishedSemaphore();
    constexpr VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    const VkSubmitInfo queueSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &device.imageAvailableSemaphore(),
        .pWaitDstStageMask = &waitStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer.commandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderFinishedSemaphore,
    };
    CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(device.familyQueues().graphics, queueSubmitInfo));

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapChain.swapChain,
        .pImageIndices = &swapChain.imageIndex,
    };
    // TODO: This...
    vkQueuePresentKHR(device.familyQueues().presentation, &presentInfo);

    swapChain.imageIndex = -1u;
    boundPipeline = VK_NULL_HANDLE;
    boundVertexBuffer = VK_NULL_HANDLE;
    boundIndexBuffer = VK_NULL_HANDLE;
    return VK_SUCCESS;
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
    CHECK_RESULT_RETURN(vkCreateDescriptorPool(device.logicalDevice(), &createInfo, nullptr, poolIterator.base()));
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
            const VkDescriptorSetLayout layout = descriptorSetLayout(allocationInfo->setLayouts[i]).layout();
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
                    CHECK_RESULT_RETURN(vkAllocateDescriptorSets(device.logicalDevice(),
                                                                 &allocateInfo,
                                                                 descriptorSetIterator.base()));
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
        CHECK_RESULT_RETURN(vkAllocateDescriptorSets(device.logicalDevice(),
                                                     &allocateInfo,
                                                     luna::core::descriptorSets.data() + oldSize));
    }
    return VK_SUCCESS;
}
void lunaWriteDescriptorSets(const uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites)
{
    using namespace luna::core;
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
            const VkDescriptorImageInfo descriptorImageInfo = {
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
                .buffer = buffers.at(bufferRegionIndex->bufferIndex).buffer(),
                .offset = bufferInfo->offset + bufferRegion.offset(),
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
    vkUpdateDescriptorSets(device.logicalDevice(), writeCount, writes.data(), 0, nullptr);
}
