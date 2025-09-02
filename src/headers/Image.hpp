//
// Created by NBT22 on 3/11/25.
//

#pragma once

#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace luna::helpers
{
// TODO: VkImageViewCreateInfo can be extended with structs in the core spec
VkResult createImageView(VkDevice logicalDevice,
                         VkImage image,
                         VkFormat format,
                         VkImageAspectFlags aspectMask,
                         uint8_t mipmapLevels,
                         VkImageView *imageView);
} // namespace luna::helpers

namespace luna
{
// TODO: Support for 1D images
class Image
{
    public:
        Image(const LunaSampledImageCreationInfo &creationInfo, uint32_t depth, uint32_t arrayLayers);

        void destroy() const;
        void erase(std::list<Image>::const_iterator iterator) const;

        [[nodiscard]] VkImageView imageView() const;
        [[nodiscard]] VkSampler sampler() const;
        [[nodiscard]] VkSampler sampler(LunaSampler sampler) const;

    private:
        VkImage image_{};
        VkImageView imageView_{};
        VmaAllocation allocation_{};
        VkSampler sampler_{};
};
} // namespace luna

#pragma region "Implmentation"

#include <volk.h>
#include "Luna.hpp"

namespace luna::helpers
{
inline VkResult createImageView(const VkDevice logicalDevice,
                                const VkImage image,
                                const VkFormat format,
                                const VkImageAspectFlags aspectMask,
                                const uint8_t mipmapLevels,
                                VkImageView *imageView)
{
    constexpr VkComponentMapping componentMapping = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    const VkImageSubresourceRange subresourceRange = {
        .aspectMask = aspectMask,
        .levelCount = mipmapLevels,
        .layerCount = 1,
    };
    const VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = componentMapping,
        .subresourceRange = subresourceRange,
    };
    CHECK_RESULT_RETURN(vkCreateImageView(logicalDevice, &createInfo, nullptr, imageView));
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna
{
inline VkImageView Image::imageView() const
{
    return imageView_;
}
inline VkSampler Image::sampler() const
{
    return sampler_;
}
} // namespace luna

#pragma endregion "Implmentation"
