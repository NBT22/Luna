//
// Created by NBT22 on 2/13/25.
//

#include <cstring>
#include <luna/core/Image.hpp>
#include <luna/core/Instance.hpp>
#include <luna/lunaInstance.h>
#include <stdexcept>

namespace luna::helpers
{
static VkResult findSwapChainFormat(const VkPhysicalDevice physicalDevice,
									const VkSurfaceKHR surface,
									const uint32_t targetFormatCount,
									const VkSurfaceFormatKHR *targetFormats,
									VkSurfaceFormatKHR &destination)
{
	destination = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR};
	uint32_t formatCount;
	CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
	if (formatCount == 0)
	{
		return VK_ERROR_UNKNOWN;
	}
	std::vector<VkSurfaceFormatKHR> formats;
	formats.reserve(formatCount);
	CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()));
	for (uint32_t i = 0; i < targetFormatCount; i++)
	{
		const VkSurfaceFormatKHR &targetFormat = targetFormats[i];
		for (uint32_t j = 0; j < formatCount; j++)
		{
			const VkSurfaceFormatKHR &format = formats[j];
			if (format.colorSpace == targetFormat.colorSpace && format.format == targetFormat.format)
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
	if (destination.format == VK_FORMAT_UNDEFINED || destination.colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR)
	{
		throw std::runtime_error("Unable to find suitable Vulkan surface format!");
	}
	return VK_SUCCESS;
}

static VkResult getSwapChainPresentMode(const VkPhysicalDevice physicalDevice,
										const VkSurfaceKHR surface,
										const uint32_t targetPresentModeCount,
										const VkPresentModeKHR *targetPresentModes,
										VkPresentModeKHR &destination)
{
	if (targetPresentModeCount == 0)
	{
		destination = VK_PRESENT_MODE_FIFO_KHR;
		return VK_SUCCESS;
	}
	uint32_t presentModeCount;
	CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
	if (presentModeCount == 0)
	{
		return VK_ERROR_UNKNOWN;
	}
	std::vector<VkPresentModeKHR> presentModes;
	presentModes.reserve(presentModeCount);
	CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
																  surface,
																  &presentModeCount,
																  presentModes.data()));
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
	// This is an assert instead of an error because VK_PRESENT_MODE_FIFO_KHR is required to be supported.
	assert(destination != VK_PRESENT_MODE_MAX_ENUM_KHR);
	return VK_SUCCESS;
}

static VkResult createSwapChainImages(const VkDevice logicalDevice, core::SwapChain &swapChain)
{
	CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(logicalDevice, swapChain.swapChain, &swapChain.imageCount, nullptr));

	swapChain.images.resize(swapChain.imageCount);
	CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(logicalDevice,
												swapChain.swapChain,
												&swapChain.imageCount,
												swapChain.images.data()));

	swapChain.imageViews.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		CHECK_RESULT_RETURN(createImageView(logicalDevice,
											swapChain.images[i],
											swapChain.format.format,
											VK_IMAGE_ASPECT_COLOR_BIT,
											1,
											&swapChain.imageViews[i]));
	}
	return VK_SUCCESS;
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
	std::vector<const char *> enabledLayers;
	enabledLayers.reserve(enabledLayerCount);
	for (uint32_t i = 0; i < creationInfo.layerCount; i++)
	{
		enabledLayers.emplace_back(creationInfo.layerNames[i]);
	}
	if (creationInfo.enableValidation)
	{
		enabledLayers.emplace_back("VK_LAYER_KHRONOS_validation");
	}

	[[maybe_unused]] bool surfaceExtensionRequested = false;
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
		.ppEnabledLayerNames = enabledLayers.data(),
		.enabledExtensionCount = creationInfo.extensionCount,
		.ppEnabledExtensionNames = creationInfo.extensionNames,
	};
	CHECK_RESULT_THROW(vkCreateInstance(&createInfo, nullptr, &instance_));
}

