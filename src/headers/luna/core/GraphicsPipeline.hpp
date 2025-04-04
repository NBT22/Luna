//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/luna.h>
#include <vector>

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

		friend VkResult(::lunaPushConstants(LunaGraphicsPipeline));

		GraphicsPipeline() = default;
		explicit GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);

		void destroy();

		VkResult bind(const LunaGraphicsPipelineBindInfo &bindInfo);
		void unbind();

	private:
		bool isDestroyed_{true};
		VkPipeline pipeline_{};
		VkPipelineLayout layout_{};
		std::vector<LunaPushConstantsRange> pushConstantsRanges_{};
		bool bound_{};
};
} // namespace luna::core

#include <luna/implementations/core/GraphicsPipeline.ipp>
