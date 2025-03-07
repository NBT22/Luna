//
// Created by NBT22 on 2/17/25.
//

#pragma once

namespace luna::helpers
{
// TODO: VkImageViewCreateInfo can be extended with structs in the core spec
inline void createImageView(const VkDevice logicalDevice,
							const VkImage image,
							const VkFormat format,
							const VkImageAspectFlags aspectMask,
							const uint8_t mipmapLevels,
							VkImageView &imageView)
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
	vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageView);
}
} // namespace luna::helpers
