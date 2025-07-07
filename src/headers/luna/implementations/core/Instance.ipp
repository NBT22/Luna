//
// Created by NBT22 on 2/15/25.
//

#pragma once

namespace luna::core
{
inline RenderPass *renderPass(const LunaRenderPass renderPass)
{
    return const_cast<RenderPass *>(static_cast<const RenderPass *>(renderPass));
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
    if (pool != nullptr)
    {
        *pool = *descriptorSetIndex->pool;
    }
    if (layout != nullptr)
    {
        *layout = *descriptorSetIndex->layout;
    }
    if (descriptorSet != nullptr)
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
} // namespace luna::core
