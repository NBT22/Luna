//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <vector>
#include <vk_mem_alloc.h>

/// A pointer to this is given to the application, so that the application can give it back to Luna, that way we can
/// determine what physical device should be used for that operation. This struct should not be used for things that are
/// entirely internal. This should contain things like indexes that can be used to find the device in a list of devices
/// that is stored in some sort of global state.
struct LunaPhysicalDeviceStruct
{};

namespace luna::core
{
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
		/// The index of the family on the GPU that will be used for graphics processing
		uint32_t graphicsFamily_ = 0;
		/// The index of the family on the GPU that will be used for presentation
		uint32_t transferFamily_ = 0;
		/// The total count of unique families
		uint32_t presentationFamily_ = 0;
		/// The index of the family on the GPU that will be used for transfer operations
		bool hasGraphics_ = false;
		/// A boolean for if there is a unique family for transfer operations
		bool hasTransfer_ = false;
		/// A boolean for if there is a unique family for presentation
		bool hasPresentation_ = false;
		uint32_t familyCount_ = 0;
		VkQueue graphicsQueue_{};
		VkQueue transferQueue_{};
		VkQueue presentQueue_{};
		std::vector<uint32_t> queueFamilyIndices_{};
		VmaAllocator allocator_{};
};
} // namespace luna::core

#include <luna/implementations/core/Device.ipp>
