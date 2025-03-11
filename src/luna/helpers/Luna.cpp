//
// Created by NBT22 on 3/1/25.
//

#include <luna/core/Instance.hpp>
#include <luna/helpers/Luna.hpp>
#include <luna/luna.h>

namespace luna::helpers
{}

VkShaderModule lunaCreateShaderModule(const uint32_t *spirv, const size_t bytes)
{
	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = bytes,
		.pCode = spirv,
	};

	VkShaderModule shaderModule;
	vkCreateShaderModule(luna::core::instance.device().logicalDevice(), &createInfo, nullptr, &shaderModule);
	return shaderModule;
}

void lunaDrawFrame()
{
	const VkCommandBuffer commandBuffer = luna::core::instance.device().commandBuffers().graphics;
	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);

	const VkSemaphore &imageAvailableSemaphore = luna::core::instance.device().imageAvailableSemaphore();
	const VkSemaphore &renderFinishedSemaphore = luna::core::instance.device().renderFinishedSemaphore();
	constexpr VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	const VkSubmitInfo queueSubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &imageAvailableSemaphore,
		.pWaitDstStageMask = &waitStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderFinishedSemaphore,
	};
	vkQueueSubmit(luna::core::instance.device().familyQueues().graphics,
				  1,
				  &queueSubmitInfo,
				  luna::core::instance.device().frameFence());

	const VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &luna::core::instance.swapChain.swapChain,
		.pImageIndices = &luna::core::instance.swapChain.imageIndex,
	};
	vkQueuePresentKHR(luna::core::instance.device().familyQueues().presentation, &presentInfo);

	luna::core::instance.swapChain.imageIndex = -1u;
	for (luna::core::GraphicsPipeline &pipeline: luna::core::instance.graphicsPipelines)
	{
		pipeline.bound_ = false;
	}
}
