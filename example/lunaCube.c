//
// Created by NBT22 on 3/11/25.
//

#include <cglm/cglm.h>
#include <lodepng.h>
#include <luna/luna.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdio.h>
#include <stdlib.h>

#pragma region typedefs
typedef struct
{
		float x, y, z;
		float u, v;
} Vertex;
#pragma endregion typedefs

#pragma region constants
/**
 * Compiled SPIRV, generated from the following GLSL
 * @code{'GLSL'}
 * layout (push_constant) uniform PushConstants {mat4 translationMatrix;} pushConstants;
 * layout (location = 0) in vec3 inPos;
 * layout (location = 1) in vec2 inUV;
 * layout (location = 0) out vec2 outUV;
 * void main() {
 *     gl_Position = pushConstants.translationMatrix * vec4(inPos, 1.0);
 *     outUV = inUV;
 * }
 * @endcode
 */
static const uint32_t VERTEX_SHADER_SPIRV[339] = {
	0x07230203, 0x00010000, 0x000d000b, 0x00000029, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001,
	0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0009000f, 0x00000000,
	0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000019, 0x00000025, 0x00000027, 0x00030003, 0x00000002,
	0x000001cc, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f,
	0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572,
	0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67, 0x65567265,
	0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006,
	0x0000000b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002,
	0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75,
	0x61747369, 0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00060005, 0x00000011, 0x68737550, 0x736e6f43,
	0x746e6174, 0x00000073, 0x00080006, 0x00000011, 0x00000000, 0x6e617274, 0x74616c73, 0x4d6e6f69, 0x69727461,
	0x00000078, 0x00060005, 0x00000013, 0x68737570, 0x736e6f43, 0x746e6174, 0x00000073, 0x00040005, 0x00000019,
	0x6f506e69, 0x00000073, 0x00040005, 0x00000025, 0x5574756f, 0x00000056, 0x00040005, 0x00000027, 0x56556e69,
	0x00000000, 0x00030047, 0x0000000b, 0x00000002, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
	0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b,
	0x00000003, 0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x00000011, 0x00000002,
	0x00040048, 0x00000011, 0x00000000, 0x00000005, 0x00050048, 0x00000011, 0x00000000, 0x00000007, 0x00000010,
	0x00050048, 0x00000011, 0x00000000, 0x00000023, 0x00000000, 0x00040047, 0x00000019, 0x0000001e, 0x00000000,
	0x00040047, 0x00000025, 0x0000001e, 0x00000000, 0x00040047, 0x00000027, 0x0000001e, 0x00000001, 0x00020013,
	0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
	0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009,
	0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006,
	0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
	0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
	0x00040018, 0x00000010, 0x00000007, 0x00000004, 0x0003001e, 0x00000011, 0x00000010, 0x00040020, 0x00000012,
	0x00000009, 0x00000011, 0x0004003b, 0x00000012, 0x00000013, 0x00000009, 0x00040020, 0x00000014, 0x00000009,
	0x00000010, 0x00040017, 0x00000017, 0x00000006, 0x00000003, 0x00040020, 0x00000018, 0x00000001, 0x00000017,
	0x0004003b, 0x00000018, 0x00000019, 0x00000001, 0x0004002b, 0x00000006, 0x0000001b, 0x3f800000, 0x00040020,
	0x00000021, 0x00000003, 0x00000007, 0x00040017, 0x00000023, 0x00000006, 0x00000002, 0x00040020, 0x00000024,
	0x00000003, 0x00000023, 0x0004003b, 0x00000024, 0x00000025, 0x00000003, 0x00040020, 0x00000026, 0x00000001,
	0x00000023, 0x0004003b, 0x00000026, 0x00000027, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000,
	0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000014, 0x00000015, 0x00000013, 0x0000000f, 0x0004003d,
	0x00000010, 0x00000016, 0x00000015, 0x0004003d, 0x00000017, 0x0000001a, 0x00000019, 0x00050051, 0x00000006,
	0x0000001c, 0x0000001a, 0x00000000, 0x00050051, 0x00000006, 0x0000001d, 0x0000001a, 0x00000001, 0x00050051,
	0x00000006, 0x0000001e, 0x0000001a, 0x00000002, 0x00070050, 0x00000007, 0x0000001f, 0x0000001c, 0x0000001d,
	0x0000001e, 0x0000001b, 0x00050091, 0x00000007, 0x00000020, 0x00000016, 0x0000001f, 0x00050041, 0x00000021,
	0x00000022, 0x0000000d, 0x0000000f, 0x0003003e, 0x00000022, 0x00000020, 0x0004003d, 0x00000023, 0x00000028,
	0x00000027, 0x0003003e, 0x00000025, 0x00000028, 0x000100fd, 0x00010038,
};
/**
 * Compiled SPIRV, generated from the following GLSL
 * @code{'GLSL'}
 * layout (binding = 0) uniform sampler2D textureSampler;
 * layout (location = 0) in vec2 inUV;
 * layout (location = 0) out vec4 outColor;
 * void main() {
 *     outColor = texture(textureSampler, inUV);
 * }
 * @endcode
 */
