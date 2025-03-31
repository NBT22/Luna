//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/luna.h>

namespace luna::core
{
struct GraphicsPipelineIndex
{
		uint32_t index;
};
class GraphicsPipeline
{
	public:
		static bool isDestroyed(const GraphicsPipeline &graphicsPipeline);

		friend void ::lunaDrawFrame();

		GraphicsPipeline() = default;
		explicit GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);

		void destroy();

		void bind(const LunaGraphicsPipelineBindInfo &bindInfo);
		void unbind();

	private:
		bool isDestroyed_{true};
		VkPipeline pipeline_{};
		VkPipelineLayout layout_{};
		bool bound_{};
};
} // namespace luna::core

#include <luna/implementations/core/GraphicsPipeline.ipp>
