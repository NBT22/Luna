//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNAINSTANCE_H
#define LUNAINSTANCE_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

VkResult lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo);
VkResult lunaDestroyInstance(void);
VkInstance lunaGetInstance(void);

VkResult lunaGetSurfaceCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities);

VkResult lunaCreateSwapChain(const LunaSwapChainCreationInfo *creationInfo);
VkFormat lunaGetSwapChainFormat(void);
VkExtent2D lunaGetSwapChainExtent(void);

void lunaSetDepthImageFormat(uint32_t formatCount, const VkFormat *formatPriorityList);
VkFormat lunaGetDepthImageFormat(void);

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
