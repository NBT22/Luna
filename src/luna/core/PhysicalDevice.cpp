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
// PhysicalDevice::PhysicalDevice(const VkPhysicalDeviceFeatures &requiredFeatures)
// {
// 	uint32_t match = -1u;
// 	uint32_t deviceCount = 0;
// 	vkEnumeratePhysicalDevices(instance.instance(), &deviceCount, nullptr);
// 	if (deviceCount == 0)
// 	{
// 		throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
// 	}
// 	VkPhysicalDevice devices[deviceCount];
// 	vkEnumeratePhysicalDevices(instance.instance(), &deviceCount, devices);
// 	for (uint32_t i = 0; i < deviceCount; i++)
// 	{
// 		device_ = devices[i];
// 		switch (VK_API_VERSION_MINOR(instance.version()))
// 		{
// 			case 1:
// 				vulkan11Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
// 				};
// 				features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
// 					.pNext = &vulkan11Features_,
// 				};
// 				break;
// 			case 2:
// 				vulkan12Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
// 				};
// 				vulkan11Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
// 					.pNext = &vulkan12Features_,
// 				};
// 				features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
// 					.pNext = &vulkan11Features_,
// 				};
// 				break;
// 			case 3:
// 				vulkan13Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
// 				};
// 				vulkan12Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
// 					.pNext = &vulkan13Features_,
// 				};
// 				vulkan11Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
// 					.pNext = &vulkan12Features_,
// 				};
// 				features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
// 					.pNext = &vulkan11Features_,
// 				};
// 				break;
// 			case 4:
// 				vulkan14Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
// 				};
// 				vulkan13Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
// 					.pNext = &vulkan14Features_,
// 				};
// 				vulkan12Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
// 					.pNext = &vulkan13Features_,
// 				};
// 				vulkan11Features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
// 					.pNext = &vulkan12Features_,
// 				};
// 				features_ = {
// 					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
// 					.pNext = &vulkan11Features_,
// 				};
// 				break;
// 			default:
// 				assert(1 <= VK_API_VERSION_MINOR(instance.version()) && VK_API_VERSION_MINOR(instance.version()) <= 4);
// 		}
// 		vkGetPhysicalDeviceFeatures2(device_, &features_);
// 		if (!checkFeatureSupport({.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .features = requiredFeatures}))
// 		{
// 			return;
// 		}
//
// 		if (!checkUsability())
// 		{
// 			continue;
// 		}
// 		if (properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
// 		{
// 			return;
// 		}
// 		match = i;
// 	}
//
// 	if (match == -1u)
// 	{
// 		throw std::runtime_error("Failed to find a suitable GPU to create Vulkan instance!");
// 	}
// }
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

VkPhysicalDevice PhysicalDevice::device() const
{
	return device_;
}
uint32_t PhysicalDevice::graphicsFamily() const
{
	return graphicsFamily_;
}
uint32_t PhysicalDevice::transferFamily() const
{
	return transferFamily_;
}
uint32_t PhysicalDevice::presentationFamily() const
{
	return presentationFamily_;
}
uint32_t PhysicalDevice::familyCount() const
{
	return familyCount_;
}

// TODO: Better family finding logic to allow for
//  1. The application to tell Luna which families it would prefer to have be shared or prefer to be alone
//  2. Ensuring that the most optimal layout is found, regardless of what order the implementation provides the families
void PhysicalDevice::findQueueFamilyIndices(const VkPhysicalDevice physicalDevice)
{
	assert(physicalDevice != VK_NULL_HANDLE);

	familyCount_ = 0;
	hasGraphics_ = false;
	hasTransfer_ = false;

	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
	VkQueueFamilyProperties families[familyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families);
	for (uint32_t index = 0; index < familyCount; index++)
	{
		if ((families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && !hasGraphics_)
		{
			graphicsFamily_ = index;
			familyCount_++;
			hasGraphics_ = true;
		} else if ((families[index].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0 && !hasTransfer_)
		{
			transferFamily_ = index;
			familyCount_++;
			hasTransfer_ = true;
		}

		if (hasGraphics_ && hasTransfer_)
		{
			break;
		}
	}
	if (!hasTransfer_ && hasGraphics_)
	{
		transferFamily_ = graphicsFamily_;
	}
}
bool PhysicalDevice::checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const
{
	const auto *requiredFeatureArray = reinterpret_cast<const VkBool32 *>(&requiredFeatures);
	constexpr int featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
	const auto *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(&features_);
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
bool PhysicalDevice::checkFeatureSupport(const VkBool32 *requiredFeatures) const
{
	assert(requiredFeatures != nullptr);

	const VkBool32 *requiredFeatureArray = reinterpret_cast<const VkBool32 *>(&requiredFeatures) + 4;
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
			const VkStructureType structureType = *reinterpret_cast<const VkStructureType *>(requiredFeatures);
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
bool PhysicalDevice::checkUsability()
{
	/** TODO: Check swapchain support, once presentation is implemented
	 *   if (!QuerySwapChainSupport(pDevice))
	 *   {
	 *   	VulkanLogError("Failed to query swap chain support!\n");
	 *   	return false;
	 *   }
	 *   if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0)
	 *   {
	 *   	continue;
	 *   }
	 */

	findQueueFamilyIndices(device_);
	if (familyCount_ == 0)
	{
		return false;
	}

	vkGetPhysicalDeviceProperties(device_, &properties_);
	vkGetPhysicalDeviceMemoryProperties(device_, &memoryProperties_);

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device_, nullptr, &extensionCount, nullptr);
	if (extensionCount == 0)
	{
		return false;
	}
	VkExtensionProperties availableExtensions[extensionCount];
	vkEnumerateDeviceExtensionProperties(device_, nullptr, &extensionCount, availableExtensions);
	for (uint32_t j = 0; j < extensionCount; j++)
	{
		if (std::strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			return true;
		}
	}

	return false;
}
} // namespace luna::core
