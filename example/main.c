//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

LunaRenderPass createRenderPass()
{
	lunaSetDepthImageFormat(2, (VkFormat[]){VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT});

	const VkAttachmentDescription2 colorAttachment = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
		.format = lunaGetSwapChainFormat(),
		.samples = VK_SAMPLE_COUNT_4_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentReference2 colorAttachmentRef = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentDescription2 depthAttachment = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
		.format = lunaGetDepthImageFormat(),
		.samples = VK_SAMPLE_COUNT_4_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentReference2 depthAttachmentReference = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentDescription2 colorResolveAttachment = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
		.format = lunaGetSwapChainFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	const VkAttachmentReference2 colorAttachmentResolveRef = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	const VkSubpassDescription2 wallSubpass = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pResolveAttachments = &colorAttachmentResolveRef,
		.pDepthStencilAttachment = &depthAttachmentReference,
	};
	const VkSubpassDescription2 uiSubpass = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pResolveAttachments = &colorAttachmentResolveRef,
	};

	const VkSubpassDependency2 dependencies[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		},
		{
			.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
			.dstSubpass = 1,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		},
	};

	LunaRenderPassCreationInfo renderPassCreationInfo = {
		.attachmentCount = 3,
		.attachments = (const VkAttachmentDescription2[]){colorAttachment, depthAttachment, colorResolveAttachment},
		.subpassCount = 2,
		.subpasses = (const VkSubpassDescription2[]){wallSubpass, uiSubpass},
		.dependencyCount = 2,
		.dependencies = dependencies,
	};
	return lunaCreateRenderPass(&renderPassCreationInfo);
}

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC))
	{
		return 1;
	}
	SDL_Window *window = SDL_CreateWindow("Luna Example",
										  1280,
										  720,
										  SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (window == NULL)
	{
		return 2;
	}


	uint32_t instanceExtensionCount = 0;
	const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
	const LunaInstanceCreationInfo instanceCreationInfo = {
		.apiVersion = VK_API_VERSION_1_2,

		.extensionCount = instanceExtensionCount,
		.extensionNames = instanceExtensions,

		.enableValidation = true,
	};
	lunaCreateInstance(&instanceCreationInfo);

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, lunaGetInstance(), NULL, &surface))
	{
		return 3;
	}
	const VkPhysicalDeviceFeatures requiredFeatures = {
		.logicOp = VK_TRUE,
		.samplerAnisotropy = VK_TRUE,
	};
	const LunaDeviceCreationInfo deviceCreationInfo = {
		.extensionCount = 1,
		.extensionNames = (const char *const[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.requiredFeatures = requiredFeatures,
		.surface = surface,
	};
	lunaAddNewDevice(&deviceCreationInfo);

	const LunaSwapChainCreationInfo swapChainCreationInfo = {
		.surface = surface,
		.width = 1280,
		.height = 720,
		.minImageCount = 3,
		.formatCount = 1,
		.formatPriorityList = (VkSurfaceFormatKHR[]){{VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR}},
		.presentModeCount = 2,
		.presentModePriorityList = (VkPresentModeKHR[]){VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR},
	};
	lunaCreateSwapChain(&swapChainCreationInfo);

	LunaRenderPass renderPass = createRenderPass();

	SDL_Event event;
	while (SDL_WaitEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				return 0;
			case SDL_EVENT_KEY_UP:
				if (event.key.scancode == SDL_SCANCODE_ESCAPE)
				{
					return 0;
				}
				break;
			default:;
		}
	}

	return 0;
}
