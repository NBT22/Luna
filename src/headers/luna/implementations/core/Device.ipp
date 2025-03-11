//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <cassert>
#include <luna/core/Device.hpp>

namespace luna::core
{
inline VkPhysicalDevice Device::physicalDevice() const
{
	return physicalDevice_;
}
inline VkDevice Device::logicalDevice() const
{
	return logicalDevice_;
}
inline VkSharingMode Device::sharingMode() const
{
	return familyCount_ == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
}
inline uint32_t Device::familyCount() const
{
	return familyCount_;
}
inline const uint32_t *Device::queueFamilyIndices() const
{
	return queueFamilyIndices_.data();
}
inline VmaAllocator Device::allocator() const
{
	return allocator_;
}
inline const FamilyValues<VkQueue> &Device::familyQueues() const
{
	return familyQueues_;
}
inline const FamilyValues<VkCommandPool> &Device::commandPools() const
{
	return commandPools_;
}
inline const FamilyValues<VkCommandBuffer> &Device::commandBuffers() const
{
	return commandBuffers_;
}
inline VkSemaphore Device::imageAvailableSemaphore() const
{
	return imageAvailableSemaphore_;
}
inline VkSemaphore Device::renderFinishedSemaphore() const
{
	return renderFinishedSemaphore_;
}
inline VkFence Device::frameFence() const
{
	return frameFence_;
}


// TODO: Better family finding logic to allow for
//  1. The application to tell Luna which families it would prefer to have be shared or prefer to be alone
//  2. Ensuring that the most optimal layout is found, regardless of what order the implementation provides the families
inline void Device::findQueueFamilyIndices(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
	assert(physicalDevice != VK_NULL_HANDLE);
	familyCount_ = 0;
	hasFamily_.graphics = false;
	hasFamily_.transfer = false;
	hasFamily_.presentation = false;

	bool presentationFound = false;
	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
	std::vector<VkQueueFamilyProperties> families;
	families.reserve(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());
	for (uint32_t index = 0; index < familyCount; index++)
	{
		VkBool32 supportsPresentation = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &supportsPresentation);
		if (!hasFamily_.graphics && (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			familyIndices_.graphics = index;
			familyCount_++;
			hasFamily_.graphics = true;

			if (supportsPresentation != 0)
			{
				familyIndices_.presentation = index;
				presentationFound = true;
			}
		} else if (!presentationFound && supportsPresentation != 0)
		{
			familyIndices_.presentation = index;
			familyCount_++;
			hasFamily_.presentation = true;
			presentationFound = true;

			if (!hasFamily_.transfer && (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			{
				familyIndices_.transfer = index;
				hasFamily_.transfer = true;
			}
		} else if (!hasFamily_.transfer && (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
		{
			familyIndices_.transfer = index;
			familyCount_++;
			hasFamily_.transfer = true;
		}

		if (hasFamily_.graphics && hasFamily_.transfer && presentationFound)
		{
			return;
		}
	}
	if (!hasFamily_.transfer && hasFamily_.graphics)
	{
		familyIndices_.transfer = familyIndices_.graphics;
	}
	if (!presentationFound)
	{
		familyCount_ = 0;
	}
}
inline bool Device::checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const
{
	const VkBool32 *requiredFeatureArray = std::bit_cast<const VkBool32 *,
														 const VkPhysicalDeviceFeatures2 *>(&requiredFeatures);
	constexpr int featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
	const VkBool32 *supportedFeatureArray = std::bit_cast<VkBool32 *, const VkPhysicalDeviceFeatures2 *>(&features_);
	for (int i = 0; i < featureCount; i++)
	{
		if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
		{
			return false;
		}
	}

	if (requiredFeatures.pNext != nullptr)
	{
		return checkFeatureSupport(static_cast<const VkBool32 *>(requiredFeatures.pNext));
	}

	return true;
}
inline bool Device::checkFeatureSupport(const VkBool32 *requiredFeatures) const
{
	assert(requiredFeatures);
	const VkBool32 *requiredFeatureArray = requiredFeatures + 2;
	switch (*std::bit_cast<const VkStructureType *, const VkBool32 *>(requiredFeatures))
	{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
		{
			constexpr int vulkan11FeatureCount = (sizeof(VkPhysicalDeviceVulkan11Features) - 16) / sizeof(VkBool32);
			const VkBool32 *supportedFeatureArray = &vulkan11Features_.storageBuffer16BitAccess;
			for (int i = 0; i < vulkan11FeatureCount; i++)
			{
				if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
				{
					return false;
				}
			}
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
		{
			constexpr int vulkan12FeatureCount = (sizeof(VkPhysicalDeviceVulkan12Features) - 16) / sizeof(VkBool32);
			const VkBool32 *supportedFeatureArray = &vulkan12Features_.samplerMirrorClampToEdge;
			for (int i = 0; i < vulkan12FeatureCount; i++)
			{
				if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
				{
					return false;
				}
			}
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
		{
			constexpr int vulkan13FeatureCount = (sizeof(VkPhysicalDeviceVulkan13Features) - 16) / sizeof(VkBool32);
			const VkBool32 *supportedFeatureArray = &vulkan13Features_.robustImageAccess;
			for (int i = 0; i < vulkan13FeatureCount; i++)
			{
				if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
				{
					return false;
				}
			}
			break;
		}
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
		{
			constexpr int vulkan14FeatureCount = (sizeof(VkPhysicalDeviceVulkan14Features) - 16) / sizeof(VkBool32);
			const VkBool32 *supportedFeatureArray = &vulkan14Features_.globalPriorityQuery;
			for (int i = 0; i < vulkan14FeatureCount; i++)
			{
				if (requiredFeatureArray[i] != 0 && supportedFeatureArray[i] == 0)
				{
					return false;
				}
			}
			break;
		}
		default:
			[[maybe_unused]] const VkStructureType
					structureType = *std::bit_cast<const VkStructureType *, const VkBool32 *>(requiredFeatures);
			assert(structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES ||
				   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES ||
				   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES ||
				   structureType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES);
	}

	const void *pNext = requiredFeatures + 2;
	if (pNext != nullptr)
	{
		return checkFeatureSupport(static_cast<const VkBool32 *>(pNext));
	}
	return true;
}
inline void Device::createCommandPoolsAndBuffers()
{
	if (hasFamily_.graphics)
	{
		const VkCommandPoolCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = familyIndices_.graphics,
		};
		vkCreateCommandPool(logicalDevice_, &createInfo, nullptr, &commandPools_.graphics);
		const VkCommandBufferAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPools_.graphics,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		vkAllocateCommandBuffers(logicalDevice_, &allocateInfo, &commandBuffers_.graphics);
	}
	if (hasFamily_.transfer)
	{
		const VkCommandPoolCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = familyIndices_.transfer,
		};
		vkCreateCommandPool(logicalDevice_, &createInfo, nullptr, &commandPools_.transfer);
		const VkCommandBufferAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPools_.transfer,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		vkAllocateCommandBuffers(logicalDevice_, &allocateInfo, &commandBuffers_.transfer);
	}
	if (hasFamily_.presentation)
	{
		const VkCommandPoolCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = familyIndices_.presentation,
		};
		vkCreateCommandPool(logicalDevice_, &createInfo, nullptr, &commandPools_.presentation);
		const VkCommandBufferAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPools_.presentation,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		vkAllocateCommandBuffers(logicalDevice_, &allocateInfo, &commandBuffers_.presentation);
	}
}
inline void Device::createSynchronizationObjects()
{
	constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore_);
	vkCreateSemaphore(logicalDevice_, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore_);

	constexpr VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	vkCreateFence(logicalDevice_, &fenceCreateInfo, nullptr, &frameFence_);
}
} // namespace luna::core
