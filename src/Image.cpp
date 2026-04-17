//
// Created by NBT22 on 3/11/25.
//

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <list>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna::helpers
{
static void pipelineBarrier(const VkCommandBuffer commandBuffer,
                            const LunaFlags sourceStageMask,
                            const LunaFlags sourceAccessMask,
                            const LunaFlags destinationStageMask,
                            const LunaFlags destinationAccessMask,
                            const VkImageLayout oldLayout,
                            const VkImageLayout newLayout,
                            const VkImage image,
                            const VkImageSubresourceRange &subresourceRange)
{
    if (vkCmdPipelineBarrier2 == nullptr)
    {
        const VkImageMemoryBarrier memoryBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = static_cast<VkAccessFlags>(sourceAccessMask),
            .dstAccessMask = static_cast<VkAccessFlags>(destinationAccessMask),
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
    } else
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
}

static void blitImage(const VkCommandBuffer commandBuffer,
                      const VkImage sourceImage,
                      const VkImageLayout sourceImageLayout,
                      const VkImage destinationImage,
                      const VkImageLayout destinationImageLayout,
                      const uint32_t regionCount,
                      const VkImageBlit2 *regions,
                      const VkFilter filter)
{
    if (vkCmdBlitImage2 == nullptr)
    {
        std::vector<VkImageBlit> regionVector;
        regionVector.reserve(regionCount);
        for (uint32_t i = 0; i < regionCount; i++)
        {
            const VkImageBlit2 &region = regions[i]; // NOLINT(*-pro-bounds-pointer-arithmetic)
            regionVector.push_back(VkImageBlit{
                .srcSubresource = region.srcSubresource,
                .srcOffsets = {region.srcOffsets[0], region.srcOffsets[1]},
                .dstSubresource = region.dstSubresource,
                .dstOffsets = {region.dstOffsets[0], region.dstOffsets[1]},
            });
        }
        assert(regionVector.size() == regionCount); // Internal state check
        vkCmdBlitImage(commandBuffer,
                       sourceImage,
                       sourceImageLayout,
                       destinationImage,
                       destinationImageLayout,
                       regionVector.size(),
                       regionVector.data(),
                       filter);
    } else
    {
        const VkBlitImageInfo2 blitImageInfo = {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage = sourceImage,
            .srcImageLayout = sourceImageLayout,
            .dstImage = destinationImage,
            .dstImageLayout = destinationImageLayout,
            .regionCount = regionCount,
            .pRegions = regions,
            .filter = filter,
        };
        vkCmdBlitImage2(commandBuffer, &blitImageInfo);
    }
}
} // namespace luna::helpers

