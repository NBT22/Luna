//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNAINSTANCE_H
#define LUNAINSTANCE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo);
VkResult lunaDestroyInstance(void);
VkInstance lunaGetInstance(void);

bool lunaIsInstanceExtensionAvailable(const char *extensionName);
bool lunaIsInstanceExtensionVersionAvailable(const char *extensionName, uint32_t extensionVersion);

VkResult lunaGetSurfaceCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities);

VkResult lunaCreateSwapchain(const LunaSwapchainCreationInfo *creationInfo);
VkFormat lunaGetSwapchainFormat(void);
VkExtent2D lunaGetSwapchainExtent(void);

void lunaSetDepthImageFormat(uint32_t formatCount, const VkFormat *formatPriorityList);
VkFormat lunaGetDepthImageFormat(void);

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
