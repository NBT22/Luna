//
// Created by NBT22 on 2/11/25.
//

#include <luna/luna.h>

void createInstanceCpp()
{
	constexpr LunaApplicationInfo applicationInfo = {
		.applicationName = "name",
		.applicationVersion = 1,
		.engineName = "name2",
		.engineVersion = 2,
		.apiVersion = VK_API_VERSION_1_2,
	};
	const LunaInstanceExtensionInfo extensionInfo = {
		.extensionCount = 2,
		.extensionNames = (const char *[]){"VK_KHR_surface", "VK_KHR_xlib_surface"},
	};
	constexpr LunaInstanceLayerInfo layerInfo = {
		.enableValidation = true,
	};
	const LunaInstance instance = lunaCreateInstance(applicationInfo, extensionInfo, layerInfo);
}

int main()
{
	createInstanceCpp();
	return 0;
}
