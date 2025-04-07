//
// Created by NBT22 on 3/11/25.
//

#pragma once

#include <luna/core/Luna.hpp>
#include "luna/core/Instance.hpp"

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

namespace luna::core
{
inline VkImageView Image::imageView() const
{
	return imageView_;
}
inline VkSampler Image::sampler() const
{
	return sampler_;
}
} // namespace luna::core
