//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNATYPES_H
#define LUNATYPES_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif

#define LUNA_DEFINE_HANDLE(object) typedef const void *object

#define LUNA_NULL_HANDLE VK_NULL_HANDLE

LUNA_DEFINE_HANDLE(LunaRenderPass);
LUNA_DEFINE_HANDLE(LunaRenderPassSubpass);
LUNA_DEFINE_HANDLE(LunaDescriptorPool);
LUNA_DEFINE_HANDLE(LunaDescriptorSetLayout);
LUNA_DEFINE_HANDLE(LunaDescriptorSet);
LUNA_DEFINE_HANDLE(LunaShaderModule);
LUNA_DEFINE_HANDLE(LunaGraphicsPipeline);
LUNA_DEFINE_HANDLE(LunaBuffer);
LUNA_DEFINE_HANDLE(LunaSampler);
LUNA_DEFINE_HANDLE(LunaImage);
LUNA_DEFINE_HANDLE(LunaCommandPool);

typedef enum
{
    LUNA_ATTACHMENT_LOAD_UNDEFINED = 1 << 0,
    LUNA_ATTACHMENT_LOAD_CLEAR = 1 << 1,
    LUNA_ATTACHMENT_LOAD_PRESERVE = 1 << 2,
} LunaAttachmentLoadMode;

typedef enum
{
    LUNA_INTERNAL_GRAPHICS_COMMAND_POOL,
} LunaInternalCommandPools;

typedef struct
{
        const uint32_t apiVersion;

        const uint32_t extensionCount;
        const char *const *extensionNames;

        bool enableValidation;
        const uint32_t layerCount;
        const char *const *layerNames;
} LunaInstanceCreationInfo;

typedef struct
{
        const uint32_t extensionCount;
        const char *const *extensionNames;

        const VkPhysicalDeviceFeatures requiredFeatures;
        VkSurfaceKHR surface;
} LunaDeviceCreationInfo;

typedef struct
{
        const uint32_t extensionCount;
        const char *const *extensionNames;

        const VkPhysicalDeviceFeatures2 requiredFeatures;
        VkSurfaceKHR surface;
} LunaDeviceCreationInfo2;

typedef struct
{
        VkSurfaceKHR surface;
        uint32_t width;
        uint32_t height;
        uint32_t minImageCount;

        uint32_t formatCount;
        const VkSurfaceFormatKHR *formatPriorityList;
        uint32_t presentModeCount;
        const VkPresentModeKHR *presentModePriorityList;

        VkImageUsageFlags imageUsage;
        VkCompositeAlphaFlagBitsKHR compositeAlpha;
} LunaSwapChainCreationInfo;

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
        const VkDescriptorBindingFlags bindingFlags;
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
        // TODO: const VkBufferView *texelBufferView;
} LunaWriteDescriptorSet;

typedef struct
{
        VkPipelineShaderStageCreateFlags flags;
        VkShaderStageFlagBits stage;
        LunaShaderModule module;
        const char *entrypoint;
        const VkSpecializationInfo *specializationInfo;
} LunaPipelineShaderStageCreationInfo;

typedef struct
{
        VkShaderStageFlags stageFlags;
        uint32_t size;

        void *const dataPointer;
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
        const LunaPipelineLayoutCreationInfo layoutCreationInfo;
        LunaRenderPassSubpass subpass;
} LunaGraphicsPipelineCreationInfo;

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

typedef struct
{
    VkDynamicState dynamicStateType;
    union
    {
        const LunaViewportBindInfo *viewportBindInfo;
        const LunaScissorBindInfo *scissorBindInfo;
    };
} LunaDynamicStateBindInfo;

typedef struct
{
        uint32_t firstSet;
        uint32_t descriptorSetCount;
        const LunaDescriptorSet *descriptorSets;
        uint32_t dynamicOffsetCount;
        const uint32_t *dynamicOffsets;
        uint32_t dynamicStateCount;
        const LunaDynamicStateBindInfo *dynamicStates;
} LunaGraphicsPipelineBindInfo;

typedef struct
{
        VkDeviceSize size;
        VkBufferCreateFlags flags;
        VkBufferUsageFlags usage;
} LunaBufferCreationInfo;

typedef struct
{
        VkSamplerCreateFlags flags;
        VkFilter magFilter;
        VkFilter minFilter;
        VkSamplerMipmapMode mipmapMode;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;
        VkSamplerAddressMode addressModeW;
        float mipLodBias;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        VkCompareOp compareOp;
        float minLod;
        float maxLod;
        VkBorderColor borderColor;
        bool unnormalizedCoordinates;
} LunaSamplerCreationInfo;

// TODO: There should also be a `LunaSampledImageCreationInfo2`
typedef struct
{
        VkImageCreateFlags flags;
        VkFormat format;
        uint32_t width;
        uint32_t height;
        uint32_t mipmapLevels;
        bool generateMipmaps;
        VkSampleCountFlagBits samples;
        VkImageUsageFlags usage;

        const void *pixels;
        VkImageLayout layout;
        VkImageAspectFlags aspectMask;
        LunaDescriptorSet descriptorSet;
        const char *descriptorLayoutBindingName;

        LunaSampler sampler;
        const LunaSamplerCreationInfo *samplerCreationInfo;

        VkPipelineStageFlags sourceStageMask;
        VkPipelineStageFlags destinationStageMask;
        VkAccessFlags destinationAccessMask;
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

#define LUNA_RENDER_PASS_WIDTH_SWAPCHAIN_WIDTH (-1u)
#define LUNA_RENDER_PASS_HEIGHT_SWAPCHAIN_HEIGHT (-1u)
typedef struct
{
        LunaRenderPass renderPass;
        uint32_t width;
        uint32_t height;
} LunaRenderPassResizeInfo;

#ifdef __cplusplus
}
#endif

#endif //LUNATYPES_H
