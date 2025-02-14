//
// Created by NBT22 on 2/13/25.
//

#include <luna/core/PhysicalDevice.hpp>
#include <luna/lunaTypes.h>
#include <vulkan/vulkan_core.h>

namespace luna::core
{
// TODO: Must be array in order to implement one GPU render and different GPU present
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

LunaPhysicalDevice pickPhysicalDeivce()
{
	return LunaPhysicalDevice();
}
} // namespace luna::core

LunaPhysicalDevice lunaPickPhysicalDevice()
{
	return luna::core::pickPhysicalDeivce();
}
