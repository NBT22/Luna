//
// Created by NBT22 on 3/11/25.
//

#include <algorithm>
#include <array>
#include <luna/core/Image.hpp>
#include <luna/core/Instance.hpp>
#include <luna/luna.h>

namespace luna::helpers
{
static uint8_t bytesPerPixel(const VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_S8_UINT:
            return 1;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_D16_UNORM:
            return 2;
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return 3;
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return 4;
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
            return 6;
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
            return 8;
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            return 12;
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
            return 16;
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
            return 24;
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return 32;
        case VK_FORMAT_UNDEFINED:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        default:
            throw std::runtime_error("Unhandled image format!");
    }
}
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
// TODO: Support filtering with something other than linear
// TODO: Check support for images with multiple layers
static void generateMipmaps(const core::CommandBuffer &commandBuffer,
                            const VkImage image,
                            VkExtent3D extent,
                            const uint32_t mipmapLevels,
                            const VkImageAspectFlags aspectMask,
                            const uint32_t arrayLayers,
                            const LunaSampledImageCreationInfo &creationInfo)
{
    for (uint32_t i = 0; i < mipmapLevels - 1; i++)
    {
        const VkOffset3D oldExtent = std::bit_cast<VkOffset3D>(extent); // NOLINT(missing-ref)
        extent.width /= 2;
        extent.height /= 2;

        const VkImageSubresourceRange subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = i,
            .levelCount = 1,
            .layerCount = arrayLayers,
        };
        const VkImageSubresourceLayers sourceSubresourceLayers = {
            .aspectMask = aspectMask,
            .mipLevel = i,
            .layerCount = arrayLayers,
        };
        const VkImageSubresourceLayers destinationSubresourceLayers = {
            .aspectMask = aspectMask,
            .mipLevel = i + 1,
            .layerCount = arrayLayers,
        };
        if (VK_API_VERSION_MINOR(core::apiVersion) >= 3)
        {
            transitionImageLayout2(commandBuffer,
                                   image,
                                   VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                   VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                   VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                   VK_ACCESS_2_TRANSFER_READ_BIT,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   subresourceRange);
        } else
        {
            transitionImageLayout(commandBuffer,
                                  image,
                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                  VK_ACCESS_TRANSFER_READ_BIT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  subresourceRange,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT);
            const VkImageBlit blitRegion = {
                .srcSubresource = sourceSubresourceLayers,
                .srcOffsets = {{}, oldExtent},
                .dstSubresource = destinationSubresourceLayers,
                .dstOffsets = {{}, std::bit_cast<VkOffset3D>(extent)},
            };
            vkCmdBlitImage(commandBuffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blitRegion,
                           VK_FILTER_LINEAR);
        }
    }
    const VkImageSubresourceRange blittedSubresourceRange = {
        .aspectMask = aspectMask,
        .levelCount = mipmapLevels - 1,
        .layerCount = arrayLayers,
    };
    const VkImageSubresourceRange lastSubresourceRange = {
        .aspectMask = aspectMask,
        .baseMipLevel = mipmapLevels - 1,
        .levelCount = 1,
        .layerCount = arrayLayers,
    };
    if (VK_API_VERSION_MINOR(core::apiVersion) >= 3)
    {
        // transitionImageLayout2(commandBuffer,
        //                        image,
        //                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        //                        VK_ACCESS_2_TRANSFER_READ_BIT,
        //                        creationInfo.destinationStageMask,
        //                        creationInfo.destinationAccessMask,
        //                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //                        creationInfo.layout,
        //                        subresourceRange);
    } else
    {
        const VkImageMemoryBarrier blittedRegionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .dstAccessMask = creationInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout = creationInfo.layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = blittedSubresourceRange,
        };
        const VkImageMemoryBarrier lastRegionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = creationInfo.destinationAccessMask,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = creationInfo.layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = lastSubresourceRange,
        };
        const std::array<VkImageMemoryBarrier, 2> memoryBarriers = {lastRegionBarrier, blittedRegionBarrier};
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             creationInfo.destinationStageMask,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             2,
                             memoryBarriers.data());
    }
}
static VkResult writeImage(const VkImage image,
                           const VkExtent3D &extent,
                           const uint32_t arrayLayers,
                           const LunaSampledImageCreationInfo &creationInfo,
                           const VkImageAspectFlags aspectMask)
{
    core::CommandBuffer &commandBuffer = core::device.commandPools().graphics.commandBuffer(1);
    if (!commandBuffer.isRecording())
    {
        const VkDevice logicalDevice = core::device;
        CHECK_RESULT_RETURN(commandBuffer.waitForFence(logicalDevice));
        CHECK_RESULT_RETURN(commandBuffer.resetFence(logicalDevice));
        CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());
    }

    const size_t bytes = extent.width * extent.height * extent.depth * bytesPerPixel(creationInfo.format);
    if (core::stagingBufferIndex == nullptr || core::bufferRegion(core::stagingBufferIndex).size() < bytes)
    {
        if (core::stagingBufferIndex != nullptr)
        {
            const auto *index = static_cast<const core::buffer::BufferRegionIndex *>(core::stagingBufferIndex);
            core::buffers.at(index->bufferIndex).destroyBufferRegion(index->bufferRegionIndex);
        }
        const LunaBufferCreationInfo bufferCreationInfo = {
            .size = bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };
        CHECK_RESULT_RETURN(core::buffer::BufferRegion::createBufferRegion(bufferCreationInfo,
                                                                           &core::stagingBufferIndex));
    }

    core::bufferRegion(core::stagingBufferIndex).copyToBuffer(static_cast<const uint8_t *>(creationInfo.pixels), bytes);
    const uint32_t mipmapLevels = creationInfo.mipmapLevels == 0 ? 1 : creationInfo.mipmapLevels;
    const VkImageSubresourceRange subresourceRange = {
        .aspectMask = aspectMask,
        .levelCount = mipmapLevels,
        .layerCount = arrayLayers,
    };
    if (VK_API_VERSION_MINOR(core::apiVersion) >= 3)
    {
        transitionImageLayout2(commandBuffer,
                               image,
                               creationInfo.sourceStageMask == VK_PIPELINE_STAGE_2_NONE
                                       ? VK_PIPELINE_STAGE_2_TRANSFER_BIT
                                       : creationInfo.sourceStageMask,
                               VK_ACCESS_2_NONE,
                               VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                               VK_ACCESS_2_TRANSFER_WRITE_BIT,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               subresourceRange);
    } else
    {
        transitionImageLayout(commandBuffer,
                              image,
                              VK_ACCESS_NONE,
                              VK_ACCESS_TRANSFER_WRITE_BIT,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              subresourceRange,
                              creationInfo.sourceStageMask == VK_PIPELINE_STAGE_NONE ? VK_PIPELINE_STAGE_TRANSFER_BIT
                                                                                     : creationInfo.sourceStageMask,
                              VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    const VkImageSubresourceLayers subresourceLayers = {
        .aspectMask = aspectMask,
        .layerCount = arrayLayers,
    };
    const VkBufferImageCopy bufferCopyInfo = {
        .bufferOffset = core::stagingBufferOffset(),
        .imageSubresource = subresourceLayers,
        .imageExtent = extent,
    };
    vkCmdCopyBufferToImage(commandBuffer,
                           core::stagingBuffer(),
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &bufferCopyInfo);

    if (creationInfo.generateMipmaps && creationInfo.mipmapLevels > 1)
    {
        generateMipmaps(commandBuffer, image, extent, mipmapLevels, aspectMask, arrayLayers, creationInfo);
    } else
    {
        if (VK_API_VERSION_MINOR(core::apiVersion) >= 3)
        {
            transitionImageLayout2(commandBuffer,
                                   image,
                                   VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                   VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                   creationInfo.destinationStageMask,
                                   creationInfo.destinationAccessMask,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   creationInfo.layout,
                                   subresourceRange);
        } else
        {
            transitionImageLayout(commandBuffer,
                                  image,
                                  VK_ACCESS_TRANSFER_WRITE_BIT,
                                  creationInfo.destinationAccessMask,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  creationInfo.layout,
                                  subresourceRange,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  creationInfo.destinationStageMask);
        }
    }

    const core::Semaphore &semaphore = commandBuffer.semaphore();
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
    CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(core::device.familyQueues().graphics,
                                                          queueSubmitInfo,
                                                          creationInfo.destinationStageMask));
    return VK_SUCCESS;
}
static VkResult createImage(const LunaSampledImageCreationInfo &creationInfo,
                            uint32_t depth,
                            uint32_t arrayLayers,
                            LunaImage *imageIndex)
{
    core::imageIndices.emplace_back(core::images.size());
    TRY_CATCH_RESULT(core::images.emplace_back(creationInfo, depth, arrayLayers));
    const core::Image &image = core::images.back();
    if (creationInfo.descriptorSet != nullptr)
    {
        assert(creationInfo.descriptorLayoutBindingName);
        const VkDescriptorImageInfo imageInfo = {
            .sampler = image.sampler(),
            .imageView = image.imageView(),
            .imageLayout = creationInfo.layout,
        };
        core::DescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        core::descriptorSet(creationInfo.descriptorSet, nullptr, &descriptorSetLayout, &descriptorSet);
        const char *bindingName = creationInfo.descriptorLayoutBindingName;
        const core::DescriptorSetLayout::Binding &binding = descriptorSetLayout.binding(bindingName);
        const VkWriteDescriptorSet writeDescriptor = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = binding.index,
            .descriptorCount = 1,
            .descriptorType = binding.type,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(core::device, 1, &writeDescriptor, 0, nullptr);
    }

    if (imageIndex != nullptr)
    {
        *imageIndex = &core::imageIndices.back();
    }
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core
{
Image::Image(const LunaSampledImageCreationInfo &creationInfo, const uint32_t depth, const uint32_t arrayLayers)
{
    assert(isDestroyed_);
    assert(creationInfo.sampler == nullptr || creationInfo.samplerCreationInfo == nullptr);
    const VkExtent3D extent = {
        .width = creationInfo.width,
        .height = creationInfo.height,
        .depth = depth == 0 ? 1 : depth,
    };
    const uint32_t mipmapLevels = creationInfo.mipmapLevels == 0 ? 1 : creationInfo.mipmapLevels;
    VkImageUsageFlags usage = creationInfo.usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    usage |= creationInfo.generateMipmaps && creationInfo.mipmapLevels > 1 ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0;
    const VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = creationInfo.flags,
        .imageType = depth == 0 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D,
        .format = creationInfo.format,
        .extent = extent,
        .mipLevels = mipmapLevels,
        .arrayLayers = arrayLayers,
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
    VkImageAspectFlags aspectMask = creationInfo.aspectMask;
    if (aspectMask == 0)
    {
        if (creationInfo.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            creationInfo.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        } else
        {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
    CHECK_RESULT_THROW(helpers::createImageView(device,
                                                image_,
                                                creationInfo.format,
                                                aspectMask,
                                                mipmapLevels,
                                                &imageView_));
    if (creationInfo.sampler != nullptr)
    {
        sampler_ = sampler(creationInfo.sampler);
    } else if (creationInfo.samplerCreationInfo != nullptr)
    {
        LunaSampler sampler;
        CHECK_RESULT_THROW(lunaCreateSampler(creationInfo.samplerCreationInfo, &sampler));
        sampler_ = core::sampler(sampler);
    }

    CHECK_RESULT_THROW(helpers::writeImage(image_, extent, arrayLayers, creationInfo, aspectMask));
    isDestroyed_ = false;
}

void Image::destroy()
{
    if (isDestroyed_)
    {
        return;
    }
    vkDestroyImageView(device, imageView_, nullptr);
    vmaDestroyImage(device.allocator(), image_, allocation_);
    isDestroyed_ = true;
}

VkSampler Image::sampler(const LunaSampler sampler)
{
    if (sampler == nullptr)
    {
        return sampler_;
    }
    sampler_ = core::sampler(sampler);
    return sampler_;
}
} // namespace luna::core

VkResult lunaCreateSampler(const LunaSamplerCreationInfo *creationInfo, LunaSampler *sampler)
{
    assert(creationInfo);
    luna::core::samplers.reserve(luna::core::samplers.size() + 1);
    const std::vector<VkSampler>::iterator samplerIterator = std::find(luna::core::samplers.begin(),
                                                                       luna::core::samplers.end(),
                                                                       VK_NULL_HANDLE);
    luna::core::samplerIndices.emplace_back(samplerIterator - luna::core::samplers.begin());
    luna::core::samplers.emplace(samplerIterator);
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
        .anisotropyEnable = creationInfo->anisotropyEnable,
        .maxAnisotropy = creationInfo->maxAnisotropy,
        .compareEnable = creationInfo->compareEnable,
        .compareOp = creationInfo->compareOp,
        .minLod = creationInfo->minLod,
        .maxLod = creationInfo->maxLod,
        .borderColor = creationInfo->borderColor,
        .unnormalizedCoordinates = creationInfo->unnormalizedCoordinates,
    };
    CHECK_RESULT_RETURN(vkCreateSampler(luna::core::device, &createInfo, nullptr, samplerIterator.base()));
    if (sampler != nullptr)
    {
        *sampler = &luna::core::samplerIndices.back();
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
