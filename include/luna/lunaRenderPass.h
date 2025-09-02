//
// Created by NBT22 on 2/19/25.
//

#ifndef LUNARENDERPASS_H
#define LUNARENDERPASS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo, LunaRenderPass *renderPass);
VkResult lunaCreateRenderPass2(const LunaRenderPassCreationInfo2 *creationInfo, LunaRenderPass *renderPass);
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(LunaRenderPass renderPass, const char *name);

VkResult lunaBeginRenderPass(LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo);
void lunaNextSubpass(void);
void lunaEndRenderPass(void);

#ifdef __cplusplus
}
#endif

#endif //LUNARENDERPASS_H
