//
// Created by NBT22 on 2/13/25.
//

#include <algorithm>
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
#include "helpers/Handle.hpp"
#include "Image.hpp"
#include "Luna.hpp"

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
    uint32_t presentModeCount = 0;
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
            if (presentModes.at(j) == mode)
            {
                destination = mode;
                break;
            }
        }
        if (destination != VK_PRESENT_MODE_MAX_ENUM_KHR)
        {
            return VK_SUCCESS;
        }
    }
    if (std::ranges::find(presentModes, VK_PRESENT_MODE_FIFO_KHR) == presentModes.end())
    {
        // FIFO is not supported
        return VK_ERROR_UNKNOWN;
    }
    destination = VK_PRESENT_MODE_FIFO_KHR;
    return VK_SUCCESS;
}

static VkResult createSwapchainImages(const VkDevice device)
{
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchain.imageCount, nullptr));

    swapchain.images.resize(swapchain.imageCount);
    CHECK_RESULT_RETURN(vkGetSwapchainImagesKHR(device,
                                                swapchain.swapchain,
                                                &swapchain.imageCount,
                                                swapchain.images.data()));

    swapchain.imageViews.resize(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        CHECK_RESULT_RETURN(createImageView(device,
                                            swapchain.images.at(i),
                                            swapchain.format.format,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            1,
                                            &swapchain.imageViews.at(i)));
    }
    return VK_SUCCESS;
}

static VkResult createSwapchain(const Device &device, const LunaSwapchainCreationInfo &creationInfo)
{
    assert(!luna::swapchain.safeToUse);

    swapchain.surface = creationInfo.surface;

    VkSurfaceCapabilitiesKHR capabilities;
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(static_cast<VkPhysicalDevice>(device),
                                                                  luna::swapchain.surface,
                                                                  &capabilities));
    capabilities.maxImageCount = capabilities.maxImageCount == 0 ? UINT32_MAX : capabilities.maxImageCount;

    CHECK_RESULT_RETURN(helpers::findSwapchainFormat(static_cast<VkPhysicalDevice>(device),
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

    CHECK_RESULT_RETURN(helpers::getSwapchainPresentMode(static_cast<VkPhysicalDevice>(device),
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
        .imageSharingMode = creationInfo.queueFamilyIndexCount == 1 ? VK_SHARING_MODE_EXCLUSIVE
                                                                    : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = creationInfo.queueFamilyIndexCount,
        .pQueueFamilyIndices = creationInfo.queueFamilyIndices,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = swapchain.compositeAlpha,
        .presentMode = swapchain.presentMode,
        .clipped = static_cast<VkBool32>(swapchain.clipped),
        .oldSwapchain = swapchain.swapchain,
    };
    CHECK_RESULT_RETURN(vkCreateSwapchainKHR(static_cast<VkDevice>(device),
                                             &createInfo,
                                             nullptr,
                                             &luna::swapchain.swapchain));

    CHECK_RESULT_RETURN(helpers::createSwapchainImages(static_cast<VkDevice>(device)));
    assert(capabilities.minImageCount <= swapchain.imageCount && swapchain.imageCount <= capabilities.maxImageCount);

    swapchain.renderSemaphores.reserve(swapchain.imageCount);
    for (uint32_t i = 0; i < swapchain.imageCount; i++)
    {
        constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        swapchain.renderSemaphores.emplace_back(static_cast<VkDevice>(device), semaphoreCreateInfo);
    }
    for (Semaphore &semaphore: swapchain.imageReadySemaphores)
    {
        CHECK_RESULT_RETURN(semaphore.create(static_cast<VkDevice>(device)));
    }

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
std::list<Device> devices{};

#ifdef LUNA_SLANG_SHADERS
slang::IGlobalSession *globalSlangSession{};
std::list<SlangSession> slangSessions{};
#endif
} // namespace luna

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
    for (luna::Device &device: luna::devices)
    {
        if (static_cast<VkPhysicalDevice>(device) == VK_NULL_HANDLE || device.isDestroyed())
        {
            continue;
        }
        CHECK_RESULT_RETURN(vkDeviceWaitIdle(static_cast<VkDevice>(device)));


        for (uint32_t i = 0; i < luna::swapchain.imageCount; i++)
        {
            vkDestroyImageView(static_cast<VkDevice>(device), luna::swapchain.imageViews.at(i), nullptr);
        }
        for (luna::Semaphore &semaphore: luna::swapchain.renderSemaphores)
        {
            semaphore.destroy(static_cast<VkDevice>(device));
        }
        for (luna::Semaphore &semaphore: luna::swapchain.imageReadySemaphores)
        {
            semaphore.destroy(static_cast<VkDevice>(device));
        }
        if (luna::swapchain.swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(static_cast<VkDevice>(device), luna::swapchain.swapchain, nullptr);
        }

        device.destroy();
    }

    luna::devices.clear();

    luna::swapchain.images.clear();
    luna::swapchain.images.shrink_to_fit();
    luna::swapchain.imageViews.clear();
    luna::swapchain.imageViews.shrink_to_fit();
    luna::swapchain.renderSemaphores.clear();
    luna::swapchain.renderSemaphores.shrink_to_fit();

    if (luna::instance != VK_NULL_HANDLE)
    {
        if (luna::swapchain.surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(luna::instance, luna::swapchain.surface, nullptr);
        }
        vkDestroyInstance(luna::instance, nullptr);
    }

    luna::swapchain.surface = VK_NULL_HANDLE;
    luna::swapchain.swapchain = VK_NULL_HANDLE;
    luna::depthImageFormat = VK_FORMAT_UNDEFINED;
    luna::apiVersion = 0;
    luna::instance = VK_NULL_HANDLE;

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
VkResult lunaCreateSwapchain(const LunaDevice device, const LunaSwapchainCreationInfo *creationInfo)
{
    assert(device != LUNA_NULL_HANDLE);
    assert(creationInfo && creationInfo->queueFamilyIndexCount != 0);
    return luna::helpers::createSwapchain(*luna::helpers::fromHandle<luna::Device>(device), *creationInfo);
}
VkSwapchainKHR lunaGetVkSwapchain()
{
    return luna::swapchain.swapchain;
}
VkFormat lunaGetSwapchainFormat()
{
    return luna::swapchain.format.format;
}
VkExtent2D lunaGetSwapchainExtent()
{
    return luna::swapchain.extent;
}
uint32_t lunaGetSwapchainImageCount()
{
    return luna::swapchain.imageCount;
}
uint32_t lunaGetSwapchainImageIndex()
{
    return luna::swapchain.imageIndex;
}
VkResult lunaGetSurfaceCapabilities(const LunaDevice device,
                                    const VkSurfaceKHR surface,
                                    VkSurfaceCapabilitiesKHR *capabilities)
{
    assert(capabilities);
    CHECK_RESULT_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            static_cast<VkPhysicalDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
            surface,
            capabilities));
    capabilities->maxImageCount = capabilities->maxImageCount == 0 ? UINT32_MAX : capabilities->maxImageCount;
    return VK_SUCCESS;
}
VkResult lunaSetDepthImageFormat(const LunaDevice device,
                                 const uint32_t formatCount,
                                 const VkFormat *formatPriorityList)
{
    assert(formatPriorityList);
    VkFormatProperties properties;
    for (uint32_t i = 0; i < formatCount; i++)
    {
        vkGetPhysicalDeviceFormatProperties(static_cast<
                                                    VkPhysicalDevice>(*luna::helpers::fromHandle<luna::Device>(device)),
                                            formatPriorityList[i],
                                            &properties);
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
