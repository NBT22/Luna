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

VkResult lunaCreateDevice(const LunaDeviceCreationInfo *creationInfo, LunaDevice *device);
VkResult lunaCreateDevice2(const LunaDeviceCreationInfo2 *creationInfo, LunaDevice *device);

VkDevice lunaGetVkDevice(LunaDevice device);
VkPhysicalDevice lunaGetPhysicalDevice(LunaDevice device);

VkResult lunaDeviceWaitIdle(LunaDevice device);

void lunaGetPhysicalDeviceProperties(LunaDevice device, VkPhysicalDeviceProperties *properties);
void lunaGetPhysicalDeviceProperties2(LunaDevice device, VkPhysicalDeviceProperties2 *properties);

// TODO (0.3.0): Remove this and replace with more flexible solution
VkResult lunaSubmitInternalComputeQueue(LunaDevice device, LunaCommandBuffer commandBuffer);

void lunaGetQueue(const LunaQueueProperties *requiredProperties, VkQueue queue);

#ifdef __cplusplus
}
#endif

#endif //LUNADEVICE_H
