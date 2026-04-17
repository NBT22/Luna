//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace luna
{
class Semaphore
{
    public:
        Semaphore() = default;
        explicit Semaphore(const VkSemaphoreCreateInfo &semaphoreCreateInfo);

        operator const VkSemaphore &() const;
        const VkSemaphore *operator&() const;
        VkSemaphore *operator&();

        void destroy();

        VkResult create();

        void setIsSignaled(bool value);
        void setStageMask(VkPipelineStageFlags value);

        [[nodiscard]] bool isSignaled() const;
        [[nodiscard]] const VkPipelineStageFlags &stageMask() const;
        [[nodiscard]] const VkSemaphore &semaphore() const;

    private:
        bool isSignaled_{};
        VkPipelineStageFlags stageMask_{};
        VkSemaphore semaphore_{};
};
} // namespace luna

#pragma region Implementation

namespace luna
{
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
inline const VkSemaphore &Semaphore::semaphore() const
{
    return semaphore_;
}
} // namespace luna

#pragma endregion Implementation
