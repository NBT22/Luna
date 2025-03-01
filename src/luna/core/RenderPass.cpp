//
// Created by NBT22 on 2/18/25.
//

#include <luna/core/RenderPass.hpp>
#include <luna/lunaRenderPass.h>

namespace luna::helpers
{
} // namespace luna::helpers

namespace luna::core
{
RenderPass::RenderPass(const LunaRenderPassCreationInfo &creationInfo, const RenderPassIndex *renderPassIndex)
{
	assert(isDestroyed_);
	subpassIndices_.reserve(creationInfo.subpassCount);
	if (creationInfo.subpassNames != nullptr)
	{
		for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
		{
			subpassIndices_.emplace_back(i, renderPassIndex);
			if (creationInfo.subpassNames[i] != nullptr)
			{
				subpassMap_[creationInfo.subpassNames[i]] = i;
			}
		}
	} else
	{
		for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
		{
			subpassIndices_.emplace_back(i, renderPassIndex);
		}
	}

	const VkRenderPassCreateInfo2 createInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
		.attachmentCount = creationInfo.attachmentCount,
		.pAttachments = creationInfo.attachments,
		.subpassCount = creationInfo.subpassCount,
		.pSubpasses = creationInfo.subpasses,
		.dependencyCount = creationInfo.dependencyCount,
		.pDependencies = creationInfo.dependencies,
		.correlatedViewMaskCount = creationInfo.correlatedViewMaskCount,
		.pCorrelatedViewMasks = creationInfo.correlatedViewMasks,
	};
	vkCreateRenderPass2(instance.device().logicalDevice(), &createInfo, nullptr, &renderPass_);
	isDestroyed_ = false;
}

inline void RenderPass::destroy()
{
	if (isDestroyed_)
	{
		return;
	}

	for (const uint32_t pipelineIndex: pipelineIndices_)
	{
		instance.graphicsPipeline(pipelineIndex).destroy();
	}
	vkDestroyRenderPass(instance.device().logicalDevice(), renderPass_, nullptr);
	isDestroyed_ = true;
}
} // namespace luna::core

LunaRenderPass lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::instance.createRenderPass(*creationInfo);
}
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(const LunaRenderPass renderPass, const char *name)
{
	if (name == nullptr)
	{
		const luna::core::RenderPassSubpassIndex *subpassIndex = luna::core::instance.renderPass(renderPass).getFirstSubpass();
		return subpassIndex;
	}
	return luna::core::instance.renderPass(renderPass).getSubpassIndexByName(name);
}
