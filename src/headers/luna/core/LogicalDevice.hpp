//
// Created by NBT22 on 2/13/25.
//

#pragma once
#include <luna/lunaLogicalDevice.h>

namespace luna::core
{
extern VkDevice device;

struct LogicalDevice
{};

LunaLogicalDevice createLogicalDevice();

} // namespace luna::core

struct LunaLogicalDeviceStruct: luna::core::LogicalDevice
{};
