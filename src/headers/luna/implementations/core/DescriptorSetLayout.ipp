//
// Created by NBT22 on 3/17/25.
//

#pragma once
#include <stdexcept>

namespace luna::core
{
inline VkDescriptorSetLayout DescriptorSetLayout::layout() const
{
	return layout_;
}
inline const DescriptorSetLayout::Binding &DescriptorSetLayout::binding(const std::string &bindingName) const
{
	return bindingIndexMap_.at(bindingName);
}

} // namespace luna::core
