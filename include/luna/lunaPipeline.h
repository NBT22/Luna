//
// Created by NBT22 on 2/28/25.
//

#ifndef LUNAPIPELINE_H
#define LUNAPIPELINE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

VkResult lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo,
                                    LunaGraphicsPipeline *pipeline);

#ifdef __cplusplus
}
#endif

#endif //LUNAPIPELINE_H
