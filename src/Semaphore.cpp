//
// Created by NBT22 on 3/29/26.
//

#include <cassert>
#include <luna/lunaSynchronization.h>
#include <luna/lunaTypes.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"
#include "Luna.hpp"

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

VkResult lunaCreateSemaphore(const LunaDevice device,
                             const LunaSemaphoreCreationInfo *creationInfo,
                             LunaSemaphore *semaphore)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo);

    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createSemaphore(*creationInfo, semaphore));
    return VK_SUCCESS;
}

void lunaDestroySemaphore(const LunaDevice device, const LunaSemaphore semaphore)
{
    assert(device != LUNA_NULL_HANDLE);
    luna::helpers::fromHandle<luna::Device>(device)->destroySemaphore(semaphore);
}
