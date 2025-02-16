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
Instance::Instance(const LunaInstanceCreationInfo &creationInfo)
{
	apiVersion_ = creationInfo.apiVersion;
	surface_ = creationInfo.surface;

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

	bool surfaceExtensionFound = false;
	for (uint32_t i = 0; i < creationInfo.extensionCount; i++)
	{
		if (std::strncmp(creationInfo.extensionNames[i], "VK_KHR_surface", 14) == 0)
		{
			surfaceExtensionFound = true;
			break;
		}
	}
	assert(surfaceExtensionFound);

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
}
} // namespace luna::core

void lunaCreateInstance(const LunaInstanceCreationInfo &creationInfo)
{
	luna::core::instance = luna::core::Instance(creationInfo);
}
