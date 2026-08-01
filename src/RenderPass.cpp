//
// Created by NBT22 on 2/18/25.
//

#include <array>
#include <cassert>
#include <cstdint>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna::helpers
{
static void createDepthAttachment(const VkSampleCountFlagBits samples,
                                  const LunaAttachmentLoadMode depthAttachmentLoadMode,
                                  VkAttachmentReference &attachmentReference,
                                  VkAttachmentDescription &attachmentDescription)
{
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    switch (depthAttachmentLoadMode)
    {
        case LUNA_ATTACHMENT_LOAD_MODE_CLEAR:
            loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case LUNA_ATTACHMENT_LOAD_MODE_PRESERVE:
            loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        default:
            break;
    }

    attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescription.format = depthImageFormat;
    attachmentDescription.samples = samples;
    attachmentDescription.loadOp = loadOp;
    attachmentDescription.storeOp = depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_MODE_PRESERVE
                                            ? VK_ATTACHMENT_STORE_OP_STORE
                                            : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}
static void createDepthAttachment2(const VkSampleCountFlagBits samples,
                                   const LunaAttachmentLoadMode depthAttachmentLoadMode,
                                   VkAttachmentReference2 &attachmentReference,
                                   VkAttachmentDescription2 &attachmentDescription)
{
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    switch (depthAttachmentLoadMode)
    {
        case LUNA_ATTACHMENT_LOAD_MODE_CLEAR:
            loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case LUNA_ATTACHMENT_LOAD_MODE_PRESERVE:
            loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        default:
            break;
    }

    attachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    attachmentDescription.format = depthImageFormat;
    attachmentDescription.samples = samples;
    attachmentDescription.loadOp = loadOp;
    attachmentDescription.storeOp = depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_MODE_PRESERVE
                                            ? VK_ATTACHMENT_STORE_OP_STORE
                                            : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}
static void createColorAttachment(const uint32_t colorAttachmentIndex,
                                  const VkSampleCountFlagBits samples,
                                  const LunaAttachmentLoadMode colorAttachmentLoadMode,
                                  std::array<VkAttachmentReference, 3> &attachmentReferences,
                                  std::array<VkAttachmentDescription, 3> &attachmentDescriptions)
{
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    switch (colorAttachmentLoadMode)
    {
        case LUNA_ATTACHMENT_LOAD_MODE_CLEAR:
            loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case LUNA_ATTACHMENT_LOAD_MODE_PRESERVE:
            loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        default:
            break;
    }

    attachmentReferences.at(1).attachment = colorAttachmentIndex;
    attachmentReferences.at(1).layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachmentDescriptions.at(colorAttachmentIndex).format = swapchain.format.format;
    attachmentDescriptions.at(colorAttachmentIndex).samples = samples;
    attachmentDescriptions.at(colorAttachmentIndex).loadOp = loadOp;
    attachmentDescriptions.at(colorAttachmentIndex).stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions.at(colorAttachmentIndex).stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    if (samples != VK_SAMPLE_COUNT_1_BIT)
    {
        attachmentDescriptions.at(colorAttachmentIndex).storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions.at(colorAttachmentIndex).finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentReferences.at(2).attachment = colorAttachmentIndex + 1;
        attachmentReferences.at(2).layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentDescriptions.at(colorAttachmentIndex + 1).format = swapchain.format.format;
        attachmentDescriptions.at(colorAttachmentIndex + 1).samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions.at(colorAttachmentIndex + 1).loadOp = loadOp;
        attachmentDescriptions.at(colorAttachmentIndex + 1).storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions.at(colorAttachmentIndex + 1).stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions.at(colorAttachmentIndex + 1).stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions.at(colorAttachmentIndex + 1).finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else
    {
        attachmentDescriptions.at(colorAttachmentIndex).storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions.at(colorAttachmentIndex).finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
}
static void createColorAttachment2(const uint32_t colorAttachmentIndex,
                                   const VkSampleCountFlagBits samples,
                                   const LunaAttachmentLoadMode colorAttachmentLoadMode,
                                   std::array<VkAttachmentReference2, 3> &attachmentReferences,
                                   std::array<VkAttachmentDescription2, 3> &attachmentDescriptions)
{
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    switch (colorAttachmentLoadMode)
    {
        case LUNA_ATTACHMENT_LOAD_MODE_CLEAR:
            loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case LUNA_ATTACHMENT_LOAD_MODE_PRESERVE:
            loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        default:
            break;
    }

    attachmentReferences.at(1).sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    attachmentReferences.at(1).attachment = colorAttachmentIndex;
    attachmentReferences.at(1).layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachmentDescriptions.at(colorAttachmentIndex).sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    attachmentDescriptions.at(colorAttachmentIndex).format = swapchain.format.format;
    attachmentDescriptions.at(colorAttachmentIndex).samples = samples;
    attachmentDescriptions.at(colorAttachmentIndex).loadOp = loadOp;
    attachmentDescriptions.at(colorAttachmentIndex).stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions.at(colorAttachmentIndex).stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    if (samples != VK_SAMPLE_COUNT_1_BIT)
    {
        attachmentDescriptions.at(colorAttachmentIndex).storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions.at(colorAttachmentIndex).finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentReferences.at(2).sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        attachmentReferences.at(2).attachment = colorAttachmentIndex + 1;
        attachmentReferences.at(2).layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentDescriptions.at(colorAttachmentIndex + 1).sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
        attachmentDescriptions.at(colorAttachmentIndex + 1).format = swapchain.format.format;
        attachmentDescriptions.at(colorAttachmentIndex + 1).samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions.at(colorAttachmentIndex + 1).loadOp = loadOp;
        attachmentDescriptions.at(colorAttachmentIndex + 1).storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions.at(colorAttachmentIndex + 1).stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions.at(colorAttachmentIndex + 1).stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions.at(colorAttachmentIndex + 1).finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else
    {
        attachmentDescriptions.at(colorAttachmentIndex).storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions.at(colorAttachmentIndex).finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
}

// TODO: Has issues with not clearing attachments
static void createAttachments(const VkSampleCountFlagBits samples,
                              const bool createDepth,
                              const LunaAttachmentLoadMode depthAttachmentLoadMode,
                              const bool createColor,
                              const LunaAttachmentLoadMode colorAttachmentLoadMode,
                              std::array<VkAttachmentReference, 3> &attachmentReferences,
                              std::array<VkAttachmentDescription, 3> &attachmentDescriptions)
{
    if (createDepth)
    {
        createDepthAttachment(samples,
                              depthAttachmentLoadMode,
                              attachmentReferences.at(0),
                              attachmentDescriptions.at(0));
    }
    if (createColor)
    {
        createColorAttachment(createDepth ? 1 : 0,
                              samples,
                              colorAttachmentLoadMode,
                              attachmentReferences,
                              attachmentDescriptions);
    }
}
static void createAttachments2(const VkSampleCountFlagBits samples,
                               const bool createDepth,
                               const LunaAttachmentLoadMode depthAttachmentLoadMode,
                               const bool createColor,
                               const LunaAttachmentLoadMode colorAttachmentLoadMode,
                               std::array<VkAttachmentReference2, 3> &attachmentReferences,
                               std::array<VkAttachmentDescription2, 3> &attachmentDescriptions)
{
    if (createDepth)
    {
        createDepthAttachment2(samples,
                               depthAttachmentLoadMode,
                               attachmentReferences.at(0),
                               attachmentDescriptions.at(0));
    }
    if (createColor)
    {
        createColorAttachment2(createDepth ? 1 : 0,
                               samples,
                               colorAttachmentLoadMode,
                               attachmentReferences,
                               attachmentDescriptions);
    }
}

static VkResult createRenderPass(const VkDevice device,
                                 const LunaRenderPassCreationInfo &creationInfo,
                                 const VkSampleCountFlagBits samples,
                                 VkRenderPass &renderPass)
{
    std::array<VkAttachmentReference, 3> attachmentReferences{};
    std::array<VkAttachmentDescription, 3> attachmentDescriptions{};
    createAttachments(samples,
                      creationInfo.createDepthAttachment,
                      creationInfo.depthAttachmentLoadMode,
                      creationInfo.createColorAttachment,
                      creationInfo.colorAttachmentLoadMode,
                      attachmentReferences,
                      attachmentDescriptions);

    std::vector<VkSubpassDescription> subpasses;
    subpasses.reserve(creationInfo.subpassCount);
    for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
    {
        const LunaSubpassCreationInfo &subpassCreationInfo = creationInfo.subpasses[i];
        assert((!subpassCreationInfo.useColorAttachment || creationInfo.createColorAttachment) &&
               (!subpassCreationInfo.useDepthAttachment || creationInfo.createDepthAttachment));
        subpasses.emplace_back(subpassCreationInfo.flags,
                               subpassCreationInfo.pipelineBindPoint,
                               subpassCreationInfo.inputAttachmentCount,
                               subpassCreationInfo.inputAttachments,
                               subpassCreationInfo.useColorAttachment ? 1u : 0,
                               subpassCreationInfo.useColorAttachment ? &attachmentReferences.at(1) : nullptr,
                               subpassCreationInfo.useColorAttachment && samples != VK_SAMPLE_COUNT_1_BIT
                                       ? &attachmentReferences.at(2)
                                       : nullptr,
                               subpassCreationInfo.useDepthAttachment ? &attachmentReferences.at(0) : nullptr,
                               subpassCreationInfo.preserveAttachmentCount,
                               subpassCreationInfo.preserveAttachments);
    }
    const uint32_t attachmentCount = static_cast<uint32_t>(creationInfo.createDepthAttachment) +
                                     static_cast<uint32_t>(creationInfo.createColorAttachment) +
                                     static_cast<uint32_t>(creationInfo.createColorAttachment &&
                                                           samples != VK_SAMPLE_COUNT_1_BIT);
    const VkRenderPassCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachmentCount,
        .pAttachments = attachmentDescriptions.data(),
        .subpassCount = creationInfo.subpassCount,
        .pSubpasses = subpasses.data(),
        .dependencyCount = creationInfo.dependencyCount,
        .pDependencies = creationInfo.dependencies,
    };
    CHECK_RESULT_RETURN(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));

    return VK_SUCCESS;
}
static VkResult createRenderPass2(const VkDevice device,
                                  const LunaRenderPassCreationInfo2 &creationInfo,
                                  const VkSampleCountFlagBits samples,
                                  VkRenderPass &renderPass)
{
    std::array<VkAttachmentReference2, 3> attachmentReferences{};
    std::array<VkAttachmentDescription2, 3> attachmentDescriptions{};
    createAttachments2(samples,
                       creationInfo.createDepthAttachment,
                       creationInfo.depthAttachmentLoadMode,
                       creationInfo.createColorAttachment,
                       creationInfo.colorAttachmentLoadMode,
                       attachmentReferences,
                       attachmentDescriptions);

    std::vector<VkSubpassDescription2> subpasses;
    subpasses.reserve(creationInfo.subpassCount);
    for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
    {
        const LunaSubpassCreationInfo2 &subpassCreationInfo = creationInfo.subpasses[i];
        assert((!subpassCreationInfo.useColorAttachment || creationInfo.createColorAttachment) &&
               (!subpassCreationInfo.useDepthAttachment || creationInfo.createDepthAttachment));
        subpasses.emplace_back(VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
                               nullptr,
                               subpassCreationInfo.flags,
                               subpassCreationInfo.pipelineBindPoint,
                               subpassCreationInfo.viewMask,
                               subpassCreationInfo.inputAttachmentCount,
                               subpassCreationInfo.inputAttachments,
                               subpassCreationInfo.useColorAttachment ? 1u : 0,
                               subpassCreationInfo.useColorAttachment ? &attachmentReferences.at(1) : nullptr,
                               subpassCreationInfo.useColorAttachment && samples != VK_SAMPLE_COUNT_1_BIT
                                       ? &attachmentReferences.at(2)
                                       : nullptr,
                               subpassCreationInfo.useDepthAttachment ? &attachmentReferences.at(0) : nullptr,
                               subpassCreationInfo.preserveAttachmentCount,
                               subpassCreationInfo.preserveAttachments);
    }
    const uint32_t attachmentCount = static_cast<uint32_t>(creationInfo.createDepthAttachment) +
                                     static_cast<uint32_t>(creationInfo.createColorAttachment) +
                                     static_cast<uint32_t>(creationInfo.createColorAttachment &&
                                                           samples != VK_SAMPLE_COUNT_1_BIT);
    const VkRenderPassCreateInfo2 createInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .attachmentCount = attachmentCount,
        .pAttachments = attachmentDescriptions.data(),
        .subpassCount = creationInfo.subpassCount,
        .pSubpasses = subpasses.data(),
        .dependencyCount = creationInfo.dependencyCount,
        .pDependencies = creationInfo.dependencies,
        .correlatedViewMaskCount = creationInfo.correlatedViewMaskCount,
        .pCorrelatedViewMasks = creationInfo.correlatedViewMasks,
    };
    CHECK_RESULT_RETURN(vkCreateRenderPass2(device, &createInfo, nullptr, &renderPass));

    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna
{
RenderPass::RenderPass(const VkDevice device,
                       const VmaAllocator &allocator,
                       const LunaRenderPassCreationInfo &creationInfo)
{
    assert(isDestroyed_);
    init_(creationInfo);
    CHECK_RESULT_THROW(helpers::createRenderPass(device, creationInfo, samples_, renderPass_));
    CHECK_RESULT_THROW(createAttachmentImages(device,
                                              creationInfo.queueFamilyIndexCount,
                                              creationInfo.queueFamilyIndices,
                                              allocator,
                                              creationInfo.createDepthAttachment));
    CHECK_RESULT_THROW(createFramebuffers(device,
                                          creationInfo.createDepthAttachment,
                                          creationInfo.framebufferAttachmentCount,
                                          creationInfo.framebufferAttachments));
    isDestroyed_ = false;
}
RenderPass::RenderPass(const VkDevice device,
                       const VmaAllocator &allocator,
                       const LunaRenderPassCreationInfo2 &creationInfo)
{
    assert(isDestroyed_);
    init_(creationInfo);
    CHECK_RESULT_THROW(helpers::createRenderPass2(device, creationInfo, samples_, renderPass_));
    CHECK_RESULT_THROW(createAttachmentImages(device,
                                              creationInfo.queueFamilyIndexCount,
                                              creationInfo.queueFamilyIndices,
                                              allocator,
                                              creationInfo.createDepthAttachment));
    CHECK_RESULT_THROW(createFramebuffers(device,
                                          creationInfo.createDepthAttachment,
                                          creationInfo.framebufferAttachmentCount,
                                          creationInfo.framebufferAttachments));
    isDestroyed_ = false;
}

void RenderPass::destroy(const VkDevice device, const VmaAllocator &allocator)
{
    if (isDestroyed_)
    {
        return;
    }
    vkDestroyImageView(device, colorImageView_, nullptr);
    vkDestroyImageView(device, depthImageView_, nullptr);
    vmaDestroyImage(allocator, colorImage_, colorImageAllocation_);
    vmaDestroyImage(allocator, depthImage_, depthImageAllocation_);
    vkDestroyRenderPass(device, renderPass_, nullptr);
    for (const VkFramebuffer &framebuffer: framebuffers_)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    name_.clear();
    name_.shrink_to_fit();
    subpassMap_.clear();
    framebuffers_.clear();
    framebuffers_.shrink_to_fit();

    isDestroyed_ = true;
}

inline VkResult RenderPass::createAttachmentImages(const VkDevice device,
                                                   const uint32_t queueFamilyIndexCount,
                                                   const uint32_t *queueFamilyIndices,
                                                   const VmaAllocator &allocator,
                                                   const bool createDepthImage)
{
    constexpr VmaAllocationCreateInfo allocationCreateInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    if (samples_ != VK_SAMPLE_COUNT_1_BIT)
    {
        const VkImageUsageFlags imageUsage = (swapchain.imageUsage & ~(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0
                                                     ? swapchain.imageUsage | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
                                                     : swapchain.imageUsage;
        const VkImageCreateInfo colorImageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = swapchain.format.format,
            .extent = maxExtent_,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = samples_,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = imageUsage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = queueFamilyIndexCount,
            .pQueueFamilyIndices = queueFamilyIndices,
        };
        CHECK_RESULT_RETURN(vmaCreateImage(allocator,
                                           &colorImageCreateInfo,
                                           &allocationCreateInfo,
                                           &colorImage_,
                                           &colorImageAllocation_,
                                           nullptr));
        CHECK_RESULT_RETURN(helpers::createImageView(device,
                                                     colorImage_,
                                                     swapchain.format.format,
                                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                                     1,
                                                     &colorImageView_));
    }

    if (createDepthImage)
    {
        const VkImageCreateInfo depthImageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depthImageFormat,
            .extent = maxExtent_,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = samples_,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = queueFamilyIndexCount,
            .pQueueFamilyIndices = queueFamilyIndices,
        };
        CHECK_RESULT_RETURN(vmaCreateImage(allocator,
                                           &depthImageCreateInfo,
                                           &allocationCreateInfo,
                                           &depthImage_,
                                           &depthImageAllocation_,
                                           nullptr));
        switch (depthImageFormat)
        {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
                CHECK_RESULT_RETURN(helpers::createImageView(device,
                                                             depthImage_,
                                                             depthImageFormat,
                                                             VK_IMAGE_ASPECT_DEPTH_BIT,
                                                             1,
                                                             &depthImageView_));
                break;
            case VK_FORMAT_S8_UINT:
                CHECK_RESULT_RETURN(helpers::createImageView(device,
                                                             depthImage_,
                                                             depthImageFormat,
                                                             VK_IMAGE_ASPECT_STENCIL_BIT,
                                                             1,
                                                             &depthImageView_));
                break;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                CHECK_RESULT_RETURN(helpers::createImageView(device,
                                                             depthImage_,
                                                             depthImageFormat,
                                                             VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                                             1,
                                                             &depthImageView_));
                break;
            default:
                assert(depthImageFormat == VK_FORMAT_D16_UNORM ||
                       depthImageFormat == VK_FORMAT_X8_D24_UNORM_PACK32 ||
                       depthImageFormat == VK_FORMAT_D32_SFLOAT ||
                       depthImageFormat == VK_FORMAT_S8_UINT ||
                       depthImageFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
                       depthImageFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
                       depthImageFormat == VK_FORMAT_D32_SFLOAT_S8_UINT);
                return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }
    }

    return VK_SUCCESS;
}

inline VkResult RenderPass::createFramebuffers(const VkDevice device,
                                               const bool createDepthAttachment,
                                               const uint32_t framebufferAttachmentCount,
                                               const VkImageView *framebufferAttachments)
{
    const uint32_t attachmentCount = framebufferAttachmentCount +
                                     static_cast<uint32_t>(createDepthAttachment) +
                                     static_cast<uint32_t>(samples_ != VK_SAMPLE_COUNT_1_BIT) +
                                     1;
    attachments_.reserve(attachmentCount);
    for (uint32_t i = 0; i < framebufferAttachmentCount; i++)
    {
        attachments_.emplace_back(framebufferAttachments[i]);
    }

    if (createDepthAttachment)
    {
        attachments_.emplace_back(depthImageView_);
    }
    if (samples_ != VK_SAMPLE_COUNT_1_BIT)
    {
        attachments_.emplace_back(colorImageView_);
    }
    attachments_.emplace_back(swapchain.imageViews.at(0));
    framebuffers_.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount - 1; i++)
    {
        const VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass_,
            .attachmentCount = attachmentCount,
            .pAttachments = attachments_.data(),
            .width = extent_.width,
            .height = extent_.height,
            .layers = 1,
        };
        CHECK_RESULT_RETURN(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers_.at(i)));
        attachments_.back() = swapchain.imageViews.at(i + 1);
    }
    const VkFramebufferCreateInfo framebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass_,
        .attachmentCount = attachmentCount,
        .pAttachments = attachments_.data(),
        .width = extent_.width,
        .height = extent_.height,
        .layers = 1,
    };
    CHECK_RESULT_RETURN(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers_.back()));
    return VK_SUCCESS;
}

