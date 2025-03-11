//
// Created by NBT22 on 2/18/25.
//

#include <algorithm>
#include <array>
#include <luna/core/RenderPass.hpp>
#include <luna/helpers/Luna.hpp>
#include <luna/lunaRenderPass.h>
#include <vk_mem_alloc.h>

// TODO: This file has large amounts of duplicated code, which leads to changes having to be made in multiple places
//  instead of just one. This is a major issue and should be addressed soon.

namespace luna::helpers
{
// TODO: Both versions of this function will create validation messages if either attachment is set to preserve
static void createAttachments(const LunaRenderPassCreationInfo &creationInfo,
							  std::array<VkAttachmentReference, 3> &attachmentReferences,
							  std::vector<VkAttachmentDescription> &attachmentDescriptions,
							  const VkSampleCountFlagBits samples)
{
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
		attachmentReferences[0] = {
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		attachmentDescriptions.emplace_back(0,
											core::instance.depthImageFormat,
											creationInfo.samples,
											loadOp,
											creationInfo.depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_PRESERVE
													? VK_ATTACHMENT_STORE_OP_STORE
													: VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_ATTACHMENT_LOAD_OP_DONT_CARE,
											VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_IMAGE_LAYOUT_UNDEFINED,
											VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
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
		attachmentReferences[1] = {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (samples != VK_SAMPLE_COUNT_1_BIT)
		{
			attachmentReferences[2] = {
				.attachment = 2,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			attachmentDescriptions.emplace_back(0,
												core::instance.swapChain.format.format,
												samples,
												loadOp,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_ATTACHMENT_LOAD_OP_DONT_CARE,
												VK_ATTACHMENT_STORE_OP_DONT_CARE,
												VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			attachmentDescriptions.emplace_back(0,
												core::instance.swapChain.format.format,
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
												core::instance.swapChain.format.format,
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
}

static void createAttachments(const LunaRenderPassCreationInfo2 &creationInfo,
							  std::array<VkAttachmentReference2, 3> &attachmentReferences,
							  std::vector<VkAttachmentDescription2> &attachmentDescriptions,
							  const VkSampleCountFlagBits samples)
{
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
		attachmentReferences[0] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		attachmentDescriptions.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
											nullptr,
											0,
											core::instance.depthImageFormat,
											samples,
											loadOp,
											creationInfo.depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_PRESERVE
													? VK_ATTACHMENT_STORE_OP_STORE
													: VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_ATTACHMENT_LOAD_OP_DONT_CARE,
											VK_ATTACHMENT_STORE_OP_DONT_CARE,
											VK_IMAGE_LAYOUT_UNDEFINED,
											VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
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
		attachmentReferences[1] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (samples != VK_SAMPLE_COUNT_1_BIT)
		{
			attachmentReferences[2] = {
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment = 2,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			attachmentDescriptions.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
												nullptr,
												0,
												core::instance.swapChain.format.format,
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
												core::instance.swapChain.format.format,
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
												core::instance.swapChain.format.format,
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
}
} // namespace luna::helpers

namespace luna::core
{
RenderPass::RenderPass(const LunaRenderPassCreationInfo &creationInfo, const RenderPassIndex *renderPassIndex)
{
	assert(isDestroyed_);
	extent_ = creationInfo.extent;
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
	samples_ = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;
	const uint32_t attachmentCount = creationInfo.attachmentCount +
									 static_cast<uint32_t>(creationInfo.createDepthAttachment) +
									 static_cast<uint32_t>(creationInfo.createColorAttachment) +
									 static_cast<uint32_t>(creationInfo.createColorAttachment &&
														   samples_ != VK_SAMPLE_COUNT_1_BIT);

	std::array<VkAttachmentReference, 3> attachmentReferences{};
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	attachmentDescriptions.reserve(attachmentCount);
	if (creationInfo.attachmentCount > 0)
	{
		std::copy_n(creationInfo.attachments, attachmentCount, attachmentDescriptions.begin());
	}
	helpers::createAttachments(creationInfo, attachmentReferences, attachmentDescriptions, samples_);

	std::vector<VkSubpassDescription> subpasses;
	subpasses.reserve(creationInfo.subpassCount);
	for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
	{
		const LunaSubpassCreationInfo &subpassCreationInfo = creationInfo.subpasses[i];
		assert((!subpassCreationInfo.useColorAttachment || creationInfo.createColorAttachment) &&
			   (!subpassCreationInfo.useDepthAttachment || creationInfo.createDepthAttachment));
		subpasses.emplace_back(subpassCreationInfo.flags,
							   subpassCreationInfo.pipelineBindPoint,
							   subpassCreationInfo.inputAttachmentCount,
							   subpassCreationInfo.inputAttachments,
							   subpassCreationInfo.useColorAttachment ? 1u : 0,
							   subpassCreationInfo.useColorAttachment ? &attachmentReferences[1] : nullptr,
							   subpassCreationInfo.useColorAttachment && samples_ != VK_SAMPLE_COUNT_1_BIT
									   ? &attachmentReferences[2]
									   : nullptr,
							   subpassCreationInfo.useDepthAttachment ? &attachmentReferences[0] : nullptr,
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

	createAttachmentImages();
	const uint32_t framebufferAttachmentCount = creationInfo.framebufferAttachmentCount +
												static_cast<uint32_t>(creationInfo.createDepthAttachment) +
												static_cast<uint32_t>(samples_ != VK_SAMPLE_COUNT_1_BIT);
	std::vector framebufferAttachments(creationInfo.framebufferAttachments,
									   creationInfo.framebufferAttachments + creationInfo.framebufferAttachmentCount);
	framebufferAttachments.reserve(framebufferAttachmentCount + 1);

	if (creationInfo.createDepthAttachment)
	{
		framebufferAttachments.emplace_back(depthImageView_);
	}
	if (samples_ != VK_SAMPLE_COUNT_1_BIT)
	{
		framebufferAttachments.emplace_back(colorImageView_);
	}
	framebufferAttachments.emplace_back(instance.swapChain.imageViews.at(0));
	createSwapChainFramebuffers(renderPass_, framebufferAttachmentCount + 1, framebufferAttachments);
	isDestroyed_ = false;
}

RenderPass::RenderPass(const LunaRenderPassCreationInfo2 &creationInfo, const RenderPassIndex *renderPassIndex)
{
	assert(isDestroyed_);
	extent_ = creationInfo.extent;
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
	samples_ = creationInfo.samples != 0 ? creationInfo.samples : VK_SAMPLE_COUNT_1_BIT;
	const uint32_t attachmentCount = creationInfo.attachmentCount +
									 static_cast<uint32_t>(creationInfo.createColorAttachment) +
									 static_cast<uint32_t>(creationInfo.createDepthAttachment) +
									 static_cast<uint32_t>(creationInfo.createColorAttachment &&
														   samples_ != VK_SAMPLE_COUNT_1_BIT);

	std::array<VkAttachmentReference2, 3> attachmentReferences{};
	std::vector<VkAttachmentDescription2> attachmentDescriptions(creationInfo.attachmentCount);
	attachmentDescriptions.reserve(attachmentCount);
	if (creationInfo.attachmentCount > 0)
	{
		std::copy_n(creationInfo.attachments, attachmentCount, attachmentDescriptions.begin());
	}
	helpers::createAttachments(creationInfo, attachmentReferences, attachmentDescriptions, samples_);

	std::vector<VkSubpassDescription2> subpasses;
	subpasses.reserve(creationInfo.subpassCount);
	for (uint32_t i = 0; i < creationInfo.subpassCount; i++)
	{
		const LunaSubpassCreationInfo2 &subpassCreationInfo = creationInfo.subpasses[i];
		assert((!subpassCreationInfo.useColorAttachment || creationInfo.createColorAttachment) &&
			   (!subpassCreationInfo.useDepthAttachment || creationInfo.createDepthAttachment));
		subpasses.emplace_back(VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
							   subpassCreationInfo.pNext,
							   subpassCreationInfo.flags,
							   subpassCreationInfo.pipelineBindPoint,
							   subpassCreationInfo.viewMask,
							   subpassCreationInfo.inputAttachmentCount,
							   subpassCreationInfo.inputAttachments,
							   subpassCreationInfo.useColorAttachment ? 1u : 0,
							   subpassCreationInfo.useColorAttachment ? &attachmentReferences[1] : nullptr,
							   subpassCreationInfo.useColorAttachment && samples_ != VK_SAMPLE_COUNT_1_BIT
									   ? &attachmentReferences[2]
									   : nullptr,
							   subpassCreationInfo.useDepthAttachment ? &attachmentReferences[0] : nullptr,
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

	createAttachmentImages();
	const uint32_t framebufferAttachmentCount = creationInfo.framebufferAttachmentCount +
												static_cast<uint32_t>(creationInfo.createDepthAttachment) +
												static_cast<uint32_t>(samples_ != VK_SAMPLE_COUNT_1_BIT);
	std::vector framebufferAttachments(creationInfo.framebufferAttachments,
									   creationInfo.framebufferAttachments + creationInfo.framebufferAttachmentCount);
	framebufferAttachments.reserve(framebufferAttachmentCount + 1);

	if (creationInfo.createDepthAttachment)
	{
		framebufferAttachments.emplace_back(depthImageView_);
	}
	if (samples_ != VK_SAMPLE_COUNT_1_BIT)
	{
		framebufferAttachments.emplace_back(colorImageView_);
	}
	framebufferAttachments.emplace_back(instance.swapChain.imageViews.at(0));
	createSwapChainFramebuffers(renderPass_, framebufferAttachmentCount + 1, framebufferAttachments);
	isDestroyed_ = false;
}

inline void RenderPass::destroy()
{
	if (isDestroyed_)
	{
		return;
	}

	for (const uint32_t pipelineIndex: pipelineIndices)
	{
		instance.graphicsPipelines.at(pipelineIndex).destroy();
	}
	vkDestroyRenderPass(instance.device().logicalDevice(), renderPass_, nullptr);
	isDestroyed_ = true;
}

// TODO: Look into VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED
inline void RenderPass::createAttachmentImages()
{
	constexpr VmaAllocationCreateInfo allocationCreateInfo = {
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	if (samples_ != VK_SAMPLE_COUNT_1_BIT)
	{
		const VkImageCreateInfo colorImageCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = instance.swapChain.format.format,
			.extent = extent_,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = samples_,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage =
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, // TODO: investigate VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			.sharingMode = instance.device().sharingMode(),
			.queueFamilyIndexCount = instance.device().familyCount(),
			.pQueueFamilyIndices = instance.device().queueFamilyIndices(),
		};
		vmaCreateImage(instance.device().allocator(),
					   &colorImageCreateInfo,
					   &allocationCreateInfo,
					   &colorImage_,
					   &colorImageAllocation_,
					   nullptr);
		helpers::createImageView(instance.device().logicalDevice(),
								 colorImage_,
								 instance.swapChain.format.format,
								 VK_IMAGE_ASPECT_COLOR_BIT,
								 1,
								 colorImageView_);
	}

	const VkImageCreateInfo depthImageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = instance.depthImageFormat,
		.extent = extent_,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = samples_,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
		.sharingMode = instance.device().sharingMode(),
		.queueFamilyIndexCount = instance.device().familyCount(),
		.pQueueFamilyIndices = instance.device().queueFamilyIndices(),
	};
	vmaCreateImage(instance.device().allocator(),
				   &depthImageCreateInfo,
				   &allocationCreateInfo,
				   &depthImage_,
				   &depthImageAllocation_,
				   nullptr);
	helpers::createImageView(instance.device().logicalDevice(),
							 depthImage_,
							 instance.depthImageFormat,
							 VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
							 1,
							 depthImageView_);
}

inline void RenderPass::createSwapChainFramebuffers(const VkRenderPass renderPass,
													const uint32_t attachmentCount,
													std::vector<VkImageView> &attachmentImages) const
{
	SwapChain &swapChain = instance.swapChain;
	swapChain.framebuffers.reserve(swapChain.imageCount);
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		attachmentImages.back() = instance.swapChain.imageViews.at(i);
		const VkFramebufferCreateInfo framebufferCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = attachmentCount,
			.pAttachments = attachmentImages.data(),
			.width = extent_.width,
			.height = extent_.height,
			.layers = 1,
		};
		vkCreateFramebuffer(instance.device().logicalDevice(),
							&framebufferCreateInfo,
							nullptr,
							&swapChain.framebuffers[i]);
	}
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
		return luna::core::instance.renderPass(renderPass).getFirstSubpass();
	}
	return luna::core::instance.renderPass(renderPass).getSubpassIndexByName(name);
}

void lunaBeginRenderPass(const LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo)
{
	assert(renderPass);
	const luna::core::Device &device = luna::core::instance.device();
	const VkCommandBuffer commandBuffer = luna::core::instance.device().commandBuffers().graphics;
	const luna::core::SwapChain &swapChain = luna::core::instance.swapChain;
	const luna::core::RenderPass &renderPassObject = luna::core::instance.renderPass(renderPass);

	if (swapChain.imageIndex == -1u)
	{
		// TODO: If one of these fails it blocks the render thread, which is unacceptable, so there should be handling
		vkWaitForFences(device.logicalDevice_, 1, &device.frameFence_, VK_TRUE, UINT64_MAX);
		vkResetFences(device.logicalDevice_, 1, &device.frameFence_);
		vkAcquireNextImageKHR(device.logicalDevice_,
							  swapChain.swapChain,
							  UINT64_MAX,
							  device.imageAvailableSemaphore_,
							  VK_NULL_HANDLE,
							  &luna::core::instance.swapChain.imageIndex);
		vkResetCommandBuffer(commandBuffer, 0);

		constexpr VkCommandBufferBeginInfo commandBufferBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		uint32_t clearValueCount = 1;
		std::vector<VkClearValue> clearValues;
		clearValues.reserve(3);
		if (renderPassObject.depthImage_ != VK_NULL_HANDLE)
		{
			clearValueCount++;
			clearValues.emplace_back(beginInfo->depthAttachmentClearValue);
		}
		if (renderPassObject.samples_ != VK_SAMPLE_COUNT_1_BIT)
		{
			clearValueCount++;
			clearValues.emplace_back(beginInfo->colorAttachmentClearValue);
		}
		clearValues.emplace_back(beginInfo->colorAttachmentClearValue);
		const VkRenderPassBeginInfo renderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderPassObject.renderPass_,
			.framebuffer = swapChain.framebuffers[swapChain.imageIndex],
			.renderArea = beginInfo->renderArea,
			.clearValueCount = clearValueCount,
			.pClearValues = clearValues.data(),
		};
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
}
