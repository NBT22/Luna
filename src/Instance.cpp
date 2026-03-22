//
// Created by NBT22 on 2/13/25.
//

#include <cassert>
#include <cstdint>
#include <cstring>
#include <list>
#include <luna/lunaInstance.h>
#include <luna/lunaTypes.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Buffer.hpp"
#include "ComputePipeline.hpp"
#include "DescriptorSetLayout.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "Image.hpp"
#include "Instance.hpp"
#include "Luna.hpp"
#include "RenderPass.hpp"
#include "helpers/Handle.hpp"

#ifdef LUNA_SLANG_SHADERS
#include "SlangSession.hpp"
#endif

namespace
{
bool volkInitialized{};
std::unordered_map<std::string, uint32_t> extensionMap;

void fillExtensionMap()
{
    uint32_t propertyCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
    std::vector<VkExtensionProperties> extensionsProperties(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, extensionsProperties.data());
    for (const VkExtensionProperties &extensionProperties: extensionsProperties)
    {
        extensionMap[extensionProperties.extensionName] = extensionProperties.specVersion;
    }
}
} // namespace

namespace luna::helpers
{
static bool isInstanceExtensionAvailable(const char *extensionName, const uint32_t version = 0)
{
    if (extensionMap.empty())
    {
        fillExtensionMap();
    }
    return extensionMap.contains(extensionName) && extensionMap.at(extensionName) >= version;
}

static VkResult findSwapchainFormat(const VkPhysicalDevice physicalDevice,
                                    const VkSurfaceKHR surface,
                                    const uint32_t targetFormatCount,
                                    const VkSurfaceFormatKHR *targetFormats,
                                    VkSurfaceFormatKHR &destination)
{
    destination = {.format = VK_FORMAT_UNDEFINED, .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR};
    uint32_t formatCount = 0;
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
            const VkSurfaceFormatKHR &format = formats.at(j);
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
    uint32_t presentModeCount = 0;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    if (presentModeCount == 0)
    {
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
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
            if (presentModes.at(j) == mode)
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

static VkResult createSwapchainImages(const VkDevice logicalDevice)
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
                                            swapchain.images.at(i),
                                            swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &swapchain.imageViews.at(i)));
    }
    return VK_SUCCESS;
}

