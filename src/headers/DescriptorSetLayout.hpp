//
// Created by NBT22 on 3/17/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <string>
#include <unordered_map>

namespace luna::core
{
struct DescriptorSetIndex
{
        const VkDescriptorPool *pool;
        const class DescriptorSetLayout *layout;
        const VkDescriptorSet *set;
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

        operator const VkDescriptorSetLayout &() const;

        void destroy();

        [[nodiscard]] const Binding &binding(const std::string &bindingName) const;

    private:
        bool isDestroyed_{true};
        VkDescriptorSetLayout layout_{};
        std::unordered_map<std::string, Binding> bindingMap_{};
};
} // namespace luna::core

#include "implementations/DescriptorSetLayout.ipp"
