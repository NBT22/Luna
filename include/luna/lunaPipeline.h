//
// Created by NBT22 on 2/28/25.
//

#ifndef LUNAPIPELINE_H
#define LUNAPIPELINE_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

VkResult lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo,
									LunaGraphicsPipeline *pipeline);

#ifdef __cplusplus
}
#endif

#endif //LUNAPIPELINE_H
