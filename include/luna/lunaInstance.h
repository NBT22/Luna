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
void lunaCreateSwapChain(const LunaSwapChainCreationInfo *creationInfo);
VkFormat lunaGetSwapChainFormat();
VkSurfaceCapabilitiesKHR lunaGetSurfaceCapabilities(VkSurfaceKHR surface);
void lunaSetDepthImageFormat(uint32_t formatCount, const VkFormat *formatPriorityList);
VkFormat lunaGetDepthImageFormat();
LunaRenderPass lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
