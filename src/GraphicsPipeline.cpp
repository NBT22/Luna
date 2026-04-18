//
// Created by NBT22 on 2/25/25.
//

#include <cassert>
#include <cstdint>
#include <luna/luna.h>
#include <luna/lunaTypes.h>
#include <stdexcept>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"
#include "GraphicsPipeline.hpp"
#include "helpers/Handle.hpp"
#include "helpers/Pipeline.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"
#include "ShaderModule.hpp"

#ifdef LUNA_SLANG_SHADERS
#include <cmath>
#include <cstddef>
#include <cstring>
#include <ranges>
#include <set>
#include <slang.h>
#include <unordered_map>
#include "SlangSession.hpp"
#endif

namespace luna::helpers
{
#ifdef LUNA_SLANG_SHADERS
struct InputBindingAttributes
{
        VkVertexInputBindingDescription bindingDescription{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{1};
};

static inline uint32_t nextBinding(const std::unordered_map<uint32_t, bool> &usedIndices,
                                   uint32_t &currentBindingIndex,
                                   const uint32_t maxVertexInputBindings)
{
    currentBindingIndex++;
    for (; usedIndices.contains(currentBindingIndex) && currentBindingIndex < maxVertexInputBindings;
         currentBindingIndex++)
    {}
    assert(currentBindingIndex < maxVertexInputBindings); // Too many bindings used
    return currentBindingIndex;
}


static inline void addBinding(std::unordered_map<uint32_t, InputBindingAttributes> &inputBindingAttributesMap,
                              const uint32_t bindingIndex,
                              const VkVertexInputRate inputRate)
{
    assert(!inputBindingAttributesMap.contains(bindingIndex)); // Internal state check.
    inputBindingAttributesMap.insert({bindingIndex,
                                      InputBindingAttributes{
                                          .bindingDescription =
                                                  {
                                                      .binding = bindingIndex,
                                                      .inputRate = inputRate,
                                                  },
                                      }});
}

static inline VkFormat getVectorFormat(const slang::TypeReflection::ScalarType type,
                                       const size_t elementCount,
                                       size_t &size)
{
    // ReSharper disable CppDFAUnreachableCode
    // NOLINTBEGIN(*-avoid-magic-numbers)
    switch (type)
    {
        case slang::TypeReflection::ScalarType::Int8:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 1;
                    return VK_FORMAT_R8_SINT;
                case 2:
                    size = 2 * 1;
                    return VK_FORMAT_R8G8_SINT;
                case 3:
                    size = 3 * 1;
                    return VK_FORMAT_R8G8B8_SINT;
                case 4:
                    size = 4 * 1;
                    return VK_FORMAT_R8G8B8A8_SINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Bool:
        case slang::TypeReflection::ScalarType::UInt8:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 1;
                    return VK_FORMAT_R8_UINT;
                case 2:
                    size = 2 * 1;
                    return VK_FORMAT_R8G8_UINT;
                case 3:
                    size = 3 * 1;
                    return VK_FORMAT_R8G8B8_UINT;
                case 4:
                    size = 4 * 1;
                    return VK_FORMAT_R8G8B8A8_UINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Int16:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 2;
                    return VK_FORMAT_R16_SINT;
                case 2:
                    size = 2 * 2;
                    return VK_FORMAT_R16G16_SINT;
                case 3:
                    size = 3 * 2;
                    return VK_FORMAT_R16G16B16_SINT;
                case 4:
                    size = 4 * 2;
                    return VK_FORMAT_R16G16B16A16_SINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::UInt16:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 2;
                    return VK_FORMAT_R16_UINT;
                case 2:
                    size = 2 * 2;
                    return VK_FORMAT_R16G16_UINT;
                case 3:
                    size = 3 * 2;
                    return VK_FORMAT_R16G16B16_UINT;
                case 4:
                    size = 4 * 2;
                    return VK_FORMAT_R16G16B16A16_UINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Int32:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 4;
                    return VK_FORMAT_R32_SINT;
                case 2:
                    size = 2 * 4;
                    return VK_FORMAT_R32G32_SINT;
                case 3:
                    size = 3 * 4;
                    return VK_FORMAT_R32G32B32_SINT;
                case 4:
                    size = 4 * 4;
                    return VK_FORMAT_R32G32B32A32_SINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::UInt32:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 4;
                    return VK_FORMAT_R32_UINT;
                case 2:
                    size = 2 * 4;
                    return VK_FORMAT_R32G32_UINT;
                case 3:
                    size = 3 * 4;
                    return VK_FORMAT_R32G32B32_UINT;
                case 4:
                    size = 4 * 4;
                    return VK_FORMAT_R32G32B32A32_UINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Int64:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 8;
                    return VK_FORMAT_R64_SINT;
                case 2:
                    size = 2 * 8;
                    return VK_FORMAT_R64G64_SINT;
                case 3:
                    size = 3 * 8;
                    return VK_FORMAT_R64G64B64_SINT;
                case 4:
                    size = 4 * 8;
                    return VK_FORMAT_R64G64B64A64_SINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::UInt64:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 8;
                    return VK_FORMAT_R64_UINT;
                case 2:
                    size = 2 * 8;
                    return VK_FORMAT_R64G64_UINT;
                case 3:
                    size = 3 * 8;
                    return VK_FORMAT_R64G64B64_UINT;
                case 4:
                    size = 4 * 8;
                    return VK_FORMAT_R64G64B64A64_UINT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Float16:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 2;
                    return VK_FORMAT_R16_SFLOAT;
                case 2:
                    size = 2 * 2;
                    return VK_FORMAT_R16G16_SFLOAT;
                case 3:
                    size = 3 * 2;
                    return VK_FORMAT_R16G16B16_SFLOAT;
                case 4:
                    size = 4 * 2;
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Float32:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 4;
                    return VK_FORMAT_R32_SFLOAT;
                case 2:
                    size = 2 * 4;
                    return VK_FORMAT_R32G32_SFLOAT;
                case 3:
                    size = 3 * 4;
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case 4:
                    size = 4 * 4;
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        case slang::TypeReflection::ScalarType::Float64:
            switch (elementCount)
            {
                case 1:
                    size = 1 * 8;
                    return VK_FORMAT_R64_SFLOAT;
                case 2:
                    size = 2 * 8;
                    return VK_FORMAT_R64G64_SFLOAT;
                case 3:
                    size = 3 * 8;
                    return VK_FORMAT_R64G64B64_SFLOAT;
                case 4:
                    size = 4 * 8;
                    return VK_FORMAT_R64G64B64A64_SFLOAT;
                default:
                    assert(false); // Invalid vector length
            }
            return VK_FORMAT_UNDEFINED;
        default:
            assert(false); // Invalid scalar type
            return VK_FORMAT_UNDEFINED;
    }
    // NOLINTEND(*-avoid-magic-numbers)
    // ReSharper restore CppDFAUnreachableCode
}

// TODO (0.3.0): Respect struct layout (aka include padding in the stride)
//  This isn't straightforward because I allow binding multiple structs to the same index, and if I get part-way
//  through determining the offsets then at THAT point I realize that the alignment was wrong I would have to somehow
//  restart the loop.
// I hate this function can I remove it? pretty please?
static inline void addParameterToBinding(InputBindingAttributes &inputBindingAttributes,
                                         slang::TypeLayoutReflection *typeLayoutReflection,
                                         size_t count = 1)
{
    size_t size = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    // ReSharper disable CppDFAUnreachableCode
    // NOLINTBEGIN(*-avoid-magic-numbers)
    switch (typeLayoutReflection->getKind())
    {
        case slang::TypeReflection::Kind::Array:
            addParameterToBinding(inputBindingAttributes,
                                  typeLayoutReflection->getElementTypeLayout(),
                                  typeLayoutReflection->getElementCount());
            return;
        case slang::TypeReflection::Kind::Struct:
            for (uint32_t i = 0; i < typeLayoutReflection->getFieldCount(); i++)
            {
                addParameterToBinding(inputBindingAttributes,
                                      typeLayoutReflection->getFieldByIndex(i)->getTypeLayout());
            }
            return;
        case slang::TypeReflection::Kind::Matrix:
            assert(typeLayoutReflection->getElementTypeLayout()->getKind() == slang::TypeReflection::Kind::Vector &&
                   typeLayoutReflection->getElementTypeLayout()->getElementTypeLayout()->getKind() ==
                           slang::TypeReflection::Kind::Scalar); // Only scalar matrices are allowed
            switch (typeLayoutReflection->getMatrixLayoutMode())
            {
                case SLANG_MATRIX_LAYOUT_ROW_MAJOR:
                    assert(typeLayoutReflection->getElementTypeLayout()->getElementCount() ==
                           typeLayoutReflection->getRowCount()); // Internal state check.
                    format = getVectorFormat(typeLayoutReflection->getElementTypeLayout()->getScalarType(),
                                             typeLayoutReflection->getRowCount(),
                                             size);
                    count = typeLayoutReflection->getColumnCount();
                    break;
                case SLANG_MATRIX_LAYOUT_COLUMN_MAJOR:
                    assert(typeLayoutReflection->getElementTypeLayout()->getElementCount() ==
                           typeLayoutReflection->getColumnCount()); // Internal state check.
                    format = getVectorFormat(typeLayoutReflection->getElementTypeLayout()->getScalarType(),
                                             typeLayoutReflection->getColumnCount(),
                                             size);
                    count = typeLayoutReflection->getRowCount();
                    break;
                default:
                    assert(false); // Invalid matrix layout mode
                    break;
            }
            break;
        case slang::TypeReflection::Kind::Vector:
            assert(typeLayoutReflection->getElementTypeLayout()->getKind() ==
                   slang::TypeReflection::Kind::Scalar); // Only scalar vectors are allowed
            format = getVectorFormat(typeLayoutReflection->getElementTypeLayout()->getScalarType(),
                                     typeLayoutReflection->getElementCount(),
                                     size);
            break;
        case slang::TypeReflection::Kind::Scalar:
            switch (typeLayoutReflection->getScalarType())
            {
                case slang::TypeReflection::ScalarType::Int8:
                    size = 1;
                    format = VK_FORMAT_R8_SINT;
                    break;
                case slang::TypeReflection::ScalarType::Bool:
                case slang::TypeReflection::ScalarType::UInt8:
                    size = 1;
                    format = VK_FORMAT_R8_UINT;
                    break;
                case slang::TypeReflection::ScalarType::Int16:
                    size = 2;
                    format = VK_FORMAT_R16_SINT;
                    break;
                case slang::TypeReflection::ScalarType::UInt16:
                    size = 2;
                    format = VK_FORMAT_R16_UINT;
                    break;
                case slang::TypeReflection::ScalarType::Int32:
                    size = 4;
                    format = VK_FORMAT_R32_SINT;
                    break;
                case slang::TypeReflection::ScalarType::UInt32:
                    size = 4;
                    format = VK_FORMAT_R32_UINT;
                    break;
                case slang::TypeReflection::ScalarType::Int64:
                    size = 8;
                    format = VK_FORMAT_R64_SINT;
                    break;
                case slang::TypeReflection::ScalarType::UInt64:
                    size = 8;
                    format = VK_FORMAT_R64_UINT;
                    break;
                case slang::TypeReflection::ScalarType::Float16:
                    size = 2;
                    format = VK_FORMAT_R16_SFLOAT;
                    break;
                case slang::TypeReflection::ScalarType::Float32:
                    size = 4;
                    format = VK_FORMAT_R32_SFLOAT;
                    break;
                case slang::TypeReflection::ScalarType::Float64:
                    size = 8;
                    format = VK_FORMAT_R64_SFLOAT;
                    break;
                default:
                    assert(false); // Invalid scalar type
                    break;
            }
            break;
        default:
            assert(false); // Unhandled attribute type
            break;
    }
    // NOLINTEND(*-avoid-magic-numbers)
    // ReSharper restore CppDFAUnreachableCode

    assert(format != VK_FORMAT_UNDEFINED); // Internal state check.

    for (size_t i = 0; i < count; i++)
    {
        VkVertexInputAttributeDescription &attributeDescription = inputBindingAttributes.attributeDescriptions.back();
        attributeDescription.format = format;
        constexpr float BYTES_PER_LOCATION = 16.0f;
        inputBindingAttributes.attributeDescriptions.emplace_back(
                attributeDescription.location +
                        static_cast<size_t>(std::ceil(static_cast<float>(size) / BYTES_PER_LOCATION)),
                attributeDescription.binding,
                VK_FORMAT_UNDEFINED,
                attributeDescription.offset + size);
        inputBindingAttributes.bindingDescription.stride += size;
    }
}

static inline void addParameterToBinding(std::unordered_map<uint32_t, InputBindingAttributes>
                                                 &inputBindingAttributesMap,
                                         const uint32_t bindingIndex,
                                         slang::VariableLayoutReflection *variableLayoutReflection)
{
    assert(inputBindingAttributesMap.contains(bindingIndex)); // Internal state check.

    addParameterToBinding(inputBindingAttributesMap.at(bindingIndex), variableLayoutReflection->getTypeLayout());
}

static inline bool parameterIsInput(slang::VariableLayoutReflection *variableLayoutReflection)
{
    for (uint32_t i = 0; i < variableLayoutReflection->getCategoryCount(); i++)
    {
        if (variableLayoutReflection->getCategoryByIndex(i) == slang::ParameterCategory::VaryingInput)
        {
            return true;
        }
    }
    return false;
}
#endif
} // namespace luna::helpers

namespace luna
{
VkResult GraphicsPipeline::bind(const LunaDevice device,
                                const LunaCommandBuffer commandBuffer,
                                const LunaGraphicsPipeline pipeline,
                                const LunaGraphicsPipelineBindInfo *bindInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);
    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::GraphicsPipeline>(pipeline)->bind(
            static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
            *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
            bindInfo == nullptr ? LunaGraphicsPipelineBindInfo{} : *bindInfo));
    assert(luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer)->isRecording()); // Internal state check
    return VK_SUCCESS;
}

