//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

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
	const LunaInstanceCreationInfo creationInfo = {
		.apiVersion = VK_API_VERSION_1_2,

		.extensionCount = instanceExtensionCount,
		.extensionNames = instanceExtensions,

		.enableValidation = true,
	};
	lunaCreateInstance(&creationInfo);

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

	LunaSwapChainCreationInfo swapChainCreationInfo = {
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
