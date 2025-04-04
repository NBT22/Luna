//
// Created by NBT22 on 3/1/25.
//

#include <luna/core/Instance.hpp>
#include <luna/core/Luna.hpp>
#include <luna/luna.h>

VkResult lunaCreateShaderModule(const uint32_t *spirv, const size_t bytes, VkShaderModule *shaderModule)
{
	const VkShaderModuleCreateInfo creationInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = bytes,
		.pCode = spirv,
	};
	CHECK_RESULT_RETURN(luna::core::instance.device_.addShaderModule(&creationInfo, shaderModule));
	return VK_SUCCESS;
}
VkResult lunaDrawFrame()
{
	luna::core::CommandBuffer &commandBuffer = luna::core::instance.commandBuffers().graphics;
	assert(commandBuffer.isRecording());
	vkCmdEndRenderPass(commandBuffer.commandBuffer());

	const VkSemaphore &imageAvailableSemaphore = luna::core::instance.device().imageAvailableSemaphore();
	const VkSemaphore &renderFinishedSemaphore = luna::core::instance.device().renderFinishedSemaphore();
	constexpr VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	const VkSubmitInfo queueSubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &imageAvailableSemaphore,
		.pWaitDstStageMask = &waitStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer.commandBuffer(),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderFinishedSemaphore,
	};
	CHECK_RESULT_RETURN(commandBuffer.submitCommandBuffer(luna::core::instance.device().familyQueues().graphics,
														  queueSubmitInfo));

	const VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &luna::core::instance.swapChain.swapChain,
		.pImageIndices = &luna::core::instance.swapChain.imageIndex,
	};
	CHECK_RESULT_RETURN(vkQueuePresentKHR(luna::core::instance.device().familyQueues().presentation, &presentInfo));

	luna::core::instance.swapChain.imageIndex = -1u;
	luna::core::instance.unbindAllPipelines();
	return VK_SUCCESS;
}
VkResult lunaCreateDescriptorPool(const LunaDescriptorPoolCreationInfo *creationInfo,
								  LunaDescriptorPool *descriptorPool)
{
	assert(creationInfo);
	return luna::core::instance.createDescriptorPool(*creationInfo, descriptorPool);
}
VkResult lunaAllocateDescriptorSets(const LunaDescriptorSetAllocationInfo *allocationInfo,
									LunaDescriptorSet *descriptorSets)
{
	assert(allocationInfo);
	if (allocationInfo->descriptorSetCount != 0)
	{
		assert(allocationInfo->setLayouts);
		return luna::core::instance.allocateDescriptorSets(*allocationInfo, descriptorSets);
	}
	return VK_SUCCESS;
}
void lunaWriteDescriptorSets(const uint32_t writeCount, const LunaWriteDescriptorSet *descriptorWrites)
{
	std::vector<VkWriteDescriptorSet> writes;
	writes.reserve(writeCount);
	for (uint32_t i = 0; i < writeCount; i++)
	{
		const auto [descriptorSetIndex,
					bindingName,
					descriptorArrayElement,
					descriptorCount,
					imageInfo,
					bufferInfo,
					texelBufferView] = descriptorWrites[i];
		luna::core::DescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		luna::core::instance.descriptorSet(descriptorSetIndex, nullptr, &descriptorSetLayout, &descriptorSet);
		const luna::core::DescriptorSetLayout::Binding &binding = descriptorSetLayout.binding(bindingName);

		writes.emplace_back(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
							nullptr,
							descriptorSet,
							binding.index,
							descriptorArrayElement,
							descriptorCount,
							binding.type,
							imageInfo,
							bufferInfo,
							texelBufferView);
	}
	vkUpdateDescriptorSets(luna::core::instance.device().logicalDevice(), writeCount, writes.data(), 0, nullptr);
}
