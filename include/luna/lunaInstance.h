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
VkSurfaceCapabilitiesKHR lunaGetSurfaceCapabilities(VkSurfaceKHR surface);

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
