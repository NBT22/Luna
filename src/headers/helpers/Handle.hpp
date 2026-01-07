//
// Created by NBT22 on 11/16/25.
//

#pragma once

#include <concepts>
#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

namespace luna
{
using HandleType = uint64_t;

class RenderPass;
struct RenderPassSubpassIndex;
class DescriptorSetLayout;
struct DescriptorSetIndex;
class ShaderModule;
class GraphicsPipeline;
class ComputePipeline;
class BufferRegionIndex;
class Image;
class CommandPool;
class SlangSession;
} // namespace luna
namespace luna::helpers
{
// static_assert(std::same_as<RenderPass , CommandPool>);

template<typename ReturnType> [[nodiscard]] ReturnType *fromHandle(std::same_as<HandleType> auto, ReturnType * = nullptr) = delete;
template<typename ReturnType> requires(std::same_as<ReturnType, RenderPass>)
RenderPass *fromHandle(const LunaRenderPass renderPass, RenderPass * = nullptr)
{
    return reinterpret_cast<RenderPass *>(renderPass);
}
template<typename ReturnType> requires(std::same_as<ReturnType, RenderPassSubpassIndex>)
RenderPassSubpassIndex *fromHandle(const LunaRenderPassSubpass renderPassSubpass, RenderPassSubpassIndex * = nullptr)
{
    return reinterpret_cast<RenderPassSubpassIndex *>(renderPassSubpass);
}
template<typename ReturnType> requires(std::same_as<ReturnType, VkDescriptorPool>)
VkDescriptorPool *fromHandle(const LunaDescriptorPool descriptorPool, VkDescriptorPool * = nullptr)
{
    return reinterpret_cast<VkDescriptorPool *>(descriptorPool);
}
template<typename ReturnType> requires(std::same_as<ReturnType, DescriptorSetLayout>)
DescriptorSetLayout *fromHandle(const LunaDescriptorSetLayout descriptorSetLayout, DescriptorSetLayout * = nullptr)
{
    return reinterpret_cast<DescriptorSetLayout *>(descriptorSetLayout);
}
template<typename ReturnType> requires(std::same_as<ReturnType, DescriptorSetIndex>)
DescriptorSetIndex *fromHandle(const LunaDescriptorSet descriptorSet, DescriptorSetIndex * = nullptr)
{
    return reinterpret_cast<DescriptorSetIndex *>(descriptorSet);
}
template<typename ReturnType> requires(std::same_as<ReturnType, ShaderModule>)
ShaderModule *fromHandle(const LunaShaderModule shaderModule, ShaderModule * = nullptr)
{
    return reinterpret_cast<ShaderModule *>(shaderModule);
}
template<typename ReturnType> requires(std::same_as<ReturnType, GraphicsPipeline>)
GraphicsPipeline *fromHandle(const LunaGraphicsPipeline graphicsPipeline, GraphicsPipeline * = nullptr)
{
    return reinterpret_cast<GraphicsPipeline *>(graphicsPipeline);
}
template<typename ReturnType> requires(std::same_as<ReturnType, ComputePipeline>)
ComputePipeline *fromHandle(const LunaComputePipeline computePipeline, ComputePipeline * = nullptr)
{
    return reinterpret_cast<ComputePipeline *>(computePipeline);
}
template<typename ReturnType> requires(std::same_as<ReturnType, BufferRegionIndex>)
BufferRegionIndex *fromHandle(const LunaBuffer buffer, BufferRegionIndex * = nullptr)
{
    return reinterpret_cast<BufferRegionIndex *>(buffer);
}
template<typename ReturnType> requires(std::same_as<ReturnType, VkSampler>)
VkSampler *fromHandle(const LunaSampler sampler, VkSampler * = nullptr)
{
    return reinterpret_cast<VkSampler *>(sampler);
}
template<typename ReturnType> requires(std::same_as<ReturnType, Image>)
Image *fromHandle(const LunaImage image, Image * = nullptr)
{
    return reinterpret_cast<Image *>(image);
}
template<typename ReturnType> requires(std::same_as<ReturnType, CommandPool>)
CommandPool *fromHandle(const LunaCommandPool commandPool, CommandPool * = nullptr)
{
    return reinterpret_cast<CommandPool *>(commandPool);
}
template<typename ReturnType> requires(std::same_as<ReturnType, SlangSession>)
SlangSession *fromHandle(const LunaSlangSession slangSession, SlangSession * = nullptr)
{
    return reinterpret_cast<SlangSession *>(slangSession);
}


template<typename T> [[nodiscard]] HandleType toHandle(T *value)
{
    return reinterpret_cast<HandleType>(value);
}
template<typename T> [[nodiscard]] HandleType toHandle(const T *value)
{
    return toHandle(const_cast<T *>(value));
}
template<typename T> [[nodiscard]] HandleType toHandle(T &value)
{
    return toHandle(&value);
}
template<typename T> [[nodiscard]] HandleType toHandle(const T &value)
{
    return toHandle(&value);
}
} // namespace luna::helpers
