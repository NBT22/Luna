//
// Created by NBT22 on 2/15/25.
//

#pragma once

#include <luna/core/LogicalDevice.hpp>
#include <luna/core/PhysicalDevice.hpp>
#include <luna/lunaTypes.h>

namespace luna::core
{

inline void Instance::addNewDevice(const LunaDeviceCreationInfo2 &creationInfo)
{
	physicalDevice_ = PhysicalDevice(creationInfo.requiredFeatures);
	logicalDevice_ = LogicalDevice(physicalDevice_, creationInfo);
}

[[nodiscard]] inline uint32_t Instance::minorVersion() const
{
	return VK_API_VERSION_MINOR(apiVersion_);
}
[[nodiscard]] inline VkInstance Instance::instance() const
{
	return instance_;
}
[[nodiscard]] inline PhysicalDevice Instance::physicalDevice() const
{
	return physicalDevice_;
}
} // namespace luna::core
