//
// Created by NBT22 on 3/11/25.
//

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <luna/lunaDevice.h>
#include <luna/lunaImage.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna::helpers
{
static void pipelineBarrier(const VkCommandBuffer commandBuffer,
                            const VkPipelineStageFlags2 sourceStageMask,
                            const VkAccessFlags2 sourceAccessMask,
                            const VkPipelineStageFlags2 destinationStageMask,
                            const VkAccessFlags2 destinationAccessMask,
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
Image::Image(Device &device,
             CommandBuffer &commandBuffer,
             const LunaImageCreationInfo &creationInfo,
             const uint32_t depth,
             const uint32_t arrayLayers)
{
    assert(creationInfo.sampler == LUNA_NULL_HANDLE || creationInfo.samplerCreationInfo == nullptr);
    if (creationInfo.sampler != LUNA_NULL_HANDLE)
    {
        sampler_ = sampler(creationInfo.sampler);
    } else if (creationInfo.samplerCreationInfo != nullptr)
    {
        LunaSampler sampler = LUNA_NULL_HANDLE;
        CHECK_RESULT_THROW(device.createSampler(*creationInfo.samplerCreationInfo, &sampler));
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
        .sharingMode = creationInfo.queueFamilyIndexCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = creationInfo.queueFamilyIndexCount,
        .pQueueFamilyIndices = creationInfo.queueFamilyIndices,
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
    CHECK_RESULT_THROW(write(device, commandBuffer, creationInfo.writeInfo));
    CHECK_RESULT_THROW(helpers::createImageView(static_cast<VkDevice>(device),
                                                image_,
                                                creationInfo.format,
                                                aspectMask_,
                                                mipmapLevels,
                                                &imageView_));
}

VkResult Image::write(Device &device, CommandBuffer &commandBuffer, const LunaImageWriteInfo &writeInfo) const
{
    const uint32_t mipmapLevels = writeInfo.mipmapLevels == 0 ? 1 : writeInfo.mipmapLevels;
    const VkImageSubresourceRange subresourceRange = {
        .aspectMask = aspectMask_,
        .levelCount = mipmapLevels,
        .layerCount = arrayLayers_,
    };
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(static_cast<VkDevice>(device)));

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
        if (writeInfo.submitInfo != nullptr)
        {
            CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(static_cast<VkDevice>(device), *writeInfo.submitInfo));
        }
        return VK_SUCCESS;
    }
    VkExtent3D extent = writeInfo.extent == nullptr ? extent_ : *writeInfo.extent;
    if (writeInfo.extent != nullptr && extent.depth == 0)
    {
        extent.depth = 1;
    }

    CHECK_RESULT_RETURN(BufferRegionIndex::resize(device, commandBuffer, device.stagingBuffer(), writeInfo.bytes));
    CHECK_RESULT_RETURN(device.stagingBuffer()->copyToBuffer(device,
                                                             commandBuffer,
                                                             static_cast<const uint8_t *>(writeInfo.pixels),
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
        .bufferOffset = device.stagingBuffer()->offset(),
        .imageSubresource = writeInfo.subresourceLayers == nullptr ? subresourceLayers : *writeInfo.subresourceLayers,
        .imageOffset = writeInfo.offset == nullptr ? VkOffset3D{} : *writeInfo.offset,
        .imageExtent = extent,
    };
    vkCmdCopyBufferToImage(commandBuffer,
                           device.stagingBuffer()->buffer(),
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

    if (writeInfo.submitInfo != nullptr)
    {
        CHECK_RESULT_RETURN(commandBuffer.endAndSubmit(static_cast<VkDevice>(device), *writeInfo.submitInfo));
    }
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

VkResult lunaCreateSampler(const LunaDevice device, const LunaSamplerCreationInfo *creationInfo, LunaSampler *sampler)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createSampler(*creationInfo, sampler));
    return VK_SUCCESS;
}
void lunaDestroySampler(const LunaDevice device, const LunaSampler sampler)
{
    assert(device != LUNA_NULL_HANDLE);
    luna::helpers::fromHandle<luna::Device>(device)->destroySampler(sampler);
}

VkResult lunaCreateImage(const LunaDevice device,
                         const LunaCommandBuffer commandBuffer,
                         const LunaImageCreationInfo *creationInfo,
                         LunaImage *image)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createImage(
            *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
            *creationInfo,
            0,
            1,
            image));
    return VK_SUCCESS;
}
VkResult lunaCreateImageArray(const LunaDevice device,
                              const LunaCommandBuffer commandBuffer,
                              const LunaImageCreationInfo *creationInfo,
                              const uint32_t arrayLayers,
                              LunaImage *image)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(creationInfo && arrayLayers);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createImage(
            *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
            *creationInfo,
            0,
            arrayLayers,
            image));
    return VK_SUCCESS;
}
VkResult lunaCreateImage3D(const LunaDevice device,
                           const LunaCommandBuffer commandBuffer,
                           const LunaImageCreationInfo *creationInfo,
                           const uint32_t depth,
                           LunaImage *image)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createImage(
            *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
            *creationInfo,
            depth,
            1,
            image));
    return VK_SUCCESS;
}
VkResult lunaCreateImage3DArray(const LunaDevice device,
                                const LunaCommandBuffer commandBuffer,
                                const LunaImageCreationInfo *creationInfo,
                                const uint32_t depth,
                                const uint32_t arrayLayers,
                                LunaImage *image)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(creationInfo && arrayLayers);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createImage(
            *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
            *creationInfo,
            depth,
            arrayLayers,
            image));
    return VK_SUCCESS;
}

