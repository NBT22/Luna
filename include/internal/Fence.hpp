//
// Created by NBT22 on 6/3/25.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace luna
{
class Fence
{
    public:
        Fence() = default;
        explicit Fence(VkDevice device, const VkFenceCreateInfo &fenceCreateInfo);

        operator const VkFence &() const;
        const VkFence *operator&() const;
        VkFence *operator&();

        void destroy(VkDevice device);

        void setWillBeSignaled(bool value);

        [[nodiscard]] bool willBeSignaled() const;

    private:
        bool willBeSignaled_{};
        VkFence fence_{};
};
} // namespace luna

#pragma region Implementation

#include <volk.h>

namespace luna
{
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

inline void Fence::setWillBeSignaled(const bool value)
{
    willBeSignaled_ = value;
}

inline bool Fence::willBeSignaled() const
{
    return willBeSignaled_;
}
} // namespace luna

#pragma endregion Implementation
