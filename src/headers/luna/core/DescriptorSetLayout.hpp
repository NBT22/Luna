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
struct DescriptorPoolIndex
{
        uint32_t index;
};
struct DescriptorSetIndex
{
        uint32_t index;
        const DescriptorSetLayoutIndex *layoutIndex;
        const DescriptorPoolIndex *poolIndex;
};

class DescriptorSetLayout
{
    public:
        struct Binding
        {
                uint32_t index;
                VkDescriptorType type;
        };

        static bool isDestroyed(const DescriptorSetLayout &layout);

        DescriptorSetLayout() = default;
        explicit DescriptorSetLayout(const LunaDescriptorSetLayoutCreationInfo &creationInfo);

        void destroy();

        [[nodiscard]] VkDescriptorSetLayout layout() const;
        [[nodiscard]] const Binding &binding(const std::string &bindingName) const;

    private:
        bool isDestroyed_{true};
        VkDescriptorSetLayout layout_{};
        std::unordered_map<std::string, Binding> bindingIndexMap_{};
};
} // namespace luna::core

#include <luna/implementations/core/DescriptorSetLayout.ipp>
