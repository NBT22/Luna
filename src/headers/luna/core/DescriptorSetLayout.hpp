//
// Created by NBT22 on 3/17/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <string>
#include <unordered_map>

namespace luna::core
{
struct DescriptorSetLayoutIndex
{
		uint32_t index;
};

class DescriptorSetLayout
{
	public:
		struct Binding
		{
				uint32_t index;
				VkDescriptorType type;
		};

		DescriptorSetLayout() = default;
		explicit DescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo);

		[[nodiscard]] VkDescriptorSetLayout layout() const;
		[[nodiscard]] const Binding &binding(const std::string &bindingName) const;

	private:
		VkDescriptorSetLayout layout_{};
		std::unordered_map<std::string, Binding> bindingIndexMap_{};
};
} // namespace luna::core

#include <luna/implementations/core/DescriptorSetLayout.ipp>
