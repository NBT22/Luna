//
// Created by NBT22 on 3/29/26.
//

#include <vulkan/vulkan_core.h>
#include "Fence.hpp"
#include "Instance.hpp"
#include "Luna.hpp"

namespace luna
{
Fence::Fence(const VkFenceCreateInfo &fenceCreateInfo)
{
    CHECK_RESULT_THROW(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence_));
}

void Fence::destroy()
{
    if (device.isDestroyed() || fence_ == VK_NULL_HANDLE)
    {
        return;
    }
    vkDestroyFence(device, fence_, nullptr);
    fence_ = VK_NULL_HANDLE;
}
} // namespace luna