namespace luna
{
Image::Image(const LunaImageCreationInfo &creationInfo, const uint32_t depth, const uint32_t arrayLayers)
{
    assert(creationInfo.sampler == LUNA_NULL_HANDLE || creationInfo.samplerCreationInfo == nullptr);
    if (creationInfo.sampler != LUNA_NULL_HANDLE)
    {
        sampler_ = sampler(creationInfo.sampler);
    } else if (creationInfo.samplerCreationInfo != nullptr)
    {
        LunaSampler sampler = LUNA_NULL_HANDLE;
        CHECK_RESULT_THROW(lunaCreateSampler(creationInfo.samplerCreationInfo, &sampler));
        sampler_ = *helpers::fromHandle<VkSampler>(sampler);
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
                                      creationInfo.allocationCreateInfo ? creationInfo.allocationCreateInfo
                                                                        : &allocationCreateInfo,
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

Image::~Image()
{
    vkDestroyImageView(device, imageView_, nullptr);
    vmaDestroyImage(device.allocator(), image_, allocation_);
}

VkResult Image::write(const LunaImageWriteInfo &writeInfo) const
{
    const uint32_t mipmapLevels = writeInfo.mipmapLevels == 0 ? 1 : writeInfo.mipmapLevels;
    const VkImageSubresourceRange subresourceRange = {
        .aspectMask = aspectMask_,
        .levelCount = mipmapLevels,
        .layerCount = arrayLayers_,
    };

    CommandBuffer &commandBuffer = device.commandPools().graphics->commandBuffer(1);
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(true));

    if (writeInfo.bytes == 0 || writeInfo.pixels == nullptr)
    {
        helpers::pipelineBarrier(commandBuffer,
                                 writeInfo.sourceStageMask == VK_PIPELINE_STAGE_2_NONE
                                         ? VK_PIPELINE_STAGE_2_TRANSFER_BIT
                                         : writeInfo.sourceStageMask,
                                 VK_ACCESS_2_NONE,
                                 writeInfo.destinationStageMask,
                                 writeInfo.destinationAccessMask,
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 layout_,
                                 image_,
                                 subresourceRange);
        CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(luna::device.familyQueues().graphics,
                                                       writeInfo.destinationStageMask));
        return VK_SUCCESS;
    }
    VkExtent3D extent = writeInfo.extent == nullptr ? extent_ : *writeInfo.extent;
    if (writeInfo.extent != nullptr && extent.depth == 0)
    {
        extent.depth = 1;
    }

    CHECK_RESULT_RETURN(BufferRegionIndex::resize(device.stagingBuffer, writeInfo.bytes));
    CHECK_RESULT_RETURN(device.stagingBuffer->copyToBuffer(static_cast<const uint8_t *>(writeInfo.pixels),
                                                           writeInfo.bytes));
    helpers::pipelineBarrier(commandBuffer,
                             writeInfo.sourceStageMask == VK_PIPELINE_STAGE_2_NONE ? VK_PIPELINE_STAGE_2_TRANSFER_BIT
                                                                                   : writeInfo.sourceStageMask,
                             VK_ACCESS_2_NONE,
                             VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                             VK_ACCESS_2_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             image_,
                             subresourceRange);

    const VkImageSubresourceLayers subresourceLayers = {
        .aspectMask = aspectMask_,
        .layerCount = arrayLayers_,
    };
    const VkBufferImageCopy bufferCopyInfo = {
        .bufferOffset = device.stagingBuffer->offset(),
        .imageSubresource = writeInfo.subresourceLayers == nullptr ? subresourceLayers : *writeInfo.subresourceLayers,
        .imageOffset = writeInfo.offset == nullptr ? VkOffset3D{} : *writeInfo.offset,
        .imageExtent = extent,
    };
    vkCmdCopyBufferToImage(commandBuffer,
                           device.stagingBuffer->buffer(),
                           image_,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &bufferCopyInfo);

    if (writeInfo.generateMipmaps && writeInfo.mipmapLevels > 1)
    {
        generateMipmaps_(commandBuffer, std::bit_cast<VkOffset3D>(extent), mipmapLevels, writeInfo);
    } else
    {
        helpers::pipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                 VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                 writeInfo.destinationStageMask,
                                 writeInfo.destinationAccessMask,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 layout_,
                                 image_,
                                 subresourceRange);
    }

    CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(luna::device.familyQueues().graphics,
                                                   writeInfo.destinationStageMask));
    return VK_SUCCESS;
}

VkSampler Image::sampler(const LunaSampler sampler) const
{
    if (sampler == LUNA_NULL_HANDLE)
    {
        return sampler_;
    }
    return *helpers::fromHandle<VkSampler>(sampler);
}

// TODO: Check support for images with multiple layers
void Image::generateMipmaps_(const CommandBuffer &commandBuffer,
                             VkOffset3D extent,
                             const uint32_t mipmapLevels,
                             const LunaImageWriteInfo &writeInfo) const
{
    for (uint32_t i = 0; i < mipmapLevels - 1; i++)
    {
        const VkOffset3D oldExtent = extent;
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
        helpers::pipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                 VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                 VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                 VK_ACCESS_2_TRANSFER_READ_BIT,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 image_,
                                 subresourceRange);
        const VkImageBlit2 blitRegion = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = sourceSubresourceLayers,
            .srcOffsets = {{}, oldExtent},
            .dstSubresource = destinationSubresourceLayers,
            .dstOffsets = {{}, extent},
        };
        helpers::blitImage(commandBuffer,
                           image_,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           image_,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blitRegion,
                           writeInfo.mipmapFilter);
    }
    const std::array<LunaImageMemoryBarrier, 2> lunaImageMemoryBarriers = {
        LunaImageMemoryBarrier{
            .sourceStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .sourceAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
            .destinationStageMask = writeInfo.destinationStageMask,
            .destinationAccessMask = writeInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout = layout_,
            .image = helpers::toHandle(this),
            .subresourceRange = VkImageSubresourceRange{.aspectMask = aspectMask_,
                                                        .levelCount = mipmapLevels - 1,
                                                        .layerCount = arrayLayers_},
        },
        LunaImageMemoryBarrier{
            .sourceStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .sourceAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .destinationStageMask = writeInfo.destinationStageMask,
            .destinationAccessMask = writeInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = layout_,
            .image = helpers::toHandle(this),
            .subresourceRange = VkImageSubresourceRange{.aspectMask = aspectMask_,
                                                        .baseMipLevel = mipmapLevels - 1,
                                                        .levelCount = 1,
                                                        .layerCount = arrayLayers_},
        },
    };
    const LunaDependencyInfo dependencyInfo = {
        .imageMemoryBarrierCount = lunaImageMemoryBarriers.size(),
        .imageMemoryBarriers = lunaImageMemoryBarriers.data(),
    };
    helpers::pipelineBarrier(commandBuffer, dependencyInfo);
}
} // namespace luna

