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
		const DescriptorSetLayout &layout = descriptorSetLayout(layoutCreationInfo.descriptorSetLayouts[i]);
		descriptorSetLayouts.emplace_back(layout.layout());
	}
	uint32_t pushConstantsOffset = 0;
	std::vector<VkPushConstantRange> pushConstantRanges;
	pushConstantRanges.reserve(layoutCreationInfo.pushConstantRangeCount);
	for (uint32_t i = 0; i < layoutCreationInfo.pushConstantRangeCount; i++)
	{
		const LunaPushConstantsRange pushConstantsRange = layoutCreationInfo.pushConstantsRanges[i];
		pushConstantsRanges_.push_back(pushConstantsRange);
		pushConstantRanges.emplace_back(pushConstantsRange.stageFlags, pushConstantsOffset, pushConstantsRange.size);
		pushConstantsOffset += pushConstantsRange.size;
	}
	const VkPipelineLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.flags = layoutCreationInfo.flags,
		.setLayoutCount = descriptorSetLayoutCount,
		.pSetLayouts = descriptorSetLayouts.data(),
		.pushConstantRangeCount = layoutCreationInfo.pushConstantRangeCount,
		.pPushConstantRanges = pushConstantRanges.data(),
	};
	CHECK_RESULT_THROW(vkCreatePipelineLayout(device.logicalDevice(), &layoutCreateInfo, nullptr, &layout_));

	const RenderPassSubpassIndex *subpassIndex = static_cast<const RenderPassSubpassIndex *>(creationInfo.subpass);
	const RenderPass &renderPass = core::renderPass(subpassIndex->renderPassIndex);
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
	CHECK_RESULT_THROW(vkCreateGraphicsPipelines(device.logicalDevice(),
												 nullptr,
												 1,
												 &pipelineCreateInfo,
												 nullptr,
												 &pipeline_));

	isDestroyed_ = false;
}
void GraphicsPipeline::destroy()
{
	if (isDestroyed_)
	{
		return;
	}
	vkDestroyPipeline(device.logicalDevice(), pipeline_, nullptr);
	vkDestroyPipelineLayout(device.logicalDevice(), layout_, nullptr);

	pushConstantsRanges_.clear();
	pushConstantsRanges_.shrink_to_fit();
	isDestroyed_ = true;
}
VkResult GraphicsPipeline::bind(const LunaGraphicsPipelineBindInfo &bindInfo) const
{
	if (pipeline_ == boundPipeline)
	{
		return VK_SUCCESS;
	}
	CommandBuffer commandBuffer = device.commandBuffers().graphics;
	if (!commandBuffer.isRecording())
	{
		CHECK_RESULT_RETURN(commandBuffer.waitForFence(device.logicalDevice()));
		CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());
	}
	vkCmdBindPipeline(commandBuffer.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
	if (bindInfo.descriptorSetCount > 0)
	{
		std::vector<VkDescriptorSet> descriptorSets;
		descriptorSets.reserve(bindInfo.descriptorSetCount);
		for (uint32_t i = 0; i < bindInfo.descriptorSetCount; i++)
		{
			descriptorSets.emplace_back(descriptorSet(bindInfo.descriptorSets[i]));
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
	boundPipeline = pipeline_;
	return VK_SUCCESS;
}
} // namespace luna::core

VkResult lunaCreateGraphicsPipeline(const LunaGraphicsPipelineCreationInfo *creationInfo,
									LunaGraphicsPipeline *pipeline)
{
	using namespace luna::core;
	assert(creationInfo);
	const std::vector<GraphicsPipeline>::iterator &pipelineIterator = std::find_if(graphicsPipelines.begin(),
																				   graphicsPipelines.end(),
																				   GraphicsPipeline::isDestroyed);
	graphicsPipelineIndices.emplace_back(pipelineIterator - graphicsPipelines.begin());
	TRY_CATCH_RESULT(graphicsPipelines.emplace(pipelineIterator, *creationInfo));
	if (pipeline != nullptr)
	{
		*pipeline = &graphicsPipelineIndices.back();
	}
	return VK_SUCCESS;
}

void lunaBindDescriptorSets(const LunaGraphicsPipeline pipeline, const LunaGraphicsPipelineBindInfo *bindInfo)
{
	std::vector<VkDescriptorSet> descriptorSets;
	descriptorSets.reserve(bindInfo->descriptorSetCount);
	for (uint32_t i = 0; i < bindInfo->descriptorSetCount; i++)
	{
		descriptorSets.emplace_back(luna::core::descriptorSet(bindInfo->descriptorSets[i]));
	}
	const uint32_t pipelineIndex = static_cast<const luna::core::GraphicsPipelineIndex *>(pipeline)->index;
	vkCmdBindDescriptorSets(luna::core::device.commandBuffers().graphics.commandBuffer(),
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							luna::core::graphicsPipelines.at(pipelineIndex).layout(),
							bindInfo->firstSet,
							bindInfo->descriptorSetCount,
							descriptorSets.data(),
							bindInfo->dynamicOffsetCount,
							bindInfo->dynamicOffsets);
}

VkResult lunaPushConstants(const LunaGraphicsPipeline pipeline)
{
	const uint32_t pipelineIndex = static_cast<const luna::core::GraphicsPipelineIndex *>(pipeline)->index;
	const luna::core::GraphicsPipeline &graphicsPipeline = luna::core::graphicsPipelines.at(pipelineIndex);
	const std::vector<LunaPushConstantsRange> &pushConstantsRanges = graphicsPipeline.pushConstantsRanges_;
	luna::core::CommandBuffer commandBuffer = luna::core::device.commandBuffers().graphics;
	if (!commandBuffer.isRecording())
	{
		CHECK_RESULT_RETURN(commandBuffer.waitForFence(luna::core::device.logicalDevice()));
		CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());
	}
	uint32_t offset = 0;
	for (const LunaPushConstantsRange &pushConstantsRange: pushConstantsRanges)
	{
		const void *pushConstantsData = static_cast<const uint8_t *>(pushConstantsRange.dataPointer) +
										pushConstantsRange.dataPointerOffset;
		vkCmdPushConstants(commandBuffer.commandBuffer(),
						   graphicsPipeline.layout_,
						   pushConstantsRange.stageFlags,
						   offset,
						   pushConstantsRange.size,
						   pushConstantsData);
		offset += pushConstantsRange.size;
	}
	return VK_SUCCESS;
}
