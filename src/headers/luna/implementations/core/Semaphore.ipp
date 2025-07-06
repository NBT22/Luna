//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <luna/core/Luna.hpp>

namespace luna::core
{
inline Semaphore::Semaphore(const VkDevice logicalDevice, const VkSemaphoreCreateInfo *semaphoreCreateInfo)
{
    if (semaphoreCreateInfo == nullptr)
    {
        return;
    }
    CHECK_RESULT_THROW(vkCreateSemaphore(logicalDevice, semaphoreCreateInfo, nullptr, &semaphore_));
}

inline Semaphore::operator const VkSemaphore &() const
{
    return semaphore_;
}
inline const VkSemaphore *Semaphore::operator&() const
{
    return &semaphore_;
}
inline VkSemaphore *Semaphore::operator&()
{
    return &semaphore_;
}

inline void Semaphore::destroy(const VkDevice logicalDevice) const
{
    vkDestroySemaphore(logicalDevice, semaphore_, nullptr);
}
inline VkResult Semaphore::recreate(const VkDevice logicalDevice, const VkSemaphoreCreateInfo *semaphoreCreateInfo)
{
    CHECK_RESULT_RETURN(vkCreateSemaphore(logicalDevice, semaphoreCreateInfo, nullptr, &semaphore_));
    return VK_SUCCESS;
}

inline void Semaphore::setIsSignaled(const bool value)
{
    isSignaled_ = value;
}
inline void Semaphore::setStageMask(const VkPipelineStageFlags value)
{
    stageMask_ = value;
}

inline bool Semaphore::isSignaled() const
{
    return isSignaled_;
}
inline const VkPipelineStageFlags &Semaphore::stageMask() const
{
    return stageMask_;
}
} // namespace luna::core
