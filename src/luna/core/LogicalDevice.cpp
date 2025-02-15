//
// Created by NBT22 on 2/13/25.
//

#include <cassert>
#include <luna/core/LogicalDevice.hpp>
#include "luna/core/Instance.hpp"

namespace luna::core
{
// TODO: Queue creation has a LONG way to go still.
//  1. It currently does not support using a presentation queue.
//  2. It does not allow the application to pick a preferred queue layout.
//  3. It does not allow the application to specify how many queues should be used per family.
//  4. It does not allow the application to specify flags for the queue families.
//  5. It does not allow the application to use VkDeviceQueueGlobalPriorityCreateInfoKHR to extend the queue structs.
LogicalDevice::LogicalDevice(const PhysicalDevice &physicalDevice, const LunaDeviceCreationInfo2 &creationInfo)
{
	constexpr float queuePriority = 1;
	const uint32_t familyCount = physicalDevice.familyCount();
	assert(familyCount == 1 || familyCount == 2);
	VkDeviceQueueCreateInfo queueCreateInfos[familyCount];
	if (familyCount == 1)
	{
		queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = physicalDevice.graphicsFamily(),
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};
	} else if (familyCount == 2)
	{
		queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = physicalDevice.graphicsFamily(),
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};
		queueCreateInfos[1] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = physicalDevice.transferFamily(),
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};
	}

	// TODO: Check extension support
	const VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = creationInfo.requiredFeatures.pNext,
		.queueCreateInfoCount = familyCount,
		.pQueueCreateInfos = queueCreateInfos,
		.enabledExtensionCount = creationInfo.extensionCount,
		.ppEnabledExtensionNames = creationInfo.extensionNames,
		.pEnabledFeatures = &creationInfo.requiredFeatures.features,
	};

	vkCreateDevice(physicalDevice.device(), &createInfo, nullptr, &device_);

	vkGetDeviceQueue(device_, physicalDevice.graphicsFamily(), 0, &graphicsQueue_);
	vkGetDeviceQueue(device_, physicalDevice.transferFamily(), 0, &transferQueue_);
}
} // namespace luna::core

void lunaAddNewDevice(const LunaDeviceCreationInfo &creationInfo)
{
	const VkPhysicalDeviceFeatures2 requiredFeatures2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.features = creationInfo.requiredFeatures,
	};
	const LunaDeviceCreationInfo2 creationInfo2 = {
		.extensionCount = creationInfo.extensionCount,
		.extensionNames = creationInfo.extensionNames,
		.requiredFeatures = requiredFeatures2,
	};
	luna::core::instance.addNewDevice(creationInfo2);
}
void lunaAddNewDevice2(const LunaDeviceCreationInfo2 &creationInfo)
{
	luna::core::instance.addNewDevice(creationInfo);
}
