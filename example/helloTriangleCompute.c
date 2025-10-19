//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
#include <luna/lunaBuffer.h>
#include <luna/lunaDevice.h>
#include <luna/lunaDrawing.h>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
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

#pragma region constants
/**
 * Compiled SPIRV, generated from the following GLSL
 * @code{'GLSL'}
 * #version 430
 * void main() {
 * }
 * @endcode
 */
static const uint32_t SHADER_SPIRV[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000025, 0x00000000, 0x00020011, 0x00000001, 0x00020011, 0x00000038,
    0x0006000b, 0x00000002, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0006000f, 0x00000005, 0x00000005, 0x6e69616d, 0x00000000, 0x0000000f, 0x00060010, 0x00000005, 0x00000011,
    0x00000001, 0x00000001, 0x00000001, 0x00060007, 0x00000001, 0x706d6f63, 0x2e657475, 0x706d6f63, 0x00000000,
    0x00640003, 0x00000002, 0x000001ae, 0x00000001, 0x4f202f2f, 0x646f4d70, 0x50656c75, 0x65636f72, 0x64657373,
    0x696c6320, 0x20746e65, 0x6b6c7576, 0x30316e61, 0x2f2f0a30, 0x4d704f20, 0x6c75646f, 0x6f725065, 0x73736563,
    0x74206465, 0x65677261, 0x6e652d74, 0x75762076, 0x6e616b6c, 0x0a302e31, 0x4f202f2f, 0x646f4d70, 0x50656c75,
    0x65636f72, 0x64657373, 0x746e6520, 0x702d7972, 0x746e696f, 0x69616d20, 0x6c230a6e, 0x20656e69, 0x76230a31,
    0x69737265, 0x34206e6f, 0x0a0a3033, 0x74786523, 0x69736e65, 0x47206e6f, 0x58455f4c, 0x68735f54, 0x72656461,
    0x616d695f, 0x6c5f6567, 0x5f64616f, 0x6d726f66, 0x65747461, 0x203a2064, 0x62616e65, 0x0a0a656c, 0x6f79616c,
    0x28207475, 0x646e6962, 0x20676e69, 0x2930203d, 0x696e7520, 0x6d726f66, 0x616d6920, 0x44326567, 0x616d6920,
    0x0a3b6567, 0x696f760a, 0x616d2064, 0x29286e69, 0x090a7b20, 0x67616d69, 0x6f745365, 0x69286572, 0x6567616d,
    0x7669202c, 0x28326365, 0x475f6c67, 0x61626f6c, 0x766e496c, 0x7461636f, 0x496e6f69, 0x79782e44, 0x76202c29,
    0x28346365, 0x5f6c6728, 0x626f6c47, 0x6e496c61, 0x61636f76, 0x6e6f6974, 0x782e4449, 0x67202b20, 0x6c475f6c,
    0x6c61626f, 0x6f766e49, 0x69746163, 0x44496e6f, 0x2029792e, 0x2c322025, 0x202c3020, 0x31202c30, 0x0a3b2929,
    0x00000a7d, 0x000a0004, 0x455f4c47, 0x735f5458, 0x65646168, 0x6d695f72, 0x5f656761, 0x64616f6c, 0x726f665f,
    0x7474616d, 0x00006465, 0x00040005, 0x00000005, 0x6e69616d, 0x00000000, 0x00040005, 0x0000000a, 0x67616d69,
    0x00000065, 0x00080005, 0x0000000f, 0x475f6c67, 0x61626f6c, 0x766e496c, 0x7461636f, 0x496e6f69, 0x00000044,
    0x00040047, 0x0000000a, 0x00000021, 0x00000000, 0x00040047, 0x0000000a, 0x00000022, 0x00000000, 0x00040047,
    0x0000000f, 0x0000000b, 0x0000001c, 0x00020013, 0x00000003, 0x00030021, 0x00000004, 0x00000003, 0x00030016,
    0x00000007, 0x00000020, 0x00090019, 0x00000008, 0x00000007, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
    0x00000002, 0x00000000, 0x00040020, 0x00000009, 0x00000000, 0x00000008, 0x0004003b, 0x00000009, 0x0000000a,
    0x00000000, 0x00040015, 0x0000000c, 0x00000020, 0x00000000, 0x00040017, 0x0000000d, 0x0000000c, 0x00000003,
    0x00040020, 0x0000000e, 0x00000001, 0x0000000d, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040017,
    0x00000010, 0x0000000c, 0x00000002, 0x00040015, 0x00000013, 0x00000020, 0x00000001, 0x00040017, 0x00000014,
    0x00000013, 0x00000002, 0x0004002b, 0x0000000c, 0x00000016, 0x00000000, 0x00040020, 0x00000017, 0x00000001,
    0x0000000c, 0x0004002b, 0x0000000c, 0x0000001a, 0x00000001, 0x0004002b, 0x0000000c, 0x0000001e, 0x00000002,
    0x0004002b, 0x00000007, 0x00000021, 0x00000000, 0x0004002b, 0x00000007, 0x00000022, 0x3f800000, 0x00040017,
    0x00000023, 0x00000007, 0x00000004, 0x00040008, 0x00000001, 0x00000007, 0x0000000b, 0x00050036, 0x00000003,
    0x00000005, 0x00000000, 0x00000004, 0x000200f8, 0x00000006, 0x00040008, 0x00000001, 0x00000008, 0x00000000,
    0x0004003d, 0x00000008, 0x0000000b, 0x0000000a, 0x0004003d, 0x0000000d, 0x00000011, 0x0000000f, 0x0007004f,
    0x00000010, 0x00000012, 0x00000011, 0x00000011, 0x00000000, 0x00000001, 0x0004007c, 0x00000014, 0x00000015,
    0x00000012, 0x00050041, 0x00000017, 0x00000018, 0x0000000f, 0x00000016, 0x0004003d, 0x0000000c, 0x00000019,
    0x00000018, 0x00050041, 0x00000017, 0x0000001b, 0x0000000f, 0x0000001a, 0x0004003d, 0x0000000c, 0x0000001c,
    0x0000001b, 0x00050080, 0x0000000c, 0x0000001d, 0x00000019, 0x0000001c, 0x00050089, 0x0000000c, 0x0000001f,
    0x0000001d, 0x0000001e, 0x00040070, 0x00000007, 0x00000020, 0x0000001f, 0x00070050, 0x00000023, 0x00000024,
    0x00000020, 0x00000021, 0x00000021, 0x00000022, 0x00040063, 0x0000000b, 0x00000015, 0x00000024, 0x00040008,
    0x00000001, 0x00000009, 0x00000000, 0x000100fd, 0x00010038,
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

int main(void)
{
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland");
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
        .requiredFeatures.shaderStorageImageWriteWithoutFormat = VK_TRUE,
        .surface = surface,
        .physicalDevicePreferenceDefinition = &physicalDevicePreferenceDefinition,
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
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .presentModeCount = 1,
        .presentModePriorityList = (VkPresentModeKHR[]){VK_PRESENT_MODE_FIFO_KHR},
    };
    CHECK_RESULT(lunaCreateSwapchain(&swapchainCreationInfo));

    const LunaDescriptorSetLayoutBinding binding = {
        .bindingName = "Output Image",
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };
    const LunaDescriptorSetLayoutCreationInfo descriptorSetLayoutCreationInfo = {
        .bindingCount = 1,
        .bindings = &binding,
    };
    LunaDescriptorSetLayout descriptorSetLayout = LUNA_NULL_HANDLE;
    CHECK_RESULT(lunaCreateDescriptorSetLayout(&descriptorSetLayoutCreationInfo, &descriptorSetLayout));

    LunaShaderModule shaderModule = LUNA_NULL_HANDLE;
    const LunaShaderModuleCreationInfo shaderModuleCreationInfo = {
        .size = sizeof(SHADER_SPIRV),
        .spirv = SHADER_SPIRV,
    };
    CHECK_RESULT(lunaCreateShaderModule(&shaderModuleCreationInfo, &shaderModule));

    LunaComputePipeline pipeline = LUNA_NULL_HANDLE;
    const LunaComputePipelineCreationInfo pipelineCreationInfo = {
        .shaderStageCreationInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .shaderStageCreationInfo.module = shaderModule,
        .layoutCreationInfo.descriptorSetLayoutCount = 1,
        .layoutCreationInfo.descriptorSetLayouts = &descriptorSetLayout,
    };
    CHECK_RESULT(lunaCreateComputePipeline(&pipelineCreationInfo, &pipeline));

    LunaDescriptorPool descriptorPool = LUNA_NULL_HANDLE;
    const LunaDescriptorPoolCreationInfo descriptorPoolCreationInfo = {
        .maxSets = 1,
        .poolSizeCount = 1,
        .poolSizes = (VkDescriptorPoolSize[]){{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
        }},
    };
    CHECK_RESULT(lunaCreateDescriptorPool(&descriptorPoolCreationInfo, &descriptorPool));

    LunaDescriptorSet descriptorSet = LUNA_NULL_HANDLE;
    const LunaDescriptorSetAllocationInfo descriptorSetAllocationInfo = {
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .setLayouts = &descriptorSetLayout,
    };
    CHECK_RESULT(lunaAllocateDescriptorSets(&descriptorSetAllocationInfo, &descriptorSet));

    const LunaDispatchComputeInfo dispatchComputeInfo = {
        .pipeline = pipeline,
        .descriptorSetBindInfo.descriptorSetCount = 1,
        .descriptorSetBindInfo.descriptorSets = &descriptorSet,
        .groupCountX = 540,
        .groupCountY = 360,
    };

    while (!shouldQuit())
    {
        CHECK_RESULT(lunaBeginFrame(false));
        lunaTransitionColorImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        lunaWriteFramebufferToDescriptor(descriptorSet);
        CHECK_RESULT(lunaDispatchCompute(&dispatchComputeInfo));
        lunaTransitionColorImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        CHECK_RESULT(lunaEndFrame());
    }
    CHECK_RESULT(lunaDestroyInstance());
    return 0;
}
