//
// Created by NBT22 on 2/11/25.
//

#include <luna/luna.h>

void createInstanceCpp()
{
	const LunaInstanceExtensionInfo extensionInfo = {
		.extensionCount = 2,
		.extensionNames = (const char *[]){"VK_KHR_surface", "VK_KHR_xlib_surface"},
	};
	constexpr LunaInstanceLayerInfo layerInfo = {
		.enableValidation = true,
	};
	constexpr VkPhysicalDeviceFeatures2 requiredFeatures = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.features = {.samplerAnisotropy = VK_TRUE},
	};
	constexpr LunaInstanceRequirements2 instanceRequirements = {
		.apiVersion = VK_API_VERSION_1_2,
		.requiredFeatures = requiredFeatures,
	};
	lunaCreateInstance2(extensionInfo, layerInfo, instanceRequirements);
}

int main()
{
	createInstanceCpp();
	return 0;
}