VkResult RenderPass::recreateFramebuffer(const VkDevice device,
                                         const uint32_t queueFamilyIndexCount,
                                         const uint32_t *queueFamilyIndices,
                                         const VmaAllocator &allocator,
                                         const uint32_t width,
                                         const uint32_t height)
{
    extent_.width = width;
    extent_.height = height;
    if (maxExtent_.width < width || maxExtent_.height < height)
    {
        maxExtent_.width = maxExtent_.width < width ? width : maxExtent_.width;
        maxExtent_.height = maxExtent_.height < height ? height : maxExtent_.height;

        vkDestroyImageView(device, colorImageView_, nullptr);
        vkDestroyImageView(device, depthImageView_, nullptr);
        vmaDestroyImage(allocator, colorImage_, colorImageAllocation_);
        vmaDestroyImage(allocator, depthImage_, depthImageAllocation_);
        CHECK_RESULT_RETURN(createAttachmentImages(device,
                                                   queueFamilyIndexCount,
                                                   queueFamilyIndices,
                                                   allocator,
                                                   depthImage_ != VK_NULL_HANDLE));
        if (samples_ != VK_SAMPLE_COUNT_1_BIT)
        {
            if (depthImage_ != VK_NULL_HANDLE)
            {
                *std::prev(std::prev(std::prev(attachments_.end()))) = depthImageView_;
            }
            *std::prev(std::prev(attachments_.end())) = colorImageView_;
        } else if (depthImage_ != VK_NULL_HANDLE)
        {
            *std::prev(std::prev(attachments_.end())) = depthImageView_;
        }
    }
    framebuffers_.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        VkFramebuffer &framebuffer = framebuffers_.at(i);
        vkDestroyFramebuffer(device, framebuffer, nullptr);
        attachments_.back() = swapchain.imageViews.at(i);
        const VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass_,
            .attachmentCount = static_cast<uint32_t>(attachments_.size()),
            .pAttachments = attachments_.data(),
            .width = width,
            .height = height,
            .layers = 1,
        };
        CHECK_RESULT_RETURN(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer));
    }
    return VK_SUCCESS;
}

