//
// Created by NBT22 on 2/13/25.
//

#include <cstring>
#include <luna/core/Device.hpp>
#include <luna/core/Instance.hpp>
#include <luna/lunaDevice.h>
#include <stdexcept>

namespace luna::core
{
Device::Device(const LunaDeviceCreationInfo2 &creationInfo)
{
	uint32_t match = -1u;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.instance(), &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
	}
	VkPhysicalDevice *devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(instance.instance(), &deviceCount, devices);
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		physicalDevice_ = devices[i];
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
		vkGetPhysicalDeviceFeatures2(physicalDevice_, &features_);
		if (!checkFeatureSupport(creationInfo.requiredFeatures) || !checkUsability(creationInfo.surface))
		{
			continue;
		}
		match = i;
		if (properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			break;
		}
	}
	delete[] devices;
	if (match == -1u)
	{
		throw std::runtime_error("Failed to find a suitable GPU for Vulkan!");
	}

	constexpr float queuePriority = 1;
	VkDeviceQueueCreateInfo *queuesCreateInfo = new VkDeviceQueueCreateInfo[familyCount_];
	switch (familyCount_)
	{
		case 3:
			queuesCreateInfo[2] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = presentationFamily_,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		case 2:
			queuesCreateInfo[1] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = hasTransfer_ ? transferFamily_ : presentationFamily_,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		case 1:
			queuesCreateInfo[0] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = graphicsFamily_,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		default:
			assert(familyCount_ == 1 || familyCount_ == 2 || familyCount_ == 3);
	}

	const VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = creationInfo.requiredFeatures.pNext,
		.queueCreateInfoCount = familyCount_,
		.pQueueCreateInfos = queuesCreateInfo,
		.enabledExtensionCount = creationInfo.extensionCount,
		.ppEnabledExtensionNames = creationInfo.extensionNames,
		.pEnabledFeatures = &creationInfo.requiredFeatures.features,
	};
	vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_);
	delete[] queuesCreateInfo;

	vkGetDeviceQueue(logicalDevice_, graphicsFamily_, 0, &graphicsQueue_);
	vkGetDeviceQueue(logicalDevice_, transferFamily_, 0, &transferQueue_);
	vkGetDeviceQueue(logicalDevice_, presentationFamily_, 0, &presentQueue_);
}
bool Device::checkUsability(const VkSurfaceKHR surface)
{
	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &count, nullptr);
	if (count == 0)
	{
		return false;
	}
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &count, nullptr);
	if (count == 0)
	{
		return false;
	}

	findQueueFamilyIndices(physicalDevice_, surface);
	if (familyCount_ == 0)
	{
		return false;
	}

	vkGetPhysicalDeviceProperties(physicalDevice_, &properties_);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, nullptr);
	if (extensionCount == 0)
	{
		return false;
	}
	VkExtensionProperties *availableExtensions = new VkExtensionProperties[extensionCount];
	vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, availableExtensions);
	for (uint32_t j = 0; j < extensionCount; j++)
	{
		if (std::strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			delete[] availableExtensions;
			return true;
		}
	}
	delete[] availableExtensions;

	return false;
}
} // namespace luna::core

void lunaAddNewDevice(const LunaDeviceCreationInfo *creationInfo)
{
	assert(creationInfo);
	const VkPhysicalDeviceFeatures2 requiredFeatures2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.features = creationInfo->requiredFeatures,
	};
	const LunaDeviceCreationInfo2 creationInfo2 = {
		.extensionCount = creationInfo->extensionCount,
		.extensionNames = creationInfo->extensionNames,
		.requiredFeatures = requiredFeatures2,
		.surface = creationInfo->surface,
	};
	luna::core::instance.addNewDevice(creationInfo2);
}
void lunaAddNewDevice2(const LunaDeviceCreationInfo2 *creationInfo)
{
	assert(creationInfo);
	luna::core::instance.addNewDevice(*creationInfo);
}
