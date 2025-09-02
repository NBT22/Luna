//
// Created by NBT22 on 6/3/25.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace luna::core
{
class Fence
{
    public:
        Fence() = default;
        Fence(VkDevice logicalDevice, const VkFenceCreateInfo *fenceCreateInfo);

        operator const VkFence &() const;
        const VkFence *operator&() const;
        VkFence *operator&();

        void destroy(VkDevice logicalDevice) const;

        void setWillBeSignaled(bool value);

        [[nodiscard]] bool willBeSignaled() const;

    private:
        bool willBeSignaled_{};
        VkFence fence_{};
};
} // namespace luna::core

#pragma region "Implmentation"

#include <volk.h>
#include "Luna.hpp"

namespace luna::core
{
inline Fence::Fence(const VkDevice logicalDevice, const VkFenceCreateInfo *fenceCreateInfo)
{
    CHECK_RESULT_THROW(vkCreateFence(logicalDevice, fenceCreateInfo, nullptr, &fence_));
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

#pragma endregion "Implmentation"
