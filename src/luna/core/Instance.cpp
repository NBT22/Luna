//
// Created by NBT22 on 2/13/25.
//

#include <cstring>
#include <luna/core/Instance.hpp>
#include <luna/helpers/Luna.hpp>
#include <luna/lunaInstance.h>
#include <stdexcept>

namespace luna::helpers
{
static void initQueueFamilyIndices(const core::Device &device, uint32_t *queueFamilyIndices)
{
	switch (device.familyCount())
	{
		case 1:
			queueFamilyIndices[0] = device.graphicsFamily();
			break;
		case 2:
			queueFamilyIndices[0] = device.graphicsFamily();
			queueFamilyIndices[1] = device.hasTransfer() ? device.transferFamily() : device.presentationFamily();
			break;
		case 3:
			queueFamilyIndices[0] = device.graphicsFamily();
			queueFamilyIndices[1] = device.presentationFamily();
			queueFamilyIndices[2] = device.transferFamily();
			break;
		default:
			assert(device.familyCount() == 1 || device.familyCount() == 2 || device.familyCount() == 3);
	}
}

static void findSwapChainFormat(const VkPhysicalDevice physicalDevice,
								const VkSurfaceKHR surface,
								const uint32_t targetFormatCount,
								VkSurfaceFormatKHR *targetFormats,
								VkSurfaceFormatKHR &destination)
{
	destination = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR};
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount == 0)
	{
		return;
	}
	VkSurfaceFormatKHR *formats = new VkSurfaceFormatKHR[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);
	for (uint32_t i = 0; i < targetFormatCount; i++)
	{
		const auto [targetFormat, targetColorSpace] = targetFormats[i];
		for (uint32_t j = 0; j < formatCount; j++)
		{
			const VkSurfaceFormatKHR format = formats[j];
			if (format.colorSpace == targetColorSpace && format.format == targetFormat)
			{
				destination = format;
				break;
			}
		}
		if (destination.format != VK_FORMAT_UNDEFINED && destination.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR)
		{
			break;
		}
	}
	delete[] formats;
	if (destination.format == VK_FORMAT_UNDEFINED || destination.colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR)
	{
		throw std::runtime_error("Unable to find suitable Vulkan surface format!");
	}
}

static void createSwapChainImages(const VkDevice logicalDevice, core::SwapChain swapChain)
{
	vkGetSwapchainImagesKHR(logicalDevice, swapChain.swapChain, &swapChain.imageCount, nullptr);

	swapChain.images = new VkImage[swapChain.imageCount];
	vkGetSwapchainImagesKHR(logicalDevice, swapChain.swapChain, &swapChain.imageCount, swapChain.images);

	swapChain.imageViews = new VkImageView[swapChain.imageCount];
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		createImageView(logicalDevice,
						swapChain.images[i],
						swapChain.format.format,
						VK_IMAGE_ASPECT_COLOR_BIT,
						1,
						swapChain.imageViews[i]);
	}
}

static void getSwapChainPresentMode(const VkPhysicalDevice physicalDevice,
									const VkSurfaceKHR surface,
									const uint32_t targetPresentModeCount,
									const VkPresentModeKHR *targetPresentModes,
									VkPresentModeKHR &destination)
{
	if (targetPresentModeCount == 0)
	{
		destination = VK_PRESENT_MODE_FIFO_KHR;
		return;
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount == 0)
	{
		return;
	}
	VkPresentModeKHR *presentModes = new VkPresentModeKHR[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
	destination = VK_PRESENT_MODE_MAX_ENUM_KHR;
	for (uint32_t i = 0; i < targetPresentModeCount; i++)
	{
		const VkPresentModeKHR mode = targetPresentModes[i];
		for (uint32_t j = 0; j < presentModeCount; j++)
		{
			if (presentModes[j] == mode)
			{
				destination = mode;
				break;
			}
		}
		if (destination != VK_PRESENT_MODE_MAX_ENUM_KHR)
		{
			break;
		}
	}
	delete[] presentModes;
	// This is an assert instead of an error because VK_PRESENT_MODE_FIFO_KHR is required to be supported.
	assert(destination != VK_PRESENT_MODE_MAX_ENUM_KHR);
}
} // namespace luna::helpers

