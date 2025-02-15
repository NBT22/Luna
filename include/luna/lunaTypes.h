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

typedef struct LunaPhysicalDeviceStruct *LunaPhysicalDevice; // TODO: Is this even needed, or is only logical needed?
typedef struct LunaLogicalDeviceStruct *LunaLogicalDevice;

struct LunaInstanceRequirements
{
		const uint32_t apiVersion;
		VkPhysicalDeviceFeatures requiredFeatures;
};

struct LunaInstanceRequirements2
{
		const uint32_t apiVersion;
		VkPhysicalDeviceFeatures2 requiredFeatures;
};

struct LunaInstanceExtensionInfo
{
		const uint32_t extensionCount;
		const char *const *extensionNames;
};

struct LunaInstanceLayerInfo
{
		bool enableValidation;
		const uint32_t layerCount;
		const char *const *layerNames;
};

#ifdef __cplusplus
}
#endif

#endif //LUNATYPES_H
