//
// Created by NBT22 on 2/18/25.
//

#include <array>
#include <cstring>
#include <luna/core/RenderPass.hpp>
#include <luna/lunaRenderPass.h>

namespace luna::helpers
{
// TODO: Both versions of this function will create validation messages if either attachment is set to preserve
static void createAttachments(const LunaRenderPassCreationInfo &creationInfo,
							  std::array<VkAttachmentReference, 3> &attachmentReferences,
							  std::vector<VkAttachmentDescription> &attachmentDescriptions,
							  const VkSampleCountFlagBits samples)
{
	if (creationInfo.createColorAttachment)
	{
		VkAttachmentLoadOp loadOp;
		switch (creationInfo.colorAttachmentLoadMode)
		{
			case LUNA_ATTACHMENT_LOAD_CLEAR:
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			case LUNA_ATTACHMENT_LOAD_PRESERVE:
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			default:
				loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
		}
		attachmentReferences[0] = {
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (samples != VK_SAMPLE_COUNT_1_BIT)
		{
			attachmentReferences[1] = {
				.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			attachmentDescriptions.emplace_back(0,
												core::instance.swapChain().format.format,
												samples,
												loadOp,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			attachmentDescriptions.emplace_back(0,
												core::instance.swapChain().format.format,
												VK_SAMPLE_COUNT_1_BIT,
												loadOp,
												VK_ATTACHMENT_STORE_OP_STORE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		} else
		{
			attachmentDescriptions.emplace_back(0,
												core::instance.swapChain().format.format,
												VK_SAMPLE_COUNT_1_BIT,
												loadOp,
												creationInfo.colorAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_UNDEFINED
														? VK_ATTACHMENT_STORE_OP_DONT_CARE
														: VK_ATTACHMENT_STORE_OP_STORE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	}
	if (creationInfo.createDepthAttachment)
	{
		VkAttachmentLoadOp loadOp;
		switch (creationInfo.depthAttachmentLoadMode)
		{
			case LUNA_ATTACHMENT_LOAD_CLEAR:
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			case LUNA_ATTACHMENT_LOAD_PRESERVE:
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			default:
				loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
		}
		attachmentReferences[2] = {
			.attachment = 2,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		attachmentDescriptions.emplace_back(0,
											core::instance.depthImageFormat,
											VK_SAMPLE_COUNT_4_BIT,
											loadOp,
											creationInfo.depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_PRESERVE
													? VK_ATTACHMENT_STORE_OP_STORE
													: VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_ATTACHMENT_LOAD_OP_DONT_CARE,
											VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_IMAGE_LAYOUT_UNDEFINED,
											VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
}
static void createAttachments(const LunaRenderPassCreationInfo2 &creationInfo,
							  std::array<VkAttachmentReference2, 3> &attachmentReferences,
							  std::vector<VkAttachmentDescription2> &attachmentDescriptions,
							  const VkSampleCountFlagBits samples)
{
	if (creationInfo.createColorAttachment)
	{
		VkAttachmentLoadOp loadOp;
		switch (creationInfo.colorAttachmentLoadMode)
		{
			case LUNA_ATTACHMENT_LOAD_CLEAR:
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			case LUNA_ATTACHMENT_LOAD_PRESERVE:
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			default:
				loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
		}
		attachmentReferences[0] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (samples != VK_SAMPLE_COUNT_1_BIT)
		{
			attachmentReferences[1] = {
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			attachmentDescriptions.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
												nullptr,
												0,
												core::instance.swapChain().format.format,
												samples,
												loadOp,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			attachmentDescriptions.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
												nullptr,
												0,
												core::instance.swapChain().format.format,
												VK_SAMPLE_COUNT_1_BIT,
												loadOp,
												VK_ATTACHMENT_STORE_OP_STORE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		} else
		{
			attachmentDescriptions.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
												nullptr,
												0,
												core::instance.swapChain().format.format,
												VK_SAMPLE_COUNT_1_BIT,
												loadOp,
												creationInfo.colorAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_UNDEFINED
														? VK_ATTACHMENT_STORE_OP_DONT_CARE
														: VK_ATTACHMENT_STORE_OP_STORE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	}
	if (creationInfo.createDepthAttachment)
	{
		VkAttachmentLoadOp loadOp;
		switch (creationInfo.depthAttachmentLoadMode)
		{
			case LUNA_ATTACHMENT_LOAD_CLEAR:
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			case LUNA_ATTACHMENT_LOAD_PRESERVE:
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			default:
				loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
		}
		attachmentReferences[2] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.attachment = 2,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		attachmentDescriptions.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
											nullptr,
											0,
											core::instance.depthImageFormat,
											VK_SAMPLE_COUNT_4_BIT,
											loadOp,
											creationInfo.depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_PRESERVE
													? VK_ATTACHMENT_STORE_OP_STORE
													: VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_ATTACHMENT_LOAD_OP_DONT_CARE,
											VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_IMAGE_LAYOUT_UNDEFINED,
											VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
}
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
			if (creationInfo.subpassNames[i] != nullptr)
			{
				subpassMap_[creationInfo.subpassNames[i]] = i;
			}
		}
	}
	for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
	{
		subpassIndices_.emplace_back(i, renderPassIndex);
	}
	const VkSampleCountFlagBits samples = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;
	const uint32_t attachmentCount = creationInfo.attachmentCount +
									 static_cast<uint32_t>(creationInfo.createColorAttachment) +
									 static_cast<uint32_t>(creationInfo.createDepthAttachment) +
									 static_cast<uint32_t>(creationInfo.createColorAttachment &&
														   samples != VK_SAMPLE_COUNT_1_BIT);

	std::array<VkAttachmentReference, 3> attachmentReferences{};
	std::vector<VkAttachmentDescription> attachmentDescriptions(creationInfo.attachmentCount);
	attachmentDescriptions.reserve(attachmentCount);
	if (creationInfo.attachmentCount > 0)
	{
		memcpy(attachmentDescriptions.data(),
			   creationInfo.attachments,
			   attachmentCount * sizeof(VkAttachmentDescription));
	}
	helpers::createAttachments(creationInfo, attachmentReferences, attachmentDescriptions, samples);

	std::vector<VkSubpassDescription> subpasses;
	subpasses.reserve(creationInfo.subpassCount);
	for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
	{
		const LunaSubpassCreationInfo subpassCreationInfo = creationInfo.subpasses[i];
		subpasses.emplace_back(subpassCreationInfo.flags,
							   subpassCreationInfo.pipelineBindPoint,
							   subpassCreationInfo.inputAttachmentCount,
							   subpassCreationInfo.inputAttachments,
							   subpassCreationInfo.useColorAttachment ? 1u : 0,
							   subpassCreationInfo.useColorAttachment ? &attachmentReferences[0] : nullptr,
							   subpassCreationInfo.useColorAttachment && samples != VK_SAMPLE_COUNT_1_BIT
									   ? &attachmentReferences[1]
									   : nullptr,
							   subpassCreationInfo.useDepthAttachment ? &attachmentReferences[2] : nullptr,
							   subpassCreationInfo.preserveAttachmentCount,
							   subpassCreationInfo.preserveAttachments);
	}
	const VkRenderPassCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = attachmentCount,
		.pAttachments = attachmentDescriptions.data(),
		.subpassCount = creationInfo.subpassCount,
		.pSubpasses = subpasses.data(),
		.dependencyCount = creationInfo.dependencyCount,
		.pDependencies = creationInfo.dependencies,
	};
	vkCreateRenderPass(instance.device().logicalDevice(), &createInfo, nullptr, &renderPass_);
	isDestroyed_ = false;
}
RenderPass::RenderPass(const LunaRenderPassCreationInfo2 &creationInfo, const RenderPassIndex *renderPassIndex)
{
	assert(isDestroyed_);
	subpassIndices_.reserve(creationInfo.subpassCount);
	if (creationInfo.subpassNames != nullptr)
	{
		for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
		{
			if (creationInfo.subpassNames[i] != nullptr)
			{
				subpassMap_[creationInfo.subpassNames[i]] = i;
			}
		}
	}
	for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
	{
		subpassIndices_.emplace_back(i, renderPassIndex);
	}
	const VkSampleCountFlagBits samples = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;
	const uint32_t attachmentCount = creationInfo.attachmentCount +
									 static_cast<uint32_t>(creationInfo.createColorAttachment) +
									 static_cast<uint32_t>(creationInfo.createDepthAttachment) +
									 static_cast<uint32_t>(creationInfo.createColorAttachment &&
														   samples != VK_SAMPLE_COUNT_1_BIT);

	std::array<VkAttachmentReference2, 3> attachmentReferences{};
	std::vector<VkAttachmentDescription2> attachmentDescriptions(creationInfo.attachmentCount);
	attachmentDescriptions.reserve(attachmentCount);
	if (creationInfo.attachmentCount > 0)
	{
		memcpy(attachmentDescriptions.data(),
			   creationInfo.attachments,
			   attachmentCount * sizeof(VkAttachmentDescription2));
	}
	helpers::createAttachments(creationInfo, attachmentReferences, attachmentDescriptions, samples);

	std::vector<VkSubpassDescription2> subpasses;
	subpasses.reserve(creationInfo.subpassCount);
	for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
	{
		const LunaSubpassCreationInfo2 subpassCreationInfo = creationInfo.subpasses[i];
		subpasses.emplace_back(VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
							   subpassCreationInfo.pNext,
							   subpassCreationInfo.flags,
							   subpassCreationInfo.pipelineBindPoint,
							   subpassCreationInfo.viewMask,
							   subpassCreationInfo.inputAttachmentCount,
							   subpassCreationInfo.inputAttachments,
							   subpassCreationInfo.useColorAttachment ? 1u : 0,
							   subpassCreationInfo.useColorAttachment ? &attachmentReferences[0] : nullptr,
							   subpassCreationInfo.useColorAttachment && samples != VK_SAMPLE_COUNT_1_BIT
									   ? &attachmentReferences[1]
									   : nullptr,
							   subpassCreationInfo.useDepthAttachment ? &attachmentReferences[2] : nullptr,
							   subpassCreationInfo.preserveAttachmentCount,
							   subpassCreationInfo.preserveAttachments);
	}
	const VkRenderPassCreateInfo2 createInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
		.attachmentCount = attachmentCount,
		.pAttachments = attachmentDescriptions.data(),
		.subpassCount = creationInfo.subpassCount,
		.pSubpasses = subpasses.data(),
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
LunaRenderPass lunaCreateRenderPass2(const LunaRenderPassCreationInfo2 *creationInfo)
{
	assert(creationInfo);
	return luna::core::instance.createRenderPass(*creationInfo);
}
LunaRenderPassSubpass lunaGetRenderPassSubpassByName(const LunaRenderPass renderPass, const char *name)
{
	if (name == nullptr)
	{
		const luna::core::RenderPassSubpassIndex *subpassIndex = luna::core::instance.renderPass(renderPass)
																		 .getFirstSubpass();
		return subpassIndex;
	}
	return luna::core::instance.renderPass(renderPass).getSubpassIndexByName(name);
}
