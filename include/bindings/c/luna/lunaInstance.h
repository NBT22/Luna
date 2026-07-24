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
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkResult lunaInitializeVolk(void);
VkResult lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo);
VkResult lunaDestroyInstance(void);
VkInstance lunaGetInstance(void);

bool lunaIsInstanceExtensionAvailable(const char *extensionName);
bool lunaIsInstanceExtensionVersionAvailable(const char *extensionName, uint32_t extensionVersion);

VkResult lunaGetSurfaceCapabilities(LunaDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities);

// TODO (0.3.0): The swapchain should not be a one-off there should be a handle associated with it
VkResult lunaCreateSwapchain(LunaDevice device, const LunaSwapchainCreationInfo *creationInfo);
VkSwapchainKHR lunaGetVkSwapchain(void);
VkFormat lunaGetSwapchainFormat(void);
VkExtent2D lunaGetSwapchainExtent(void);
uint32_t lunaGetSwapchainImageCount(void);
uint32_t lunaGetSwapchainImageIndex(void);

VkResult lunaSetDepthImageFormat(LunaDevice device, uint32_t formatCount, const VkFormat *formatPriorityList);
VkFormat lunaGetDepthImageFormat(void);

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
