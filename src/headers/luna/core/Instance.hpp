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
extern class Instance instance;

class Instance
{
	public:
		friend VkResult buffer::BufferRegion::createBuffer(const LunaBufferCreationInfo &, LunaBuffer *index);
		friend VkResult(::lunaDrawBuffer(LunaBuffer,
										 LunaGraphicsPipeline,
										 const LunaGraphicsPipelineBindInfo *,
										 uint32_t,
										 uint32_t,
										 uint32_t,
										 uint32_t));
		friend VkResult(::lunaDrawBufferIndirect(LunaBuffer,
												 LunaGraphicsPipeline,
												 const LunaGraphicsPipelineBindInfo *,
												 LunaBuffer,
												 VkDeviceSize,
												 uint32_t,
												 uint32_t));
		friend VkResult(::lunaDrawBufferIndirectCount(LunaBuffer,
													  LunaGraphicsPipeline,
													  const LunaGraphicsPipelineBindInfo *,
													  LunaBuffer,
													  VkDeviceSize,
													  LunaBuffer,
													  VkDeviceSize,
													  uint32_t,
													  uint32_t));
		friend VkResult(::lunaDrawBufferIndexed(LunaBuffer,
												LunaBuffer,
												VkDeviceSize,
												VkIndexType,
												LunaGraphicsPipeline,
												const LunaGraphicsPipelineBindInfo *,
												uint32_t,
												uint32_t,
												uint32_t,
												int32_t,
												uint32_t));
		friend VkResult(::lunaDrawBufferIndexedIndirect(LunaBuffer,
														LunaBuffer,
														VkDeviceSize,
														VkIndexType,
														LunaGraphicsPipeline,
														const LunaGraphicsPipelineBindInfo *,
														LunaBuffer,
														VkDeviceSize,
														uint32_t,
														uint32_t));
		friend VkResult(::lunaDrawBufferIndexedIndirectCount(LunaBuffer,
															 LunaBuffer,
															 VkDeviceSize indexOffset,
															 VkIndexType indexType,
															 LunaGraphicsPipeline,
															 const LunaGraphicsPipelineBindInfo *,
															 LunaBuffer,
															 VkDeviceSize,
															 LunaBuffer,
															 VkDeviceSize,
															 uint32_t,
															 uint32_t));
		friend VkResult(::lunaPushConstants(LunaGraphicsPipeline));
		friend VkResult(::lunaCreateShaderModule(const uint32_t *, size_t, VkShaderModule *));
		friend void ::lunaWriteDescriptorSets(uint32_t, const LunaWriteDescriptorSet *);

		Instance() = default;
		explicit Instance(const LunaInstanceCreationInfo &creationInfo);

		VkResult destroy();

		void unbindAllPipelines();
		VkResult addNewDevice(const LunaDeviceCreationInfo2 &creationInfo);
		VkResult createSwapChain(const LunaSwapChainCreationInfo &creationInfo);
		VkResult createRenderPass(const LunaRenderPassCreationInfo *creationInfo, LunaRenderPass *renderPass);
		VkResult createRenderPass(const LunaRenderPassCreationInfo2 *creationInfo2, LunaRenderPass *renderPass);
		VkResult createDescriptorPool(const LunaDescriptorPoolCreationInfo &creationInfo,
									  LunaDescriptorPool *descriptorPool);
		VkResult createDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo,
										   LunaDescriptorSetLayout *descriptorSetLayout);
		VkResult allocateDescriptorSets(const LunaDescriptorSetAllocationInfo &allocationInfo,
										LunaDescriptorSet *descriptorSets);
		VkResult createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo,
										LunaGraphicsPipeline *pipeline);
		VkResult allocateBuffer(const LunaBufferCreationInfo &creationInfo, std::vector<Buffer>::iterator *iterator);
		VkResult createStagingBuffer(size_t size);
		void copyToStagingBuffer(const uint8_t *data, size_t size) const;
		VkResult createSampler(const LunaSamplerCreationInfo &creationInfo, LunaSampler *sampler);
		VkResult createImage(const LunaSampledImageCreationInfo &creationInfo,
							 uint32_t depth,
							 uint32_t arrayLayers,
							 LunaImage *imageIndex);

		[[nodiscard]] uint32_t minorVersion() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] const Device &device() const;
		[[nodiscard]] FamilyValues<CommandBuffer> &commandBuffers();
		[[nodiscard]] const FamilyValues<CommandBuffer> &commandBuffers() const;
		[[nodiscard]] VkSurfaceKHR surface() const;
		[[nodiscard]] const RenderPass &renderPass(LunaRenderPass renderPass) const;
		[[nodiscard]] VkDescriptorPool descriptorPool(LunaDescriptorPool descriptorPool) const;
		[[nodiscard]] const DescriptorSetLayout &descriptorSetLayout(LunaDescriptorSetLayout layout) const;
		[[nodiscard]] VkDescriptorSet descriptorSet(LunaDescriptorSet descriptorSet) const;
		void descriptorSet(LunaDescriptorSet index,
						   VkDescriptorPool *pool,
						   DescriptorSetLayout *layout,
						   VkDescriptorSet *descriptorSet) const;
		[[nodiscard]] Buffer &buffer(uint32_t index);
		[[nodiscard]] const buffer::BufferRegion &bufferRegion(LunaBuffer buffer) const;
		[[nodiscard]] const buffer::BufferRegion &bufferRegion(buffer::BufferRegionIndex index) const;
		[[nodiscard]] VkBuffer stagingBuffer() const;
		[[nodiscard]] size_t stagingBufferOffset() const;
		[[nodiscard]] VkSampler sampler(LunaSampler sampler) const;
		[[nodiscard]] VkSampler sampler(const SamplerIndex *sampler) const;
		[[nodiscard]] const Image &image(LunaImage image) const;

		SwapChain swapChain{};
		VkFormat depthImageFormat{};

	private:
		uint32_t apiVersion_{};
		VkInstance instance_{};
		Device device_{};
		VkSurfaceKHR surface_{};

		[[nodiscard]] RenderPass &renderPass_(LunaRenderPass index);
		std::list<RenderPassIndex> renderPassIndices_{};
		std::vector<RenderPass> renderPasses_{};

		std::list<DescriptorPoolIndex> descriptorPoolIndices_{};
		std::list<DescriptorSetLayoutIndex> descriptorSetLayoutIndices_{};
		std::list<DescriptorSetIndex> descriptorSetIndices_{};
		std::vector<VkDescriptorPool> descriptorPools_{};
		std::vector<DescriptorSetLayout> descriptorSetLayouts_{};
		std::vector<VkDescriptorSet> descriptorSets_{};

		std::list<GraphicsPipelineIndex> graphicsPipelineIndices_{};
		std::vector<GraphicsPipeline> graphicsPipelines_{};

		std::list<buffer::BufferRegionIndex> bufferRegionIndices_{};
		std::vector<Buffer> buffers_{};
		LunaBuffer stagingBuffer_{};

		std::list<SamplerIndex> samplerIndices_{};
		std::vector<VkSampler> samplers_{};
		std::list<ImageIndex> imageIndices_{};
		std::vector<Image> images_{};
};
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
