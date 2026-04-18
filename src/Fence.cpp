//
// Created by NBT22 on 3/29/26.
//

#include <vulkan/vulkan_core.h>
#include "Fence.hpp"
#include "Luna.hpp"

namespace luna
{
Fence::Fence(const VkDevice device, const VkFenceCreateInfo &fenceCreateInfo)
{
    CHECK_RESULT_THROW(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence_));
}

void Fence::destroy(const VkDevice device)
{
    vkDestroyFence(static_cast<VkDevice>(device), fence_, nullptr);
    fence_ = VK_NULL_HANDLE;
}
} // namespace luna