VkResult Instance::destroy()
{
	const VkDevice logicalDevice = device_.logicalDevice();
	CHECK_RESULT_RETURN(vkDeviceWaitIdle(logicalDevice));


	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		vkDestroyFramebuffer(logicalDevice, swapChain.framebuffers.at(i), nullptr);
		vkDestroyImageView(logicalDevice, swapChain.imageViews.at(i), nullptr);
	}
	vkDestroySwapchainKHR(logicalDevice, swapChain.swapChain, nullptr);

	for (const VkSampler sampler: samplers_)
	{
		vkDestroySampler(logicalDevice, sampler, nullptr);
	}
	for (Image image: images_)
	{
		image.destroy();
	}

	for (GraphicsPipeline pipeline: graphicsPipelines_)
	{
		pipeline.destroy();
	}
	for (RenderPass renderPass: renderPasses_)
	{
		renderPass.destroy();
	}

	for (const VkDescriptorPool descriptorPool: descriptorPools_)
	{
		vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
	}
	for (DescriptorSetLayout descriptorSetLayout: descriptorSetLayouts_)
	{
		descriptorSetLayout.destroy();
	}

	for (Buffer buffer: buffers_)
	{
		buffer.destroy();
	}

	device_.destroy();
	vkDestroySurfaceKHR(instance_, surface_, nullptr);
	vkDestroyInstance(instance_, nullptr);


	swapChain.images.clear();
	swapChain.images.shrink_to_fit();
	swapChain.imageViews.clear();
	swapChain.imageViews.shrink_to_fit();
	swapChain.framebuffers.clear();
	swapChain.framebuffers.shrink_to_fit();

	samplerIndices_.clear();
	samplers_.clear();
	samplers_.shrink_to_fit();
	imageIndices_.clear();
	images_.clear();
	images_.shrink_to_fit();

	graphicsPipelineIndices_.clear();
	graphicsPipelines_.clear();
	graphicsPipelines_.shrink_to_fit();
	renderPassIndices_.clear();
	renderPasses_.clear();
	renderPasses_.shrink_to_fit();

	descriptorPoolIndices_.clear();
	descriptorSetLayoutIndices_.clear();
	descriptorSetIndices_.clear();
	descriptorPools_.clear();
	descriptorPools_.shrink_to_fit();
	descriptorSetLayouts_.clear();
	descriptorSetLayouts_.shrink_to_fit();
	descriptorSets_.clear();
	descriptorSets_.shrink_to_fit();

	bufferRegionIndices_.clear();
	buffers_.clear();
	buffers_.shrink_to_fit();

	return VK_SUCCESS;
}

