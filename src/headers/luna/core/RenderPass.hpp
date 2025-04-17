//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <luna/luna.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>

namespace luna::core
{
struct RenderPassIndex
{
        uint32_t index;
};

struct RenderPassSubpassIndex
{
        uint32_t index;
        const RenderPassIndex *renderPassIndex;
};

// TODO: Check if attachment is requested to be used without being created
// TODO: Support for multiple color attachments
class RenderPass
{
    public:
        static bool isDestroyed(const RenderPass &renderPass);

        friend VkResult(::lunaBeginRenderPass(LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo));

        RenderPass() = default;
        RenderPass(const LunaRenderPassCreationInfo *creationInfo,
                   const LunaRenderPassCreationInfo2 *creationInfo2,
                   const RenderPassIndex *renderPassIndex);

        void destroy();

        const RenderPassSubpassIndex *getFirstSubpass() const;
        const RenderPassSubpassIndex *getSubpassIndexByName(const std::string &name) const;
        VkResult createAttachmentImages(bool createDepthImage);
        VkResult createSwapChainFramebuffers(VkRenderPass renderPass,
                                             uint32_t attachmentCount,
                                             std::vector<VkImageView> &attachmentImages) const;

        [[nodiscard]] VkRenderPass renderPass() const;

    private:
        bool isDestroyed_{true};
        VkRenderPass renderPass_{};
        std::string name_{};
        std::vector<RenderPassSubpassIndex> subpassIndices_{};
        std::unordered_map<std::string, uint32_t> subpassMap_{};
        VkSampleCountFlagBits samples_{};
        VkExtent3D extent_{};
        VmaAllocation colorImageAllocation_{};
        VmaAllocation depthImageAllocation_{};
        VkImage colorImage_{};
        VkImage depthImage_{};
        VkImageView colorImageView_{};
        VkImageView depthImageView_{};
};
} // namespace luna::core

#include <luna/implementations/core/RenderPass.ipp>
