//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/PhysicalDevice.hpp>

namespace luna::core
{
extern class Instance instance;

class Instance
{
	public:
		Instance() = default;
		Instance(const LunaInstanceExtensionInfo &extensionInfo,
				 const LunaInstanceLayerInfo &layerInfo,
				 const LunaInstanceRequirements &instanceRequirements);
		Instance(const LunaInstanceExtensionInfo &extensionInfo,
				 const LunaInstanceLayerInfo &layerInfo,
				 const LunaInstanceRequirements2 &instanceRequirements);

		[[nodiscard]] uint32_t version() const;
		[[nodiscard]] VkInstance instance() const;
		[[nodiscard]] PhysicalDevice physicalDevice() const;

	private:
		VkInstance createInstance(const LunaInstanceExtensionInfo &extensionInfo,
								  const LunaInstanceLayerInfo &layerInfo,
								  uint32_t apiVersion);

		uint32_t apiVersion_ = 0;
		VkInstance instance_ = VK_NULL_HANDLE;
		PhysicalDevice physicalDevice_;
};
} // namespace luna::core
