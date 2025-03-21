//
// Created by NBT22 on 2/17/25.
//

#pragma once

#include <luna/core/DescriptorSetLayout.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace luna::core
{
struct SwapChain
{
		VkSurfaceFormatKHR format;
		VkExtent2D extent;
		VkPresentModeKHR presentMode;
		uint32_t imageCount;
		uint32_t imageIndex;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> framebuffers;
};
struct DescriptorPoolIndex
{
		uint32_t index;
};
struct DescriptorSetIndex
{
		uint32_t index;
		const DescriptorSetLayoutIndex *layoutIndex;
		const DescriptorPoolIndex *poolIndex;
};
struct SamplerIndex
{
		uint32_t index;
};
} // namespace luna::core