VkResult RenderPass::begin(const VkDevice device,
                           CommandBuffer &commandBuffer,
                           const LunaRenderPassBeginInfo &beginInfo) const
{
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(device));

    uint32_t clearValueCount = 1;
    std::vector<VkClearValue> clearValues;
    clearValues.reserve(3);
    if (depthImage_ != VK_NULL_HANDLE)
    {
        clearValueCount++;
        clearValues.emplace_back(beginInfo.depthAttachmentClearValue);
    }
    if (samples_ != VK_SAMPLE_COUNT_1_BIT)
    {
        clearValueCount++;
        clearValues.emplace_back(beginInfo.colorAttachmentClearValue);
    }
    clearValues.emplace_back(beginInfo.colorAttachmentClearValue);
    const VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass_,
        .framebuffer = framebuffers_.at(swapchain.imageIndex),
        .renderArea = beginInfo.renderArea,
        .clearValueCount = clearValueCount,
        .pClearValues = clearValues.data(),
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return VK_SUCCESS;
}
} // namespace luna


VkResult lunaCreateRenderPass(const LunaDevice device,
                              const LunaRenderPassCreationInfo *creationInfo,
                              LunaRenderPass *renderPass)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createRenderPass(*creationInfo, renderPass));
    return VK_SUCCESS;
}

