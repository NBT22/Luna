//
// Created by NBT22 on 2/13/25.
//

#include <cassert>
#include <cstring>
#include <luna/core/Instance.hpp>
#include <luna/core/PhysicalDevice.hpp>
#include <stdexcept>

namespace luna::core
{
PhysicalDevice::PhysicalDevice(const VkPhysicalDeviceFeatures2 &requiredFeatures)
{
	uint32_t match = -1u;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.instance(), &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
	}
	VkPhysicalDevice devices[deviceCount];
	vkEnumeratePhysicalDevices(instance.instance(), &deviceCount, devices);
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		device_ = devices[i];
		switch (instance.minorVersion())
		{
			case 1:
				vulkan11Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
				};
				features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
					.pNext = &vulkan11Features_,
				};
				break;
			case 2:
				vulkan12Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				};
				vulkan11Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
					.pNext = &vulkan12Features_,
				};
				features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
					.pNext = &vulkan11Features_,
				};
				break;
			case 3:
				vulkan13Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				};
				vulkan12Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
					.pNext = &vulkan13Features_,
				};
				vulkan11Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
					.pNext = &vulkan12Features_,
				};
				features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
					.pNext = &vulkan11Features_,
				};
				break;
			case 4:
				vulkan14Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
				};
				vulkan13Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
					.pNext = &vulkan14Features_,
				};
				vulkan12Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
					.pNext = &vulkan13Features_,
				};
				vulkan11Features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
					.pNext = &vulkan12Features_,
				};
				features_ = {
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
					.pNext = &vulkan11Features_,
				};
				break;
			default:
				assert(1 <= instance.minorVersion() && instance.minorVersion() <= 4);
		}
		vkGetPhysicalDeviceFeatures2(device_, &features_);
		if (!checkFeatureSupport(requiredFeatures))
		{
			return;
		}

		if (!checkUsability())
		{
			continue;
		}
		if (properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			return;
		}
		match = i;
	}

	if (match == -1u)
	{
		throw std::runtime_error("Failed to find a suitable GPU to create Vulkan instance!");
	}
}
} // namespace luna::core
