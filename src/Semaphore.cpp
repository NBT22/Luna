//
// Created by NBT22 on 3/29/26.
//

#include <cassert>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "Semaphore.hpp"

namespace luna
{
Semaphore::Semaphore(const VkDevice device, const VkSemaphoreCreateInfo &semaphoreCreateInfo)
{
    CHECK_RESULT_THROW(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore_));
}

void Semaphore::destroy(const VkDevice device)
{
    vkDestroySemaphore(device, semaphore_, nullptr);
    semaphore_ = VK_NULL_HANDLE;
}

VkResult Semaphore::create(const VkDevice device)
{
    assert(semaphore_ == VK_NULL_HANDLE);

    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    CHECK_RESULT_RETURN(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore_));
    return VK_SUCCESS;
}
} // namespace luna