VkResult Instance::createSwapChain(const LunaSwapChainCreationInfo &creationInfo)
{
	surface_ = creationInfo.surface;

	VkSurfaceCapabilitiesKHR capabilities;
	CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_.physicalDevice(), surface_, &capabilities));
	capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;

	CHECK_RESULT_RETURN(helpers::findSwapChainFormat(device_.physicalDevice(),
													 surface_,
													 creationInfo.formatCount,
													 creationInfo.formatPriorityList,
													 swapChain.format));

	swapChain.extent = capabilities.currentExtent;
	if (swapChain.extent.width == UINT32_MAX || swapChain.extent.height == UINT32_MAX)
	{
		swapChain.extent.width = creationInfo.width;
		swapChain.extent.height = creationInfo.height;
	}
	assert(capabilities.minImageExtent.width <= swapChain.extent.width &&
		   swapChain.extent.width <= capabilities.maxImageExtent.width);
	assert(capabilities.minImageExtent.height <= swapChain.extent.height &&
		   swapChain.extent.height <= capabilities.maxImageExtent.height);

	CHECK_RESULT_RETURN(helpers::getSwapChainPresentMode(device_.physicalDevice(),
														 surface_,
														 creationInfo.presentModeCount,
														 creationInfo.presentModePriorityList,
														 swapChain.presentMode));

	swapChain.imageCount = creationInfo.minImageCount;
	assert(capabilities.minImageCount <= swapChain.imageCount && swapChain.imageCount <= capabilities.maxImageCount);

	const VkCompositeAlphaFlagBitsKHR compositeAlpha = creationInfo.compositeAlpha == 0
															   ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
															   : creationInfo.compositeAlpha;
	const VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface_,
		.minImageCount = swapChain.imageCount,
		.imageFormat = swapChain.format.format,
		.imageColorSpace = swapChain.format.colorSpace,
		.imageExtent = swapChain.extent,
		.imageArrayLayers = 1,
		.imageUsage = creationInfo.imageUsage == 0 ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : creationInfo.imageUsage,
		.imageSharingMode = device_.sharingMode(),
		.queueFamilyIndexCount = device_.familyCount(),
		.pQueueFamilyIndices = device_.queueFamilyIndices(),
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = compositeAlpha,
		.presentMode = swapChain.presentMode,
		.clipped = VK_TRUE, // TODO: Support applications being able to set this... somehow
	};
	CHECK_RESULT_RETURN(vkCreateSwapchainKHR(device_.logicalDevice(), &createInfo, nullptr, &swapChain.swapChain));

	CHECK_RESULT_RETURN(helpers::createSwapChainImages(device_.logicalDevice(), swapChain));
	swapChain.imageIndex = -1u;
	return VK_SUCCESS;
}
VkResult Instance::createImage(const LunaSampledImageCreationInfo &creationInfo,
							   uint32_t depth,
							   uint32_t arrayLayers,
							   LunaImage *imageIndex)
{
	assert(!creationInfo.descriptorSet || creationInfo.descriptorLayoutBindingName);
	LunaDescriptorSet descriptorSetIndex;
	const char *bindingName = creationInfo.descriptorLayoutBindingName != nullptr
									  ? creationInfo.descriptorLayoutBindingName
									  : "Image";
	imageIndices_.emplace_back(images_.size());
	TRY_CATCH_RESULT(images_.emplace_back(creationInfo, depth, arrayLayers));
	const Image image = images_.back();
	if (creationInfo.descriptorSet != nullptr)
	{
		descriptorSetIndex = creationInfo.descriptorSet;
	} else
	{
		const VkDescriptorType descriptorType = image.sampler() == nullptr ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
																		   : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		const LunaDescriptorSetLayoutBinding binding = {
			.bindingName = bindingName,
			.descriptorType = descriptorType,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};
		const LunaDescriptorSetLayoutCreationInfo descriptorSetLayoutCreationInfo = {
			.bindingCount = 1,
			.bindings = &binding,
		};
		LunaDescriptorSetLayout descriptorSetLayout;
		CHECK_RESULT_RETURN(createDescriptorSetLayout(descriptorSetLayoutCreationInfo, &descriptorSetLayout));

		const VkDescriptorPoolSize poolSize = {
			.type = descriptorType,
			.descriptorCount = 1,
		};
		// ReSharper disable once CppVariableCanBeMadeConstexpr It's wrong again; memory locations are not constant -_-
		const LunaDescriptorPoolCreationInfo descriptorPoolCreationInfo = {
			.maxSets = 1,
			.poolSizeCount = 1,
			.poolSizes = &poolSize,
		};
		LunaDescriptorPool descriptorPool;
		CHECK_RESULT_RETURN(createDescriptorPool(descriptorPoolCreationInfo, &descriptorPool));
		const LunaDescriptorSetAllocationInfo descriptorSetAllocationInfo = {
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.setLayouts = &descriptorSetLayout,
		};
		CHECK_RESULT_RETURN(allocateDescriptorSets(descriptorSetAllocationInfo, &descriptorSetIndex));
	}

	const VkDescriptorImageInfo imageInfo = {
		.sampler = image.sampler(),
		.imageView = image.imageView(),
		.imageLayout = creationInfo.layout,
	};
	DescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	Instance::descriptorSet(descriptorSetIndex, nullptr, &descriptorSetLayout, &descriptorSet);
	const DescriptorSetLayout::Binding binding = descriptorSetLayout.binding(bindingName);
	const VkWriteDescriptorSet writeDescriptor = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = binding.index,
		.descriptorCount = 1,
		.descriptorType = binding.type,
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(device_.logicalDevice(), 1, &writeDescriptor, 0, nullptr);
	if (imageIndex != nullptr)
	{
		*imageIndex = &imageIndices_.back();
	}
	return VK_SUCCESS;
}
} // namespace luna::core

VkResult lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo)
{
	assert(creationInfo);
	TRY_CATCH_RESULT(luna::core::instance = luna::core::Instance(*creationInfo));
	return VK_SUCCESS;
}
VkResult lunaDestroyInstance()
{
	return luna::core::instance.destroy();
}
VkInstance lunaGetInstance()
{
	return luna::core::instance.instance();
}
VkResult lunaCreateSwapChain(const LunaSwapChainCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::instance.createSwapChain(*creationInfo);
}
VkFormat lunaGetSwapChainFormat()
{
	return luna::core::instance.swapChain.format.format;
}
VkExtent2D lunaGetSwapChainExtent()
{
	return luna::core::instance.swapChain.extent;
}
VkResult lunaGetSurfaceCapabilities(const VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities)
{
	assert(capabilities);
	CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(luna::core::instance.device().physicalDevice(),
																  surface,
																  capabilities));
	capabilities->maxImageCount = capabilities->maxImageCount == 0 ? UINT32_MAX : capabilities->maxImageCount;
	return VK_SUCCESS;
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
