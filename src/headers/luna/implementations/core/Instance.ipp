//
// Created by NBT22 on 2/15/25.
//

#pragma once

namespace luna::core
{
inline void Instance::addNewDevice(const LunaDeviceCreationInfo2 &creationInfo)
{
	device_ = Device(creationInfo);
}
inline size_t Instance::createRenderPass(const LunaRenderPassCreationInfo &creationInfo)
{
	renderPasses_.emplace_back(creationInfo);
	return renderPasses_.size() - 1;
}

inline uint32_t Instance::minorVersion() const
{
	return VK_API_VERSION_MINOR(apiVersion_);
}
inline VkInstance Instance::instance() const
{
	return instance_;
}
inline Device Instance::device() const
{
	return device_;
}
inline VkSurfaceKHR Instance::surface() const
{
	return surface_;
}
inline SwapChain Instance::swapChain() const
{
	return swapChain_;
}
inline RenderPass Instance::renderPass(const uint32_t index) const
{
	return renderPasses_.at(index);
}
} // namespace luna::core
