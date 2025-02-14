//
// Created by NBT22 on 2/13/25.
//

#include <cassert>
#include <cstring>
#include <luna/lunaInstance.h>

static VkInstance instance = VK_NULL_HANDLE;

LunaInstance lunaCreateInstance(const LunaApplicationInfo &applicationInfo,
								const LunaInstanceExtensionInfo &extensionInfo,
								const LunaInstanceLayerInfo &layerInfo)
{
	const VkApplicationInfo vulkanApplicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr, // TODO: VkApplicationParametersEXT?
		.pApplicationName = applicationInfo.applicationName,
		.applicationVersion = applicationInfo.applicationVersion,
		.pEngineName = applicationInfo.engineName,
		.engineVersion = applicationInfo.engineVersion,
		.apiVersion = applicationInfo.apiVersion,
	};
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
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
			assert("Failed to find Khronos validation layer!");
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

	vkCreateInstance(&createInfo, nullptr, &instance);
	return nullptr;
}

namespace luna::core
{}
