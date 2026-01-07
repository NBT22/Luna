//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNATYPES_H
#define LUNATYPES_H

#ifdef __cplusplus
extern "C"
{
// ReSharper disable CppVariableCanBeMadeConstexpr
// NOLINTBEGIN(*-macro-usage, *-enum-size, *-use-using, *-use-enum-class)
#else
#include <stdbool.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#define LUNA_DEFINE_HANDLE(handle) typedef uint64_t handle

#define LUNA_NULL_HANDLE 0LL

LUNA_DEFINE_HANDLE(LunaRenderPass);
LUNA_DEFINE_HANDLE(LunaRenderPassSubpass);
LUNA_DEFINE_HANDLE(LunaDescriptorPool);
LUNA_DEFINE_HANDLE(LunaDescriptorSetLayout);
LUNA_DEFINE_HANDLE(LunaDescriptorSet);
LUNA_DEFINE_HANDLE(LunaShaderModule);
LUNA_DEFINE_HANDLE(LunaGraphicsPipeline);
LUNA_DEFINE_HANDLE(LunaComputePipeline);
LUNA_DEFINE_HANDLE(LunaBuffer);
LUNA_DEFINE_HANDLE(LunaSampler);
LUNA_DEFINE_HANDLE(LunaImage);
LUNA_DEFINE_HANDLE(LunaCommandPool);
LUNA_DEFINE_HANDLE(LunaSlangSession);

static const uint32_t LUNA_RENDER_PASS_WIDTH_SWAPCHAIN_WIDTH = -1u;
static const uint32_t LUNA_RENDER_PASS_HEIGHT_SWAPCHAIN_HEIGHT = -1u;

typedef enum
{
    LUNA_INTERNAL_GRAPHICS_COMMAND_POOL,
} LunaInternalCommandPools;

typedef struct
{
        VkInstanceCreateFlags flags;
        uint32_t apiVersion;

        uint32_t extensionCount;
        const char *const *extensionNames;
        bool enableValidation;

        uint32_t layerCount;
        const char *const *layerNames;
} LunaInstanceCreationInfo;

typedef struct
{
        // TODO (0.3.0): Allow a priority list for preferredDeviceType and potentially add more options
        VkPhysicalDeviceType preferredDeviceType;
} LunaPhysicalDevicePreferenceDefinition;

// TODO (0.3.0): Remove duplicate structures and prefer the `[...]2` structures
typedef struct
{
        uint32_t extensionCount;
        const char *const *extensionNames;
        VkPhysicalDeviceFeatures requiredFeatures;
        VkSurfaceKHR surface;

        const LunaPhysicalDevicePreferenceDefinition *physicalDevicePreferenceDefinition;
} LunaDeviceCreationInfo;

typedef struct
{
        uint32_t extensionCount;
        const char *const *extensionNames;
        VkPhysicalDeviceFeatures2 requiredFeatures;
        VkSurfaceKHR surface;

        const LunaPhysicalDevicePreferenceDefinition *physicalDevicePreferenceDefinition;
} LunaDeviceCreationInfo2;

typedef struct
{
        VkSurfaceKHR surface;
        uint32_t width;
        uint32_t height;
        uint32_t formatCount;
        const VkSurfaceFormatKHR *formatPriorityList;
        VkImageUsageFlags imageUsage;
        // TODO (0.3.0): Clang-Tidy: Enum value of type 'VkCompositeAlphaFlagBitsKHR' initialized with invalid value of 0, enum doesn't have a zero-value enumerator
        VkCompositeAlphaFlagBitsKHR compositeAlpha;
        uint32_t presentModeCount;
        const VkPresentModeKHR *presentModePriorityList;
        bool clipped;
} LunaSwapchainCreationInfo;

typedef enum
{
    LUNA_ATTACHMENT_LOAD_MODE_UNDEFINED,
    LUNA_ATTACHMENT_LOAD_MODE_CLEAR,
    LUNA_ATTACHMENT_LOAD_MODE_PRESERVE,
} LunaAttachmentLoadMode;

typedef struct
{
        const char *name;
        VkSubpassDescriptionFlags flags;
        VkPipelineBindPoint pipelineBindPoint;
        uint32_t inputAttachmentCount;
        const VkAttachmentReference *inputAttachments;
        bool useColorAttachment;
        bool useDepthAttachment;
        uint32_t preserveAttachmentCount;
        const uint32_t *preserveAttachments;
} LunaSubpassCreationInfo;

typedef struct
{
        VkSampleCountFlagBits samples;
        bool createColorAttachment;
        LunaAttachmentLoadMode colorAttachmentLoadMode;
        bool createDepthAttachment;
        LunaAttachmentLoadMode depthAttachmentLoadMode;

        // uint32_t attachmentCount;
        // const VkAttachmentDescription *attachments;

        uint32_t subpassCount;
        const LunaSubpassCreationInfo *subpasses;

        uint32_t dependencyCount;
        const VkSubpassDependency *dependencies;

        VkExtent3D extent;
        VkExtent3D maxExtent;
        uint32_t framebufferAttachmentCount;
        const VkImageView *framebufferAttachments;
} LunaRenderPassCreationInfo;

typedef struct
{
        const char *name;
        VkSubpassDescriptionFlags flags;
        VkPipelineBindPoint pipelineBindPoint;
        uint32_t viewMask;
        uint32_t inputAttachmentCount;
        const VkAttachmentReference2 *inputAttachments;
        bool useColorAttachment;
        bool useDepthAttachment;
        uint32_t preserveAttachmentCount;
        const uint32_t *preserveAttachments;
} LunaSubpassCreationInfo2;

typedef struct
{
        VkSampleCountFlagBits samples;
        bool createColorAttachment;
        LunaAttachmentLoadMode colorAttachmentLoadMode;
        bool createDepthAttachment;
        LunaAttachmentLoadMode depthAttachmentLoadMode;

        // uint32_t attachmentCount;
        // const VkAttachmentDescription2 *attachments;

        uint32_t subpassCount;
        const LunaSubpassCreationInfo2 *subpasses;

        uint32_t dependencyCount;
        const VkSubpassDependency2 *dependencies;

        uint32_t correlatedViewMaskCount;
        const uint32_t *correlatedViewMasks;

        VkExtent3D extent;
        VkExtent3D maxExtent;
        uint32_t framebufferAttachmentCount;
        const VkImageView *framebufferAttachments;
} LunaRenderPassCreationInfo2;

typedef struct
{
        VkDescriptorPoolCreateFlags flags;
        uint32_t maxSets;
        uint32_t poolSizeCount;
        const VkDescriptorPoolSize *poolSizes;
} LunaDescriptorPoolCreationInfo;

typedef struct
{
        const char *bindingName;
        VkDescriptorType descriptorType;
        uint32_t descriptorCount;
        VkShaderStageFlags stageFlags;
        const VkSampler *immutableSamplers;
        VkDescriptorBindingFlags bindingFlags;
} LunaDescriptorSetLayoutBinding;

typedef struct
{
        VkDescriptorSetLayoutCreateFlags flags;
        uint32_t bindingCount;
        const LunaDescriptorSetLayoutBinding *bindings;
} LunaDescriptorSetLayoutCreationInfo;

typedef struct
{
        LunaDescriptorPool descriptorPool;
        uint32_t descriptorSetCount;
        const LunaDescriptorSetLayout *setLayouts;
} LunaDescriptorSetAllocationInfo;

typedef struct
{
        LunaSampler sampler;
        LunaImage image;
        VkImageLayout imageLayout;
} LunaDescriptorImageInfo;

typedef struct
{
        LunaBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize range;
} LunaDescriptorBufferInfo;

typedef struct
{
        LunaDescriptorSet descriptorSet;
        const char *bindingName;
        uint32_t descriptorArrayElement;
        uint32_t descriptorCount;
        const LunaDescriptorImageInfo *imageInfo;
        const LunaDescriptorBufferInfo *bufferInfo;
        const VkBufferView *texelBufferView; // TODO: Currently ignored
} LunaWriteDescriptorSet;

typedef struct
{
        size_t size;
        const uint32_t *spirv;
} LunaSpirvShaderModuleCreationInfo;

typedef struct
{
        const char *name;
        const char *value;
} LunaSlangPreprocessorMacroDescription;

typedef enum
{
    LUNA_SLANG_COMPILER_OPTION_VALUE_TYPE_INT,
    LUNA_SLANG_COMPILER_OPTION_VALUE_TYPE_STRING,
} LunaSlangCompilerOptionValueType;

typedef struct
{
        LunaSlangCompilerOptionValueType type;
        int32_t intValue0;
        int32_t intValue1;
        const char *stringValue0;
        const char *stringValue1;
} LunaSlangCompilerOptionValue;

typedef struct
{
        int name;
        LunaSlangCompilerOptionValue value;
} LunaSlangCompilerOption;

typedef struct
{
        const char *spirvProfile;
        LunaSlangCompilerOption *targetCompilerOptions;
        uint32_t targetCompilerOptionCount;
        const char *const *searchPaths;
        uint32_t searchPathCount;
        LunaSlangPreprocessorMacroDescription *preprocessorMacros;
        uint32_t preprocessorMacroCount;
        LunaSlangCompilerOption *compilerOptions;
        uint32_t compilerOptionCount;
        bool useColumnMajorMatrices;
        bool enableEffectAnnotations;
        bool allowGlslSyntax;
        bool skipSpirvValidation;
} LunaSlangSessionCreationInfo;

typedef struct
{
        const char *moduleName;
        const char *modulePath;
        const char *sourceString;
        const char *entryPoint;
        LunaSlangSessionCreationInfo *sessionCreationInfo;
        LunaSlangSession *session;
} LunaSlangShaderModuleCreationInfo;

typedef enum
{
    LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SPIRV,
    LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SLANG,
} LunaShaderModuleCreationInfoType;

typedef union
{
        LunaSpirvShaderModuleCreationInfo spirv;
        LunaSlangShaderModuleCreationInfo slang;
} LunaShaderModuleCreationInfoTypeUnion;

typedef struct
{
        LunaShaderModuleCreationInfoType creationInfoType;
        LunaShaderModuleCreationInfoTypeUnion creationInfoUnion;
} LunaShaderModuleCreationInfo;

typedef struct
{
        VkPipelineShaderStageCreateFlags flags;
        VkShaderStageFlagBits stage;
        LunaShaderModule module;
        const char *entryPoint;
        const VkSpecializationInfo *specializationInfo;
} LunaPipelineShaderStageCreationInfo;

typedef struct
{
        VkShaderStageFlags stageFlags;
        uint32_t size;

        const void *dataPointer;
        uint32_t dataPointerOffset;
} LunaPushConstantsRange;

typedef struct
{
        VkPipelineLayoutCreateFlags flags;
        uint32_t descriptorSetLayoutCount;
        const LunaDescriptorSetLayout *descriptorSetLayouts;
        uint32_t pushConstantRangeCount;
        const LunaPushConstantsRange *pushConstantsRanges;
} LunaPipelineLayoutCreationInfo;

typedef struct
{
        VkPipelineCreateFlags flags;
        uint32_t shaderStageCount;
        const LunaPipelineShaderStageCreationInfo *shaderStages;
        const VkPipelineVertexInputStateCreateInfo *vertexInputState;
        const VkPipelineInputAssemblyStateCreateInfo *inputAssemblyState;
        const VkPipelineTessellationStateCreateInfo *tessellationState;
        const VkPipelineViewportStateCreateInfo *viewportState;
        const VkPipelineRasterizationStateCreateInfo *rasterizationState;
        const VkPipelineMultisampleStateCreateInfo *multisampleState;
        const VkPipelineDepthStencilStateCreateInfo *depthStencilState;
        const VkPipelineColorBlendStateCreateInfo *colorBlendState;
        const VkPipelineDynamicStateCreateInfo *dynamicState;
        LunaPipelineLayoutCreationInfo layoutCreationInfo;
        LunaRenderPassSubpass subpass;
} LunaGraphicsPipelineCreationInfo;

typedef struct
{
        uint32_t shaderModuleCreationInfoCount;
        const LunaShaderModuleCreationInfo *shaderModuleCreationInfos;
        uint32_t shaderModuleCount;
        LunaShaderModule *shaderModules;
        const char *const *entryPoints;

        VkPipelineCreateFlags flags;
        // TODO (0.3.0): There needs to be a way to null these pointers
        const VkPipelineInputAssemblyStateCreateInfo *inputAssemblyState;
        const VkPipelineTessellationStateCreateInfo *tessellationState;
        const VkPipelineViewportStateCreateInfo *viewportState;
        const VkPipelineRasterizationStateCreateInfo *rasterizationState;
        const VkPipelineMultisampleStateCreateInfo *multisampleState;
        const VkPipelineDepthStencilStateCreateInfo *depthStencilState;
        const VkPipelineColorBlendStateCreateInfo *colorBlendState;
        const VkPipelineDynamicStateCreateInfo *dynamicState;
        LunaPipelineLayoutCreationInfo layoutCreationInfo;
        LunaRenderPassSubpass subpass;
} LunaGraphicsPipelineUsingReflectionCreationInfo; // TODO (0.3.0): Name this better

typedef struct
{
        VkPipelineCreateFlags flags;
        LunaPipelineShaderStageCreationInfo shaderStageCreationInfo;
        LunaPipelineLayoutCreationInfo layoutCreationInfo;
} LunaComputePipelineCreationInfo;

typedef struct
{
        uint32_t firstViewport;
        uint32_t viewportCount;
        const VkViewport *viewports;
} LunaViewportBindInfo;

typedef struct
{
        uint32_t firstScissor;
        uint32_t scissorCount;
        const VkRect2D *scissors;
} LunaScissorBindInfo;

typedef union
{
        const LunaViewportBindInfo *viewportBindInfo;
        const LunaScissorBindInfo *scissorBindInfo;
} LunaDynamicStateBindInfoUnion;

typedef struct
{
        VkDynamicState dynamicStateType;
        LunaDynamicStateBindInfoUnion bindInfo;
} LunaDynamicStateBindInfo;

typedef struct
{
        uint32_t firstSet;
        uint32_t descriptorSetCount;
        const LunaDescriptorSet *descriptorSets;
        uint32_t dynamicOffsetCount;
        const uint32_t *dynamicOffsets;
} LunaDescriptorSetBindInfo;

typedef struct
{
        LunaDescriptorSetBindInfo descriptorSetBindInfo;
        uint32_t dynamicStateCount;
        const LunaDynamicStateBindInfo *dynamicStates;
} LunaGraphicsPipelineBindInfo;

typedef struct
{
        LunaComputePipeline pipeline;
        LunaDescriptorSetBindInfo descriptorSetBindInfo;
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;
} LunaDispatchComputeInfo;

typedef struct
{
        VkDeviceSize size;
        VkBufferCreateFlags flags;
        VkBufferUsageFlags usage;

        const VmaAllocationCreateInfo *allocationCreateInfo;
} LunaBufferCreationInfo;

typedef struct
{
        VkDeviceSize bytes;
        const void *data;
        VkDeviceSize offset;
        VkPipelineStageFlags stageFlags;
} LunaBufferWriteInfo;

typedef struct
{
        VkSamplerCreateFlags flags;
        VkFilter magFilter;
        VkFilter minFilter;
        VkSamplerMipmapMode mipmapMode;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;
        VkSamplerAddressMode addressModeW;
        float mipmapLodBias;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        VkCompareOp compareOp;
        float minLod;
        float maxLod;
        VkBorderColor borderColor;
        bool unnormalizedCoordinates;
} LunaSamplerCreationInfo;

typedef struct
{
        size_t bytes;
        const void *pixels;
        VkImageSubresourceLayers *subresourceLayers;
        const VkOffset3D *offset;
        const VkExtent3D *extent;
        uint32_t mipmapLevels;
        bool generateMipmaps;

        VkPipelineStageFlags sourceStageMask;
        VkPipelineStageFlags destinationStageMask;
        VkAccessFlags destinationAccessMask;

        LunaDescriptorSet descriptorSet;
        const char *descriptorLayoutBindingName;
} LunaImageWriteInfo;

// TODO: There should also be a `LunaSampledImageCreationInfo2`
typedef struct
{
        VkImageCreateFlags flags;
        VkFormat format;
        uint32_t width;
        uint32_t height;
        // TODO (0.3.0): Clang-Tidy: Enum value of type 'VkSampleCountFlagBits' initialized with invalid value of 0, enum doesn't have a zero-value enumerator
        VkSampleCountFlagBits samples;
        VkImageUsageFlags usage;
        VkImageLayout layout;
        VkImageAspectFlags aspectMask;
        LunaImageWriteInfo writeInfo;

        LunaSampler sampler;
        const LunaSamplerCreationInfo *samplerCreationInfo;

        const VmaAllocationCreateInfo *allocationCreateInfo;
} LunaSampledImageCreationInfo;

typedef struct
{
        VkRect2D renderArea;
        VkClearValue depthAttachmentClearValue;
        VkClearValue colorAttachmentClearValue;
} LunaRenderPassBeginInfo;

typedef struct
{
        VkCommandPoolCreateFlags flags;
        VkQueueFlags requiredQueueFlags;
        bool requireQueuePresentationSupport;
} LunaCommandPoolCreationInfo;

typedef struct
{
        LunaRenderPass renderPass;
        uint32_t width;
        uint32_t height;
} LunaRenderPassResizeInfo;

#ifdef __cplusplus
// NOLINTEND(*-macro-usage, *-enum-size, *-use-using, *-use-enum-class)
}
#endif

#endif //LUNATYPES_H