VkResult lunaCreateSampler(const LunaSamplerCreationInfo *creationInfo, LunaSampler *sampler)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::device.createSampler(*creationInfo, sampler));
    return VK_SUCCESS;
}
void lunaDestroySampler(const LunaSampler sampler)
{
    luna::device.destroySampler(sampler);
}

VkResult lunaCreateImage(const LunaImageCreationInfo *creationInfo, LunaImage *image)
{
    assert(creationInfo);
    return luna::device.createImage(*creationInfo, 0, 1, image);
}
VkResult lunaCreateImageArray(const LunaImageCreationInfo *creationInfo, const uint32_t arrayLayers, LunaImage *image)
{
    assert(creationInfo && arrayLayers);
    return luna::device.createImage(*creationInfo, 0, arrayLayers, image);
}
VkResult lunaCreateImage3D(const LunaImageCreationInfo *creationInfo, const uint32_t depth, LunaImage *image)
{
    assert(creationInfo);
    return luna::device.createImage(*creationInfo, depth, 1, image);
}
VkResult lunaCreateImage3DArray(const LunaImageCreationInfo *creationInfo,
                                const uint32_t depth,
                                const uint32_t arrayLayers,
                                LunaImage *image)
{
    assert(creationInfo && arrayLayers);
    return luna::device.createImage(*creationInfo, depth, arrayLayers, image);
}

VkResult lunaUpdateImage(const LunaImage image, const LunaImageWriteInfo *writeInfo)
{
    assert(image);
    assert(writeInfo);

    const luna::Image *imageObject = luna::helpers::fromHandle<luna::Image>(image);
    CHECK_RESULT_RETURN(imageObject->write(*writeInfo));
    if (writeInfo->descriptorSet != LUNA_NULL_HANDLE)
    {
        imageObject->updateDescriptorBinding(luna::device,
                                             writeInfo->descriptorSet,
                                             writeInfo->descriptorLayoutBindingName,
                                             writeInfo->descriptorArrayElement);
    }
    return VK_SUCCESS;
}

VkResult lunaBlitImageToSwapchain(const LunaImage image, const VkImageBlit2 *blitRegion)
{
    assert(image);
    assert(blitRegion);

    luna::CommandBuffer &commandBuffer = luna::device.commandPools().graphics->commandBuffer();
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(true));

    constexpr VkImageSubresourceRange subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .layerCount = 1,
    };

    const VkImage swapchainImage = luna::swapchain.images.at(luna::swapchain.imageIndex);

    luna::helpers::pipelineBarrier(commandBuffer,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_ACCESS_2_NONE,
                                   VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                   VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   swapchainImage,
                                   subresourceRange);
    luna::helpers::blitImage(commandBuffer,
                             luna::helpers::fromHandle<luna::Image>(image)->image(),
                             VK_IMAGE_LAYOUT_GENERAL,
                             swapchainImage,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             1,
                             blitRegion,
                             VK_FILTER_NEAREST);
    luna::helpers::pipelineBarrier(commandBuffer,
                                   VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                   VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                   swapchainImage,
                                   subresourceRange);

    return VK_SUCCESS;
}

VkResult lunaCopyImageToBuffer(const LunaImage image,
                               const LunaBuffer buffer,
                               const uint32_t regionCount,
                               const VkBufferImageCopy *regions)
{
    assert(image != LUNA_NULL_HANDLE && buffer != LUNA_NULL_HANDLE);

    luna::CommandBuffer &commandBuffer = luna::device.commandPools().graphics->commandBuffer(1);
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(true));
    const luna::Image &imageObject = *luna::helpers::fromHandle<luna::Image>(image);
    const luna::BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);

    std::vector<VkBufferImageCopy> regionsVector;
    regionsVector.reserve(regionCount);
    for (uint32_t i = 0; i < regionCount; i++)
    {
        const VkBufferImageCopy &region = regions[i];
        regionsVector.emplace_back(region.bufferOffset + bufferRegionIndex.offset(),
                                   region.bufferRowLength,
                                   region.bufferImageHeight,
                                   region.imageSubresource,
                                   region.imageOffset,
                                   region.imageExtent);
    }

    vkCmdCopyImageToBuffer(commandBuffer,
                           imageObject.image(),
                           imageObject.layout(),
                           bufferRegionIndex.buffer(),
                           regionCount,
                           regions);
    CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(luna::device.familyQueues().graphics));
    return VK_SUCCESS;
}

void lunaDestroyImage(const LunaImage image)
{
    luna::device.destroyImage(image);
}
