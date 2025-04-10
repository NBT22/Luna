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
#include <vector>

namespace luna::core
{
[[nodiscard]] const RenderPass &renderPass(LunaRenderPass renderPass);
[[nodiscard]] VkDescriptorPool descriptorPool(LunaDescriptorPool descriptorPool);
[[nodiscard]] const DescriptorSetLayout &descriptorSetLayout(LunaDescriptorSetLayout layout);
[[nodiscard]] VkDescriptorSet descriptorSet(LunaDescriptorSet descriptorSet);
void descriptorSet(LunaDescriptorSet index,
				   VkDescriptorPool *pool,
				   DescriptorSetLayout *layout,
				   VkDescriptorSet *descriptorSet);
[[nodiscard]] const buffer::BufferRegion &bufferRegion(LunaBuffer buffer);
[[nodiscard]] const buffer::BufferRegion &bufferRegion(buffer::BufferRegionIndex index);
[[nodiscard]] VkBuffer stagingBuffer();
[[nodiscard]] size_t stagingBufferOffset();
[[nodiscard]] VkSampler sampler(LunaSampler sampler);

extern SwapChain swapChain;
extern VkFormat depthImageFormat;

extern uint32_t apiVersion;
extern VkInstance instance;
extern Device device;
extern VkSurfaceKHR surface;

extern VkPipeline boundPipeline;
extern VkBuffer boundVertexBuffer;
extern VkBuffer boundIndexBuffer;

extern std::list<RenderPassIndex> renderPassIndices;
extern std::vector<RenderPass> renderPasses;

extern std::list<DescriptorPoolIndex> descriptorPoolIndices;
extern std::list<DescriptorSetLayoutIndex> descriptorSetLayoutIndices;
extern std::list<DescriptorSetIndex> descriptorSetIndices;
extern std::vector<VkDescriptorPool> descriptorPools;
extern std::vector<DescriptorSetLayout> descriptorSetLayouts;
extern std::vector<VkDescriptorSet> descriptorSets;

extern std::list<GraphicsPipelineIndex> graphicsPipelineIndices;
extern std::vector<GraphicsPipeline> graphicsPipelines;

extern std::list<buffer::BufferRegionIndex> bufferRegionIndices;
extern std::vector<Buffer> buffers;
extern LunaBuffer stagingBufferIndex;

extern std::list<SamplerIndex> samplerIndices;
extern std::vector<VkSampler> samplers;
extern std::list<ImageIndex> imageIndices;
extern std::vector<Image> images;
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
