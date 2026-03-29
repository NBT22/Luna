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

Fence::~Fence()
{
    vkDestroyFence(device, fence_, nullptr);
}
} // namespace luna
