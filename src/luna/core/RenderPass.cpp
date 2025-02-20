//
// Created by NBT22 on 2/18/25.
//

#include <luna/core/RenderPass.hpp>
#include <luna/lunaRenderPass.h>

namespace luna::core
{
RenderPass::RenderPass(const LunaRenderPassCreationInfo &creationInfo)
{
	fillMap(creationInfo.attachmentNames, creationInfo.attachmentCount);
	fillMap(creationInfo.subpassNames, creationInfo.subpassCount);
	fillMap(creationInfo.dependencyNames, creationInfo.dependencyCount);
	fillMap(creationInfo.correlatedViewMaskNames, creationInfo.correlatedViewMaskCount);

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
}
} // namespace luna::core

LunaRenderPassSubpass lunaGetRenderPassSubpassByName(LunaRenderPass renderPass, const char *name)
{
	const size_t renderPassIndex = reinterpret_cast<size_t>(renderPass);
	const luna::core::RenderPass renderPassObject = luna::core::instance.renderPass(renderPassIndex);
	return reinterpret_cast<LunaRenderPassSubpass>(renderPassObject.getInfoIndexByName(name));
}