namespace luna::core
{
Instance instance;
Instance::Instance(const LunaInstanceCreationInfo &creationInfo)
{
	apiVersion_ = creationInfo.apiVersion;

	const uint32_t enabledLayerCount = creationInfo.enableValidation ? creationInfo.layerCount + 1
																	 : creationInfo.layerCount;
	const char **enabledLayers = new const char *[enabledLayerCount];
	for (uint32_t i = 0; i < creationInfo.layerCount; i++)
	{
		enabledLayers[i] = creationInfo.layerNames[i];
	}
	if (creationInfo.enableValidation)
	{
		enabledLayers[enabledLayerCount - 1] = "VK_LAYER_KHRONOS_validation";
	}

	bool surfaceExtensionRequested = false;
	for (uint32_t i = 0; i < creationInfo.extensionCount; i++)
	{
		if (std::strncmp(creationInfo.extensionNames[i], "VK_KHR_surface", 14) == 0)
		{
			surfaceExtensionRequested = true;
			break;
		}
	}
	assert(surfaceExtensionRequested);

	const VkApplicationInfo vulkanApplicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = creationInfo.apiVersion,
	};
	const VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &vulkanApplicationInfo,
		.enabledLayerCount = enabledLayerCount,
		.ppEnabledLayerNames = enabledLayers,
		.enabledExtensionCount = creationInfo.extensionCount,
		.ppEnabledExtensionNames = creationInfo.extensionNames,
	};
	vkCreateInstance(&createInfo, nullptr, &instance_);
	delete[] enabledLayers;
}

void Instance::createSwapChain(const LunaSwapChainCreationInfo &creationInfo)
{
	if (minimized)
	{
		return;
	}

	surface_ = creationInfo.surface;

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_.physicalDevice(), surface_, &capabilities);
	capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;

	if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0)
	{
		minimized = true;
		return;
	}

	uint32_t *queueFamilyIndices = new uint32_t[device_.familyCount()];
	helpers::initQueueFamilyIndices(device_, queueFamilyIndices);

	helpers::findSwapChainFormat(device_.physicalDevice(),
								 surface_,
								 creationInfo.formatCount,
								 creationInfo.formatPriorityList,
								 swapChain_.format);

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

	helpers::getSwapChainPresentMode(device_.physicalDevice(),
									 surface_,
									 creationInfo.presentModeCount,
									 creationInfo.presentModePriorityList,
									 swapChain_.presentMode);

	swapChain_.imageCount = creationInfo.minImageCount;
	assert(capabilities.minImageCount <= swapChain_.imageCount && swapChain_.imageCount <= capabilities.maxImageCount);

	const VkCompositeAlphaFlagBitsKHR compositeAlpha = creationInfo.compositeAlpha == 0
															   ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
															   : creationInfo.compositeAlpha;
	const VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface_,
		.minImageCount = swapChain_.imageCount,
		.imageFormat = swapChain_.format.format,
		.imageColorSpace = swapChain_.format.colorSpace,
		.imageExtent = swapChain_.extent,
		.imageArrayLayers = 1,
		.imageUsage = creationInfo.imageUsage == 0 ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : creationInfo.imageUsage,
		.imageSharingMode = device_.hasPresentation() ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = device_.familyCount(),
		.pQueueFamilyIndices = queueFamilyIndices,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = compositeAlpha,
		.presentMode = swapChain_.presentMode,
		.clipped = VK_TRUE, // TODO: Support applications being able to set this... somehow
	};
	vkCreateSwapchainKHR(device_.logicalDevice(), &createInfo, nullptr, &swapChain_.swapChain);
	delete[] queueFamilyIndices;

	helpers::createSwapChainImages(device_.logicalDevice(), swapChain_);
}
} // namespace luna::core

void lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo)
{
	assert(creationInfo);
	luna::core::instance = luna::core::Instance(*creationInfo);
}
VkInstance lunaGetInstance()
{
	return luna::core::instance.instance();
}
void lunaCreateSwapChain(const LunaSwapChainCreationInfo *creationInfo)
{
	assert(creationInfo);
	luna::core::instance.createSwapChain(*creationInfo);
}
VkFormat lunaGetSwapChainFormat()
{
	return luna::core::instance.swapChain().format.format;
}
VkExtent2D lunaGetSwapChainExtent()
{
	return luna::core::instance.swapChain().extent;
}
VkSurfaceCapabilitiesKHR lunaGetSurfaceCapabilities(const VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(luna::core::instance.device().physicalDevice(), surface, &capabilities);
	capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;
	return capabilities;
}
void lunaSetDepthImageFormat(const uint32_t formatCount, const VkFormat *formatPriorityList)
{
	assert(formatPriorityList);
	VkFormatProperties properties;
	for (uint32_t i = 0; i < formatCount; i++)
	{
		vkGetPhysicalDeviceFormatProperties(luna::core::instance.device().physicalDevice(),
											formatPriorityList[i],
											&properties);
		if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			luna::core::instance.depthImageFormat = formatPriorityList[i];
			return;
		}
	}
}
VkFormat lunaGetDepthImageFormat()
{
	return luna::core::instance.depthImageFormat;
}
