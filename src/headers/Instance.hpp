//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "Image.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"

namespace luna
{
[[nodiscard]] const RenderPass *renderPass(LunaRenderPass renderPass);
[[nodiscard]] const VkDescriptorPool *descriptorPool(LunaDescriptorPool descriptorPool);
[[nodiscard]] const DescriptorSetLayout *descriptorSetLayout(LunaDescriptorSetLayout layout);
[[nodiscard]] const VkDescriptorSet *descriptorSet(LunaDescriptorSet descriptorSet);
void descriptorSet(LunaDescriptorSet index,
                   VkDescriptorPool *pool,
                   DescriptorSetLayout *layout,
                   VkDescriptorSet *descriptorSet);
[[nodiscard]] size_t stagingBufferOffset();
[[nodiscard]] VkSampler sampler(LunaSampler sampler);

extern Swapchain swapchain;
extern VkFormat depthImageFormat;
extern uint32_t apiVersion;
extern VkInstance instance;
extern Device device;
extern LunaBuffer stagingBuffer;
extern VkPipeline boundPipeline;
extern LunaBuffer boundVertexBuffer;
extern LunaBuffer boundIndexBuffer;

extern std::list<RenderPass> renderPasses;
extern std::list<DescriptorSetLayout> descriptorSetLayouts;
extern std::list<VkDescriptorPool> descriptorPools;
extern std::list<VkDescriptorSet> descriptorSets;
extern std::list<DescriptorSetIndex> descriptorSetIndices;
extern std::list<GraphicsPipeline> graphicsPipelines;
extern std::list<Buffer> buffers;
extern std::list<buffer::BufferRegionIndex> bufferRegionIndices;
extern std::list<VkSampler> samplers;
extern std::list<Image> images;
} // namespace luna

#pragma region "Implmentation"

namespace luna
{
inline const RenderPass *renderPass(const LunaRenderPass renderPass)
{
    return static_cast<const RenderPass *>(renderPass);
}
inline const VkDescriptorPool *descriptorPool(const LunaDescriptorPool descriptorPool)
{
    return static_cast<const VkDescriptorPool *>(descriptorPool);
}
inline const DescriptorSetLayout *descriptorSetLayout(const LunaDescriptorSetLayout layout)
{
    return static_cast<const DescriptorSetLayout *>(layout);
}
inline const VkDescriptorSet *descriptorSet(const LunaDescriptorSet descriptorSet)
{
    return static_cast<const DescriptorSetIndex *>(descriptorSet)->set;
}
inline void descriptorSet(const LunaDescriptorSet index,
                          VkDescriptorPool *pool,
                          DescriptorSetLayout *layout,
                          VkDescriptorSet *descriptorSet)
{
    const DescriptorSetIndex *descriptorSetIndex = static_cast<const DescriptorSetIndex *>(index);
    if (pool != VK_NULL_HANDLE)
    {
        *pool = *descriptorSetIndex->pool;
    }
    if (layout != nullptr)
    {
        *layout = *descriptorSetIndex->layout;
    }
    if (descriptorSet != VK_NULL_HANDLE)
    {
        *descriptorSet = *descriptorSetIndex->set;
    }
}

inline size_t stagingBufferOffset()
{
    if (stagingBuffer == nullptr)
    {
        return -1ull;
    }
    return static_cast<const buffer::BufferRegionIndex *>(stagingBuffer)->offset();
}
inline VkSampler sampler(const LunaSampler sampler)
{
    return *static_cast<const VkSampler *>(sampler);
}
} // namespace luna

#pragma endregion "Implmentation"
