//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <list>
#include "Buffer.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "Image.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"

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
extern LunaBuffer stagingBuffer;
extern VkPipeline boundPipeline;
extern LunaBuffer boundVertexBuffer;
extern LunaBuffer boundIndexBuffer;
extern VkDeviceSize boundIndexBufferOffset;

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

#include "implementations/Instance.ipp"
