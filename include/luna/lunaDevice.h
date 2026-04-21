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
#include <stdint.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateDevice(const LunaDeviceCreationInfo *creationInfo, LunaDevice *device);
VkResult lunaCreateDevice2(const LunaDeviceCreationInfo2 *creationInfo, LunaDevice *device);

VkDevice lunaGetVkDevice(LunaDevice device);
VkPhysicalDevice lunaGetPhysicalDevice(LunaDevice device);

VkResult lunaDeviceWaitIdle(LunaDevice device);

void lunaGetPhysicalDeviceProperties(LunaDevice device, VkPhysicalDeviceProperties *properties);
void lunaGetPhysicalDeviceProperties2(LunaDevice device, VkPhysicalDeviceProperties2 *properties);

const LunaQueueFamilyProperties *lunaGetQueueFamilies(LunaDevice device);
uint32_t lunaGetQueueFamilyIndex(LunaDevice device, const LunaQueueFamilyProperties *requiredProperties);

#ifdef __cplusplus
}
#endif

#endif //LUNADEVICE_H
