//
// Created by NBT22 on 2/15/25.
//

#include <luna/luna.h>
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
 *
 * #extension GL_EXT_shader_image_load_formatted : enable
 *
 * layout (binding = 0) uniform image2D image;
 *
 * const float width = 1080;
 * const float height = 720;
 * const float left = width / 4;
 * const float right = 3 * width / 4 - 1;
 * const float top = height / 4;
 * const float bottom = 3 * height / 4 - 1;
 * const float middle = width / 2;
 *
 * void main() {
 *     if (gl_GlobalInvocationID.x < left || gl_GlobalInvocationID.x > right || gl_GlobalInvocationID.y < top || gl_GlobalInvocationID.y > bottom ||
 *         (gl_GlobalInvocationID.y < ((top - bottom) / (middle - left)) * (gl_GlobalInvocationID.x - left) + bottom) ||
 *         (gl_GlobalInvocationID.y < ((top - bottom) / (left - middle)) * (gl_GlobalInvocationID.x - right) + bottom)) {
 *         imageStore(image, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(0, 0, 0, 1));
 *     } else {
 *         imageStore(image, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(1, 0, 0, 1));
 *     }
 * }
 * @endcode
 */
#include "spv.h"
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
        .setLayoutCount = 1,
        .setLayouts = &descriptorSetLayout,
    };
    CHECK_RESULT(lunaAllocateDescriptorSets(&descriptorSetAllocationInfo, &descriptorSet));

    const LunaDispatchInfo dispatchComputeInfo = {
        .pipeline = pipeline,
        .descriptorSetBindInfo.descriptorSetCount = 1,
        .descriptorSetBindInfo.descriptorSets = &descriptorSet,
        .groupCountX = 1080,
        .groupCountY = 720,
    };

    while (!shouldQuit())
    {
        CHECK_RESULT(lunaBeginFrame(false));
        lunaTransitionColorImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        lunaWriteFramebufferToDescriptor(descriptorSet);
        CHECK_RESULT(lunaDispatch(&dispatchComputeInfo));
        lunaTransitionColorImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        CHECK_RESULT(lunaEndFrame());
    }
    CHECK_RESULT(lunaDestroyInstance());
    return 0;
}
