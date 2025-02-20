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

typedef void *LunaRenderPass;
typedef void *LunaRenderPassSubpass;

typedef struct
{
		const uint32_t apiVersion;

		const uint32_t extensionCount;
		const char *const *extensionNames;

		bool enableValidation;
		const uint32_t layerCount;
		const char *const *layerNames;
} LunaInstanceCreationInfo;

typedef struct
{
		const uint32_t extensionCount;
		const char *const *extensionNames;

		const VkPhysicalDeviceFeatures requiredFeatures;
		VkSurfaceKHR surface;
} LunaDeviceCreationInfo;

typedef struct
{
		const uint32_t extensionCount;
		const char *const *extensionNames;

		const VkPhysicalDeviceFeatures2 requiredFeatures;
		VkSurfaceKHR surface;
} LunaDeviceCreationInfo2;

typedef struct
{
		VkSurfaceKHR surface;
		uint32_t width;
		uint32_t height;
		uint32_t minImageCount;

		uint32_t formatCount;
		VkSurfaceFormatKHR *formatPriorityList;
		uint32_t presentModeCount;
		VkPresentModeKHR *presentModePriorityList;

		VkImageUsageFlags imageUsage;
		VkCompositeAlphaFlagBitsKHR compositeAlpha;
} LunaSwapChainCreationInfo;

typedef struct
{
		uint32_t attachmentCount;
		const VkAttachmentDescription2 *attachments;
		const char **attachmentNames;

		uint32_t subpassCount;
		const VkSubpassDescription2 *subpasses;
		const char **subpassNames;

		uint32_t dependencyCount;
		const VkSubpassDependency2 *dependencies;
		const char **dependencyNames;

		uint32_t correlatedViewMaskCount;
		const uint32_t *correlatedViewMasks;
		const char **correlatedViewMaskNames;
} LunaRenderPassCreationInfo;

#ifdef __cplusplus
}
#endif

#endif //LUNATYPES_H
