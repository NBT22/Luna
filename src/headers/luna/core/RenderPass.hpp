//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace luna::core
{
struct RenderPassSubpassIndex
{
        uint32_t index;
        const class RenderPass *renderPass;
};

// TODO: Check if attachment is requested to be used without being created
// TODO: Support for multiple color attachments
class RenderPass
{
    public:
        static bool isDestroyed(const RenderPass &renderPass);

        friend VkResult(::lunaBeginRenderPass(LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo));

        RenderPass() = default;
        explicit RenderPass(const LunaRenderPassCreationInfo &creationInfo);
        explicit RenderPass(const LunaRenderPassCreationInfo2 &creationInfo);

        operator const VkRenderPass &() const;

        void destroy();

        const RenderPassSubpassIndex *getUnnamedSubpass() const;
        const RenderPassSubpassIndex *getSubpassIndexByName(const std::string &name) const;
        VkResult createAttachmentImages(bool createDepthImage);
        VkResult createFramebuffers(bool createDepthAttachment,
                                    uint32_t framebufferAttachmentCount,
                                    const VkImageView *framebufferAttachments);
        VkResult recreateFramebuffer(const Device &device, const Swapchain &swapchain, uint32_t width, uint32_t height);

    private:
        void init_(const LunaRenderPassCreationInfo &creationInfo);
        void init_(const LunaRenderPassCreationInfo2 &creationInfo);

        bool isDestroyed_{true};
        VkRenderPass renderPass_{};
        std::string name_{};
        RenderPassSubpassIndex unnamedSubpass_{};
        std::unordered_map<std::string, RenderPassSubpassIndex> subpassMap_{};
        VkSampleCountFlagBits samples_{};
        VkExtent3D extent_{};
        VkExtent3D maxExtent_{};
        VmaAllocation colorImageAllocation_{};
        VmaAllocation depthImageAllocation_{};
        VkImage colorImage_{};
        VkImage depthImage_{};
        VkImageView colorImageView_{};
        VkImageView depthImageView_{};
        std::vector<VkImageView> attachments_{};
        std::vector<VkFramebuffer> framebuffers_{};
};
} // namespace luna::core

#include <luna/implementations/core/RenderPass.ipp>
