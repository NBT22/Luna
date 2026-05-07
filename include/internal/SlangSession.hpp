//
// Created by NBT22 on 12/5/25.
//

#pragma once

#ifdef LUNA_SLANG_SHADERS

#include <luna/luna.h>
#include <shader-slang/slang.h>

static_assert(true);
namespace luna
{
class SlangSession
{
    public:
        explicit SlangSession(const LunaSlangSessionCreationInfo &creationInfo);
        explicit SlangSession(const slang::SessionDesc &sessionDescription);

        template<std::derived_from<ISlangUnknown> T> [[nodiscard]] T *addComponent(T *component);
        template<std::derived_from<ISlangUnknown> ThisType,
                 std::derived_from<ISlangUnknown> ConstructedType,
                 typename... Args>
        [[nodiscard]] SlangResult addComponent(ThisType *thisObject,
                                               SlangResult (ThisType::*method)(Args...),
                                               Args... args);

        [[nodiscard]] slang::ISession *session();
        [[nodiscard]] const slang::ISession *session() const;

    private:
        slang::ISession *session_{};
        slang::IModule *lunaModule_{};
        std::vector<ISlangUnknown *> components_{};
};

extern slang::IGlobalSession *globalSlangSession;
extern std::list<SlangSession> slangSessions;

} // namespace luna

#pragma region Implementation

#include "Instance.hpp"
#include "Shaders.hpp"

namespace luna
{
inline SlangSession::SlangSession(const LunaSlangSessionCreationInfo &creationInfo)
{
    if (globalSlangSession == nullptr)
    {
        constexpr SlangGlobalSessionDesc globalSessionDescription{};
        slang::createGlobalSession(&globalSessionDescription, &globalSlangSession);
        assert(globalSlangSession);
    }

    std::vector<slang::CompilerOptionEntry> targetCompilerOptions;
    targetCompilerOptions.reserve(creationInfo.targetCompilerOptionCount);
    for (uint32_t i = 0; i < creationInfo.targetCompilerOptionCount; i++)
    {
        const LunaSlangCompilerOption &compilerOption = creationInfo.targetCompilerOptions[i];
        const LunaSlangCompilerOptionValue &value = compilerOption.value;
        targetCompilerOptions.emplace_back(static_cast<slang::CompilerOptionName>(compilerOption.name),
                                           slang::CompilerOptionValue{
                                               .kind = static_cast<slang::CompilerOptionValueKind>(value.type),
                                               .intValue0 = value.intValue0,
                                               .intValue1 = value.intValue1,
                                               .stringValue0 = value.stringValue0,
                                               .stringValue1 = value.stringValue1,
                                           });
    }

    slang::TargetDesc targetDescription{};
    targetDescription.format = SLANG_SPIRV;
    targetDescription.profile = globalSlangSession->findProfile(creationInfo.spirvProfile);
    targetDescription.compilerOptionEntries = targetCompilerOptions.data();
    targetDescription.compilerOptionEntryCount = targetCompilerOptions.size();

    std::vector<slang::PreprocessorMacroDesc> preprocessorMacroDescriptions;
    preprocessorMacroDescriptions.reserve(creationInfo.preprocessorMacroCount);
    for (uint32_t i = 0; i < creationInfo.preprocessorMacroCount; i++)
    {
        const LunaSlangPreprocessorMacroDescription &macroDescription = creationInfo.preprocessorMacros[i];
        preprocessorMacroDescriptions.emplace_back(macroDescription.name, macroDescription.value);
    }
    std::vector<slang::CompilerOptionEntry> compilerOptions;
    compilerOptions.reserve(creationInfo.compilerOptionCount);
    for (uint32_t i = 0; i < creationInfo.compilerOptionCount; i++)
    {
        const LunaSlangCompilerOption &compilerOption = creationInfo.compilerOptions[i];
        const LunaSlangCompilerOptionValue &value = compilerOption.value;
        compilerOptions.emplace_back(static_cast<slang::CompilerOptionName>(compilerOption.name),
                                     slang::CompilerOptionValue{
                                         .kind = static_cast<slang::CompilerOptionValueKind>(value.type),
                                         .intValue0 = value.intValue0,
                                         .intValue1 = value.intValue1,
                                         .stringValue0 = value.stringValue0,
                                         .stringValue1 = value.stringValue1,
                                     });
    }

    slang::SessionDesc sessionDescription{};
    sessionDescription.targets = &targetDescription;
    sessionDescription.targetCount = 1;
    sessionDescription.searchPaths = creationInfo.searchPaths;
    sessionDescription.searchPathCount = creationInfo.searchPathCount;
    sessionDescription.preprocessorMacros = preprocessorMacroDescriptions.data();
    sessionDescription.preprocessorMacroCount = static_cast<SlangInt>(preprocessorMacroDescriptions.size());
    sessionDescription.compilerOptionEntries = compilerOptions.data();
    sessionDescription.compilerOptionEntryCount = compilerOptions.size();
    sessionDescription.defaultMatrixLayoutMode = creationInfo.useColumnMajorMatrices ? SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
                                                                                     : SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    sessionDescription.enableEffectAnnotations = creationInfo.enableEffectAnnotations;
    sessionDescription.allowGLSLSyntax = creationInfo.allowGlslSyntax;
    sessionDescription.skipSPIRVValidation = creationInfo.skipSpirvValidation;

    globalSlangSession->createSession(sessionDescription, &session_);
    lunaModule_ = session_->loadModuleFromSourceString("Luna", "", LUNA_SLANG_HEADER);
}
inline SlangSession::SlangSession(const slang::SessionDesc &sessionDescription)
{
    if (globalSlangSession == nullptr)
    {
        constexpr SlangGlobalSessionDesc globalSessionDescription{};
        slang::createGlobalSession(&globalSessionDescription, &globalSlangSession);
        assert(globalSlangSession);
    }

    globalSlangSession->createSession(sessionDescription, &session_);
    lunaModule_ = session_->loadModuleFromSourceString("Luna", "", LUNA_SLANG_HEADER);
}

// TODO: Apparently I hate this function lmao
template<std::derived_from<ISlangUnknown> T> T *SlangSession::addComponent(T *component)
{
    components_.emplace_back(component);
    return component;
}
template<std::derived_from<ISlangUnknown> ThisType, std::derived_from<ISlangUnknown> ConstructedType, typename... Args>
SlangResult SlangSession::addComponent(ThisType *thisObject, SlangResult (ThisType::*method)(Args...), Args... args)
{
    if constexpr (std::same_as<ConstructedType **, std::remove_cvref_t<decltype((args, ...))>>)
    {
        const SlangResult result = (thisObject->*method)(args...);
        components_.emplace_back(*std::get<sizeof...(Args) - 1>(std::forward_as_tuple(args...)));
        return result;
    } else
    {
        ConstructedType **constructedObject = std::get<sizeof...(Args) - 2>(std::forward_as_tuple(args...));
        const SlangResult result = (thisObject->*method)(args...);
        components_.emplace_back(*constructedObject);
        return result;
    }
}

inline slang::ISession *SlangSession::session()
{
    return session_;
}
inline const slang::ISession *SlangSession::session() const
{
    return session_;
}
} // namespace luna

#pragma endregion Implementation

#else
namespace luna
{
class SlangSession
{};
} // namespace luna
#endif