static const uint32_t FRAGMENT_SHADER_SPIRV[159] = {
	0x07230203, 0x00010000, 0x000d000b, 0x00000014, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001,
	0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
	0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00000011, 0x00030010, 0x00000004, 0x00000007, 0x00030003,
	0x00000002, 0x000001cc, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c,
	0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65,
	0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f,
	0x726f6c6f, 0x00000000, 0x00060005, 0x0000000d, 0x74786574, 0x53657275, 0x6c706d61, 0x00007265, 0x00040005,
	0x00000011, 0x56556e69, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d,
	0x00000021, 0x00000000, 0x00040047, 0x0000000d, 0x00000022, 0x00000000, 0x00040047, 0x00000011, 0x0000001e,
	0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020,
	0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
	0x00000008, 0x00000009, 0x00000003, 0x00090019, 0x0000000a, 0x00000006, 0x00000001, 0x00000000, 0x00000000,
	0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x0000000b, 0x0000000a, 0x00040020, 0x0000000c, 0x00000000,
	0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040017, 0x0000000f, 0x00000006, 0x00000002,
	0x00040020, 0x00000010, 0x00000001, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011, 0x00000001, 0x00050036,
	0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000b, 0x0000000e,
	0x0000000d, 0x0004003d, 0x0000000f, 0x00000012, 0x00000011, 0x00050057, 0x00000007, 0x00000013, 0x0000000e,
	0x00000012, 0x0003003e, 0x00000009, 0x00000013, 0x000100fd, 0x00010038,
};

static const Vertex vertices[] = {
	{.x = -0.5f, .y = -0.5f, .z = 0.5f},
	{.x = 0.5f, .y = -0.5f, .z = 0.5f, .u = 1},
	{.x = 0.5f, .y = 0.5f, .z = 0.5f, .u = 1, .v = 1},
	{.x = -0.5f, .y = 0.5f, .z = 0.5f, .v = 1},
	{.x = -0.5f, .y = -0.5f, .z = -0.5f, .u = 1},
	{.x = 0.5f, .y = -0.5f, .z = -0.5f},
	{.x = 0.5f, .y = 0.5f, .z = -0.5f, .v = 1},
	{.x = -0.5f, .y = 0.5f, .z = -0.5f, .u = 1, .v = 1},

	{.x = -0.5f, .y = 0.5f, .z = -0.5f, .v = 1},
	{.x = 0.5f, .y = 0.5f, .z = -0.5f, .u = 1, .v = 1},
	{.x = 0.5f, .y = 0.5f, .z = 0.5f, .u = 1},
	{.x = -0.5f, .y = 0.5f, .z = 0.5f},
	{.x = -0.5f, .y = -0.5f, .z = -0.5f, .u = 1, .v = 1},
	{.x = 0.5f, .y = -0.5f, .z = -0.5f, .v = 1},
	{.x = 0.5f, .y = -0.5f, .z = 0.5f},
	{.x = -0.5f, .y = -0.5f, .z = 0.5f, .u = 1},

	{.x = 0.5f, .y = -0.5f, .z = -0.5f, .u = 1, .v = 1},
	{.x = 0.5f, .y = 0.5f, .z = 0.5f},
	{.x = -0.5f, .y = -0.5f, .z = -0.5f, .v = 1},
	{.x = -0.5f, .y = 0.5f, .z = 0.5f, .u = 1},
};
static const uint32_t indices[] = {
	0,	1,	2,	2,	3,	0,	4,	5, 6,  6,  7, 4,  8,  9, 10, 10, 11, 8,
	12, 13, 14, 14, 15, 12, 16, 6, 17, 17, 1, 16, 18, 7, 19, 19, 0,	 18,
};
#pragma endregion constants

static void createRenderPass(const VkExtent3D extent, LunaRenderPass *renderPass)
{
	lunaSetDepthImageFormat(2, (VkFormat[]){VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT});

	const LunaRenderPassCreationInfo renderPassCreationInfo = {
		.samples = VK_SAMPLE_COUNT_4_BIT,
		.createColorAttachment = true,
		.colorAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
		.createDepthAttachment = true,
		.depthAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
		.subpassCount = 1,
		.subpasses = (const LunaSubpassCreationInfo[]){{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.useColorAttachment = true,
			.useDepthAttachment = true,
		}},
		.dependencyCount = 1,
		.dependencies = (const VkSubpassDependency[]){{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		}},
		.extent = extent,
	};
	lunaCreateRenderPass(&renderPassCreationInfo, renderPass);
}

