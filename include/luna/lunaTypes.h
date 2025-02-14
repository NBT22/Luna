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

typedef struct LunaInstanceStruct *LunaInstance;
typedef struct LunaPhysicalDeviceStruct *LunaPhysicalDevice;

struct LunaApplicationInfo
{
		const char *applicationName;
		const uint32_t applicationVersion;
		const char *engineName;
		const uint32_t engineVersion;
		const uint32_t apiVersion;
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
