//
// Created by NBT22 on 11/25/25.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <luna/lunaTypes.h>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#ifdef LUNA_SLANG_SHADERS
#include <shader-slang/slang-com-helper.h>
#include <shader-slang/slang-com-ptr.h>
#endif

namespace luna
{
class ShaderModule
{
#ifdef LUNA_SLANG_SHADERS
        using SlangProgram = slang::IComponentType *;
#else
        using SlangProgram = char *;
#endif

    public:
        explicit ShaderModule(VkDevice device, const LunaShaderModuleCreationInfo &creationInfo);

        operator const VkShaderModule &() const;

        void destroy(VkDevice device);

        [[nodiscard]] size_t size() const;
        [[nodiscard]] const std::vector<uint32_t> &spirv() const;
        [[nodiscard]] VkShaderModule module() const;
        [[nodiscard]] const std::string &entryPoint() const;
        [[nodiscard]] SlangProgram slangProgram() const;

    private:
        size_t size_{};
        std::vector<uint32_t> spirv_{};
        VkShaderModule module_{};
        std::string entryPoint_{};
        SlangProgram slangProgram_{};
};
} // namespace luna

#pragma region Implementation

namespace luna
{
inline ShaderModule::operator const VkShaderModule &() const
{
    return module_;
}

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
inline const std::string &ShaderModule::entryPoint() const
{
    return entryPoint_;
}
inline ShaderModule::SlangProgram ShaderModule::slangProgram() const
{
    return slangProgram_;
}
} // namespace luna

#pragma endregion Implementation
