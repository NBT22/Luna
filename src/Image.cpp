//
// Created by NBT22 on 3/11/25.
//

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <list>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "Semaphore.hpp"

namespace luna::helpers
{
static void transitionImageLayout(const VkCommandBuffer &commandBuffer,
                                  const VkImage image,
                                  const VkAccessFlags sourceAccessMask,
                                  const VkAccessFlags destinationAccessMask,
                                  const VkImageLayout oldLayout,
                                  const VkImageLayout newLayout,
                                  const VkImageSubresourceRange &subresourceRange,
                                  const VkPipelineStageFlags sourceStageMask,
                                  const VkPipelineStageFlags destinationStageMask)
{
    const VkImageMemoryBarrier memoryBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = sourceAccessMask,
        .dstAccessMask = destinationAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = subresourceRange,
    };
    vkCmdPipelineBarrier(commandBuffer,
                         sourceStageMask,
                         destinationStageMask,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &memoryBarrier);
}
static void transitionImageLayout2(const VkCommandBuffer &commandBuffer,
                                   const VkImage image,
                                   const VkPipelineStageFlags2 sourceStageMask,
                                   const VkAccessFlags2 sourceAccessMask,
                                   const VkPipelineStageFlags2 destinationStageMask,
                                   const VkAccessFlags2 destinationAccessMask,
                                   const VkImageLayout oldLayout,
                                   const VkImageLayout newLayout,
                                   const VkImageSubresourceRange &subresourceRange)
{
    const VkImageMemoryBarrier2 memoryBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = sourceStageMask,
        .srcAccessMask = sourceAccessMask,
        .dstStageMask = destinationStageMask,
        .dstAccessMask = destinationAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = subresourceRange,
    };
    const VkDependencyInfo dependencyInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &memoryBarrier,
    };
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

