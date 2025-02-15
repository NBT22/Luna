//
// Created by NBT22 on 2/11/25.
//

#include <luna/luna.h>

int main()
{
	const LunaInstanceExtensionInfo extensionInfo = {
		.extensionCount = 2,
		.extensionNames = (const char *[]){"VK_KHR_surface", "VK_KHR_xlib_surface"},
	};
	constexpr LunaInstanceLayerInfo layerInfo = {
		.enableValidation = true,
	};
	constexpr LunaInstanceRequirements instanceRequirements = {
		.apiVersion = VK_API_VERSION_1_2,
		.requiredFeatures = {.samplerAnisotropy = VK_TRUE},
	};
	lunaCreateInstance(extensionInfo, layerInfo, instanceRequirements);
	return 0;
}