// TODO (0.3.0): Base pipeline
GraphicsPipeline::GraphicsPipeline(const VkDevice device, const LunaGraphicsPipelineCreationInfo &creationInfo)
{
    assert(isDestroyed_);
    assert(!(creationInfo.shaderStageCount > 0 && // NOLINT(*-simplify-boolean-expr) In order to preserve clarity
             creationInfo.shaderStages == nullptr));

    CHECK_RESULT_THROW(helpers::createPipelineLayout(device,
                                                     creationInfo.layoutCreationInfo,
                                                     pushConstantsRanges_,
                                                     &layout_));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(creationInfo.shaderStageCount);
    for (uint32_t i = 0; i < creationInfo.shaderStageCount; i++)
    {
        // I literally have an assert to ensure it isn't
        // ReSharper disable once CppDFANullDereference
        const LunaPipelineShaderStageCreationInfo &shaderStage = creationInfo.shaderStages[i];
        shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                  nullptr,
                                  shaderStage.flags,
                                  shaderStage.stage,
                                  helpers::fromHandle<ShaderModule>(shaderStage.module)->module(),
                                  shaderStage.entryPoint == nullptr ? "main" : shaderStage.entryPoint,
                                  shaderStage.specializationInfo);
    }

    const RenderPassSubpassIndex *subpassIndex = helpers::fromHandle<RenderPassSubpassIndex>(creationInfo.subpass);
    const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = creationInfo.flags,
        .stageCount = creationInfo.shaderStageCount,
        .pStages = shaderStages.data(),
        .pVertexInputState = creationInfo.vertexInputState,
        .pInputAssemblyState = creationInfo.inputAssemblyState,
        .pTessellationState = creationInfo.tessellationState,
        .pViewportState = creationInfo.viewportState,
        .pRasterizationState = creationInfo.rasterizationState,
        .pMultisampleState = creationInfo.multisampleState,
        .pDepthStencilState = creationInfo.depthStencilState,
        .pColorBlendState = creationInfo.colorBlendState,
        .pDynamicState = creationInfo.dynamicState,
        .layout = layout_,
        .renderPass = *subpassIndex->renderPass,
        .subpass = subpassIndex->index,
    };
    CHECK_RESULT_THROW(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline_));

    isDestroyed_ = false;
}
GraphicsPipeline::GraphicsPipeline(const VkDevice device,
                                   const LunaGraphicsPipelineUsingReflectionCreationInfo &creationInfo)
{
#ifdef LUNA_SLANG_SHADERS
    // TODO (0.3.0): Support for using a binding name to refer to a descriptor set in the shader

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
    const uint32_t maxVertexInputBindings = physicalDeviceProperties.limits.maxVertexInputBindings;

    std::vector<ShaderModule *> pipelineShaderModules;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (uint32_t i = 0; i < creationInfo.shaderModuleCreationInfoCount; i++)
    {
        LunaShaderModule module = LUNA_NULL_HANDLE;
        CHECK_RESULT_THROW(device.createShaderModule(creationInfo.shaderModuleCreationInfos[i], &module));
        pipelineShaderModules.emplace_back(helpers::fromHandle<ShaderModule>(module));
    }
    for (uint32_t i = 0; i < creationInfo.shaderModuleCount; i++)
    {
        if (i < creationInfo.shaderModuleCreationInfoCount)
        {
            creationInfo.shaderModules[i] = helpers::toHandle(pipelineShaderModules.at(i));
        } else
        {
            pipelineShaderModules.emplace_back(helpers::fromHandle<ShaderModule>(creationInfo.shaderModules[i]));
        }
    }
    for (const ShaderModule *shaderModule: pipelineShaderModules)
    {
        constexpr const char *bindingIndexAttributeName = "Luna_vertexBindingIndex";
        constexpr const char *bindingNameAttributeName = "Luna_vertexBindingName";
        constexpr const char *inputRateInstanceAttributeName = "Luna_VERTEX_INPUT_RATE_INSTANCE";
        slang::ProgramLayout *programLayout = shaderModule->slangProgram()->getLayout();
        slang::EntryPointReflection *entryPointReflection =
                programLayout->findEntryPointByName(shaderModule->entryPoint().c_str());

        switch (entryPointReflection->getStage())
        {
            case SLANG_STAGE_VERTEX:
                shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                          nullptr,
                                          0,
                                          VK_SHADER_STAGE_VERTEX_BIT,
                                          shaderModule->module(),
                                          shaderModule->entryPoint().c_str(),
                                          nullptr);
                break;
            case SLANG_STAGE_FRAGMENT:
                shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                          nullptr,
                                          0,
                                          VK_SHADER_STAGE_FRAGMENT_BIT,
                                          shaderModule->module(),
                                          shaderModule->entryPoint().c_str(),
                                          nullptr);
                continue;
            default:
                assert(false); // Unhandled shader stage
                continue;
        }

        const uint32_t parameterCount = entryPointReflection->getParameterCount();
        std::unordered_map<uint32_t, bool> usedIndices;
        bool inputRateVertexBindingNeeded = false;
        bool inputRateInstanceBindingNeeded = false;
        std::set<std::string> indexlessNamedBindings;
        for (uint32_t i = 0; i < parameterCount; i++)
        {
            slang::VariableReflection *variableReflection =
                    entryPointReflection->getParameterByIndex(i)
                            ->getVariable(); // TODO (0.3.0): This can segfault. There are probably other cases of the same as well.
            slang::Attribute *bindingIndexAttribute =
                    variableReflection->findUserAttributeByName(globalSlangSession, bindingIndexAttributeName);
            slang::Attribute *bindingNameAttribute =
                    variableReflection->findUserAttributeByName(globalSlangSession, bindingNameAttributeName);
            const bool inputRateInstance =
                    variableReflection->findUserAttributeByName(globalSlangSession, inputRateInstanceAttributeName) !=
                    nullptr;
            if (bindingIndexAttribute != nullptr)
            {
                int intBindingIndex = 0;
                bindingIndexAttribute->getArgumentValueInt(0, &intBindingIndex);
                const uint32_t bindingIndex = static_cast<uint32_t>(intBindingIndex);
                assert(bindingIndex < maxVertexInputBindings); // Too high of a binding used
                if (usedIndices.contains(bindingIndex))
                {
                    assert(usedIndices.at(bindingIndex) ==
                           inputRateInstance); // All uses of a binding index must have the same input rate
                } else
                {
                    usedIndices.insert({bindingIndex, inputRateInstance});
                }

                if (bindingNameAttribute != nullptr)
                {
                    const std::string bindingName = bindingNameAttribute->getArgumentValueString(0, nullptr);
                    if (namedBindingsMap_.contains(bindingName))
                    {
                        assert(namedBindingsMap_.at(bindingName) ==
                               bindingIndex); // All usages of a binding name must be bound to the same index
                        assert(!indexlessNamedBindings.contains(bindingName)); // Internal state check.
                    } else
                    {
                        if (indexlessNamedBindings.contains(bindingName))
                        {
                            indexlessNamedBindings.erase(bindingName);
                        }
                        namedBindingsMap_.insert({bindingName, bindingIndex});
                    }
                }
            } else if (bindingNameAttribute != nullptr)
            {
                const std::string bindingName = bindingNameAttribute->getArgumentValueString(0, nullptr);
                if (!namedBindingsMap_.contains(bindingName) && !indexlessNamedBindings.contains(bindingName))
                {
                    indexlessNamedBindings.insert(bindingName);
                }
            } else
            {
                constexpr const char *inputRateVertexAttributeName = "Luna_VERTEX_INPUT_RATE_VERTEX";
                assert(bindingIndexAttribute == nullptr && bindingNameAttribute == nullptr); // Internal state check.
                assert(!inputRateInstance ||
                       (variableReflection->findUserAttributeByName(globalSlangSession, inputRateVertexAttributeName) ==
                        nullptr)); // Must not have both input rates on the same variable
                inputRateInstanceBindingNeeded |= inputRateInstance;
                inputRateVertexBindingNeeded |= !inputRateInstance;
            }
        }
        const size_t bindingCount = usedIndices.size() +
                                    indexlessNamedBindings.size() +
                                    namedBindingsMap_.size() +
                                    static_cast<uint8_t>(inputRateVertexBindingNeeded) +
                                    static_cast<uint8_t>(inputRateInstanceBindingNeeded);
        assert(bindingCount < maxVertexInputBindings); // Too many bindings used

        uint32_t currentBindingIndex = -1u; // Intended to overflow on first call of helpers::nextBinding
        std::unordered_map<uint32_t, helpers::InputBindingAttributes> inputBindingAttributesMap;
        inputBindingAttributesMap.reserve(bindingCount);
        for (uint32_t i = 0; i < parameterCount; i++)
        {
            slang::VariableLayoutReflection *variableLayoutReflection = entryPointReflection->getParameterByIndex(i);
            if (!helpers::parameterIsInput(variableLayoutReflection))
            {
                continue;
            }
            slang::VariableReflection *variableReflection = variableLayoutReflection->getVariable();
            slang::Attribute *bindingIndexAttribute =
                    variableReflection->findUserAttributeByName(globalSlangSession, bindingIndexAttributeName);
            slang::Attribute *bindingNameAttribute =
                    variableReflection->findUserAttributeByName(globalSlangSession, bindingNameAttributeName);
            const VkVertexInputRate inputRate =
                    variableReflection->findUserAttributeByName(globalSlangSession, inputRateInstanceAttributeName) !=
                                    nullptr
                            ? VK_VERTEX_INPUT_RATE_INSTANCE
                            : VK_VERTEX_INPUT_RATE_VERTEX;

            // ReSharper disable once CppDFAUnusedValue
            uint32_t bindingIndex = -1u;
            if (bindingIndexAttribute != nullptr)
            {
                int intBindingIndex = 0;
                bindingIndexAttribute->getArgumentValueInt(0, &intBindingIndex);
                bindingIndex = static_cast<uint32_t>(intBindingIndex);
            } else if (bindingNameAttribute != nullptr)
            {
                const std::string bindingName = bindingNameAttribute->getArgumentValueString(0, nullptr);
                const bool mapHasBinding = namedBindingsMap_.contains(bindingName);
                if (!mapHasBinding)
                {
                    bindingIndex = helpers::nextBinding(usedIndices, currentBindingIndex, maxVertexInputBindings);
                    namedBindingsMap_.insert({bindingName, bindingIndex});
                } else
                {
                    bindingIndex = namedBindingsMap_.at(bindingName);
                }
            } else
            {
                uint32_t &index = inputRate == VK_VERTEX_INPUT_RATE_INSTANCE ? inputRateInstanceBindingIndex_
                                                                             : inputRateVertexBindingIndex_;
                if (index == -1u)
                {
                    index = helpers::nextBinding(usedIndices, currentBindingIndex, maxVertexInputBindings);
                }
                bindingIndex = index;
            }
            assert(bindingIndex != -1u); // Internal state check.
            if (!inputBindingAttributesMap.contains(bindingIndex))
            {
                helpers::addBinding(inputBindingAttributesMap, bindingIndex, inputRate);
            }
            helpers::addParameterToBinding(inputBindingAttributesMap, bindingIndex, variableLayoutReflection);
        }

        for (helpers::InputBindingAttributes &inputBindingAttributes: inputBindingAttributesMap | std::views::values)
        {
            bindingDescriptions.push_back(inputBindingAttributes.bindingDescription);
            attributeDescriptions.insert(attributeDescriptions.end(),
                                         inputBindingAttributes.attributeDescriptions.begin(),
                                         inputBindingAttributes.attributeDescriptions.end() - 1);
        }

        vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
            .pVertexBindingDescriptions = bindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };
    }


    constexpr VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    const VkViewport viewport = {
        .width = static_cast<float>(swapchain.extent.width),
        .height = static_cast<float>(swapchain.extent.height),
        .maxDepth = 1,
    };
    const VkRect2D scissor = {
        .extent = {.width = swapchain.extent.width, .height = swapchain.extent.height},
    };
    const VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    constexpr VkPipelineRasterizationStateCreateInfo rasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1,
    };

    constexpr VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_4_BIT,
    };

    constexpr VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .maxDepthBounds = 1,
    };

    constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
    };
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    CHECK_RESULT_THROW(helpers::createPipelineLayout(creationInfo.layoutCreationInfo, pushConstantsRanges_, &layout_));
    const RenderPassSubpassIndex *subpassIndex = helpers::fromHandle<RenderPassSubpassIndex>(creationInfo.subpass);
    const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = creationInfo.flags,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = creationInfo.inputAssemblyState != nullptr ? creationInfo.inputAssemblyState
                                                                          : &inputAssemblyState,
        .pTessellationState = creationInfo.tessellationState,
        .pViewportState = creationInfo.viewportState != nullptr ? creationInfo.viewportState : &viewportState,
        .pRasterizationState = creationInfo.rasterizationState != nullptr ? creationInfo.rasterizationState
                                                                          : &rasterizationState,
        .pMultisampleState = creationInfo.multisampleState != nullptr ? creationInfo.multisampleState
                                                                      : &multisampleState,
        .pDepthStencilState = creationInfo.depthStencilState != nullptr ? creationInfo.depthStencilState
                                                                        : &depthStencilState,
        .pColorBlendState = creationInfo.colorBlendState != nullptr ? creationInfo.colorBlendState : &colorBlendState,
        .pDynamicState = creationInfo.dynamicState,
        .layout = layout_,
        .renderPass = *subpassIndex->renderPass,
        .subpass = subpassIndex->index,
    };
    CHECK_RESULT_THROW(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline_));
