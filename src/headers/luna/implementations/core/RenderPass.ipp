//
// Created by NBT22 on 2/18/25.
//

#pragma once

namespace luna::core
{
inline bool RenderPass::isDestroyed(const RenderPass &renderPass)
{
    return renderPass.isDestroyed_;
}

inline RenderPass::operator const VkRenderPass &() const
{
    return renderPass_;
}

inline const RenderPassSubpassIndex *RenderPass::getUnnamedSubpass() const
{
    return &unnamedSubpass_;
}
inline const RenderPassSubpassIndex *RenderPass::getSubpassIndexByName(const std::string &name) const
{
    assert(!subpassMap_.empty());
    try
    {
        return &subpassMap_.at(name);
    } catch (const std::out_of_range &)
    {
        assert(subpassMap_.contains(name));
        return nullptr;
    }
}
inline VkResult RenderPass::recreateFramebuffer(const Device &device,
                                                const SwapChain &swapchain,
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
        vmaDestroyImage(device.allocator(), colorImage_, colorImageAllocation_);
        vmaDestroyImage(device.allocator(), depthImage_, depthImageAllocation_);
        CHECK_RESULT_RETURN(createAttachmentImages(depthImage_ != VK_NULL_HANDLE));
        attachments_.at(attachments_.size() - 3) = depthImageView_;
        attachments_.at(attachments_.size() - 2) = colorImageView_;
    }
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

inline void RenderPass::init_(const LunaRenderPassCreationInfo &creationInfo)
{
    extent_ = creationInfo.extent;
    maxExtent_.width = creationInfo.maxExtent.width != 0 ? creationInfo.maxExtent.width : creationInfo.extent.width;
    maxExtent_.height = creationInfo.maxExtent.height != 0 ? creationInfo.maxExtent.height : creationInfo.extent.height;
    maxExtent_.depth = creationInfo.maxExtent.depth != 0 ? creationInfo.maxExtent.depth : creationInfo.extent.depth;
    samples_ = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;

    for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
    {
        const RenderPassSubpassIndex index = {
            .index = i,
            .renderPass = this,
        };
        if (creationInfo.subpasses[i].name != nullptr)
        {
            subpassMap_.emplace(creationInfo.subpasses[i].name, index);
        } else
        {
            unnamedSubpass_ = index;
        }
    }
}
inline void RenderPass::init_(const LunaRenderPassCreationInfo2 &creationInfo)
{
    extent_ = creationInfo.extent;
    maxExtent_.width = creationInfo.maxExtent.width != 0 ? creationInfo.maxExtent.width : creationInfo.extent.width;
    maxExtent_.height = creationInfo.maxExtent.height != 0 ? creationInfo.maxExtent.height : creationInfo.extent.height;
    maxExtent_.depth = creationInfo.maxExtent.depth != 0 ? creationInfo.maxExtent.depth : creationInfo.extent.depth;
    samples_ = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;

    for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
    {
        const RenderPassSubpassIndex index = {
            .index = i,
            .renderPass = this,
        };
        if (creationInfo.subpasses[i].name != nullptr)
        {
            subpassMap_.emplace(creationInfo.subpasses[i].name, index);
        } else
        {
            unnamedSubpass_ = index;
        }
    }
}
} // namespace luna::core
