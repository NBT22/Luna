//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <luna/core/Luna.hpp>

namespace luna::core
{
inline Fence::Fence(const VkDevice logicalDevice, const VkFenceCreateInfo *fenceCreateInfo)
{
    vkCreateFence(logicalDevice, fenceCreateInfo, nullptr, &fence_);
}

inline Fence::operator const VkFence &() const
{
    return fence_;
}
inline const VkFence *Fence::operator&() const
{
    return &fence_;
}
inline VkFence *Fence::operator&()
{
    return &fence_;
}

inline void Fence::destroy(const VkDevice logicalDevice) const
{
    vkDestroyFence(logicalDevice, fence_, nullptr);
}

inline void Fence::setWillBeSignaled(const bool value)
{
    willBeSignaled_ = value;
}

inline bool Fence::willBeSignaled() const
{
    return willBeSignaled_;
}
} // namespace luna::core
