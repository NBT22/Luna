//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/lunaPhysicalDevice.h>

namespace luna::core
{
extern VkPhysicalDevice physicalDevice;

class PhysicalDevice
{};

LunaPhysicalDevice pickPhysicalDeivce();
} // namespace luna::core

struct LunaPhysicalDeviceStruct: luna::core::PhysicalDevice
{};