static VkResult createImage(const LunaSampledImageCreationInfo &creationInfo,
                            uint32_t depth,
                            uint32_t arrayLayers,
                            LunaImage *imageIndex)
{
    TRY_CATCH_RESULT(luna::images.emplace_back(creationInfo, depth, arrayLayers));
    const Image &image = images.back();
    if (creationInfo.writeInfo.descriptorSet != nullptr)
    {
        image.updateDescriptorBinding(device,
                                      creationInfo.writeInfo.descriptorSet,
                                      creationInfo.writeInfo.descriptorLayoutBindingName);
    }

    if (imageIndex != nullptr)
    {
        *imageIndex = &images.back();
    }
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna
{
Image::Image(const LunaSampledImageCreationInfo &creationInfo, const uint32_t depth, const uint32_t arrayLayers)
{
    assert(creationInfo.sampler == LUNA_NULL_HANDLE || creationInfo.samplerCreationInfo == nullptr);
    if (creationInfo.sampler != LUNA_NULL_HANDLE)
    {
        sampler_ = sampler(creationInfo.sampler);
    } else if (creationInfo.samplerCreationInfo != nullptr)
    {
        LunaSampler sampler = LUNA_NULL_HANDLE;
        CHECK_RESULT_THROW(lunaCreateSampler(creationInfo.samplerCreationInfo, &sampler));
        sampler_ = luna::sampler(sampler);
    }
    extent_.width = creationInfo.width;
    extent_.height = creationInfo.height;
    extent_.depth = depth == 0 ? 1 : depth;
    arrayLayers_ = arrayLayers;
    const uint32_t mipmapLevels = creationInfo.writeInfo.mipmapLevels == 0 ? 1 : creationInfo.writeInfo.mipmapLevels;
    VkImageUsageFlags usage = creationInfo.usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    usage |= creationInfo.writeInfo.generateMipmaps && creationInfo.writeInfo.mipmapLevels > 1
                     ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                     : 0;
    const VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = creationInfo.flags,
        .imageType = depth == 0 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D,
        .format = creationInfo.format,
        .extent = extent_,
        .mipLevels = mipmapLevels,
        .arrayLayers = arrayLayers_,
        .samples = creationInfo.samples == 0 ? VK_SAMPLE_COUNT_1_BIT : creationInfo.samples,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = device.sharingMode(),
        .queueFamilyIndexCount = device.familyCount(),
        .pQueueFamilyIndices = device.queueFamilyIndices(),
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    constexpr VmaAllocationCreateInfo allocationCreateInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    VmaAllocationInfo allocationInfo;
    CHECK_RESULT_THROW(vmaCreateImage(device.allocator(),
                                      &imageCreateInfo,
                                      &allocationCreateInfo,
                                      &image_,
                                      &allocation_,
                                      &allocationInfo));
    layout_ = creationInfo.layout;
    aspectMask_ = creationInfo.aspectMask;
    if (aspectMask_ == 0)
    {
        if (layout_ == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            layout_ == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
        {
            aspectMask_ = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        } else
        {
            aspectMask_ = VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
    CHECK_RESULT_THROW(write(creationInfo.writeInfo));
    CHECK_RESULT_THROW(helpers::createImageView(device,
                                                image_,
                                                creationInfo.format,
                                                aspectMask_,
                                                mipmapLevels,
                                                &imageView_));
}

void Image::destroy() const
{
    vkDestroyImageView(device, imageView_, nullptr);
    vmaDestroyImage(device.allocator(), image_, allocation_);
}
void Image::erase(const std::list<Image>::const_iterator iterator) const
{
    vkDestroyImageView(device, imageView_, nullptr);
    vmaDestroyImage(device.allocator(), image_, allocation_);
    images.erase(iterator);
}

VkResult Image::write(const LunaImageWriteInfo &writeInfo) const
{
    if (writeInfo.bytes == 0 || writeInfo.pixels == nullptr)
    {
        return VK_SUCCESS;
    }
    VkExtent3D extent = writeInfo.extent == nullptr ? extent_ : *writeInfo.extent;
    if (writeInfo.extent != nullptr && extent.depth == 0)
    {
        extent.depth = 1;
    }
    CommandBuffer &commandBuffer = device.commandPools().graphics.commandBuffer(1);
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(luna::device, true));

    if (stagingBuffer == nullptr || stagingBuffer->size() < writeInfo.bytes)
    {
        if (stagingBuffer != nullptr)
        {
            lunaDestroyBuffer(stagingBuffer);
        }
        const LunaBufferCreationInfo bufferCreationInfo = {
            .size = writeInfo.bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };
        LunaBuffer stagingBufferHandle = stagingBuffer;
        CHECK_RESULT_RETURN(lunaCreateBuffer(&bufferCreationInfo, &stagingBufferHandle));
        stagingBuffer = static_cast<const BufferRegionIndex *>(stagingBufferHandle);
    }

    stagingBuffer->bufferRegion()->copyToBuffer(static_cast<const uint8_t *>(writeInfo.pixels), writeInfo.bytes);
    const uint32_t mipmapLevels = writeInfo.mipmapLevels == 0 ? 1 : writeInfo.mipmapLevels;
    const VkImageSubresourceRange subresourceRange = {
        .aspectMask = aspectMask_,
        .levelCount = mipmapLevels,
        .layerCount = arrayLayers_,
    };
    if (VK_API_VERSION_MINOR(luna::apiVersion) >= 3 && device.vulkan13Features().synchronization2 == VK_TRUE)
    {
        helpers::transitionImageLayout2(commandBuffer,
                                        image_,
                                        writeInfo.sourceStageMask == VK_PIPELINE_STAGE_2_NONE
                                                ? VK_PIPELINE_STAGE_2_TRANSFER_BIT
                                                : writeInfo.sourceStageMask,
                                        VK_ACCESS_2_NONE,
                                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                        VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                        VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        subresourceRange);
    } else
    {
        constexpr VkPipelineStageFlags transferStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        helpers::transitionImageLayout(commandBuffer,
                                       image_,
                                       VK_ACCESS_NONE,
                                       VK_ACCESS_TRANSFER_WRITE_BIT,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       subresourceRange,
                                       writeInfo.sourceStageMask == VK_PIPELINE_STAGE_NONE ? transferStageMask
                                                                                           : writeInfo.sourceStageMask,
                                       VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    const VkImageSubresourceLayers subresourceLayers = {
        .aspectMask = aspectMask_,
        .layerCount = arrayLayers_,
    };
    const VkBufferImageCopy bufferCopyInfo = {
        .bufferOffset = stagingBufferOffset(),
        .imageSubresource = writeInfo.subresourceLayers == nullptr ? subresourceLayers : *writeInfo.subresourceLayers,
        .imageOffset = writeInfo.offset == nullptr ? VkOffset3D{} : *writeInfo.offset,
        .imageExtent = extent,
    };
    vkCmdCopyBufferToImage(commandBuffer,
                           *stagingBuffer->buffer(),
                           image_,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &bufferCopyInfo);

    if (writeInfo.generateMipmaps && writeInfo.mipmapLevels > 1)
    {
        generateMipmaps_(commandBuffer, std::bit_cast<VkOffset3D>(extent), mipmapLevels, writeInfo);
    } else
    {
        if (VK_API_VERSION_MINOR(luna::apiVersion) >= 3 && device.vulkan13Features().synchronization2 == VK_TRUE)
        {
            helpers::transitionImageLayout2(commandBuffer,
                                            image_,
                                            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                            VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                            writeInfo.destinationStageMask,
                                            writeInfo.destinationAccessMask,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                            layout_,
                                            subresourceRange);
        } else
        {
            helpers::transitionImageLayout(commandBuffer,
                                           image_,
                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                           writeInfo.destinationAccessMask,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           layout_,
                                           subresourceRange,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           writeInfo.destinationStageMask);
        }
    }

    const Semaphore &semaphore = commandBuffer.semaphore();
    const VkSubmitInfo queueSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = semaphore.isSignaled() ? 1u : 0u,
        .pWaitSemaphores = semaphore.isSignaled() ? &semaphore : nullptr,
        .pWaitDstStageMask = &semaphore.stageMask(),
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &semaphore,
    };
    CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(luna::device.familyQueues().graphics,
                                                          queueSubmitInfo,
                                                          writeInfo.destinationStageMask));
    return VK_SUCCESS;
}

VkSampler Image::sampler(const LunaSampler sampler) const
{
    if (sampler == nullptr)
    {
        return sampler_;
    }
    return luna::sampler(sampler);
}

// TODO: Support filtering with something other than linear
// TODO: Check support for images with multiple layers
void Image::generateMipmaps_(const CommandBuffer &commandBuffer,
                             VkOffset3D extent,
                             uint32_t mipmapLevels,
                             const LunaImageWriteInfo &writeInfo) const
{
    for (uint32_t i = 0; i < mipmapLevels - 1; i++)
    {
        const VkOffset3D oldExtent = extent; // NOLINT(missing-ref)
        extent.x /= 2;
        extent.y /= 2;

        const VkImageSubresourceRange subresourceRange = {
            .aspectMask = aspectMask_,
            .baseMipLevel = i,
            .levelCount = 1,
            .layerCount = arrayLayers_,
        };
        const VkImageSubresourceLayers sourceSubresourceLayers = {
            .aspectMask = aspectMask_,
            .mipLevel = i,
            .layerCount = arrayLayers_,
        };
        const VkImageSubresourceLayers destinationSubresourceLayers = {
            .aspectMask = aspectMask_,
            .mipLevel = i + 1,
            .layerCount = arrayLayers_,
        };
        if (VK_API_VERSION_MINOR(luna::apiVersion) >= 3 && device.vulkan13Features().synchronization2 == VK_TRUE)
        {
            helpers::transitionImageLayout2(commandBuffer,
                                            image_,
                                            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                            VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                            VK_ACCESS_2_TRANSFER_READ_BIT,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            subresourceRange);
        } else
        {
            helpers::transitionImageLayout(commandBuffer,
                                           image_,
                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_ACCESS_TRANSFER_READ_BIT,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           subresourceRange,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT);
        }
        const VkImageBlit blitRegion = {
            .srcSubresource = sourceSubresourceLayers,
            .srcOffsets = {{}, oldExtent},
            .dstSubresource = destinationSubresourceLayers,
            .dstOffsets = {{}, extent},
        };
        vkCmdBlitImage(commandBuffer,
                       image_,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image_,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &blitRegion,
                       VK_FILTER_LINEAR);
    }
    const VkImageSubresourceRange blittedSubresourceRange = {
        .aspectMask = aspectMask_,
        .levelCount = mipmapLevels - 1,
        .layerCount = arrayLayers_,
    };
    const VkImageSubresourceRange lastSubresourceRange = {
        .aspectMask = aspectMask_,
        .baseMipLevel = mipmapLevels - 1,
        .levelCount = 1,
        .layerCount = arrayLayers_,
    };
    if (VK_API_VERSION_MINOR(luna::apiVersion) >= 3 && device.vulkan13Features().synchronization2 == VK_TRUE)
    {
        const VkImageMemoryBarrier2 blittedRegionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
            .dstAccessMask = writeInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout = layout_,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image_,
            .subresourceRange = blittedSubresourceRange,
        };
        const VkImageMemoryBarrier2 lastRegionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstAccessMask = writeInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = layout_,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image_,
            .subresourceRange = lastSubresourceRange,
        };
        const std::array<VkImageMemoryBarrier2, 2> memoryBarriers = {lastRegionBarrier, blittedRegionBarrier};
        const VkDependencyInfo dependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = memoryBarriers.data(),
        };
        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    } else
    {
        const VkImageMemoryBarrier blittedRegionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .dstAccessMask = writeInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout = layout_,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image_,
            .subresourceRange = blittedSubresourceRange,
        };
        const VkImageMemoryBarrier lastRegionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = writeInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = layout_,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image_,
            .subresourceRange = lastSubresourceRange,
        };
        const std::array<VkImageMemoryBarrier, 2> memoryBarriers = {lastRegionBarrier, blittedRegionBarrier};
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             writeInfo.destinationStageMask,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             2,
                             memoryBarriers.data());
    }
}

} // namespace luna

