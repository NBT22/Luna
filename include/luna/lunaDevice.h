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

VkResult lunaCreateDevice(const LunaDeviceCreationInfo *creationInfo);
VkResult lunaCreateDevice2(const LunaDeviceCreationInfo2 *creationInfo);

VkDevice lunaGetDevice(void);
VkPhysicalDevice lunaGetPhysicalDevice(void);

VkResult lunaDeviceWaitIdle(void);

void lunaGetPhysicalDeviceProperties(VkPhysicalDeviceProperties *properties);
void lunaGetPhysicalDeviceProperties2(VkPhysicalDeviceProperties2 *properties);

// TODO (0.3.0): Remove this and replace with more flexible solution
VkResult lunaSubmitInternalComputeQueue(LunaCommandBuffer commandBuffer, bool waitForFence);

void lunaGetQueue(const LunaQueueProperties *requiredProperties, VkQueue queue);

#ifdef __cplusplus
}
#endif

#endif //LUNADEVICE_H
