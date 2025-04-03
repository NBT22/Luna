//
// Created by NBT22 on 2/19/25.
//

#ifndef LUNARENDERPASS_H
#define LUNARENDERPASS_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

void lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo, LunaRenderPass *renderPass);
void lunaCreateRenderPass2(const LunaRenderPassCreationInfo2 *creationInfo, LunaRenderPass *renderPass);
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(LunaRenderPass renderPass, const char *name);

void lunaBeginRenderPass(LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNARENDERPASS_H
