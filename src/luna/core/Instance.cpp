//
// Created by NBT22 on 2/13/25.
//

#include <cstring>
#include <luna/core/Instance.hpp>
#include <luna/lunaInstance.h>
#include <stdexcept>

namespace luna::helpers
{
static VkResult findSwapchainFormat(const VkPhysicalDevice physicalDevice,
                                    const VkSurfaceKHR surface,
                                    const uint32_t targetFormatCount,
                                    const VkSurfaceFormatKHR *targetFormats,
                                    VkSurfaceFormatKHR &destination)
{
    destination = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR};
    uint32_t formatCount;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
    if (formatCount == 0)
    {
        return VK_ERROR_UNKNOWN;
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()));
    for (uint32_t i = 0; i < targetFormatCount; i++)
    {
        const VkSurfaceFormatKHR &targetFormat = targetFormats[i];
        for (uint32_t j = 0; j < formatCount; j++)
        {
            const VkSurfaceFormatKHR &format = formats[j];
            if (format.colorSpace == targetFormat.colorSpace && format.format == targetFormat.format)
            {
                destination = format;
                break;
            }
        }
        if (destination.format != VK_FORMAT_UNDEFINED && destination.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR)
        {
            break;
        }
    }
    if (destination.format == VK_FORMAT_UNDEFINED || destination.colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR)
    {
        throw std::runtime_error("Unable to find suitable Vulkan surface format!");
    }
    return VK_SUCCESS;
}

static VkResult getSwapchainPresentMode(const VkPhysicalDevice physicalDevice,
                                        const VkSurfaceKHR surface,
                                        const uint32_t targetPresentModeCount,
                                        const VkPresentModeKHR *targetPresentModes,
                                        VkPresentModeKHR &destination)
{
    if (targetPresentModeCount == 0)
    {
        // TODO: Check if fifo is somehow missing
        destination = VK_PRESENT_MODE_FIFO_KHR;
        return VK_SUCCESS;
    }
    uint32_t presentModeCount;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    if (presentModeCount == 0)
    {
        return VK_ERROR_UNKNOWN;
    }
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                                  surface,
                                                                  &presentModeCount,
                                                                  presentModes.data()));
    destination = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (uint32_t i = 0; i < targetPresentModeCount; i++)
    {
        const VkPresentModeKHR mode = targetPresentModes[i];
        for (uint32_t j = 0; j < presentModeCount; j++)
        {
            if (presentModes[j] == mode)
            {
                destination = mode;
                break;
            }
        }
        if (destination != VK_PRESENT_MODE_MAX_ENUM_KHR)
        {
            break;
        }
    }
    // This is an assert instead of an error because VK_PRESENT_MODE_FIFO_KHR is required to be supported.
    assert(destination != VK_PRESENT_MODE_MAX_ENUM_KHR);
    return VK_SUCCESS;
}

static VkResult createSwapchainImages(const VkDevice logicalDevice, core::Swapchain &swapchain)
{
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(logicalDevice, swapchain.swapchain, &swapchain.imageCount, nullptr));

    swapchain.images.resize(swapchain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(logicalDevice,
                                                swapchain.swapchain,
                                                &swapchain.imageCount,
                                                swapchain.images.data()));

    swapchain.imageViews.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(logicalDevice,
                                            swapchain.images[i],
                                            swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &swapchain.imageViews[i]));
    }
    return VK_SUCCESS;
}

static VkResult createSwapchain(const LunaSwapchainCreationInfo &creationInfo)
{
    assert(!core::swapchain.safeToUse);
    core::swapchain.surface = creationInfo.surface;

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core::device,
                                                                  core::swapchain.surface,
                                                                  &capabilities));
    capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;

    CHECK_RESULT_RETURN(helpers::findSwapchainFormat(core::device,
                                                     core::swapchain.surface,
                                                     creationInfo.formatCount,
                                                     creationInfo.formatPriorityList,
                                                     core::swapchain.format));

    core::swapchain.extent = capabilities.currentExtent;
    if (core::swapchain.extent.width == UINT32_MAX || core::swapchain.extent.height == UINT32_MAX)
    {
        core::swapchain.extent.width = creationInfo.width;
        core::swapchain.extent.height = creationInfo.height;
    }
    assert(capabilities.minImageExtent.width <= core::swapchain.extent.width &&
           core::swapchain.extent.width <= capabilities.maxImageExtent.width);
    assert(capabilities.minImageExtent.height <= core::swapchain.extent.height &&
           core::swapchain.extent.height <= capabilities.maxImageExtent.height);

    CHECK_RESULT_RETURN(helpers::getSwapchainPresentMode(core::device,
                                                         core::swapchain.surface,
                                                         creationInfo.presentModeCount,
                                                         creationInfo.presentModePriorityList,
                                                         core::swapchain.presentMode));

    constexpr VkImageUsageFlags colorAttachmentUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    core::swapchain.imageUsage = creationInfo.imageUsage == 0 ? colorAttachmentUsage : creationInfo.imageUsage;
    core::swapchain.compositeAlpha = creationInfo.compositeAlpha == 0 ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
                                                                      : creationInfo.compositeAlpha;
    const VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = core::swapchain.surface,
        .minImageCount = capabilities.minImageCount,
        .imageFormat = core::swapchain.format.format,
        .imageColorSpace = core::swapchain.format.colorSpace,
        .imageExtent = core::swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = core::swapchain.imageUsage,
        .imageSharingMode = core::device.sharingMode(),
        .queueFamilyIndexCount = core::device.familyCount(),
        .pQueueFamilyIndices = core::device.queueFamilyIndices(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = core::swapchain.compositeAlpha,
        .presentMode = core::swapchain.presentMode,
        .clipped = VK_TRUE, // TODO: Support applications being able to set this... somehow
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(core::device, &createInfo, nullptr, &core::swapchain.swapchain));

    CHECK_RESULT_RETURN(helpers::createSwapchainImages(core::device, core::swapchain));
    assert(capabilities.minImageCount <= core::swapchain.imageCount &&
           core::swapchain.imageCount <= capabilities.maxImageCount);
    CHECK_RESULT_RETURN(core::device.createSemaphores(core::swapchain.imageCount));
    core::swapchain.imageIndex = -1u;
    core::swapchain.safeToUse = true;
    core::swapchain.safeToUse.notify_all();
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core
{
Swapchain swapchain{};
VkFormat depthImageFormat{};
uint32_t apiVersion{};
VkInstance instance{};
Device device{};
LunaBuffer stagingBuffer{};
VkPipeline boundPipeline{};
LunaBuffer boundVertexBuffer{};
LunaBuffer boundIndexBuffer{};
VkDeviceSize boundIndexBufferOffset{};

std::list<RenderPass> renderPasses{};
std::list<DescriptorSetLayout> descriptorSetLayouts{};
std::list<VkDescriptorPool> descriptorPools{};
std::list<VkDescriptorSet> descriptorSets{};
std::list<DescriptorSetIndex> descriptorSetIndices{};
std::list<GraphicsPipeline> graphicsPipelines{};
std::list<Buffer> buffers{};
std::list<buffer::BufferRegionIndex> bufferRegionIndices{};
std::list<VkSampler> samplers{};
std::list<Image> images{};
} // namespace luna::core

