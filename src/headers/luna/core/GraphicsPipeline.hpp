//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/luna.h>
#include <string>
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
		friend void ::lunaDrawFrame();

		GraphicsPipeline() = default;
		explicit GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);
		void destroy();

		void bind();

	private:
		bool isDestroyed_ = true;
		VkPipeline pipeline_{};
		std::string name_{};
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts_{};
		VkPipelineLayout layout_{};
		bool bound_{};
};
} // namespace luna::core
