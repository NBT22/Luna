//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/PhysicalDevice.hpp>
#include <luna/lunaTypes.h>
#include "LogicalDevice.hpp"

namespace luna::core
{
extern class Instance instance;

class Instance
{
	public:
		Instance() = default;
		Instance(const LunaInstanceCreationInfo &creationInfo, uint32_t apiVersion);

		void addNewDevice(const LunaDeviceCreationInfo2 &creationInfo);

		[[nodiscard]] uint32_t minorVersion() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] PhysicalDevice physicalDevice() const;

	private:
		uint32_t apiVersion_ = 0;
		VkInstance instance_ = VK_NULL_HANDLE;
		PhysicalDevice physicalDevice_{};
		LogicalDevice logicalDevice_{};
};
} // namespace luna::core
