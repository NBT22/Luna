//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <list>
#include <luna/core/Buffer.hpp>
#include <luna/core/DescriptorSetLayout.hpp>
#include <luna/core/Device.hpp>
#include <luna/core/GraphicsPipeline.hpp>
#include <luna/core/Image.hpp>
#include <luna/core/Luna.hpp>
#include <luna/core/RenderPass.hpp>

namespace luna::core
{
[[nodiscard]] RenderPass *renderPass(LunaRenderPass renderPass);
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
extern VkPipeline boundPipeline;
extern VkBuffer boundVertexBuffer;
extern VkBuffer boundIndexBuffer;
extern LunaBuffer stagingBuffer;

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
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
