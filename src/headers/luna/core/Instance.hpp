//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/Device.hpp>
#include <luna/lunaTypes.h>
#include <memory>

struct SwapChain
{
		VkSurfaceFormatKHR format;
		VkExtent2D extent;
		VkPresentModeKHR presentMode;
		uint32_t imageCount;
		VkSwapchainKHR swapChain;
		VkImage *images;
		VkImageView *imageViews;
};

namespace luna::core
{
extern class Instance instance;

class Instance
{
	public:
		Instance() = default;
		explicit Instance(const LunaInstanceCreationInfo &creationInfo);

		void addNewDevice(const LunaDeviceCreationInfo2 &creationInfo);
		void createSwapChain(const LunaSwapChainCreationInfo &creationInfo);

		[[nodiscard]] uint32_t minorVersion() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] Device device() const;
		[[nodiscard]] VkSurfaceKHR surface() const;

		bool minimized = false;

	private:

		uint32_t apiVersion_ = 0;
		VkInstance instance_ = VK_NULL_HANDLE;
		Device device_{};
		VkSurfaceKHR surface_{};
		SwapChain swapChain_{};
};
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
