//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "ComputePipeline.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"

#ifdef LUNA_SLANG_SHADERS
#include <shader-slang/slang.h>
#endif

namespace luna
{
extern Swapchain swapchain;
extern VkFormat depthImageFormat;
extern uint32_t apiVersion;
extern VkInstance instance;
extern Device device;
// TODO (0.3.0): This should be able to have a minimum size (and just always resize to `std::max(newSize, minSize)`),
//  that way it can shrink if it grows to an absurd size
extern BufferRegionIndex *stagingBuffer;
extern VkPipeline boundPipeline;
extern LunaBuffer boundVertexBuffer;
extern LunaBuffer boundIndexBuffer;

extern std::list<RenderPass> renderPasses;
extern std::list<DescriptorSetLayout> descriptorSetLayouts;
extern std::list<VkDescriptorPool> descriptorPools;
extern std::list<VkDescriptorSet> descriptorSets;
extern std::list<DescriptorSetIndex> descriptorSetIndices;
extern std::list<GraphicsPipeline> graphicsPipelines;
extern std::list<ComputePipeline> computePipelines;
extern std::list<Buffer> buffers;
extern std::list<BufferRegionIndex> bufferRegionIndices;
extern std::list<VkSampler> samplers;
extern std::list<Image> images;
} // namespace luna
