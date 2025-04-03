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

void lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo);
VkInstance lunaGetInstance();

void lunaGetSurfaceCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities);

void lunaCreateSwapChain(const LunaSwapChainCreationInfo *creationInfo);
VkFormat lunaGetSwapChainFormat();
VkExtent2D lunaGetSwapChainExtent();

void lunaSetDepthImageFormat(uint32_t formatCount, const VkFormat *formatPriorityList);
VkFormat lunaGetDepthImageFormat();

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
