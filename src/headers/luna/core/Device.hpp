//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <vector>
#include <vk_mem_alloc.h>

namespace luna::core
{
template<typename T> struct FamilyValues
{
	T graphics;
	T transfer;
	T presentation;
};

class Device
{
	public:
		Device() = default;
		explicit Device(const LunaDeviceCreationInfo2 &creationInfo);

		/// A getter for the @c physicalDevice_ value
		/// @return The Vulkan handle for the physical device described in this instance
		[[nodiscard]] VkPhysicalDevice physicalDevice() const;
		/// A getter for the @c logicalDevice_ value
		/// @return The Vulkan handle for the physical device described in this instance
		[[nodiscard]] VkDevice logicalDevice() const;
		[[nodiscard]] VkSharingMode sharingMode() const;
		/// A getter for the @c familyCount_ value
		/// @return The total count of unique families
		[[nodiscard]] uint32_t familyCount() const;
		[[nodiscard]] const uint32_t *queueFamilyIndices() const;
		[[nodiscard]] VmaAllocator allocator() const;

	private:
		void findQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		void initQueueFamilyIndices();
		[[nodiscard]] bool checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const;
		[[nodiscard]] bool checkFeatureSupport(const VkBool32 *requiredFeatures) const;
		[[nodiscard]] bool checkUsability(VkSurfaceKHR surface);
		void createCommandPoolsAndBuffers();

		/// The actual device
		VkPhysicalDevice physicalDevice_{};
		VkDevice logicalDevice_{};
		VkPhysicalDeviceVulkan14Features vulkan14Features_{};
		VkPhysicalDeviceVulkan13Features vulkan13Features_{};
		VkPhysicalDeviceVulkan12Features vulkan12Features_{};
		VkPhysicalDeviceVulkan11Features vulkan11Features_{};
		VkPhysicalDeviceFeatures2 features_{};
		VkPhysicalDeviceProperties properties_{};
		VkPhysicalDeviceMemoryProperties memoryProperties_{};
		VmaAllocator allocator_{};
		uint32_t familyCount_{};
		std::vector<uint32_t> queueFamilyIndices_{};
		FamilyValues<bool> hasFamily_{};
		FamilyValues<VkQueue> familyQueues_{};
		FamilyValues<uint32_t> familyIndices_{};
		FamilyValues<VkCommandPool> commandPools_{};
		FamilyValues<VkCommandBuffer> commandBuffers_{};
};
} // namespace luna::core

#include <luna/implementations/core/Device.ipp>
