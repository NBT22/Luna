//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNATYPES_H
#define LUNATYPES_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct LunaInstanceCreationInfo
{
		const uint32_t apiVersion;

		const uint32_t extensionCount;
		const char *const *extensionNames;

		bool enableValidation;
		const uint32_t layerCount;
		const char *const *layerNames;

		VkSurfaceKHR surface;
};

struct LunaDeviceCreationInfo
{
		const uint32_t extensionCount;
		const char *const *extensionNames;

		const VkPhysicalDeviceFeatures requiredFeatures;
};

struct LunaDeviceCreationInfo2
{
		const uint32_t extensionCount;
		const char *const *extensionNames;

		const VkPhysicalDeviceFeatures2 requiredFeatures;
};

#ifdef __cplusplus
}
#endif

#endif //LUNATYPES_H
