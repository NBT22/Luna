//
// Created by NBT22 on 2/13/25.
//

#include <cstring>
#include <luna/core/Instance.hpp>
#include <luna/lunaInstance.h>
#include <stdexcept>

namespace luna::core
{
Instance instance;
Instance::Instance(const LunaInstanceExtensionInfo &extensionInfo,
				   const LunaInstanceLayerInfo &layerInfo,
				   const LunaInstanceRequirements &instanceRequirements):
	physicalDevice_(createInstance(extensionInfo, layerInfo, instanceRequirements.apiVersion),
					VK_API_VERSION_MINOR(instanceRequirements.apiVersion),
					instanceRequirements.requiredFeatures)
{}
Instance::Instance(const LunaInstanceExtensionInfo &extensionInfo,
				   const LunaInstanceLayerInfo &layerInfo,
				   const LunaInstanceRequirements2 &instanceRequirements):
	physicalDevice_(createInstance(extensionInfo, layerInfo, instanceRequirements.apiVersion),
					VK_API_VERSION_MINOR(instanceRequirements.apiVersion),
					instanceRequirements.requiredFeatures)
{}
uint32_t Instance::version() const
{
	return VK_API_VERSION_MINOR(apiVersion_);
}
VkInstance Instance::instance() const
{
	return instance_;
}
PhysicalDevice Instance::physicalDevice() const
{
	return physicalDevice_;
}
VkInstance Instance::createInstance(const LunaInstanceExtensionInfo &extensionInfo,
									const LunaInstanceLayerInfo &layerInfo,
									const uint32_t apiVersion)
{
	apiVersion_ = apiVersion;

	const VkApplicationInfo vulkanApplicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = apiVersion,
	};
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.pApplicationInfo = &vulkanApplicationInfo,
		.enabledLayerCount = layerInfo.layerCount,
		.ppEnabledLayerNames = layerInfo.layerNames,
		.enabledExtensionCount = extensionInfo.extensionCount,
		.ppEnabledExtensionNames = extensionInfo.extensionNames,
	};
	if (layerInfo.enableValidation)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		VkLayerProperties availableLayers[layerCount];
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
		bool found = false;
		for (uint32_t i = 0; i < layerCount; i++)
		{
			if (std::strncmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation", 27) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			throw std::runtime_error("Failed to find Khronos validation layer!");
		}
		if (createInfo.enabledLayerCount == 0)
		{
			createInfo.enabledLayerCount = 1;
			createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
		} else
		{
			createInfo.enabledLayerCount++;
			createInfo.ppEnabledLayerNames = new char *[createInfo.enabledLayerCount];
		}
	}

	vkCreateInstance(&createInfo, nullptr, &instance_);
	return instance_;
}
} // namespace luna::core

void lunaCreateInstance(const LunaInstanceExtensionInfo &extensionInfo,
						const LunaInstanceLayerInfo &layerInfo,
						const LunaInstanceRequirements &instanceRequirements)
{
	luna::core::instance = luna::core::Instance(extensionInfo, layerInfo, instanceRequirements);
}

void lunaCreateInstance2(const LunaInstanceExtensionInfo &extensionInfo,
						 const LunaInstanceLayerInfo &layerInfo,
						 const LunaInstanceRequirements2 &instanceRequirements)
{
	luna::core::instance = luna::core::Instance(extensionInfo, layerInfo, instanceRequirements);
}
