//
// Created by NBT22 on 11/25/25.
//

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Instance.hpp"
#include "Luna.hpp"
#include "luna/lunaTypes.h"
#include "ShaderModule.hpp"

namespace luna
{
ShaderModule::ShaderModule(const LunaShaderModuleCreationInfo &creationInfo):
    size_(creationInfo.size),
    spirv_(creationInfo.spirv, creationInfo.spirv + creationInfo.size / 4)
{
    assert(creationInfo.size % 4 == 0);
    const VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = creationInfo.size,
        .pCode = creationInfo.spirv,
    };
    CHECK_RESULT_THROW(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module_));
}

ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(device, module_, nullptr);
}
ShaderModule::operator const VkShaderModule &() const
{
    return module_;
}
} // namespace luna
