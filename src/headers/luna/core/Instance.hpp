//
// Created by NBT22 on 2/13/25.
//

#pragma once

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
		VkImage *images; // TODO: free me
		VkImageView *imageViews; // TODO: free me
};

class Instance
{
	public:
		Instance() = default;
		explicit Instance(const LunaInstanceCreationInfo &creationInfo);

		void addNewDevice(const LunaDeviceCreationInfo2 &creationInfo);
		void createSwapChain(const LunaSwapChainCreationInfo &creationInfo);
		const RenderPassIndex *createRenderPass(const LunaRenderPassCreationInfo &creationInfo);
		const RenderPassIndex *createRenderPass(const LunaRenderPassCreationInfo2 &creationInfo);
		const GraphicsPipelineIndex *createGraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);

		[[nodiscard]] uint32_t minorVersion() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] Device device() const;
		[[nodiscard]] VkSurfaceKHR surface() const;
		[[nodiscard]] SwapChain swapChain() const;
		[[nodiscard]] const RenderPass &renderPass(uint32_t index) const;
		[[nodiscard]] const RenderPass &renderPass(LunaRenderPass index) const;
		[[nodiscard]] const RenderPass &renderPass(RenderPassIndex index) const;
		[[nodiscard]] GraphicsPipeline graphicsPipeline(uint32_t index) const;

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
};
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
