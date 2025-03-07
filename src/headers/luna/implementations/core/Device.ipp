//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <cassert>

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

// TODO: Better family finding logic to allow for
//  1. The application to tell Luna which families it would prefer to have be shared or prefer to be alone
//  2. Ensuring that the most optimal layout is found, regardless of what order the implementation provides the families
inline void Device::findQueueFamilyIndices(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
	assert(physicalDevice != VK_NULL_HANDLE);
	familyCount_ = 0;
	hasGraphics_ = false;
	hasTransfer_ = false;
	hasPresentation_ = false;

	bool presentationFound = false;
	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
	VkQueueFamilyProperties *families = new VkQueueFamilyProperties[familyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families);
	for (uint32_t index = 0; index < familyCount; index++)
	{
		VkBool32 supportsPresentation = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &supportsPresentation);
		if (!hasGraphics_ && (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			graphicsFamily_ = index;
			familyCount_++;
			hasGraphics_ = true;

			if (supportsPresentation != 0)
			{
				presentationFamily_ = index;
				presentationFound = true;
			}
		} else if (!presentationFound && supportsPresentation != 0)
		{
			presentationFamily_ = index;
			familyCount_++;
			hasPresentation_ = true;
			presentationFound = true;

			if (!hasTransfer_ && (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			{
				transferFamily_ = index;
				hasTransfer_ = true;
			}
		} else if (!hasTransfer_ && (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
		{
			transferFamily_ = index;
			familyCount_++;
			hasTransfer_ = true;
		}

		if (hasGraphics_ && hasTransfer_ && presentationFound)
		{
			break;
		}
	}
	if (!hasTransfer_ && hasGraphics_)
	{
		transferFamily_ = graphicsFamily_;
	}
	if (!presentationFound)
	{
		familyCount_ = 0;
	}
	delete[] families;
}
inline bool Device::checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const
{
	const VkBool32 *requiredFeatureArray = reinterpret_cast<const VkBool32 *>(&requiredFeatures);
	constexpr int featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
	const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&features_);
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
	switch (*reinterpret_cast<const VkStructureType *>(requiredFeatures))
	{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
		{
			constexpr int vulkan11FeatureCount = (sizeof(VkPhysicalDeviceVulkan11Features) - 16) / sizeof(VkBool32);
			const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&vulkan11Features_) + 4;
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
			const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&vulkan12Features_) + 4;
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
			const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&vulkan13Features_) + 4;
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
			const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&vulkan14Features_) + 4;
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
					structureType = *reinterpret_cast<const VkStructureType *>(requiredFeatures);
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
} // namespace luna::core