VkResult lunaUpdateImage(const LunaDevice device,
                         const LunaCommandBuffer commandBuffer,
                         const LunaImage image,
                         const LunaImageWriteInfo *writeInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(image);
    assert(writeInfo);

    const luna::Image *imageObject = luna::helpers::fromHandle<luna::Image>(image);
    CHECK_RESULT_RETURN(imageObject->write(*luna::helpers::fromHandle<luna::Device>(device),
                                           *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                                           *writeInfo));
    if (writeInfo->descriptorSet != LUNA_NULL_HANDLE)
    {
        imageObject->updateDescriptorBinding(lunaGetVkDevice(device),
                                             writeInfo->descriptorSet,
                                             writeInfo->descriptorLayoutBindingName,
                                             writeInfo->descriptorArrayElement);
    }
    return VK_SUCCESS;
}

VkResult lunaCopyImageToBuffer(const LunaDevice device,
                               const LunaCommandBuffer commandBuffer,
                               const LunaImage image,
                               const LunaBuffer buffer,
                               const uint32_t regionCount,
                               const VkBufferImageCopy *regions,
                               const LunaCommandBufferSubmitInfo *submitInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(image != LUNA_NULL_HANDLE && buffer != LUNA_NULL_HANDLE);

    const luna::Device &deviceObject = *luna::helpers::fromHandle<luna::Device>(device);
    luna::CommandBuffer &commandBufferObject = *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer);
    CHECK_RESULT_RETURN(commandBufferObject.ensureIsRecording(static_cast<VkDevice>(deviceObject)));
    const luna::Image &imageObject = *luna::helpers::fromHandle<luna::Image>(image);
    const luna::BufferRegionIndex &bufferRegionIndex = *luna::helpers::fromHandle<luna::BufferRegionIndex>(buffer);

    vkCmdCopyImageToBuffer(commandBufferObject,
                           imageObject.image(),
                           imageObject.layout(),
                           bufferRegionIndex.buffer(),
                           regionCount,
                           regions);
    if (submitInfo != nullptr)
    {
        CHECK_RESULT_RETURN(commandBufferObject.endAndSubmit(static_cast<VkDevice>(*luna::helpers::fromHandle<
                                                                                   luna::Device>(device)),
                                                             *submitInfo));
    }
    return VK_SUCCESS;
}

void lunaDestroyImage(const LunaDevice device, const LunaImage image)
{
    assert(device != LUNA_NULL_HANDLE);
    luna::helpers::fromHandle<luna::Device>(device)->destroyImage(image);
}
