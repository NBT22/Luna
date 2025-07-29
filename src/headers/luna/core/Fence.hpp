//
// Created by NBT22 on 6/3/25.
//

#pragma once

#include <volk.h>

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

#include <luna/implementations/core/Fence.ipp>
