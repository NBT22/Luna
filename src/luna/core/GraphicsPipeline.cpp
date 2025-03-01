//
// Created by NBT22 on 2/25/25.
//

#include <luna/core/GraphicsPipeline.hpp>
#include <luna/core/Instance.hpp>
#include <luna/lunaPipeline.h>

namespace luna::core
{
// TODO: Base pipeline
GraphicsPipeline::GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo)
{
	assert(isDestroyed_);
	name_ = creationInfo.uniqueName != nullptr ? creationInfo.uniqueName : "";

	const uint32_t descriptorSetLayoutCount = creationInfo.layoutCreationInfo->descriptorSetLayoutCount;
	descriptorSetLayouts_.reserve(descriptorSetLayoutCount);
	for (uint32_t i = 0; i < descriptorSetLayoutCount; i++)
	{
		const auto [flags, bindingCount, bindings] = creationInfo.layoutCreationInfo->descriptorSetLayouts[i];
		const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = flags,
			.bindingCount = bindingCount,
			.pBindings = bindings,
		};
		descriptorSetLayouts_.emplace_back();
		vkCreateDescriptorSetLayout(instance.device().logicalDevice(),
									&layoutCreateInfo,
									nullptr,
									&descriptorSetLayouts_.back());
	}
	const VkPipelineLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.flags = creationInfo.flags,
		.setLayoutCount = descriptorSetLayoutCount,
		.pSetLayouts = &descriptorSetLayouts_.front(),
		.pushConstantRangeCount = creationInfo.layoutCreationInfo->pushConstantRangeCount,
		.pPushConstantRanges = creationInfo.layoutCreationInfo->pushConstantRanges,
	};
	vkCreatePipelineLayout(instance.device().logicalDevice(), &layoutCreateInfo, nullptr, &layout_);

	const RenderPassSubpassIndex *subpassIndex = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass);
	const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.flags = creationInfo.flags,
		.stageCount = creationInfo.shaderStageCount,
		.pStages = creationInfo.shaderStages,
		.pVertexInputState = creationInfo.vertexInputState,
		.pInputAssemblyState = creationInfo.inputAssemblyState,
		.pTessellationState = creationInfo.tessellationState,
		.pViewportState = creationInfo.viewportState,
		.pRasterizationState = creationInfo.rasterizationState,
		.pMultisampleState = creationInfo.multisampleState,
		.pDepthStencilState = creationInfo.depthStencilState,
		.pColorBlendState = creationInfo.colorBlendState,
		.pDynamicState = creationInfo.dynamicState,
		.layout = layout_,
		.renderPass = instance.renderPass(subpassIndex->renderPassIndex).renderPass(),
		.subpass = subpassIndex->index,
	};
	vkCreateGraphicsPipelines(instance.device().logicalDevice(), nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline_);

	isDestroyed_ = false;
}
void GraphicsPipeline::destroy()
{
	if (isDestroyed_)
	{
		return;
	}

	vkDestroyPipeline(instance.device().logicalDevice(), pipeline_, nullptr);
	isDestroyed_ = true;
}
} // namespace luna::core

LunaGraphicsPipeline lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::instance.createGraphicsPipeline(*creationInfo);
}