#else
    (void)device;
    (void)creationInfo;
    throw std::runtime_error("Unable to create shader using reflection without source shader!");
#endif
}
void GraphicsPipeline::destroy(const VkDevice device)
{
    if (isDestroyed_)
    {
        return;
    }
    vkDestroyPipeline(device, pipeline_, nullptr);
    vkDestroyPipelineLayout(device, layout_, nullptr);

    pushConstantsRanges_.clear();
    pushConstantsRanges_.shrink_to_fit();
    isDestroyed_ = true;
}
VkResult GraphicsPipeline::pushConstants(const VkDevice device, CommandBuffer &commandBuffer) const
{
    const std::vector<LunaPushConstantsRange> &pushConstantsRanges = pushConstantsRanges_;
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(device));
    uint32_t offset = 0;
    for (const LunaPushConstantsRange &pushConstantsRange: pushConstantsRanges)
    {
        const void *pushConstantsData = static_cast<const uint8_t *>(pushConstantsRange.dataPointer) +
                                        pushConstantsRange.dataPointerOffset;
        vkCmdPushConstants(commandBuffer,
                           layout_,
                           pushConstantsRange.stageFlags,
                           offset,
                           pushConstantsRange.size,
                           pushConstantsData);
        offset += pushConstantsRange.size;
    }
    return VK_SUCCESS;
}
VkResult GraphicsPipeline::bind(const VkDevice device,
                                CommandBuffer &commandBuffer,
                                const LunaGraphicsPipelineBindInfo &bindInfo) const
{
    CHECK_RESULT_RETURN(commandBuffer.ensureIsRecording(device));
    for (uint32_t i = 0; i < bindInfo.dynamicStateCount; i++)
    {
        const LunaDynamicStateBindInfo &dynamicState = bindInfo.dynamicStates[i];
        switch (dynamicState.dynamicStateType)
        {
            case VK_DYNAMIC_STATE_VIEWPORT:
                vkCmdSetViewport(commandBuffer,
                                 dynamicState.bindInfo.viewportBindInfo->firstViewport,
                                 dynamicState.bindInfo.viewportBindInfo->viewportCount,
                                 dynamicState.bindInfo.viewportBindInfo->viewports);
                break;
            case VK_DYNAMIC_STATE_SCISSOR:
                vkCmdSetScissor(commandBuffer,
                                dynamicState.bindInfo.scissorBindInfo->firstScissor,
                                dynamicState.bindInfo.scissorBindInfo->scissorCount,
                                dynamicState.bindInfo.scissorBindInfo->scissors);
                break;
            default:
                throw std::runtime_error("Unhandled dynamic state type!");
        }
    }
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
    if (bindInfo.descriptorSetBindInfo.descriptorSetCount > 0)
    {
        std::vector<VkDescriptorSet> descriptorSetsVector;
        descriptorSetsVector.reserve(bindInfo.descriptorSetBindInfo.descriptorSetCount);
        for (uint32_t i = 0; i < bindInfo.descriptorSetBindInfo.descriptorSetCount; i++)
        {
            descriptorSetsVector.emplace_back(
                    *helpers::fromHandle<DescriptorSetIndex>(bindInfo.descriptorSetBindInfo.descriptorSets[i])->set);
        }
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout_,
                                bindInfo.descriptorSetBindInfo.firstSet,
                                bindInfo.descriptorSetBindInfo.descriptorSetCount,
                                descriptorSetsVector.data(),
                                bindInfo.descriptorSetBindInfo.dynamicOffsetCount,
                                bindInfo.descriptorSetBindInfo.dynamicOffsets);
    }
    return VK_SUCCESS;
}
} // namespace luna

