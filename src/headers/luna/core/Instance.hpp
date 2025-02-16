//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/Device.hpp>
#include <luna/lunaTypes.h>

struct SwapChainSupportDetails
{
		uint32_t formatCount;
		uint32_t presentModeCount;
		VkSurfaceFormatKHR *formats;
		VkPresentModeKHR *presentMode;
		VkSurfaceCapabilitiesKHR capabilities;
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

		[[nodiscard]] uint32_t minorVersion() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] Device device() const;

		VkSurfaceKHR surface{};

	private:
		void querySwapChainSupport();

		uint32_t apiVersion_ = 0;
		VkInstance instance_ = VK_NULL_HANDLE;
		Device device_{};
		SwapChainSupportDetails swapChainSupport_{};
};
} // namespace luna::core

#include <luna/implementations/core/Instance.ipp>
