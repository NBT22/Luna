//
// Created by NBT22 on 4/21/26.
//

#ifndef LUNA_LUNASYNCHRONIZATION_H
#define LUNA_LUNASYNCHRONIZATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateSemaphore(LunaDevice device,
                             const LunaSemaphoreCreationInfo *creationInfo,
                             LunaSemaphore *semaphore);
void lunaDestroySemaphore(LunaDevice device, LunaSemaphore semaphore);

// TODO (0.3.0): Implement these
VkResult lunaCreateFence(LunaDevice device, const LunaFenceCreationInfo *creationInfo, LunaFence *fence);
void lunaDestroyFence(LunaDevice device, LunaFence *fence);

#ifdef __cplusplus
}
#endif

#endif //LUNA_LUNASYNCHRONIZATION_H
