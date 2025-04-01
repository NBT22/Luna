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

	const LunaPipelineLayoutCreationInfo layoutCreationInfo = creationInfo.layoutCreationInfo == nullptr
																	  ? LunaPipelineLayoutCreationInfo{}
																	  : *creationInfo.layoutCreationInfo;
	const uint32_t descriptorSetLayoutCount = layoutCreationInfo.descriptorSetLayoutCount;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.reserve(descriptorSetLayoutCount);
	for (uint32_t i = 0; i < descriptorSetLayoutCount; i++)
	{
		const DescriptorSetLayout &layout = instance.descriptorSetLayout(layoutCreationInfo.descriptorSetLayouts[i]);
		descriptorSetLayouts.emplace_back(layout.layout());
	}
	const VkPipelineLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.flags = layoutCreationInfo.flags,
		.setLayoutCount = descriptorSetLayoutCount,
		.pSetLayouts = descriptorSetLayouts.data(),
		.pushConstantRangeCount = layoutCreationInfo.pushConstantRangeCount,
		.pPushConstantRanges = layoutCreationInfo.pushConstantRanges,
	};
	vkCreatePipelineLayout(instance.device().logicalDevice(), &layoutCreateInfo, nullptr, &layout_);

	const RenderPassSubpassIndex *subpassIndex = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass);
	const RenderPass &renderPass = instance.renderPass(subpassIndex->renderPassIndex);
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
		.renderPass = renderPass.renderPass(),
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
	vkDestroyPipelineLayout(instance.device().logicalDevice(), layout_, nullptr);
	isDestroyed_ = true;
}
void GraphicsPipeline::bind(const LunaGraphicsPipelineBindInfo &bindInfo)
{
	if (bound_)
	{
		return;
	}
	instance.unbindAllPipelines();
	CommandBuffer commandBuffer = instance.commandBuffers().graphics;
	if (!commandBuffer.isRecording())
	{
		commandBuffer.waitForFence(instance.device().logicalDevice());
		commandBuffer.beginSingleUseCommandBuffer();
	}
	vkCmdBindPipeline(commandBuffer.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
	if (bindInfo.descriptorSetCount > 0)
	{
		std::vector<VkDescriptorSet> descriptorSets;
		descriptorSets.reserve(bindInfo.descriptorSetCount);
		for (uint32_t i = 0; i < bindInfo.descriptorSetCount; i++)
		{
			descriptorSets.emplace_back(instance.descriptorSet(bindInfo.descriptorSets[i]));
		}
		vkCmdBindDescriptorSets(commandBuffer.commandBuffer(),
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								layout_,
								bindInfo.firstSet,
								bindInfo.descriptorSetCount,
								descriptorSets.data(),
								bindInfo.dynamicOffsetCount,
								bindInfo.dynamicOffsets);
	}
	bound_ = true;
}
} // namespace luna::core

LunaGraphicsPipeline lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::instance.createGraphicsPipeline(*creationInfo);
}
