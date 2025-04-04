//
// Created by NBT22 on 3/11/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <vk_mem_alloc.h>

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

namespace luna::core
{
struct ImageIndex
{
		uint32_t index;
};

// TODO: Support for 1D images
class Image
{
	public:
		explicit Image(const LunaSampledImageCreationInfo &creationInfo, uint32_t depth, uint32_t arrayLayers);

		void destroy();

		[[nodiscard]] VkImageView imageView() const;
		[[nodiscard]] VkSampler sampler() const;

	private:
		bool isDestroyed_{true};
		VkImage image_{};
		VkImageView imageView_{};
		VmaAllocation allocation_{};
		VkSampler sampler_{};
};
} // namespace luna::core

#include <luna/implementations/core/Image.ipp>
