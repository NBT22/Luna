//
// Created by NBT22 on 11/25/25.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace luna
{
class ShaderModule
{
    public:
        explicit ShaderModule(const LunaShaderModuleCreationInfo &creationInfo);

        ~ShaderModule();

        operator const VkShaderModule &() const;

        [[nodiscard]] size_t size() const;
        [[nodiscard]] const std::vector<uint32_t> &spirv() const;
        [[nodiscard]] VkShaderModule module() const;

    private:
        size_t size_{};
        std::vector<uint32_t> spirv_{};
        VkShaderModule module_{};
};
} // namespace luna

#pragma region "Implmentation"

namespace luna
{
inline size_t ShaderModule::size() const
{
    return size_;
}
inline const std::vector<uint32_t> &ShaderModule::spirv() const
{
    return spirv_;
}
inline VkShaderModule ShaderModule::module() const
{
    return module_;
}
} // namespace luna

#pragma endregion "Implmentation"
