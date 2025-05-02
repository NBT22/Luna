//
// Created by NBT22 on 5/1/25.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace luna::core
{
class Semaphore
{
    public:
        Semaphore() = default;
        explicit Semaphore(VkDevice logicalDevice, const VkSemaphoreCreateInfo *semaphoreCreateInfo);

        operator const VkSemaphore &() const;
        const VkSemaphore *operator &() const;

        void destroy(VkDevice logicalDevice) const;

        void setIsSignaled(bool value);
        void setStageMask(VkPipelineStageFlags value);

        [[nodiscard]] bool isSignaled() const;
        [[nodiscard]] const VkPipelineStageFlags &stageMask() const;

    private:
        bool isDestroyed_{true};
        bool isSignaled_{};
        VkPipelineStageFlags stageMask_{};
        VkSemaphore semaphore_{};
};
} // namespace luna::core

#include <luna/implementations/core/Semaphore.ipp>