VkResult lunaCreateSampler(const LunaSamplerCreationInfo *creationInfo, LunaSampler *sampler)
{
    assert(creationInfo);
    luna::samplers.emplace_back();
    const VkSamplerCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .flags = creationInfo->flags,
        .magFilter = creationInfo->magFilter,
        .minFilter = creationInfo->minFilter,
        .mipmapMode = creationInfo->mipmapMode,
        .addressModeU = creationInfo->addressModeU,
        .addressModeV = creationInfo->addressModeV,
        .addressModeW = creationInfo->addressModeW,
        .mipLodBias = creationInfo->mipLodBias,
        .anisotropyEnable = static_cast<VkBool32>(creationInfo->anisotropyEnable),
        .maxAnisotropy = creationInfo->maxAnisotropy,
        .compareEnable = static_cast<VkBool32>(creationInfo->compareEnable),
        .compareOp = creationInfo->compareOp,
        .minLod = creationInfo->minLod,
        .maxLod = creationInfo->maxLod,
        .borderColor = creationInfo->borderColor,
        .unnormalizedCoordinates = static_cast<VkBool32>(creationInfo->unnormalizedCoordinates),
    };
    CHECK_RESULT_RETURN(vkCreateSampler(luna::device, &createInfo, nullptr, &luna::samplers.back()));
    if (sampler != nullptr)
    {
        *sampler = static_cast<LunaSampler>(&luna::samplers.back());
    }
    return VK_SUCCESS;
}
VkResult lunaCreateImage(const LunaSampledImageCreationInfo *creationInfo, LunaImage *image)
{
    assert(creationInfo);
    return luna::helpers::createImage(*creationInfo, 0, 1, image);
}
VkResult lunaCreateImageArray(const LunaSampledImageCreationInfo *creationInfo,
                              const uint32_t arrayLayers,
                              LunaImage *image)
{
    assert(creationInfo && arrayLayers);
    return luna::helpers::createImage(*creationInfo, 0, arrayLayers, image);
}
VkResult lunaCreateImage3D(const LunaSampledImageCreationInfo *creationInfo, const uint32_t depth, LunaImage *image)
{
    assert(creationInfo);
    return luna::helpers::createImage(*creationInfo, depth, 1, image);
}
VkResult lunaCreateImage3DArray(const LunaSampledImageCreationInfo *creationInfo,
                                const uint32_t depth,
                                const uint32_t arrayLayers,
                                LunaImage *image)
{
    assert(creationInfo && arrayLayers);
    return luna::helpers::createImage(*creationInfo, depth, arrayLayers, image);
}

VkResult lunaUpdateImage(const LunaImage image, const LunaImageWriteInfo *writeInfo)
{
    assert(image);
    assert(writeInfo);

    const luna::Image *imageObject = static_cast<const luna::Image *>(image);
    CHECK_RESULT_RETURN(imageObject->write(*writeInfo));
    if (writeInfo->descriptorSet != nullptr)
    {
        imageObject->updateDescriptorBinding(luna::device,
                                             writeInfo->descriptorSet,
                                             writeInfo->descriptorLayoutBindingName);
    }
    return VK_SUCCESS;
}

void lunaDestroyImage(LunaImage image)
{
    (void)image;
}
