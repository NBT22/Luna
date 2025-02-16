//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <luna/core/Device.hpp>
#include <luna/lunaTypes.h>

namespace luna::core
{
inline void Instance::addNewDevice(const LunaDeviceCreationInfo2 &creationInfo)
{
	device_ = Device(creationInfo);
}

inline void Instance::querySwapChainSupport()
{
	// vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_.device(), surface_, &swapChainSupport_.capabilities);
	// vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_.device(), surface_, &swapChainSupport_.formatCount, nullptr);
	// if (swapChainSupport_.formatCount != 0)
	// {
	// 	free(swapChainSupport_.formats);
	// 	swapChainSupport_.formats = malloc(sizeof(*swapChainSupport_.formats) * swapChainSupport_.formatCount);
	// 	CheckAlloc(swapChainSupport_.formats);
	// 	VulkanTest(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_.device(),
	// 													surface_,
	// 													&swapChainSupport_.formatCount,
	// 													swapChainSupport_.formats),
	// 			   "Failed to query Vulkan surface color formats!");
	// }
	//
	// VulkanTest(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_.device(),
	// 													 surface_,
	// 													 &swapChainSupport_.presentModeCount,
	// 													 nullptr),
	// 		   "Failed to query Vulkan surface presentation modes!");
	// if (swapChainSupport_.presentModeCount != 0)
	// {
	// 	free(swapChainSupport_.presentMode);
	// 	swapChainSupport_.presentMode = calloc(swapChainSupport_.presentModeCount,
	// 										   sizeof(*swapChainSupport_.presentMode));
	// 	CheckAlloc(swapChainSupport_.presentMode);
	// 	VulkanTest(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_.device(),
	// 														 surface_,
	// 														 &swapChainSupport_.presentModeCount,
	// 														 swapChainSupport_.presentMode),
	// 			   "Failed to query Vulkan surface presentation modes!");
	// }
}

[[nodiscard]] inline uint32_t Instance::minorVersion() const
{
	return VK_API_VERSION_MINOR(apiVersion_);
}
[[nodiscard]] inline VkInstance Instance::instance() const
{
	return instance_;
}
[[nodiscard]] inline VkSurfaceKHR Instance::surface() const
{
	return surface_;
}
[[nodiscard]] inline Device Instance::device() const
{
	return device_;
}
} // namespace luna::core
