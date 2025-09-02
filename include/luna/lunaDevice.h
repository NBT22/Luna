//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNADEVICE_H
#define LUNADEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

VkResult lunaAddNewDevice(const LunaDeviceCreationInfo *creationInfo);
VkResult lunaAddNewDevice2(const LunaDeviceCreationInfo2 *creationInfo);

VkPhysicalDeviceProperties lunaGetPhysicalDeviceProperties(void);
VkPhysicalDeviceProperties2 lunaGetPhysicalDeviceProperties2(void);

#ifdef __cplusplus
}
#endif

#endif //LUNADEVICE_H