static VkResult createSwapchain(const LunaSwapchainCreationInfo &creationInfo)
{
    assert(!luna::swapchain.safeToUse);
    swapchain.surface = creationInfo.surface;

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(luna::device,
                                                                  luna::swapchain.surface,
                                                                  &capabilities));
    capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;

    CHECK_RESULT_RETURN(helpers::findSwapchainFormat(luna::device,
                                                     luna::swapchain.surface,
                                                     creationInfo.formatCount,
                                                     creationInfo.formatPriorityList,
                                                     luna::swapchain.format));

    swapchain.extent = capabilities.currentExtent;
    if (swapchain.extent.width == UINT32_MAX || swapchain.extent.height == UINT32_MAX)
    {
        swapchain.extent.width = creationInfo.width;
        swapchain.extent.height = creationInfo.height;
    }
    assert(capabilities.minImageExtent.width <= luna::swapchain.extent.width &&
           luna::swapchain.extent.width <= capabilities.maxImageExtent.width);
    assert(capabilities.minImageExtent.height <= luna::swapchain.extent.height &&
           luna::swapchain.extent.height <= capabilities.maxImageExtent.height);

    CHECK_RESULT_RETURN(helpers::getSwapchainPresentMode(luna::device,
                                                         luna::swapchain.surface,
                                                         creationInfo.presentModeCount,
                                                         creationInfo.presentModePriorityList,
                                                         luna::swapchain.presentMode));

    constexpr VkImageUsageFlags colorAttachmentUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain.imageUsage = creationInfo.imageUsage == 0 ? colorAttachmentUsage : creationInfo.imageUsage;
    swapchain.compositeAlpha = creationInfo.compositeAlpha == 0 ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
                                                                : creationInfo.compositeAlpha;
    swapchain.clipped = creationInfo.clipped;
    const VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = swapchain.surface,
        .minImageCount = capabilities.minImageCount,
        .imageFormat = swapchain.format.format,
        .imageColorSpace = swapchain.format.colorSpace,
        .imageExtent = swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = swapchain.imageUsage,
        .imageSharingMode = device.sharingMode(),
        .queueFamilyIndexCount = device.familyCount(),
        .pQueueFamilyIndices = device.queueFamilyIndices(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = swapchain.compositeAlpha,
        .presentMode = swapchain.presentMode,
        .clipped = static_cast<VkBool32>(swapchain.clipped),
        .oldSwapchain = swapchain.swapchain,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(luna::device, &createInfo, nullptr, &luna::swapchain.swapchain));

    CHECK_RESULT_RETURN(helpers::createSwapchainImages(luna::device));
    assert(capabilities.minImageCount <= luna::swapchain.imageCount &&
           luna::swapchain.imageCount <= capabilities.maxImageCount);
    CHECK_RESULT_RETURN(luna::device.createSemaphores(luna::swapchain.imageCount));

    constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    CHECK_RESULT_RETURN(
            luna::device.commandPools().graphics->commandBuffer().resizeArray(luna::device,
                                                                              *luna::device.commandPools().graphics,
                                                                              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                              nullptr,
                                                                              &semaphoreCreateInfo,
                                                                              luna::swapchain.imageCount));

    swapchain.imageIndex = -1u;
    swapchain.safeToUse = true;
    swapchain.safeToUse.notify_all();
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna
{
Swapchain swapchain{};
VkFormat depthImageFormat{};
uint32_t apiVersion{};
VkInstance instance{};
Device device{};
BufferRegionIndex *stagingBuffer{};
VkPipeline boundPipeline{};
LunaBuffer boundVertexBuffer{};
LunaBuffer boundIndexBuffer{};

#ifdef LUNA_SLANG_SHADERS
slang::IGlobalSession *globalSlangSession{};
std::list<SlangSession> slangSessions{};
#endif

std::list<RenderPass> renderPasses{};
std::list<DescriptorSetLayout> descriptorSetLayouts{};
std::list<VkDescriptorPool> descriptorPools{};
std::list<VkDescriptorSet> descriptorSets{};
std::list<DescriptorSetIndex> descriptorSetIndices{};
std::list<GraphicsPipeline> graphicsPipelines{};
std::list<ComputePipeline> computePipelines{};
std::list<Buffer> buffers{};
std::list<BufferRegionIndex> bufferRegionIndices{};
std::list<VkSampler> samplers{};
std::list<Image> images{};
} // namespace luna

const LunaCommandPool LUNA_INTERNAL_GRAPHICS_COMMAND_POOL =
        luna::helpers::toHandle(luna::device.commandPools().graphics);
const LunaCommandPool LUNA_INTERNAL_COMPUTE_COMMAND_POOL = luna::helpers::toHandle(luna::device.commandPools().compute);

