//
// Created by NBT22 on 3/17/25.
//

#pragma once
#include <stdexcept>

namespace luna::core
{
inline bool DescriptorSetLayout::isDestroyed(const DescriptorSetLayout &layout)
{
    return layout.isDestroyed_;
}

inline DescriptorSetLayout::operator const VkDescriptorSetLayout &() const
{
    return layout_;
}

inline const DescriptorSetLayout::Binding &DescriptorSetLayout::binding(const std::string &bindingName) const
{
    return bindingMap_.at(bindingName);
}

} // namespace luna::core
