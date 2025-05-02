//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <luna/core/Instance.hpp>

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

inline const RenderPassSubpassIndex *RenderPass::getFirstSubpass() const
{
    assert(!subpassIndices_.empty());
    return subpassIndices_.data();
}
inline const RenderPassSubpassIndex *RenderPass::getSubpassIndexByName(const std::string &name) const
{
    assert(!subpassMap_.empty());
    try
    {
        return &subpassIndices_.at(subpassMap_.at(name));
    } catch (const std::out_of_range &)
    {
        assert(subpassMap_.contains(name));
        return nullptr;
    }
}

inline void RenderPass::init_(const LunaRenderPassCreationInfo &creationInfo, const RenderPassIndex *renderPassIndex)
{
    extent_ = creationInfo.extent;
    subpassIndices_.reserve(creationInfo.subpassCount);
    samples_ = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;

    for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
    {
        if (creationInfo.subpasses[i].name != nullptr)
        {
            subpassMap_[creationInfo.subpasses[i].name] = i;
        }
        subpassIndices_.emplace_back(i, renderPassIndex);
    }
}
inline void RenderPass::init_(const LunaRenderPassCreationInfo2 &creationInfo, const RenderPassIndex *renderPassIndex)
{
    extent_ = creationInfo.extent;
    subpassIndices_.reserve(creationInfo.subpassCount);
    samples_ = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;

    for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
    {
        if (creationInfo.subpasses[i].name != nullptr)
        {
            subpassMap_[creationInfo.subpasses[i].name] = i;
        }
        subpassIndices_.emplace_back(i, renderPassIndex);
    }
}
} // namespace luna::core
