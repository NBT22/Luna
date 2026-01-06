//
// Created by NBT22 on 11/25/25.
//

#include <array>
#include <cassert>
#include <fstream>
#include <sstream>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "luna/lunaTypes.h"
#include "ShaderModule.hpp"
#include "SlangSession.hpp"

#ifdef LUNA_SLANG_SHADERS
#include <shader-slang/slang-com-helper.h>
#include <shader-slang/slang-com-ptr.h>
#include <shader-slang/slang.h>
#endif

namespace luna
{
ShaderModule::ShaderModule(const LunaShaderModuleCreationInfo &creationInfo)
{
    if (creationInfo.creationInfoType == LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SPIRV)
    {
        size_ = creationInfo.creationInfoUnion.spirv.size;
        spirv_.insert(spirv_.begin(),
                      creationInfo.creationInfoUnion.spirv.spirv,
                      creationInfo.creationInfoUnion.spirv.spirv + creationInfo.creationInfoUnion.spirv.size / 4);

        const VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size_,
            .pCode = spirv_.data(),
        };
        CHECK_RESULT_THROW(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module_));
        return;
    }
#ifdef LUNA_SLANG_SHADERS
    const LunaSlangShaderModuleCreationInfo &slangShaderModuleCreationInfo = creationInfo.creationInfoUnion.slang;

    SlangSession *slangSession = slangShaderModuleCreationInfo.session == nullptr
                                         ? nullptr
                                         : helpers::fromHandle<SlangSession>(*slangShaderModuleCreationInfo.session);
    if (slangSession == nullptr)
    {
        if (slangShaderModuleCreationInfo.sessionCreationInfo != nullptr)
        {
            slangSessions.emplace_back(*slangShaderModuleCreationInfo.sessionCreationInfo);
            slangSession = &slangSessions.back();
        } else
        {
            if (globalSlangSession == nullptr)
            {
                constexpr SlangGlobalSessionDesc globalSessionDescription{};
                slang::createGlobalSession(&globalSessionDescription, &globalSlangSession);
                assert(globalSlangSession);
            }

            slang::TargetDesc targetDescription{};
            targetDescription.format = SLANG_SPIRV;
            switch (apiVersion)
            {
                default:
                case 0:
                    targetDescription.profile = globalSlangSession->findProfile("spirv_1_0");
                    break;
                case 1:
                    targetDescription.profile = globalSlangSession->findProfile("spirv_1_3");
                    break;
                case 2:
                    targetDescription.profile = globalSlangSession->findProfile("spirv_1_5");
                    break;
                case 3:
                case 4:
                    targetDescription.profile = globalSlangSession->findProfile("spirv_1_6");
                    break;
            }

            slang::SessionDesc sessionDescription{};
            sessionDescription.targets = &targetDescription;
            sessionDescription.targetCount = 1;
            // TODO (0.3.0): The VulkanUseEntryPointName compiler option should be automatically added to user-defined compiler options
            std::array<slang::CompilerOptionEntry, 2> compilerOptionEntries{
                slang::CompilerOptionEntry{
                    .name = slang::CompilerOptionName::VulkanUseEntryPointName,
                },
                slang::CompilerOptionEntry{
                    .name = slang::CompilerOptionName::Optimization,
                    .value = {.intValue0 = 3},
                },
            };
            sessionDescription.compilerOptionEntries = compilerOptionEntries.data();
            sessionDescription.compilerOptionEntryCount = compilerOptionEntries.size();

            slangSessions.emplace_back(sessionDescription);
            slangSession = &slangSessions.back();
        }
    }

    entryPoint_ = slangShaderModuleCreationInfo.entryPoint == nullptr ? "main"
                                                                      : slangShaderModuleCreationInfo.entryPoint;

    slang::IBlob *blob{};
    slang::IModule *module = slangSession->addComponent(
            slangSession->session()->loadModuleFromSourceString(slangShaderModuleCreationInfo.moduleName,
                                                                slangShaderModuleCreationInfo.modulePath,
                                                                slangShaderModuleCreationInfo.sourceString,
                                                                &blob));
    if (module == nullptr)
    {
        throw std::runtime_error(std::string{"Failed to compile shader! Compiler log:\n"} +
                                 static_cast<const char *>(blob->getBufferPointer()));
    }

    slang::IEntryPoint *entryPoint{};
    slangSession->addComponent<slang::IModule, slang::IEntryPoint>(module,
                                                                   &slang::IModule::findEntryPointByName,
                                                                   entryPoint_.c_str(),
                                                                   &entryPoint);
    const std::array<slang::IComponentType *, 2> componentTypes = {module, entryPoint};
    slang::IComponentType *composedProgram{};
    if (slangSession->addComponent<slang::ISession, slang::IComponentType>(slangSession->session(),
                                                                       &slang::ISession::createCompositeComponentType,
                                                                       componentTypes.data(),
                                                                       static_cast<SlangInt>(componentTypes.size()),
                                                                       &composedProgram,
                                                                       &blob) != 0)
    {
        throw std::runtime_error(std::string{"Failed to compose program! Compiler log:\n"} +
                                 static_cast<const char *>(blob->getBufferPointer()));
    }
    composedProgram->link(&slangProgram_);
    Slang::ComPtr<slang::IBlob> spirvCode{};
    slangProgram_->getEntryPointCode(0, 0, spirvCode.writeRef());

    size_ = spirvCode->getBufferSize();
    assert(size_ % 4 == 0);
    spirv_.insert(spirv_.begin(),
                  static_cast<const uint32_t *>(spirvCode->getBufferPointer()),
                  static_cast<const uint32_t *>(spirvCode->getBufferPointer()) + size_ / 4);

    const VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size_,
        .pCode = spirv_.data(),
    };
    CHECK_RESULT_THROW(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module_));
#else
    assert(creationInfo.creationInfoType == LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SPIRV);
#endif
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