VkResult lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo)
{
    assert(creationInfo);
    luna::core::apiVersion = creationInfo->apiVersion;

    const uint32_t enabledLayerCount = creationInfo->enableValidation ? creationInfo->layerCount + 1
                                                                      : creationInfo->layerCount;
    std::vector<const char *> enabledLayers;
    enabledLayers.reserve(enabledLayerCount);
    for (uint32_t i = 0; i < creationInfo->layerCount; i++)
    {
        enabledLayers.emplace_back(creationInfo->layerNames[i]);
    }
    if (creationInfo->enableValidation)
    {
        enabledLayers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    const VkApplicationInfo vulkanApplicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = creationInfo->apiVersion,
    };
    const VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &vulkanApplicationInfo,
        .enabledLayerCount = enabledLayerCount,
        .ppEnabledLayerNames = enabledLayers.data(),
        .enabledExtensionCount = creationInfo->extensionCount,
        .ppEnabledExtensionNames = creationInfo->extensionNames,
    };
    CHECK_RESULT_RETURN(vkCreateInstance(&createInfo, nullptr, &luna::core::instance));
    return VK_SUCCESS;
}
VkResult lunaDestroyInstance()
{
    using namespace luna::core;
    CHECK_RESULT_RETURN(vkDeviceWaitIdle(device));


    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        vkDestroyImageView(device, swapchain.imageViews.at(i), nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);

    for (const VkSampler sampler: samplers)
    {
        vkDestroySampler(device, sampler, nullptr);
    }
    for (const Image image: images)
    {
        image.destroy();
    }

    for (GraphicsPipeline pipeline: graphicsPipelines)
    {
        pipeline.destroy();
    }
    for (RenderPass renderPass: renderPasses)
    {
        renderPass.destroy();
    }

    for (const VkDescriptorPool descriptorPool: descriptorPools)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    for (DescriptorSetLayout descriptorSetLayout: descriptorSetLayouts)
    {
        descriptorSetLayout.destroy();
    }

    swapchain.images.clear();
    swapchain.images.shrink_to_fit();
    swapchain.imageViews.clear();
    swapchain.imageViews.shrink_to_fit();

    samplers.clear();
    images.clear();

    graphicsPipelines.clear();
    renderPasses.clear();

    descriptorSetIndices.clear();
    descriptorPools.clear();
    descriptorSetLayouts.clear();
    descriptorSets.clear();

    bufferRegionIndices.clear();
    buffers.clear();
    stagingBuffer = nullptr;

    device.destroy();
    vkDestroySurfaceKHR(instance, swapchain.surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    return VK_SUCCESS;
}
VkInstance lunaGetInstance()
{
    return luna::core::instance;
}
VkResult lunaCreateSwapchain(const LunaSwapchainCreationInfo *creationInfo)
{
    assert(creationInfo);
    return luna::helpers::createSwapchain(*creationInfo);
}
VkFormat lunaGetSwapchainFormat()
{
    return luna::core::swapchain.format.format;
}
VkExtent2D lunaGetSwapchainExtent()
{
    return luna::core::swapchain.extent;
}
VkResult lunaGetSurfaceCapabilities(const VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities)
{
    assert(capabilities);
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(luna::core::device, surface, capabilities));
    capabilities->maxImageCount = capabilities->maxImageCount == 0 ? UINT32_MAX : capabilities->maxImageCount;
    return VK_SUCCESS;
}
void lunaSetDepthImageFormat(const uint32_t formatCount, const VkFormat *formatPriorityList)
{
    assert(formatPriorityList);
    VkFormatProperties properties;
    for (uint32_t i = 0; i < formatCount; i++)
    {
        vkGetPhysicalDeviceFormatProperties(luna::core::device, formatPriorityList[i], &properties);
        if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            luna::core::depthImageFormat = formatPriorityList[i];
            return;
        }
    }
}
VkFormat lunaGetDepthImageFormat()
{
    return luna::core::depthImageFormat;
}
