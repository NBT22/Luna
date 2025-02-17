//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNATYPES_H
#define LUNATYPES_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif

typedef struct LunaInstanceCreationInfoStruct LunaInstanceCreationInfo;
typedef struct LunaDeviceCreationInfoStruct LunaDeviceCreationInfo;
typedef struct LunaDeviceCreationInfo2Struct LunaDeviceCreationInfo2;
typedef struct LunaSwapChainCreationInfoStruct LunaSwapChainCreationInfo;

struct LunaInstanceCreationInfoStruct
{
		const uint32_t apiVersion;

		const uint32_t extensionCount;
		const char *const *extensionNames;

		bool enableValidation;
		const uint32_t layerCount;
		const char *const *layerNames;
};

struct LunaDeviceCreationInfoStruct
{
		const uint32_t extensionCount;
		const char *const *extensionNames;

		const VkPhysicalDeviceFeatures requiredFeatures;
		VkSurfaceKHR surface;
};

struct LunaDeviceCreationInfo2Struct
{
		const uint32_t extensionCount;
		const char *const *extensionNames;

		const VkPhysicalDeviceFeatures2 requiredFeatures;
		VkSurfaceKHR surface;
};

struct LunaSwapChainCreationInfoStruct
{
		VkSurfaceKHR surface;
		uint32_t width;
		uint32_t height;
		uint32_t minImageCount;

		uint32_t formatCount;
		VkSurfaceFormatKHR *formatPriorityList;
		uint32_t presentModeCount;
		VkPresentModeKHR *presentModePriorityList;
};

#ifdef __cplusplus
}
#endif

#endif //LUNATYPES_H
