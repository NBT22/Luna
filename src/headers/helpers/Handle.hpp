//
// Created by NBT22 on 11/16/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

namespace luna
{
using HandleType = void *;

class RenderPass;
struct RenderPassSubpassIndex;
class DescriptorSetLayout;
struct DescriptorSetIndex;
class GraphicsPipeline;
class ComputePipeline;
class BufferRegionIndex;
class Image;
class CommandPool;
} // namespace luna
namespace luna::helpers
{
template<typename ReturnType> constexpr ReturnType *fromHandle(std::same_as<HandleType> auto) = delete;
template<> constexpr RenderPass *fromHandle(const LunaRenderPass renderPass)
{
    return static_cast<RenderPass *>(renderPass);
}
template<> constexpr RenderPassSubpassIndex *fromHandle(const LunaRenderPassSubpass renderPassSubpass)
{
    return static_cast<RenderPassSubpassIndex *>(renderPassSubpass);
}
template<> constexpr VkDescriptorPool *fromHandle(const LunaDescriptorPool descriptorPool)
{
    return static_cast<VkDescriptorPool *>(descriptorPool);
}
template<> constexpr DescriptorSetLayout *fromHandle(const LunaDescriptorSetLayout descriptorSetLayout)
{
    return static_cast<DescriptorSetLayout *>(descriptorSetLayout);
}
template<> constexpr DescriptorSetIndex *fromHandle(const LunaDescriptorSet descriptorSet)
{
    return static_cast<DescriptorSetIndex *>(descriptorSet);
}
template<> constexpr VkShaderModule *fromHandle(const LunaShaderModule shaderModule)
{
    return static_cast<VkShaderModule *>(shaderModule);
}
template<> constexpr GraphicsPipeline *fromHandle(const LunaGraphicsPipeline graphicsPipeline)
{
    return static_cast<GraphicsPipeline *>(graphicsPipeline);
}
template<> constexpr ComputePipeline *fromHandle(const LunaComputePipeline computePipeline)
{
    return static_cast<ComputePipeline *>(computePipeline);
}
template<> constexpr BufferRegionIndex *fromHandle(const LunaBuffer buffer)
{
    return static_cast<BufferRegionIndex *>(buffer);
}
template<> constexpr VkSampler *fromHandle(const LunaSampler sampler)
{
    return static_cast<VkSampler *>(sampler);
}
template<> constexpr Image *fromHandle(const LunaImage image)
{
    return static_cast<Image *>(image);
}
template<> constexpr CommandPool *fromHandle(const LunaCommandPool commandPool)
{
    return static_cast<CommandPool *>(commandPool);
}


template<typename T> constexpr HandleType toHandle(T *value)
{
    return static_cast<HandleType>(value);
}
template<typename T> constexpr HandleType toHandle(const T *value)
{
    return toHandle(const_cast<T *>(value));
}
template<typename T> constexpr HandleType toHandle(T &value)
{
    return toHandle(&value);
}
template<typename T> constexpr HandleType toHandle(const T &value)
{
    return toHandle(&value);
}
} // namespace luna::helpers
