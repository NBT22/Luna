//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaInstance.h>
#include <luna/lunaPipeline.h>
#include <luna/lunaRenderPass.h>
#include <luna/lunaTypes.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#define CHECK_RESULT(value) \
    if ((value) < 0) \
    { \
        return 5; \
    }

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

static bool shouldQuit(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                return true;
            case SDL_EVENT_KEY_UP:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                {
                    return true;
                }
            default:;
        }
    }
    return false;
}

static VkResult createRenderPass(const VkExtent3D extent, LunaRenderPass *renderPass)
{
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    const LunaRenderPassCreationInfo renderPassCreationInfo = {
        .samples = VK_SAMPLE_COUNT_4_BIT,
        .createColorAttachment = true,
        .colorAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_CLEAR,
        .subpassCount = 1,
        .subpasses = (const LunaSubpassCreationInfo[]){{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .useColorAttachment = true,
        }},
        .dependencyCount = 1,
        .dependencies = &dependency,
        .extent = extent,
    };
    return lunaCreateRenderPass(&renderPassCreationInfo, renderPass);
}

static VkResult createGraphicsPipeline(LunaRenderPassSubpass subpass, LunaGraphicsPipeline *pipeline)
{
    const VkExtent2D swapchainExtent = lunaGetSwapchainExtent();

    LunaShaderModule vertexShaderModule = LUNA_NULL_HANDLE;
    LunaShaderModule fragmentShaderModule = LUNA_NULL_HANDLE;
    const LunaShaderModuleCreationInfo vertexShaderCreationInfo = {
        .size = sizeof(VERTEX_SHADER_SPIRV),
        .spirv = VERTEX_SHADER_SPIRV,
    };
    VkResult vertexShaderCreationResult = lunaCreateShaderModule(&vertexShaderCreationInfo, &vertexShaderModule);
    if (vertexShaderCreationResult != VK_SUCCESS)
    {
        return vertexShaderCreationResult;
    }
    const LunaShaderModuleCreationInfo fragmentShaderCreationInfo = {
        .size = sizeof(FRAGMENT_SHADER_SPIRV),
        .spirv = FRAGMENT_SHADER_SPIRV,
    };
    VkResult fragmentShaderCreationResult = lunaCreateShaderModule(&fragmentShaderCreationInfo, &fragmentShaderModule);
    if (fragmentShaderCreationResult != VK_SUCCESS)
    {
        return fragmentShaderCreationResult;
    }
    const LunaPipelineShaderStageCreationInfo shaderStages[2] = {
        {
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
        },
        {
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
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
        .vertexAttributeDescriptionCount = sizeof(vertexAttributeDescriptions) / sizeof(*vertexAttributeDescriptions),
        .pVertexAttributeDescriptions = vertexAttributeDescriptions,
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    const VkViewport viewport = {
        .width = (float)swapchainExtent.width,
        .height = (float)swapchainExtent.height,
        .maxDepth = 1,
    };
    const VkRect2D scissor = {
        .extent = swapchainExtent,
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
        .lineWidth = 1,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_4_BIT,
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT,
    };
    const VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    const LunaGraphicsPipelineCreationInfo pipelineCreationInfo = {
        .shaderStageCount = sizeof(shaderStages) / sizeof(*shaderStages),
        .shaderStages = shaderStages,
        .vertexInputState = &vertexInputInfo,
        .inputAssemblyState = &inputAssembly,
        .viewportState = &viewportState,
        .rasterizationState = &rasterizer,
        .multisampleState = &multisampling,
        .colorBlendState = &colorBlending,
        .subpass = subpass,
    };
    return lunaCreateGraphicsPipeline(&pipelineCreationInfo, pipeline);
}

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
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
        // TODO: Using validation requires having the Vulkan SDK installed
        .enableValidation = true,
#endif
    };
    if (lunaCreateInstance(&instanceCreationInfo) != VK_SUCCESS)
    {
        return 3;
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface(window, lunaGetInstance(), NULL, &surface))
    {
        return 4;
    }
    const LunaDeviceCreationInfo deviceCreationInfo = {
        .extensionCount = 1,
        .extensionNames = (const char *const[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
        .surface = surface,
    };
    CHECK_RESULT(lunaAddNewDevice(&deviceCreationInfo));

    const VkExtent3D extent = {
        .width = 1080,
        .height = 720,
        .depth = 1,
    };
    const LunaSwapchainCreationInfo swapchainCreationInfo = {
        .surface = surface,
        .width = extent.width,
        .height = extent.height,
        .formatCount = 1,
        .formatPriorityList = (VkSurfaceFormatKHR[]){{VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR}},
        .presentModeCount = 1,
        .presentModePriorityList = (VkPresentModeKHR[]){VK_PRESENT_MODE_FIFO_KHR},
    };
    CHECK_RESULT(lunaCreateSwapchain(&swapchainCreationInfo));

    LunaRenderPass renderPass = LUNA_NULL_HANDLE;
    CHECK_RESULT(createRenderPass(extent, &renderPass));

    LunaGraphicsPipeline graphicsPipeline = LUNA_NULL_HANDLE;
    CHECK_RESULT(createGraphicsPipeline(lunaGetRenderPassSubpassByName(renderPass, NULL), &graphicsPipeline));

    const LunaBufferCreationInfo bufferCreationInfo = {
        .size = sizeof(vertices),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    };
    LunaBuffer vertexBuffer = LUNA_NULL_HANDLE;
    CHECK_RESULT(lunaCreateBuffer(&bufferCreationInfo, &vertexBuffer));
    lunaWriteDataToBuffer(vertexBuffer, vertices, sizeof(vertices), 0);

    const LunaRenderPassBeginInfo beginInfo = {
        .renderArea.extent.width = extent.width,
        .renderArea.extent.height = extent.height,
    };

    while (!shouldQuit())
    {
        CHECK_RESULT(lunaBeginRenderPass(renderPass, &beginInfo));
        CHECK_RESULT(lunaDrawBuffer(vertexBuffer,
                                    graphicsPipeline,
                                    (LunaGraphicsPipelineBindInfo[]){0},
                                    sizeof(vertices) / sizeof(*vertices),
                                    1,
                                    0,
                                    0));
        lunaEndRenderPass();
        CHECK_RESULT(lunaPresentSwapchain());
    }
    CHECK_RESULT(lunaDestroyInstance());
    return 0;
}
