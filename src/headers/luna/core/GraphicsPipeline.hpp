//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/lunaTypes.h>
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
		GraphicsPipeline() = default;
		explicit GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);
		void destroy();

	private:
		bool isDestroyed_ = true;
		VkPipeline pipeline_{};
		std::string name_{};
		std::pmr::vector<VkDescriptorSetLayout> descriptorSetLayouts_{};
		VkPipelineLayout layout_{};

};
} // namespace luna::core
