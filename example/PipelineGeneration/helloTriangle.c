//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
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
const char VERTEX_SHADER[] = "import Luna;\n\
\n\
struct Vertex {\n\
    float2 pos;\n\
    float3 color;\n\
}\n\
\n\
[shader(\"vertex\")]\n\
float4 main([Luna::VERTEX_INPUT_RATE_VERTEX] Vertex inVertex, out float3 color) : SV_Position {\n\
    color = inVertex.color;\n\
    return float4(inVertex.pos, 0, 1);\n\
}\n\
";

const char FRAGMENT_SHADER[] = "import Luna;\n\
\n\
[shader(\"fragment\")]\n\
float4 main(float3 inColor) : SV_Target {\n\
    return float4(inColor, 1);\n\
}\n\
";

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
        .colorAttachmentLoadMode = LUNA_ATTACHMENT_LOAD_MODE_CLEAR,
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

static VkResult createGraphicsPipeline(const LunaRenderPassSubpass subpass, LunaGraphicsPipeline *pipeline)
{
    const LunaShaderModuleCreationInfo vertexShaderCreationInfo = {
        .creationInfoType = LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SLANG,
        .creationInfoUnion.slang.moduleName = "vertexShader",
        .creationInfoUnion.slang.sourceString = VERTEX_SHADER,
    };
    const LunaShaderModuleCreationInfo fragmentShaderCreationInfo = {
        .creationInfoType = LUNA_SHADER_MODULE_CREATION_INFO_TYPE_SLANG,
        .creationInfoUnion.slang.moduleName = "fragmentShader",
        .creationInfoUnion.slang.sourceString = FRAGMENT_SHADER,
    };
    LunaShaderModuleCreationInfo shaderModuleCreationInfos[] = {
        vertexShaderCreationInfo,
        fragmentShaderCreationInfo,
    };

    const LunaGraphicsPipelineUsingReflectionCreationInfo pipelineCreationInfo = {
        .shaderModuleCreationInfoCount = 2,
        .shaderModuleCreationInfos = shaderModuleCreationInfos,
        .subpass = subpass,
    };
    return lunaCreateGraphicsPipelineUsingReflection(&pipelineCreationInfo, pipeline);
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
    const LunaPhysicalDevicePreferenceDefinition physicalDevicePreferenceDefinition = {
        .preferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
    };
    const LunaDeviceCreationInfo deviceCreationInfo = {
        .extensionCount = 1,
        .extensionNames = (const char *const[]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
        .surface = surface,
        .physicalDevicePreferenceDefinition = &physicalDevicePreferenceDefinition,
    };
    CHECK_RESULT(lunaCreateDevice(&deviceCreationInfo));

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
    LunaBufferWriteInfo vertexBufferWriteInfo = {
        .bytes = sizeof(vertices),
        .data = vertices,
        .stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    };
    lunaWriteDataToBuffer(vertexBuffer, &vertexBufferWriteInfo);

    const LunaRenderPassBeginInfo beginInfo = {
        .renderArea.extent.width = extent.width,
        .renderArea.extent.height = extent.height,
    };

    while (!shouldQuit())
    {
        CHECK_RESULT(lunaBeginFrame(false));
        CHECK_RESULT(lunaBeginRenderPass(renderPass, &beginInfo));
        CHECK_RESULT(lunaDrawBuffer(vertexBuffer,
                                    graphicsPipeline,
                                    (LunaGraphicsPipelineBindInfo[]){0},
                                    sizeof(vertices) / sizeof(*vertices),
                                    1,
                                    0,
                                    0));
        lunaEndRenderPass();
        CHECK_RESULT(lunaEndFrame());
    }
    CHECK_RESULT(lunaDestroyInstance());
    return 0;
}
