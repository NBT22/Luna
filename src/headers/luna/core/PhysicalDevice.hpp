//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <vulkan/vulkan.h>

/// A pointer to this is given to the application, so that the application can give it back to Luna, that way we can
/// determine what physical device should be used for that operation. This struct should not be used for things that are
/// entirely internal. This should contain things like indexes that can be used to find the device in a list of devices
/// that is stored in some sort of global state.
struct LunaPhysicalDeviceStruct
{};

namespace luna::core
{
class PhysicalDevice
{
	public:
		PhysicalDevice() = default;
		// explicit PhysicalDevice(const VkPhysicalDeviceFeatures &requiredFeatures);
		explicit PhysicalDevice(const VkPhysicalDeviceFeatures2 &requiredFeatures);

		/// A getter for the @c device_ value
		/// @return The Vulkan handle for the physical device described in this instance
		[[nodiscard]] VkPhysicalDevice device() const;
		/// A getter for the @c graphicsFamily_ value
		/// @return The index of the family on the GPU that will be used for graphics processing
		[[nodiscard]] uint32_t graphicsFamily() const;
		/// A getter for the @c transferFamily_ value
		/// @return The index of the family on the GPU that will be used for transfer operations
		[[nodiscard]] uint32_t transferFamily() const;
		/// A getter for the @c presentationFamily_ value
		/// @return The index of the family on the GPU that will be used for presentation
		[[nodiscard]] uint32_t presentationFamily() const;
		/// A getter for the @c familyCount_ value
		/// @return The total count of unique families
		[[nodiscard]] uint32_t familyCount() const;
		// /// A getter for the @c hasPresentation_ value
		// /// @return A boolean for if there is a unique family for presentation
		// [[nodiscard]] bool hasPresentation() const;
		// /// A getter for the @c hasTransfer_ value
		// /// @return A boolean for if there is a unique family for transfer operations
		// [[nodiscard]] bool hasTransfer() const;

	private:
		/// The actual device
		VkPhysicalDevice device_ = VK_NULL_HANDLE;
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
		/// Boolean for if this device will be used for presentation. True if the device will be used for presentation,
		/// false if the device will only be used for rendering but never for presentation.
		/// @note This will eventually be used for the case of if there is one device used for rendering and a separate
		///  device used for presentation, but that ability is not a high priority, so for the time being this is uesd
		///  to indicate if the library is being used to draw to the screen at all or if it is just rendering but never
		///  presenting. TODO: As of right now, presentation is not supported so this value is always false.
		bool usesPresentation_ = false;


		void findQueueFamilyIndices(VkPhysicalDevice physicalDevice);
		void findQueueFamilyIndicesWithPresentation(VkPhysicalDevice physicalDevice);
		[[nodiscard]] bool checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const;
		[[nodiscard]] bool checkFeatureSupport(const VkBool32 *requiredFeatures) const;
		[[nodiscard]] bool checkUsability();
};
} // namespace luna::core
