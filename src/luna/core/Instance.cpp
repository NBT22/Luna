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
Instance::Instance(const LunaInstanceCreationInfo &creationInfo, const uint32_t apiVersion)
{
	apiVersion_ = apiVersion;

	const uint32_t enabledLayerCount = creationInfo.enableValidation ? creationInfo.layerCount + 1
																	 : creationInfo.layerCount;
	const char *enabledLayers[enabledLayerCount];
	for (uint32_t i = 0; i < creationInfo.layerCount; i++)
	{
		enabledLayers[i] = creationInfo.layerNames[i];
	}
	if (creationInfo.enableValidation)
	{
		enabledLayers[enabledLayerCount - 1] = "VK_LAYER_KHRONOS_validation";
	}

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	VkLayerProperties availableLayers[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
	uint32_t found = 0;
	for (uint32_t i = 0; i < layerCount; i++)
	{
		for (uint32_t j = 0; j < enabledLayerCount; j++)
		{
			if (std::strncmp(availableLayers[i].layerName, enabledLayers[j], VK_MAX_EXTENSION_NAME_SIZE) == 0)
			{
				found++;
				break;
			}
		}
	}
	if (found < enabledLayerCount)
	{
		throw std::runtime_error("Failed to find enabled layers!");
	}

	const VkApplicationInfo vulkanApplicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = apiVersion,
	};
	// TODO: Check extension support
	const VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &vulkanApplicationInfo,
		.enabledLayerCount = enabledLayerCount,
		.ppEnabledLayerNames = enabledLayers,
		.enabledExtensionCount = creationInfo.extensionCount,
		.ppEnabledExtensionNames = creationInfo.extensionNames,
	};
	vkCreateInstance(&createInfo, nullptr, &instance_);
}
} // namespace luna::core

void lunaCreateInstance(const LunaInstanceCreationInfo &creationInfo, const uint32_t apiVersion)
{
	luna::core::instance = luna::core::Instance(creationInfo, apiVersion);
}
