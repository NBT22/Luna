//
// Created by NBT22 on 11/16/25.
//

#pragma once

#include <concepts>
#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

namespace luna
{
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

using HandleType = uint64_t;

template<typename T> concept HandleData = std::same_as<T, RenderPass> ||
                                          std::same_as<T, RenderPassSubpassIndex> ||
                                          std::same_as<T, VkDescriptorPool> ||
                                          std::same_as<T, DescriptorSetLayout> ||
                                          std::same_as<T, DescriptorSetIndex> ||
                                          std::same_as<T, ShaderModule> ||
                                          std::same_as<T, GraphicsPipeline> ||
                                          std::same_as<T, ComputePipeline> ||
                                          std::same_as<T, BufferRegionIndex> ||
                                          std::same_as<T, VkSampler> ||
                                          std::same_as<T, Image> ||
                                          std::same_as<T, CommandPool> ||
                                          std::same_as<T, SlangSession>;
} // namespace luna
namespace luna::helpers
{
template<HandleData ReturnType, std::same_as<HandleType> T> [[nodiscard]] ReturnType *fromHandle(T handle) noexcept
{
    return reinterpret_cast<ReturnType *>(handle);
}


template<HandleData T> [[nodiscard]] HandleType toHandle(T *value) noexcept
{
    return reinterpret_cast<HandleType>(value);
}
template<HandleData T> [[nodiscard]] HandleType toHandle(const T *value) noexcept
{
    return toHandle(const_cast<T *>(value));
}
template<HandleData T> [[nodiscard]] HandleType toHandle(T &value) noexcept
{
    return toHandle(&value);
}
template<HandleData T> [[nodiscard]] HandleType toHandle(const T &value) noexcept
{
    return toHandle(&value);
}
} // namespace luna::helpers
