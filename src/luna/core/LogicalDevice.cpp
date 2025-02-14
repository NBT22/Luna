//
// Created by NBT22 on 2/13/25.
//

#include <luna/core/LogicalDevice.hpp>

namespace luna::core
{
VkDevice device;

LunaLogicalDevice createLogicalDevice()
{
	return LunaLogicalDevice();
}
} // namespace luna::core

LunaLogicalDevice lunaCreateLogicalDevice()
{
	return luna::core::createLogicalDevice();
}
