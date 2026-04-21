//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <cstdint>
#include <luna/lunaTypes.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace luna
{
struct RenderPassSubpassIndex
{
        uint32_t index;
        const RenderPass *renderPass;
};

// TODO: Check if attachment is requested to be used without being created
// TODO: Support for multiple color attachments
class RenderPass
{
    public:
        static bool isDestroyed(const RenderPass &renderPass);

        RenderPass() = default;
        explicit RenderPass(VkDevice device,
                            const VmaAllocator &allocator,
                            const LunaRenderPassCreationInfo &creationInfo);
        explicit RenderPass(VkDevice device,
                            const VmaAllocator &allocator,
                            const LunaRenderPassCreationInfo2 &creationInfo);

        operator const VkRenderPass &() const;

        void destroy(VkDevice device, const VmaAllocator &allocator);

        [[nodiscard]] VkResult createAttachmentImages(VkDevice device,
                                                      uint32_t queueFamilyIndexCount,
                                                      const uint32_t *queueFamilyIndices,
                                                      const VmaAllocator &allocator,
                                                      bool createDepthImage);
        [[nodiscard]] VkResult createFramebuffers(VkDevice device,
                                                  bool createDepthAttachment,
                                                  uint32_t framebufferAttachmentCount,
                                                  const VkImageView *framebufferAttachments);
        [[nodiscard]] VkResult recreateFramebuffer(VkDevice device,
                                                   uint32_t queueFamilyIndexCount,
                                                   const uint32_t *queueFamilyIndices,
                                                   const VmaAllocator &allocator,
                                                   uint32_t width,
                                                   uint32_t height);
        [[nodiscard]] VkResult begin(VkDevice device,
                                     CommandBuffer &commandBuffer,
                                     const LunaRenderPassBeginInfo &beginInfo) const;

        [[nodiscard]] const RenderPassSubpassIndex &getUnnamedSubpass() const;
        [[nodiscard]] RenderPassSubpassIndex *getSubpassIndexByName(const std::string &name);

    private:
        void init_(const LunaRenderPassCreationInfo &creationInfo);
        void init_(const LunaRenderPassCreationInfo2 &creationInfo);

        bool isDestroyed_{true};
        VkRenderPass renderPass_{};
        std::string name_{};
        RenderPassSubpassIndex unnamedSubpass_{};
        std::unordered_map<std::string, RenderPassSubpassIndex> subpassMap_{};
        VkSampleCountFlagBits samples_{VK_SAMPLE_COUNT_1_BIT};
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
} // namespace luna

#pragma region Implementation

#include <cassert>
#include <stdexcept>
#include <volk.h>

namespace luna
{
inline bool RenderPass::isDestroyed(const RenderPass &renderPass)
{
    return renderPass.isDestroyed_;
}

inline RenderPass::operator const VkRenderPass &() const
{
    return renderPass_;
}

inline const RenderPassSubpassIndex &RenderPass::getUnnamedSubpass() const
{
    return unnamedSubpass_;
}
inline RenderPassSubpassIndex *RenderPass::getSubpassIndexByName(const std::string &name)
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
} // namespace luna

#pragma endregion Implementation
