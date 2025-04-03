//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#pragma region typedefs
typedef struct
{
		float x, y;
		float r, g, b;
} Vertex;
#pragma endregion typedefs

#pragma region constants
/**
 * Compiled SPIRV, generated from the following GLSL
 * @code{'GLSL'}
 * layout (location = 0) in vec3 inPos;
 * layout (location = 1) in vec3 inColor;
 * layout (location = 0) out vec4 outColor;
 * void main() {
 *     gl_Position = vec4(inPos, 1.0);
 *     outColor = vec4(inColor, 1.0);
 * }
 * @endcode
 */
static const uint32_t VERTEX_SHADER_SPIRV[280] = {
	0x07230203, 0x00010000, 0x000d000b, 0x00000022, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001,
	0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0009000f, 0x00000000,
	0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x0000001b, 0x0000001c, 0x00030003, 0x00000002,
	0x000001cc, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f,
	0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572,
	0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67, 0x65567265,
	0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006,
	0x0000000b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002,
	0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75,
	0x61747369, 0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00040005, 0x00000012, 0x6f506e69, 0x00000073,
	0x00050005, 0x0000001b, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00040005, 0x0000001c, 0x6f436e69, 0x00726f6c,
	0x00030047, 0x0000000b, 0x00000002, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048,
	0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003,
	0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00040047, 0x00000012, 0x0000001e, 0x00000000,
	0x00040047, 0x0000001b, 0x0000001e, 0x00000000, 0x00040047, 0x0000001c, 0x0000001e, 0x00000001, 0x00020013,
	0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
	0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009,
	0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006,
	0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
	0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
	0x00040017, 0x00000010, 0x00000006, 0x00000003, 0x00040020, 0x00000011, 0x00000001, 0x00000010, 0x0004003b,
	0x00000011, 0x00000012, 0x00000001, 0x0004002b, 0x00000006, 0x00000014, 0x3f800000, 0x00040020, 0x00000019,
	0x00000003, 0x00000007, 0x0004003b, 0x00000019, 0x0000001b, 0x00000003, 0x0004003b, 0x00000011, 0x0000001c,
	0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
	0x00000010, 0x00000013, 0x00000012, 0x00050051, 0x00000006, 0x00000015, 0x00000013, 0x00000000, 0x00050051,
	0x00000006, 0x00000016, 0x00000013, 0x00000001, 0x00050051, 0x00000006, 0x00000017, 0x00000013, 0x00000002,
	0x00070050, 0x00000007, 0x00000018, 0x00000015, 0x00000016, 0x00000017, 0x00000014, 0x00050041, 0x00000019,
	0x0000001a, 0x0000000d, 0x0000000f, 0x0003003e, 0x0000001a, 0x00000018, 0x0004003d, 0x00000010, 0x0000001d,
	0x0000001c, 0x00050051, 0x00000006, 0x0000001e, 0x0000001d, 0x00000000, 0x00050051, 0x00000006, 0x0000001f,
	0x0000001d, 0x00000001, 0x00050051, 0x00000006, 0x00000020, 0x0000001d, 0x00000002, 0x00070050, 0x00000007,
	0x00000021, 0x0000001e, 0x0000001f, 0x00000020, 0x00000014, 0x0003003e, 0x0000001b, 0x00000021, 0x000100fd,
	0x00010038,
};
/**
 * Compiled SPIRV, generated from the following GLSL
 * @code{'GLSL'}
 * layout (location = 0) in vec4 inColor;
 * layout (location = 0) out vec4 outColor;
 * void main() {
 *     outColor = inColor;
 * }
 * @endcode
 */
static const uint32_t FRAGMENT_SHADER_SPIRV[112] = {
	0x07230203, 0x00010000, 0x000d000b, 0x0000000d, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001,
	0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
	0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00030010, 0x00000004, 0x00000007, 0x00030003,
	0x00000002, 0x000001cc, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c,
	0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65,
	0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f,
	0x726f6c6f, 0x00000000, 0x00040005, 0x0000000b, 0x6f436e69, 0x00726f6c, 0x00040047, 0x00000009, 0x0000001e,
	0x00000000, 0x00040047, 0x0000000b, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
	0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020,
	0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a,
	0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
	0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b, 0x0003003e,
	0x00000009, 0x0000000c, 0x000100fd, 0x00010038,
};

static const Vertex vertices[3] = {
	{.x = 0.0f, .y = -0.5f, .r = 1},
	{.x = 0.5f, .y = 0.5f, .g = 1},
	{.x = -0.5f, .y = 0.5f, .b = 1},
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

static void createGraphicsPipeline(LunaRenderPassSubpass subpass, LunaGraphicsPipeline *pipeline)
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
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, r),
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
		.subpass = subpass,
	};
	lunaCreateGraphicsPipeline(&pipelineCreationInfo, pipeline);
}

int main()
{
	if (!SDL_Init(SDL_INIT_EVENTS))
	{
		return 1;
	}
	SDL_Window *window = SDL_CreateWindow("Luna Example", 1080, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
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
		.width = 1080,
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

	LunaGraphicsPipeline graphicsPipeline;
	createGraphicsPipeline(lunaGetRenderPassSubpassByName(renderPass, NULL), &graphicsPipeline);

	LunaBufferCreationInfo bufferCreationInfo = {
		.size = sizeof(vertices),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};
	LunaBuffer vertexBuffer;
	lunaCreateBuffer(&bufferCreationInfo, &vertexBuffer);
	lunaWriteDataToBuffer(vertexBuffer, vertices, sizeof(vertices));

	const LunaRenderPassBeginInfo beginInfo = {
		.renderArea.extent.width = extent.width,
		.renderArea.extent.height = extent.height,
		.depthAttachmentClearValue.depthStencil.depth = 1,
	};

#ifdef ALWAYS_UPDATE
	SDL_Event event;
	while (true)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					lunaCleanup();
					return 0;
				case SDL_EVENT_KEY_UP:
					if (event.key.scancode == SDL_SCANCODE_ESCAPE)
					{
						lunaCleanup();
						return 0;
					}
					break;
				default:;
			}
		}
		lunaBeginRenderPass(renderPass, &beginInfo);
		lunaDrawBuffer(&drawInfo);
		lunaDrawFrame();
	}
#else
	SDL_Event event;
	while (SDL_WaitEvent(&event))
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
		lunaBeginRenderPass(renderPass, &beginInfo);
		lunaDrawBuffer(vertexBuffer,
					   graphicsPipeline,
					   (LunaGraphicsPipelineBindInfo[]){0},
					   sizeof(vertices) / sizeof(*vertices),
					   1,
					   0,
					   0);
		lunaDrawFrame();
	}
#endif
}
