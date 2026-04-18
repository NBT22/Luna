//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <cstdint>
#include <list>
#include <vulkan/vulkan_core.h>
#include "Device.hpp"
#include "Luna.hpp"

#ifdef LUNA_SLANG_SHADERS
#include <shader-slang/slang.h>
#endif

namespace luna
{
extern Swapchain swapchain;
extern VkFormat depthImageFormat;
extern uint32_t apiVersion;
extern VkInstance instance;
extern std::list<Device> devices;
} // namespace luna
