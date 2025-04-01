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
		friend const buffer::BufferRegionIndex *buffer::BufferRegion::createBuffer(const LunaBufferCreationInfo &);
		friend void ::lunaDrawBuffer(const LunaVertexBufferDrawInfo *);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
		friend VkShaderModule(::lunaCreateShaderModule(const uint32_t *, size_t));
#pragma GCC diagnostic pop

		Instance() = default;
		explicit Instance(const LunaInstanceCreationInfo &creationInfo);

		void destroy();

		void unbindAllPipelines();
		void addNewDevice(const LunaDeviceCreationInfo2 &creationInfo);
		void createSwapChain(const LunaSwapChainCreationInfo &creationInfo);
		const RenderPassIndex *createRenderPass(const LunaRenderPassCreationInfo *creationInfo);
		const RenderPassIndex *createRenderPass(const LunaRenderPassCreationInfo2 *creationInfo2);
		const DescriptorPoolIndex *createDescriptorPool(const LunaDescriptorPoolCreationInfo &creationInfo);
		const DescriptorSetLayoutIndex *createDescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo
																		  &creationInfo);
		void allocateDescriptorSets(const LunaDescriptorSetAllocationInfo &allocationInfo,
									LunaDescriptorSet *descriptorSets);
		const GraphicsPipelineIndex *createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);
		std::vector<Buffer>::iterator allocateBuffer(const LunaBufferCreationInfo &creationInfo);
		void createStagingBuffer(size_t size);
		void copyToStagingBuffer(const uint8_t *data, size_t size) const;
		const SamplerIndex *createSampler(const LunaSamplerCreationInfo &creationInfo);
		const ImageIndex *createImage(const LunaSampledImageCreationInfo &creationInfo,
									  uint32_t depth,
									  uint32_t arrayLayers);

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
