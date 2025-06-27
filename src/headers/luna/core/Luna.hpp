//
// Created by NBT22 on 2/17/25.
//

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace luna::helpers
{
class VkResultException final: public std::exception
{
    public:
        explicit VkResultException(const VkResult result): result(result) {}
        VkResult result;
};
} // namespace luna::helpers

// TODO: This really needs more work. It should log a message (or not, based on settings provided by the application).
//  Maybe also non fatal? I'm not really sure what the best way to do this function is.
#define CHECK_RESULT_RETURN(value) \
    if (const VkResult result = value; result != VK_SUCCESS) \
    { \
        return result; \
    }
#define CHECK_RESULT_THROW(value) \
    if (const VkResult result = value; result != VK_SUCCESS) \
    { \
        throw luna::helpers::VkResultException(result); \
    }
#define TRY_CATCH_RESULT(expression) \
    try \
    { \
        expression; \
    } catch (const luna::helpers::VkResultException &exception) \
    { \
        return exception.result; \
    }

namespace luna::core
{
struct SwapChain
{
        VkSurfaceKHR surface;
        uint32_t imageCount;
        VkSurfaceFormatKHR format;
        VkExtent2D extent;
        VkImageUsageFlags imageUsage;
        VkPresentModeKHR presentMode;
        VkCompositeAlphaFlagBitsKHR compositeAlpha;
        VkSwapchainKHR swapChain;
        uint32_t imageIndex;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
};
} // namespace luna::core
