//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/PhysicalDevice.hpp>
#include <luna/lunaTypes.h>

struct LunaLogicalDeviceStruct
{};

namespace luna::core
{
class LogicalDevice
{
	private:
		VkDevice device_{};
		VkQueue graphicsQueue_{};
		VkQueue transferQueue_{};
		VkQueue presentQueue_{};

	public:
		LogicalDevice() = default;
		explicit LogicalDevice(const PhysicalDevice &physicalDevice, const LunaDeviceCreationInfo2 &creationInfo);
};

} // namespace luna::core