VkResult lunaCreateGraphicsPipeline(const LunaDevice device,
                                    const LunaGraphicsPipelineCreationInfo *creationInfo,
                                    LunaGraphicsPipeline *pipeline)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo);

    CHECK_RESULT_RETURN(luna::helpers::fromHandle<luna::Device>(device)->createGraphicsPipeline(*creationInfo,
                                                                                                pipeline));
    return VK_SUCCESS;
}
VkResult lunaCreateGraphicsPipelineUsingReflection(const LunaGraphicsPipelineUsingReflectionCreationInfo *creationInfo,
                                                   LunaGraphicsPipeline *pipeline)
{
    assert(creationInfo);
    (void)pipeline;
    // TRY_CATCH_RESULT(luna::device.graphicsPipelines.emplace_back(*creationInfo));
    // if (pipeline != nullptr)
    // {
    //     *pipeline = luna::helpers::toHandle(&luna::device.graphicsPipelines.back());
    // }
    return VK_ERROR_UNKNOWN;
}

void lunaBindDescriptorSets(const LunaCommandBuffer commandBuffer,
                            const LunaGraphicsPipeline pipeline,
                            const LunaDescriptorSetBindInfo *bindInfo)
{
    assert(commandBuffer != LUNA_NULL_HANDLE);

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(bindInfo->descriptorSetCount);
    for (uint32_t i = 0; i < bindInfo->descriptorSetCount; i++)
    {
        descriptorSets.emplace_back(
                *luna::helpers::fromHandle<luna::DescriptorSetIndex>(bindInfo->descriptorSets[i])->set);
    }
    vkCmdBindDescriptorSets(*luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            luna::helpers::fromHandle<luna::GraphicsPipeline>(pipeline)->layout(),
                            bindInfo->firstSet,
                            bindInfo->descriptorSetCount,
                            descriptorSets.data(),
                            bindInfo->dynamicOffsetCount,
                            bindInfo->dynamicOffsets);
}

VkResult lunaPushConstants(const LunaDevice device,
                           const LunaCommandBuffer commandBuffer,
                           const LunaGraphicsPipeline pipeline)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(commandBuffer != LUNA_NULL_HANDLE);

    return luna::helpers::fromHandle<luna::GraphicsPipeline>(pipeline)->pushConstants(
            static_cast<VkDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
            *luna::helpers::fromHandle<luna::CommandBuffer>(commandBuffer));
}