static void createGraphicsPipeline(LunaRenderPassSubpass subpass,
								   const LunaDescriptorSetLayout *descriptorSetLayouts,
								   mat4 *const pushConstantData,
								   LunaGraphicsPipeline *pipeline)
{
	const VkExtent2D swapChainExtent = lunaGetSwapChainExtent();

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;
	lunaCreateShaderModule(VERTEX_SHADER_SPIRV, sizeof(VERTEX_SHADER_SPIRV), &vertexShaderModule);
	lunaCreateShaderModule(FRAGMENT_SHADER_SPIRV, sizeof(FRAGMENT_SHADER_SPIRV), &fragmentShaderModule);
	if (!vertexShaderModule || !fragmentShaderModule)
	{
		// TODO: Figure out how to handle Luna functions failing
		return;
	}
	const VkPipelineShaderStageCreateInfo shaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertexShaderModule,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragmentShaderModule,
			.pName = "main",
		},
	};

	const VkVertexInputBindingDescription inputBindingDescription = {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription vertexAttributeDescriptions[2] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, u),
		},
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &inputBindingDescription,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions = vertexAttributeDescriptions,
	};

	const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	const VkViewport viewport = {
		.width = (float)swapChainExtent.width,
		.height = (float)swapChainExtent.height,
		.maxDepth = 1,
	};
	const VkRect2D scissor = {
		.extent = swapChainExtent,
	};
	const VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1,
	};

	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT,
		.minSampleShading = 1,
	};

	const VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.maxDepthBounds = 1,
	};

	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
						  VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT,
	};
	const VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	const LunaPushConstantsRange pushConstantsRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.size = 64,
		.dataPointer = pushConstantData,
	};
	const LunaPipelineLayoutCreationInfo layoutCreationInfo = {
		.descriptorSetLayoutCount = 1,
		.descriptorSetLayouts = descriptorSetLayouts,
		.pushConstantRangeCount = 1,
		.pushConstantsRanges = &pushConstantsRange,
	};

	const LunaGraphicsPipelineCreationInfo pipelineCreationInfo = {
		.shaderStageCount = 2,
		.shaderStages = shaderStages,
		.vertexInputState = &vertexInputInfo,
		.inputAssemblyState = &inputAssembly,
		.viewportState = &viewportState,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencil,
		.colorBlendState = &colorBlending,
		.layoutCreationInfo = &layoutCreationInfo,
		.subpass = subpass,
	};
	lunaCreateGraphicsPipeline(&pipelineCreationInfo, pipeline);
}

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return 1;
	}
	SDL_Window *window = SDL_CreateWindow("Luna Example", 720, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (window == NULL)
	{
		return 2;
	}


	uint32_t instanceExtensionCount = 0;
	const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
	const LunaInstanceCreationInfo instanceCreationInfo = {
		.apiVersion = VK_API_VERSION_1_2,

		.extensionCount = instanceExtensionCount,
		.extensionNames = instanceExtensions,

#ifndef NDEBUG
		.enableValidation = true,
#endif
	};
	lunaCreateInstance(&instanceCreationInfo);

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, lunaGetInstance(), NULL, &surface))
	{
		return 3;
	}
	const VkPhysicalDeviceFeatures requiredFeatures = {
		.logicOp = VK_TRUE,
		.samplerAnisotropy = VK_TRUE,
	};
	const LunaDeviceCreationInfo deviceCreationInfo = {
		.extensionCount = 1,
		.extensionNames = (const char *const[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.requiredFeatures = requiredFeatures,
		.surface = surface,
	};
	lunaAddNewDevice(&deviceCreationInfo);

	const VkExtent3D extent = {
		.width = 720,
		.height = 720,
		.depth = 1,
	};
	const LunaSwapChainCreationInfo swapChainCreationInfo = {
		.surface = surface,
		.width = extent.width,
		.height = extent.height,
		.minImageCount = 3,
		.formatCount = 1,
		.formatPriorityList = (VkSurfaceFormatKHR[]){{VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR}},
		.presentModeCount = 2,
		.presentModePriorityList = (VkPresentModeKHR[]){VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR},
	};
	lunaCreateSwapChain(&swapChainCreationInfo);

	LunaRenderPass renderPass;
	createRenderPass(extent, &renderPass);

	const LunaDescriptorSetLayoutBinding binding = {
		.bindingName = "Texture",
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	const LunaDescriptorSetLayoutCreationInfo descriptorSetLayoutCreationInfo = {
		.bindingCount = 1,
		.bindings = &binding,
	};
	LunaDescriptorSetLayout descriptorSetLayout;
	lunaCreateDescriptorSetLayout(&descriptorSetLayoutCreationInfo, &descriptorSetLayout);

	const LunaDescriptorPoolCreationInfo descriptorPoolCreationInfo = {
		.maxSets = 1,
		.poolSizeCount = 1,
		.poolSizes = (VkDescriptorPoolSize[]){{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
		}},
	};
	LunaDescriptorPool descriptorPool;
	lunaCreateDescriptorPool(&descriptorPoolCreationInfo, &descriptorPool);
	const LunaDescriptorSetAllocationInfo descriptorSetAllocationInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.setLayouts = &descriptorSetLayout,
	};
	LunaDescriptorSet descriptorSet;
	lunaAllocateDescriptorSets(&descriptorSetAllocationInfo, &descriptorSet);

	uint8_t *pixels;
	uint32_t width;
	uint32_t height;
	uint32_t result = lodepng_decode32_file(&pixels, &width, &height, "logo.png");
	if (result != 0)
	{
		printf("\x1b[31mGot error %d (%s) when loading image!\n", result, lodepng_error_text(result));
		fflush(stdout);
		return result; // NOLINT(*-narrowing-conversions)
	}

	const LunaSamplerCreationInfo samplerCreationInfo = {
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.maxAnisotropy = 1,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	};
	const LunaSampledImageCreationInfo imageCreationInfo = {
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.width = width,
		.height = height,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.pixels = pixels,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.descriptorSet = descriptorSet,
		.descriptorLayoutBindingName = "Texture",
		.samplerCreationInfo = &samplerCreationInfo,
		.sourceStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		.destinationStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
	};
	lunaCreateImage(&imageCreationInfo, NULL);
	free(pixels);

	mat4 viewMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_lookat((vec3){2.0f, 2.0f, 2.0f}, GLM_VEC3_ZERO, (vec3){0.0f, 0.0f, -1.0f}, viewMatrix);
	mat4 projectionMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_perspective(0.7853981633974483f, 1, 0.1f, 4.25f, projectionMatrix);
	mat4 transformMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_mul(projectionMatrix, viewMatrix, transformMatrix);

	LunaGraphicsPipeline graphicsPipeline;
	createGraphicsPipeline(lunaGetRenderPassSubpassByName(renderPass, NULL),
						   &descriptorSetLayout,
						   &transformMatrix,
						   &graphicsPipeline);

	LunaBufferCreationInfo vertexBufferCreationInfo = {
		.size = sizeof(vertices),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	LunaBuffer vertexBuffer;
	lunaCreateBuffer(&vertexBufferCreationInfo, &vertexBuffer);
	lunaWriteDataToBuffer(vertexBuffer, vertices, sizeof(vertices));

	LunaBufferCreationInfo indexBufferCreationInfo = {
		.size = sizeof(indices),
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	};
	LunaBuffer indexBuffer;
	lunaCreateBuffer(&indexBufferCreationInfo, &indexBuffer);
	lunaWriteDataToBuffer(indexBuffer, indices, sizeof(indices));

	const LunaRenderPassBeginInfo beginInfo = {
		.renderArea.extent.width = extent.width,
		.renderArea.extent.height = extent.height,
		.depthAttachmentClearValue.depthStencil.depth = 1,
	};
	const LunaGraphicsPipelineBindInfo bindInfo = {
		.descriptorSetCount = 1,
		.descriptorSets = &descriptorSet,
	};

	SDL_Event event;
	while (true)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					lunaDestroyInstance();
					return 0;
				case SDL_EVENT_KEY_UP:
					if (event.key.scancode == SDL_SCANCODE_ESCAPE)
					{
						lunaDestroyInstance();
						return 0;
					}
					break;
				default:;
			}
		}
		glm_rotate(transformMatrix, 0.00015f, GLM_ZUP);
		lunaBeginRenderPass(renderPass, &beginInfo);
		lunaPushConstants(graphicsPipeline);
		lunaDrawBufferIndexed(vertexBuffer,
							  indexBuffer,
							  0,
							  VK_INDEX_TYPE_UINT32,
							  graphicsPipeline,
							  &bindInfo,
							  sizeof(indices) / sizeof(*indices),
							  1,
							  0,
							  0,
							  0);
		lunaDrawFrame();
	}
}
