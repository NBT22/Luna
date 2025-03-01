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

LunaRenderPass lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo);
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(LunaRenderPass renderPass, const char *name);

#ifdef __cplusplus
}
#endif

#endif //LUNARENDERPASS_H