VkResult lunaCreateRenderPass2(const LunaDevice device,
                               const LunaRenderPassCreationInfo2 *creationInfo,
                               LunaRenderPass *renderPass)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createRenderPass(*creationInfo, renderPass));
    return VK_SUCCESS;
}

LunaRenderPassSubpass lunaGetRenderPassSubpassByName(const LunaRenderPass renderPass, const char *name)
{
    if (name == nullptr)
    {
        return luna::helpers::toHandle(luna::helpers::fromHandle<luna::RenderPass>(renderPass)->getUnnamedSubpass());
    }
    return luna::helpers::toHandle(
            luna::helpers::fromHandle<luna::RenderPass>(renderPass)->getSubpassIndexByName(name));
}

VkImage lunaGetRenderPassDepthImage(const LunaRenderPass renderPass)
{
    assert(renderPass != LUNA_NULL_HANDLE);
    return luna::helpers::fromHandle<luna::RenderPass>(renderPass)->depthImage();
}

VkResult lunaBeginRenderPass(const LunaDevice device,
                             const LunaCommandBuffer commandBuffer,
                             const LunaRenderPass renderPass,
                             const LunaRenderPassBeginInfo *beginInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(renderPass);
    assert(beginInfo);
    return luna::helpers::fromHandle<luna::RenderPass>(renderPass)
            ->begin(lunaGetVkDevice(device),
                    *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                    *beginInfo);
}

void lunaNextSubpass(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer)->isRecording());
    vkCmdNextSubpass(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer), VK_SUBPASS_CONTENTS_INLINE);
}

void lunaEndRenderPass(const LunaCommandBuffer commandBuffer)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);
    assert(luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer)->isRecording());
    vkCmdEndRenderPass(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer));
}
