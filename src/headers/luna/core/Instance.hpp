//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/Buffer.hpp>
#include <luna/core/Device.hpp>
#include <luna/core/GraphicsPipeline.hpp>
#include <luna/core/RenderPass.hpp>
#include <vector>

namespace luna::core
{
extern class Instance instance;
struct SwapChain
{
		VkSurfaceFormatKHR format;
		VkExtent2D extent;
		VkPresentModeKHR presentMode;
		uint32_t imageCount;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> framebuffers;
};

class Instance
{
	public:
		friend const buffer::BufferRegionIndex *buffer::BufferRegion::createBuffer(const LunaBufferCreationInfo &);
		Instance() = default;
		explicit Instance(const LunaInstanceCreationInfo &creationInfo);

		void addNewDevice(const LunaDeviceCreationInfo2 &creationInfo);
		void createSwapChain(const LunaSwapChainCreationInfo &creationInfo);
		const RenderPassIndex *createRenderPass(const LunaRenderPassCreationInfo &creationInfo);
		const RenderPassIndex *createRenderPass(const LunaRenderPassCreationInfo2 &creationInfo);
		const GraphicsPipelineIndex *createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);
		uint32_t allocateBuffer(const LunaBufferCreationInfo &creationInfo);

		[[nodiscard]] uint32_t minorVersion() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] const Device &device() const;
		[[nodiscard]] VkSurfaceKHR surface() const;
		[[nodiscard]] const SwapChain &swapChain() const;
		[[nodiscard]] SwapChain &swapChain();
		[[nodiscard]] RenderPass &renderPass(LunaRenderPass index);
		[[nodiscard]] GraphicsPipeline &graphicsPipeline(uint32_t index);

		bool minimized = false;
		VkFormat depthImageFormat{};

	private:
		uint32_t apiVersion_{};
		VkInstance instance_{};
		Device device_{};
		VkSurfaceKHR surface_{};
		SwapChain swapChain_{};
		std::vector<RenderPassIndex> renderPassIndices_{};
		std::unordered_map<std::string, uint32_t> renderPassMap_{};
		std::vector<RenderPass> renderPasses_{};
		std::vector<GraphicsPipelineIndex> graphicsPipelineIndices_{};
		std::unordered_map<std::string, uint32_t> graphicsPipelineMap_{};
		std::vector<GraphicsPipeline> graphicsPipelines_{};
		std::vector<buffer::BufferRegionIndex> bufferRegionIndices_{};
		std::vector<Buffer> buffers_{};
};
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
