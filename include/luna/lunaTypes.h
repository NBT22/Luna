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

typedef const void *LunaRenderPass;
typedef const void *LunaRenderPassSubpass;
typedef const void *LunaGraphicsPipeline;

typedef enum
{
	LUNA_ATTACHMENT_LOAD_UNDEFINED = 1 << 0,
	LUNA_ATTACHMENT_LOAD_CLEAR = 1 << 1,
	LUNA_ATTACHMENT_LOAD_PRESERVE = 1 << 2,
} LunaAttachmentLoadMode;

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
		VkSurfaceFormatKHR *formatPriorityList;
		uint32_t presentModeCount;
		VkPresentModeKHR *presentModePriorityList;

		VkImageUsageFlags imageUsage;
		VkCompositeAlphaFlagBitsKHR compositeAlpha;
} LunaSwapChainCreationInfo;

typedef struct
{
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

		uint32_t attachmentCount;
		const VkAttachmentDescription *attachments;

		uint32_t subpassCount;
		const LunaSubpassCreationInfo *subpasses;
		const char **subpassNames;

		uint32_t dependencyCount;
		const VkSubpassDependency *dependencies;

		const char *uniqueName;
} LunaRenderPassCreationInfo;

typedef struct
{
		const void *pNext;
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

		uint32_t attachmentCount;
		const VkAttachmentDescription2 *attachments;

		uint32_t subpassCount;
		const LunaSubpassCreationInfo2 *subpasses;
		const char **subpassNames;

		uint32_t dependencyCount;
		const VkSubpassDependency2 *dependencies;

		uint32_t correlatedViewMaskCount;
		const uint32_t *correlatedViewMasks;

		const char *uniqueName;
} LunaRenderPassCreationInfo2;

typedef struct
{
		VkDescriptorSetLayoutCreateFlags flags;
		uint32_t bindingCount;
		const VkDescriptorSetLayoutBinding *bindings;
} LunaDescriptorSetLayoutCreationInfo;

typedef struct
{
		VkPipelineLayoutCreateFlags flags;
		uint32_t descriptorSetLayoutCount;
		const LunaDescriptorSetLayoutCreationInfo *descriptorSetLayouts;
		uint32_t pushConstantRangeCount;
		const VkPushConstantRange *pushConstantRanges;
} LunaPipelineLayoutCreationInfo;

typedef struct
{
		VkPipelineCreateFlags flags;
		uint32_t shaderStageCount;
		const VkPipelineShaderStageCreateInfo *shaderStages;
		const VkPipelineVertexInputStateCreateInfo *vertexInputState;
		const VkPipelineInputAssemblyStateCreateInfo *inputAssemblyState;
		const VkPipelineTessellationStateCreateInfo *tessellationState;
		const VkPipelineViewportStateCreateInfo *viewportState;
		const VkPipelineRasterizationStateCreateInfo *rasterizationState;
		const VkPipelineMultisampleStateCreateInfo *multisampleState;
		const VkPipelineDepthStencilStateCreateInfo *depthStencilState;
		const VkPipelineColorBlendStateCreateInfo *colorBlendState;
		const VkPipelineDynamicStateCreateInfo *dynamicState;
		const LunaPipelineLayoutCreationInfo *layoutCreationInfo;
		LunaRenderPassSubpass subpass;

		const char *uniqueName;
} LunaGraphicsPipelineCreationInfo;

#ifdef __cplusplus
}
#endif

#endif //LUNATYPES_H
