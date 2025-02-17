//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <luna/core/Device.hpp>
#include <luna/lunaTypes.h>

#define clamp(val, min, max) ((val) < (min) ? (min) : (val) > (max) ? (max) : (val))

namespace luna::core
{
inline void Instance::addNewDevice(const LunaDeviceCreationInfo2 &creationInfo)
{
	device_ = Device(creationInfo);
}
inline void Instance::createSwapChain(const LunaSwapChainCreationInfo &creationInfo)
{
	if (minimized)
	{
		return;
	}

	surface_ = creationInfo.surface;

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_.physicalDevice(), surface_, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device_.physicalDevice(), surface_, &formatCount, nullptr);
	if (formatCount == 0)
	{
		return;
	}
	VkSurfaceFormatKHR formats[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(device_.physicalDevice(), surface_, &formatCount, formats);

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device_.physicalDevice(), surface_, &presentModeCount, nullptr);
	if (presentModeCount == 0)
	{
		return;
	}
	VkPresentModeKHR presentModes[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(device_.physicalDevice(), surface_, &presentModeCount, presentModes);

	if (!capabilities.currentExtent.width || !capabilities.currentExtent.height)
	{
		minimized = true;
		return;
	}

	uint32_t pQueueFamilyIndices[device_.familyCount()];
	switch (device_.familyCount())
	{
		case 1:
			pQueueFamilyIndices[0] = device_.graphicsFamily();
			break;
		case 2:
			pQueueFamilyIndices[0] = device_.graphicsFamily();
			pQueueFamilyIndices[1] = device_.hasTransfer() ? device_.transferFamily() : device_.presentationFamily();
			break;
		case 3:
			pQueueFamilyIndices[0] = device_.graphicsFamily();
			pQueueFamilyIndices[1] = device_.presentationFamily();
			pQueueFamilyIndices[2] = device_.transferFamily();
			break;
		default:
			assert(device_.familyCount() == 1 || device_.familyCount() == 2 || device_.familyCount() == 3);
	}

	swapChain_.format = {.format = VK_FORMAT_MAX_ENUM, .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR};
	for (uint32_t i = 0; i < creationInfo.formatCount; i++)
	{
		const auto [targetFormat, targetColorSpace] = creationInfo.formatPriorityList[i];
		for (uint32_t j = 0; j < formatCount; j++)
		{
			const VkSurfaceFormatKHR format = formats[j];
			if (format.colorSpace == targetColorSpace && format.format == targetFormat)
			{
				swapChain_.format = format;
				break;
			}
		}
		if (swapChain_.format.format != VK_FORMAT_MAX_ENUM &&
			swapChain_.format.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR)
		{
			break;
		}
	}
	assert(swapChain_.format.format != VK_FORMAT_MAX_ENUM &&
		   swapChain_.format.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR);

	swapChain_.extent = capabilities.currentExtent;
	if (swapChain_.extent.width == UINT32_MAX || swapChain_.extent.height == UINT32_MAX)
	{
		swapChain_.extent.width = creationInfo.width;
		swapChain_.extent.height = creationInfo.height;
	}
	assert(capabilities.minImageExtent.width <= swapChain_.extent.width &&
		   swapChain_.extent.width <= capabilities.maxImageExtent.width);
	assert(capabilities.minImageExtent.height <= swapChain_.extent.height &&
		   swapChain_.extent.height <= capabilities.maxImageExtent.height);

	swapChain_.presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	for (uint32_t i = 0; i < creationInfo.presentModeCount; i++)
	{
		const VkPresentModeKHR mode = presentModes[i];
		for (uint32_t j = 0; j < presentModeCount; j++)
		{
			if (presentModes[j] == mode)
			{
				swapChain_.presentMode = mode;
				break;
			}
		}
		if (swapChain_.presentMode != VK_PRESENT_MODE_MAX_ENUM_KHR)
		{
			break;
		}
	}
	assert(swapChain_.presentMode != VK_PRESENT_MODE_MAX_ENUM_KHR);

	swapChain_.imageCount = creationInfo.minImageCount;
	assert(capabilities.minImageCount <= swapChain_.imageCount && swapChain_.imageCount <= capabilities.maxImageCount);

	const VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface_,
		.minImageCount = swapChain_.imageCount,
		.imageFormat = swapChain_.format.format,
		.imageColorSpace = swapChain_.format.colorSpace,
		.imageExtent = swapChain_.extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = device_.hasPresentation() ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = device_.familyCount(),
		.pQueueFamilyIndices = pQueueFamilyIndices,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = swapChain_.presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};
	vkCreateSwapchainKHR(device_.logicalDevice(), &createInfo, nullptr, &swapChain_.swapChain);

	vkGetSwapchainImagesKHR(device_.logicalDevice(), swapChain_.swapChain, &swapChain_.imageCount, nullptr);
	swapChain_.images = static_cast<VkImage *>(calloc(swapChain_.imageCount, sizeof(VkImage)));
	vkGetSwapchainImagesKHR(device_.logicalDevice(), swapChain_.swapChain, &swapChain_.imageCount, swapChain_.images);
}

inline uint32_t Instance::minorVersion() const
{
	return VK_API_VERSION_MINOR(apiVersion_);
}
inline VkInstance Instance::instance() const
{
	return instance_;
}
inline Device Instance::device() const
{
	return device_;
}
inline VkSurfaceKHR Instance::surface() const
{
	return surface_;
}
} // namespace luna::core
