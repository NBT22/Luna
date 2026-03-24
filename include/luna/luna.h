//
// Created by NBT22 on 2/11/25.
//

#ifndef LUNA_H
#define LUNA_H

#ifdef __cplusplus
extern "C"
{
#endif

// NOLINTBEGIN(*-include-cleaner)
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaImage.h>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
// NOLINTEND(*-include-cleaner)

// TODO (0.3.0): Rename functions so that they are lunaObjectAction instead of lunaActionObject

/**
 * @brief Create a new descriptor pool.
 * @param[in] creationInfo A pointer to the @c LunaDescriptorPoolCreationInfo structure containing information about how to create the descriptor pool.
 * @param[out] descriptorPool A pointer to the @c LunaDescriptorPool handle in which the resulting descriptor pool will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDescriptorPool.html
 */
VkResult lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo,
                                  LunaDescriptorPool *descriptorPool);
/**
 * @brief Create a new descriptor set layout.
 * @param[in] creationInfo A pointer to the @c LunaDescriptorSetLayoutCreationInfo structure containing information about how to create the descriptor set layout.
 * @param[out] descriptorSetLayout A pointer to the @c LunaDescriptorSetLayout handle in which the resulting descriptor set layout will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDescriptorSetLayout.html
 */
VkResult lunaCreateDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo *creationInfo,
                                       LunaDescriptorSetLayout *descriptorSetLayout);
/**
 * @brief Allocate a new descriptor set.
 * @param[in] allocationInfo A pointer to the @c LunaDescriptorSetAllocationInfo structure containing information about how the descriptor set should be allocated.
 * @param[out] descriptorSets A pointer to the @c LunaDescriptorSet handle in which the resulting descriptor set will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateDescriptorSets.html
 */
VkResult lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
                                    LunaDescriptorSet *descriptorSets);
/**
 * @brief Update the contents of a descriptor set.
 * @param[in] descriptorWriteCount The number of elements in the @c descriptorWrites array.
 * @param[in] descriptorWrites An array of @c LunaWriteDescriptorSet structures containing information about what to write.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkUpdateDescriptorSets.html
 */
VkResult lunaWriteDescriptorSets(uint32_t descriptorWriteCount, const LunaWriteDescriptorSet *descriptorWrites);

/**
 * @brief Create a new shader module.
 * @param[in] creationInfo A pointer to the @c LunaShaderModuleCreationInfo structure containing information about how to create the shader module.
 * @param[out] shaderModule A pointer to the @c LunaShaderModule handle in which the resulting shader module will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateShaderModule.html
 */
VkResult lunaCreateShaderModule(const LunaShaderModuleCreationInfo *creationInfo, LunaShaderModule *shaderModule);

// TODO (0.3.0): Add lunaDestroyShaderModule

/**
 * @brief Create a new graphics pipeline.
 * @param[in] creationInfo A pointer to the @c LunaGraphicsPipelineCreationInfo structure containing information about how to create the graphics pipeline.
 * @param[out] pipeline A pointer to the @c LunaGraphicsPipeline handle in which the resulting graphics pipeline will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateGraphicsPipelines.html
 */
VkResult lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo,
                                    LunaGraphicsPipeline *pipeline);

/**
 * @brief Create a new graphics pipeline.
 * @param[in] creationInfo A pointer to the @c LunaGraphicsPipelineCreationInfo structure containing information about how to create the graphics pipeline.
 * @param[out] pipeline A pointer to the @c LunaGraphicsPipeline handle in which the resulting graphics pipeline will be returned.
 */
VkResult lunaCreateGraphicsPipelineUsingReflection(const LunaGraphicsPipelineUsingReflectionCreationInfo *creationInfo,
                                                   LunaGraphicsPipeline *pipeline);


/**
 * @brief Create a new compute pipeline.
 * @param[in] creationInfo A pointer to the @c LunaComputePipelineCreationInfo structure containing information about how to create the compute pipeline.
 * @param[out] pipeline A pointer to the @c LunaComputePipeline handle in which the resulting compute pipeline will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateComputePipeline.html
 */
VkResult lunaCreateComputePipeline(const LunaComputePipelineCreationInfo *creationInfo, LunaComputePipeline *pipeline);
/**
 * @brief Dispatch a compute pipeline.
 * @param[in] info A pointer to the @c LunaDispatchCompute structure containing information about what to dispatch.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdDispatch.html
 */
VkResult lunaDispatch(const LunaDispatchInfo *info);
VkResult lunaDispatchBase(const LunaDispatchBaseInfo *info);

/**
 * @brief Create a new command pool.
 * @param[in] creationInfo A pointer to the @c LunaCommandPoolCreationInfo structure containing information about how to create the command pool.
 * @param[out] commandPool A pointer to the @c LunaCommandPool handle in which the resulting command pool will be returned.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateCommandPool.html
 */
VkResult lunaCreateCommandPool(const LunaCommandPoolCreationInfo *creationInfo, LunaCommandPool *commandPool);
/**
 * @brief Reset a command pool.
 * @param[in] commandPool The command pool to reset.
 * @param[in] flags A bitmask of @c VkCommandPoolResetFlagBits specifying options for how to reset the command pool.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkResetCommandPool.html
 * @warning This function waits for fences with a timeout of @c UINT64_MAX.
 */
VkResult lunaResetCommandPool(LunaCommandPool commandPool, VkCommandPoolResetFlags flags);
/**
 * @brief Reset a command pool.
 * @param[in] commandPool The command pool to reset.
 * @param[in] flags A bitmask of @c VkCommandPoolResetFlagBits specifying options for how to reset the command pool.
 * @param[in] timeout The time, in nanoseconds, to block for when waiting for fences.
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkResetCommandPool.html
 */
VkResult lunaResetCommandPoolWithTimeout(LunaCommandPool commandPool, VkCommandPoolResetFlags flags, size_t timeout);

/**
 * @brief Insert a pipeline memory dependency
 * @param[in] dependencyInfo
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdPipelineBarrier.html
 * @see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdPipelineBarrier2.html
 */
VkResult lunaPipelineBarrier(const LunaDependencyInfo *dependencyInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNA_H
