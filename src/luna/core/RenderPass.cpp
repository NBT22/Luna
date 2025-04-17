//
// Created by NBT22 on 2/18/25.
//

#include <algorithm>
#include <array>
#include <luna/core/Image.hpp>
#include <luna/core/RenderPass.hpp>
#include <luna/lunaRenderPass.h>
#include <vk_mem_alloc.h>

namespace luna::helpers
{
// It's just wrong -_-
// ReSharper disable CppDFAConstantConditions
// ReSharper disable CppDFAUnreachableCode
static void createDepthAttachment(const VkSampleCountFlagBits samples,
								  const LunaAttachmentLoadMode depthAttachmentLoadMode,
								  const std::array<VkAttachmentReference *, 3> *attachmentReferences = nullptr,
								  std::array<VkAttachmentDescription, 3> *attachmentDescriptions = nullptr,
								  const std::array<VkAttachmentReference2 *, 3> *attachmentReferences2 = nullptr,
								  std::array<VkAttachmentDescription2, 3> *attachmentDescriptions2 = nullptr)
{
	VkAttachmentLoadOp loadOp;
	switch (depthAttachmentLoadMode)
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
	if (attachmentReferences2 != nullptr && attachmentDescriptions2 != nullptr)
	{
		*(*attachmentReferences2)[0] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		(*attachmentDescriptions2)[0] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
			.format = core::depthImageFormat,
			.samples = samples,
			.loadOp = loadOp,
			.storeOp = depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_PRESERVE ? VK_ATTACHMENT_STORE_OP_STORE
																				: VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
	} else
	{
		*(*attachmentReferences)[0] = {
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		(*attachmentDescriptions)[0] = {
			.format = core::depthImageFormat,
			.samples = samples,
			.loadOp = loadOp,
			.storeOp = depthAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_PRESERVE ? VK_ATTACHMENT_STORE_OP_STORE
																				: VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
	}
}
static void createColorAttachment(const uint32_t colorAttachmentIndex,
								  const VkSampleCountFlagBits samples,
								  const LunaAttachmentLoadMode colorAttachmentLoadMode,
								  const std::array<VkAttachmentReference *, 3> *attachmentReferences = nullptr,
								  std::array<VkAttachmentDescription, 3> *attachmentDescriptions = nullptr,
								  const std::array<VkAttachmentReference2 *, 3> *attachmentReferences2 = nullptr,
								  std::array<VkAttachmentDescription2, 3> *attachmentDescriptions2 = nullptr)
{
	VkAttachmentLoadOp loadOp;
	switch (colorAttachmentLoadMode)
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
	if (attachmentReferences2 != nullptr && attachmentDescriptions2 != nullptr)
	{
		*(*attachmentReferences2)[1] = {
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.attachment = colorAttachmentIndex,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (samples != VK_SAMPLE_COUNT_1_BIT)
		{
			*(*attachmentReferences2)[2] = {
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment = colorAttachmentIndex + 1,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			(*attachmentDescriptions2)[colorAttachmentIndex] = {
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format = core::swapChain.format.format,
				.samples = samples,
				.loadOp = loadOp,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			(*attachmentDescriptions2)[colorAttachmentIndex + 1] = {
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format = core::swapChain.format.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = loadOp,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};
		} else
		{
			(*attachmentDescriptions2)[colorAttachmentIndex] = {
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format = core::swapChain.format.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = loadOp,
				.storeOp = colorAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_UNDEFINED ? VK_ATTACHMENT_STORE_OP_DONT_CARE
																					 : VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};
		}
	} else
	{
		*(*attachmentReferences)[1] = {
			.attachment = colorAttachmentIndex,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (samples != VK_SAMPLE_COUNT_1_BIT)
		{
			*(*attachmentReferences)[2] = {
				.attachment = colorAttachmentIndex + 1,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			(*attachmentDescriptions)[colorAttachmentIndex] = {
				.format = core::swapChain.format.format,
				.samples = samples,
				.loadOp = loadOp,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};
			(*attachmentDescriptions)[colorAttachmentIndex + 1] = {
				.format = core::swapChain.format.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = loadOp,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};
		} else
		{
			(*attachmentDescriptions)[colorAttachmentIndex] = {
				.format = core::swapChain.format.format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = loadOp,
				.storeOp = colorAttachmentLoadMode == LUNA_ATTACHMENT_LOAD_UNDEFINED ? VK_ATTACHMENT_STORE_OP_DONT_CARE
																					 : VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};
		}
	}
}
// ReSharper restore CppDFAUnreachableCode
// ReSharper restore CppDFAConstantConditions

// TODO: Has issues with not clearing attachments
static void createAttachments(const VkSampleCountFlagBits samples,
							  const bool createColor,
							  const LunaAttachmentLoadMode colorAttachmentLoadMode,
							  const bool createDepth,
							  const LunaAttachmentLoadMode depthAttachmentLoadMode,
							  const std::array<VkAttachmentReference *, 3> *attachmentReferences = nullptr,
							  std::array<VkAttachmentDescription, 3> *attachmentDescriptions = nullptr,
							  const std::array<VkAttachmentReference2 *, 3> *attachmentReferences2 = nullptr,
							  std::array<VkAttachmentDescription2, 3> *attachmentDescriptions2 = nullptr)
{
	assert((attachmentReferences && attachmentDescriptions) || (attachmentReferences2 && attachmentDescriptions2));
	if (createDepth)
	{
		createDepthAttachment(samples,
							  depthAttachmentLoadMode,
							  attachmentReferences,
							  attachmentDescriptions,
							  attachmentReferences2,
							  attachmentDescriptions2);
	}
	if (createColor)
	{
		createColorAttachment(createDepth ? 1 : 0,
							  samples,
							  colorAttachmentLoadMode,
							  attachmentReferences,
							  attachmentDescriptions,
							  attachmentReferences2,
							  attachmentDescriptions2);
	}
}

static VkResult createRenderPass(const LunaRenderPassCreationInfo &creationInfo,
								 const uint32_t attachmentCount,
								 const VkSampleCountFlagBits samples,
								 VkRenderPass &renderPass)
{
	std::array<VkAttachmentReference *, 3> attachmentReferences{};
	attachmentReferences[0] = creationInfo.createDepthAttachment ? new VkAttachmentReference{} : nullptr;
	attachmentReferences[1] = creationInfo.createColorAttachment ? new VkAttachmentReference{} : nullptr;
	attachmentReferences[2] = creationInfo.createColorAttachment && samples != VK_SAMPLE_COUNT_1_BIT
									  ? new VkAttachmentReference{}
									  : nullptr;
	std::array<VkAttachmentDescription, 3> attachmentDescriptions{};
	createAttachments(samples,
					  creationInfo.createColorAttachment,
					  creationInfo.colorAttachmentLoadMode,
					  creationInfo.createDepthAttachment,
					  creationInfo.depthAttachmentLoadMode,
					  &attachmentReferences,
					  &attachmentDescriptions);

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
							   attachmentReferences[1],
							   attachmentReferences[2],
							   attachmentReferences[0],
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
	CHECK_RESULT_RETURN(vkCreateRenderPass(core::device.logicalDevice(), &createInfo, nullptr, &renderPass));
	return VK_SUCCESS;
}
static VkResult createRenderPass2(const LunaRenderPassCreationInfo2 &creationInfo,
								  const uint32_t attachmentCount,
								  const VkSampleCountFlagBits samples,
								  VkRenderPass &renderPass)
{
	std::array<VkAttachmentReference2 *, 3> attachmentReferences{};
	std::array<VkAttachmentDescription2, 3> attachmentDescriptions{};
	createAttachments(samples,
					  creationInfo.createColorAttachment,
					  creationInfo.colorAttachmentLoadMode,
					  creationInfo.createDepthAttachment,
					  creationInfo.depthAttachmentLoadMode,
					  nullptr,
					  nullptr,
					  &attachmentReferences,
					  &attachmentDescriptions);

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
							   attachmentReferences[1],
							   attachmentReferences[2],
							   attachmentReferences[0],
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
	CHECK_RESULT_RETURN(vkCreateRenderPass2(core::device.logicalDevice(), &createInfo, nullptr, &renderPass));
	return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core
{
RenderPass::RenderPass(const LunaRenderPassCreationInfo *creationInfo,
					   const LunaRenderPassCreationInfo2 *creationInfo2,
					   const RenderPassIndex *renderPassIndex)
{
	assert(isDestroyed_ && creationInfo);
	extent_ = creationInfo->extent;
	subpassIndices_.reserve(creationInfo->subpassCount);
	samples_ = creationInfo->samples != 0 ? creationInfo->samples : VK_SAMPLE_COUNT_1_BIT;
	const uint32_t attachmentCount = static_cast<uint32_t>(creationInfo->createDepthAttachment) +
									 static_cast<uint32_t>(creationInfo->createColorAttachment) +
									 static_cast<uint32_t>(creationInfo->createColorAttachment &&
														   samples_ != VK_SAMPLE_COUNT_1_BIT);

	if (creationInfo2 != nullptr)
	{
		for (uint32_t i = 0; i < creationInfo2->subpassCount; i++)
		{
			if (creationInfo2->subpasses[i].name != nullptr)
			{
				subpassMap_[creationInfo2->subpasses[i].name] = i;
			}
			subpassIndices_.emplace_back(i, renderPassIndex);
		}
		CHECK_RESULT_THROW(helpers::createRenderPass2(*creationInfo2, attachmentCount, samples_, renderPass_));
	} else
	{
		for (uint32_t i = 0; i < creationInfo->subpassCount; i++)
		{
			if (creationInfo->subpasses[i].name != nullptr)
			{
				subpassMap_[creationInfo->subpasses[i].name] = i;
			}
			subpassIndices_.emplace_back(i, renderPassIndex);
		}
		CHECK_RESULT_THROW(helpers::createRenderPass(*creationInfo, attachmentCount, samples_, renderPass_));
	}

	CHECK_RESULT_THROW(createAttachmentImages(creationInfo->createDepthAttachment));
	const uint32_t framebufferAttachmentCount = creationInfo->framebufferAttachmentCount +
												static_cast<uint32_t>(creationInfo->createDepthAttachment) +
												static_cast<uint32_t>(samples_ != VK_SAMPLE_COUNT_1_BIT);
	std::vector framebufferAttachments(creationInfo->framebufferAttachments,
									   creationInfo->framebufferAttachments + creationInfo->framebufferAttachmentCount);
	framebufferAttachments.reserve(framebufferAttachmentCount + 1);

	if (creationInfo->createDepthAttachment)
	{
		framebufferAttachments.emplace_back(depthImageView_);
	}
	if (samples_ != VK_SAMPLE_COUNT_1_BIT)
	{
		framebufferAttachments.emplace_back(colorImageView_);
	}
	framebufferAttachments.emplace_back(swapChain.imageViews.at(0));
	CHECK_RESULT_THROW(createSwapChainFramebuffers(renderPass_,
												   framebufferAttachmentCount + 1,
												   framebufferAttachments));
	isDestroyed_ = false;
}

void RenderPass::destroy()
{
	if (isDestroyed_)
	{
		return;
	}
	vkDestroyImageView(device.logicalDevice(), colorImageView_, nullptr);
	vkDestroyImageView(device.logicalDevice(), depthImageView_, nullptr);
	vmaDestroyImage(device.allocator(), colorImage_, colorImageAllocation_);
	vmaDestroyImage(device.allocator(), depthImage_, depthImageAllocation_);
	vkDestroyRenderPass(device.logicalDevice(), renderPass_, nullptr);
	name_.clear();
	name_.shrink_to_fit();
	subpassIndices_.clear();
	subpassIndices_.shrink_to_fit();
	subpassMap_.clear();

	isDestroyed_ = true;
}

// TODO: Look into VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED
inline VkResult RenderPass::createAttachmentImages(const bool createDepthImage)
{
	constexpr VmaAllocationCreateInfo allocationCreateInfo = {
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	if (samples_ != VK_SAMPLE_COUNT_1_BIT)
	{
		const VkImageCreateInfo colorImageCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = swapChain.format.format,
			.extent = extent_,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = samples_,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage =
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, // TODO: investigate VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
			.sharingMode = device.sharingMode(),
			.queueFamilyIndexCount = device.familyCount(),
			.pQueueFamilyIndices = device.queueFamilyIndices(),
		};
		CHECK_RESULT_RETURN(vmaCreateImage(device.allocator(),
										   &colorImageCreateInfo,
										   &allocationCreateInfo,
										   &colorImage_,
										   &colorImageAllocation_,
										   nullptr));
		CHECK_RESULT_RETURN(helpers::createImageView(device.logicalDevice(),
													 colorImage_,
													 swapChain.format.format,
													 VK_IMAGE_ASPECT_COLOR_BIT,
													 1,
													 &colorImageView_));
	}

	if (createDepthImage)
	{
		const VkImageCreateInfo depthImageCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = depthImageFormat,
			.extent = extent_,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = samples_,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
			.sharingMode = device.sharingMode(),
			.queueFamilyIndexCount = device.familyCount(),
			.pQueueFamilyIndices = device.queueFamilyIndices(),
		};
		CHECK_RESULT_RETURN(vmaCreateImage(device.allocator(),
										   &depthImageCreateInfo,
										   &allocationCreateInfo,
										   &depthImage_,
										   &depthImageAllocation_,
										   nullptr));
		CHECK_RESULT_RETURN(helpers::createImageView(device.logicalDevice(),
													 depthImage_,
													 depthImageFormat,
													 VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
													 1,
													 &depthImageView_));
	}

	return VK_SUCCESS;
}

inline VkResult RenderPass::createSwapChainFramebuffers(const VkRenderPass renderPass,
														const uint32_t attachmentCount,
														std::vector<VkImageView> &attachmentImages) const
{
	swapChain.framebuffers.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < swapChain.imageCount; i++)
	{
		attachmentImages.back() = swapChain.imageViews.at(i);
		const VkFramebufferCreateInfo framebufferCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = attachmentCount,
			.pAttachments = attachmentImages.data(),
			.width = extent_.width,
			.height = extent_.height,
			.layers = 1,
		};
		CHECK_RESULT_RETURN(vkCreateFramebuffer(device.logicalDevice(),
												&framebufferCreateInfo,
												nullptr,
												&swapChain.framebuffers[i]));
	}
	return VK_SUCCESS;
}
} // namespace luna::core

VkResult lunaCreateRenderPass(const LunaRenderPassCreationInfo *creationInfo, LunaRenderPass *renderPass)
{
	using namespace luna::core;
	assert(creationInfo);
	const std::vector<RenderPass>::iterator &renderPassIterator = std::find_if(renderPasses.begin(),
																			   renderPasses.end(),
																			   RenderPass::isDestroyed);

	renderPassIndices.emplace_back(renderPassIterator - renderPasses.begin());
	TRY_CATCH_RESULT(renderPasses.emplace(renderPassIterator, creationInfo, nullptr, &renderPassIndices.back()));
	if (renderPass != nullptr)
	{
		*renderPass = &renderPassIndices.back();
	}
	return VK_SUCCESS;
}

VkResult lunaCreateRenderPass2(const LunaRenderPassCreationInfo2 *creationInfo, LunaRenderPass *renderPass)
{
	using namespace luna::core;
	assert(creationInfo);
	const std::vector<RenderPass>::iterator &renderPassIterator = std::find_if(renderPasses.begin(),
																			   renderPasses.end(),
																			   RenderPass::isDestroyed);

	const LunaRenderPassCreationInfo renderPassCreationInfo = {
		.samples = creationInfo->samples,
		.createColorAttachment = creationInfo->createColorAttachment,
		.colorAttachmentLoadMode = creationInfo->colorAttachmentLoadMode,
		.createDepthAttachment = creationInfo->createDepthAttachment,
		.depthAttachmentLoadMode = creationInfo->depthAttachmentLoadMode,
		// .attachmentCount = creationInfo->attachmentCount,
		.subpassCount = creationInfo->subpassCount,
		.dependencyCount = creationInfo->dependencyCount,
		.extent = creationInfo->extent,
		.framebufferAttachmentCount = creationInfo->framebufferAttachmentCount,
	};
	renderPassIndices.emplace_back(renderPassIterator - renderPasses.begin());
	TRY_CATCH_RESULT(renderPasses.emplace(renderPassIterator,
										  &renderPassCreationInfo,
										  creationInfo,
										  &renderPassIndices.back()));
	if (renderPass != nullptr)
	{
		*renderPass = &renderPassIndices.back();
	}
	return VK_SUCCESS;
}

LunaRenderPassSubpass lunaGetRenderPassSubpassByName(const LunaRenderPass renderPass, const char *name)
{
	if (name == nullptr)
	{
		return luna::core::renderPass(renderPass).getFirstSubpass();
	}
	return luna::core::renderPass(renderPass).getSubpassIndexByName(name);
}

VkResult lunaBeginRenderPass(const LunaRenderPass renderPass, const LunaRenderPassBeginInfo *beginInfo)
{
	using namespace luna::core;
	assert(renderPass);
	CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	const RenderPass &renderPassObject = luna::core::renderPass(renderPass);

	if (swapChain.imageIndex == -1u)
	{
		// TODO: If this fails it blocks the render thread, which is unacceptable, so there should be handling
		CHECK_RESULT_RETURN(commandBuffer.waitForFence(device.logicalDevice_));
		CHECK_RESULT_RETURN(commandBuffer.resetFence(device.logicalDevice_));
		CHECK_RESULT_RETURN(vkAcquireNextImageKHR(device.logicalDevice_,
												  swapChain.swapChain,
												  UINT64_MAX,
												  device.imageAvailableSemaphore_,
												  VK_NULL_HANDLE,
												  &swapChain.imageIndex));
		CHECK_RESULT_RETURN(commandBuffer.beginSingleUseCommandBuffer());

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
		vkCmdBeginRenderPass(commandBuffer.commandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	return VK_SUCCESS;
}
void lunaNextSubpass()
{
	const luna::core::CommandBuffer &commandBuffer = luna::core::device.commandBuffers().graphics;
	assert(commandBuffer.isRecording());
	vkCmdNextSubpass(commandBuffer.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
}
void lunaEndRenderPass()
{
	const luna::core::CommandBuffer &commandBuffer = luna::core::device.commandBuffers().graphics;
	assert(commandBuffer.isRecording());
	vkCmdEndRenderPass(commandBuffer.commandBuffer());
}
