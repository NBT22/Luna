//
// Created by NBT22 on 2/17/25.
//

#pragma once

#include <vulkan/vulkan.h>

namespace luna::helpers
{
// TODO: VkImageViewCreateInfo can be extended with structs in the core spec
void createImageView(VkDevice logicalDevice,
					 VkImage image,
					 VkFormat format,
					 VkImageAspectFlags aspectMask,
					 uint8_t mipmapLevels,
					 VkImageView &imageView);
} // namespace luna::helpers

#include <luna/implementations/helpers/Luna.ipp>
