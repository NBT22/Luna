//
// Created by NBT22 on 2/11/25.
//

#include <luna/luna.h>
#include <luna/lunaLogicalDevice.h>

int main()
{
	const LunaInstanceCreationInfo creationInfo = {
		.extensionCount = 2,
		.extensionNames = (const char *[]){"VK_KHR_surface", "VK_KHR_xlib_surface"},

		.enableValidation = true,
	};
	lunaCreateInstance(creationInfo, VK_API_VERSION_1_2);

	constexpr VkPhysicalDeviceFeatures requiredFeatures = {
		.logicOp = VK_TRUE,
		.samplerAnisotropy = VK_TRUE,
	};
	constexpr LunaDeviceCreationInfo deviceCreationInfo = {
		.extensionCount = 1,
		.extensionNames = (const char *const[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.requiredFeatures = requiredFeatures,
	};
	lunaAddNewDevice(deviceCreationInfo);
	return 0;
}