VkResult lunaInitializeVolk()
{
    if (!volkInitialized)
    {
        CHECK_RESULT_RETURN(volkInitialize());
        volkInitialized = true;
    }
    return VK_SUCCESS;
}
VkResult lunaCreateInstance(const LunaInstanceCreationInfo *creationInfo)
{
    assert(creationInfo);
    luna::apiVersion = creationInfo->apiVersion == 0 ? VK_API_VERSION_1_0 : creationInfo->apiVersion;

    CHECK_RESULT_RETURN(lunaInitializeVolk());

    std::vector<const char *> enabledLayers(creationInfo->layerNames,
                                            creationInfo->layerNames + creationInfo->layerCount);
    if (creationInfo->enableValidation)
    {
        uint32_t propertyCount = 0;
        vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
        std::vector<VkLayerProperties> properties(propertyCount);
        vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());
        constexpr const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
        constexpr size_t validationLayerNameLength = std::char_traits<char>::length(validationLayerName);
        for (const VkLayerProperties &layerProperties: properties)
        {
            if (std::strncmp(layerProperties.layerName, validationLayerName, validationLayerNameLength) == 0)
            {
                enabledLayers.emplace_back(validationLayerName);
                break;
            }
        }
    }

    const VkApplicationInfo vulkanApplicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = luna::apiVersion,
    };
    const VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = creationInfo->flags,
        .pApplicationInfo = &vulkanApplicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
        .ppEnabledLayerNames = enabledLayers.data(),
        .enabledExtensionCount = creationInfo->extensionCount,
        .ppEnabledExtensionNames = creationInfo->extensionNames,
    };
    CHECK_RESULT_RETURN(vkCreateInstance(&createInfo, nullptr, &luna::instance));
    volkLoadInstanceOnly(luna::instance);
    return VK_SUCCESS;
}
VkResult lunaDestroyInstance()
{
    using namespace luna;

    if (static_cast<VkPhysicalDevice>(device) != VK_NULL_HANDLE)
    {
        BufferRegionIndex::waitForCleanupThread();
        CHECK_RESULT_RETURN(vkDeviceWaitIdle(device));


        for (uint32_t i = 0; i < swapchain.imageCount; i++)
        {
            vkDestroyImageView(device, swapchain.imageViews.at(i), nullptr);
        }
        if (swapchain.swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
        }

        for (const VkSampler sampler: samplers)
        {
            vkDestroySampler(device, sampler, nullptr);
        }
        samplers.clear();
        images.clear();

        for (GraphicsPipeline pipeline: graphicsPipelines)
        {
            pipeline.destroy();
        }
        for (RenderPass renderPass: renderPasses)
        {
            renderPass.destroy();
        }

        computePipelines.clear();

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

        graphicsPipelines.clear();
        renderPasses.clear();

        descriptorSetIndices.clear();
        descriptorPools.clear();
        descriptorSetLayouts.clear();
        descriptorSets.clear();

        bufferRegionIndices.clear();
        BufferRegionIndex::waitForCleanupThread();

        device.destroy();
    }

    if (instance != VK_NULL_HANDLE)
    {
        if (swapchain.surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, swapchain.surface, nullptr);
        }
        vkDestroyInstance(instance, nullptr);
    }

    swapchain.surface = VK_NULL_HANDLE;
    swapchain.swapchain = VK_NULL_HANDLE;
    depthImageFormat = VK_FORMAT_UNDEFINED;
    apiVersion = 0;
    instance = VK_NULL_HANDLE;
    device = Device();
    stagingBuffer = nullptr;
    boundPipeline = VK_NULL_HANDLE;
    boundVertexBuffer = LUNA_NULL_HANDLE;
    boundIndexBuffer = LUNA_NULL_HANDLE;

    return VK_SUCCESS;
}
VkInstance lunaGetInstance()
{
    return luna::instance;
}
bool lunaIsInstanceExtensionAvailable(const char *extensionName)
{
    return luna::helpers::isInstanceExtensionAvailable(extensionName);
}
bool lunaIsInstanceExtensionVersionAvailable(const char *extensionName, const uint32_t extensionVersion)
{
    return luna::helpers::isInstanceExtensionAvailable(extensionName, extensionVersion);
}
VkResult lunaCreateSwapchain(const LunaSwapchainCreationInfo *creationInfo)
{
    assert(creationInfo);
    return luna::helpers::createSwapchain(*creationInfo);
}
VkFormat lunaGetSwapchainFormat()
{
    return luna::swapchain.format.format;
}
VkExtent2D lunaGetSwapchainExtent()
{
    return luna::swapchain.extent;
}
VkResult lunaGetSurfaceCapabilities(const VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities)
{
    assert(capabilities);
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(luna::device, surface, capabilities));
    capabilities->maxImageCount = capabilities->maxImageCount == 0 ? UINT32_MAX : capabilities->maxImageCount;
    return VK_SUCCESS;
}
VkResult lunaSetDepthImageFormat(const uint32_t formatCount, const VkFormat *formatPriorityList)
{
    assert(formatPriorityList);
    VkFormatProperties properties;
    for (uint32_t i = 0; i < formatCount; i++)
    {
        vkGetPhysicalDeviceFormatProperties(luna::device, formatPriorityList[i], &properties);
        if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            luna::depthImageFormat = formatPriorityList[i];
            return VK_SUCCESS;
        }
    }
    return VK_ERROR_FORMAT_NOT_SUPPORTED;
}
VkFormat lunaGetDepthImageFormat()
{
    return luna::depthImageFormat;
}
