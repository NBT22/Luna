//
// Created by NBT22 on 2/17/25.
//

#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <exception>
#include <luna/lunaTypes.h>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "Semaphore.hpp"

static_assert(VK_ACCESS_NONE == VK_ACCESS_2_NONE);
static_assert(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT == VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT);
static_assert(VK_ACCESS_TRANSFER_READ_BIT == VK_ACCESS_2_TRANSFER_READ_BIT);
static_assert(VK_ACCESS_TRANSFER_WRITE_BIT == VK_ACCESS_2_TRANSFER_WRITE_BIT);
static_assert(VK_PIPELINE_STAGE_NONE == VK_PIPELINE_STAGE_2_NONE);
static_assert(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT == VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
static_assert(VK_PIPELINE_STAGE_TRANSFER_BIT == VK_PIPELINE_STAGE_2_TRANSFER_BIT);

namespace luna::helpers
{
class VkResultException final: public std::exception
{
    public:
        explicit VkResultException(const VkResult result): result(result) {}
        VkResult result;
};

void pipelineBarrier(VkCommandBuffer commandBuffer, const LunaDependencyInfo &dependencyInfo);
} // namespace luna::helpers

namespace luna
{
struct Swapchain
{
        static constexpr uint32_t FRAMES_IN_FLIGHT = 1;

        VkSurfaceKHR surface{};
        VkSurfaceFormatKHR format{};
        VkExtent2D extent{};
        VkImageUsageFlags imageUsage{};
        VkCompositeAlphaFlagBitsKHR compositeAlpha{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
        VkPresentModeKHR presentMode{};
        bool clipped{};

        std::atomic_bool safeToUse{};
        VkSwapchainKHR swapchain{};
        uint32_t imageCount{};
        uint32_t imageIndex{};
        std::vector<VkImage> images{};
        std::vector<VkImageView> imageViews{};
        std::vector<Semaphore> renderSemaphores{};
        uint32_t frameIndex{};
        std::array<Semaphore, FRAMES_IN_FLIGHT> imageReadySemaphores{};
};
} // namespace luna

#ifdef _MSC_VER
#define SUPRESS_MSVC_WARNING(NUMBER) __pragma(warning(suppress : NUMBER))
#else
#define SUPRESS_MSVC_WARNING(NUMBER)
#endif

// TODO: This really needs more work. It should log a message (or not, based on settings provided by the application).
//  Maybe also non fatal? I'm not really sure what the best way to do this function is.
#define CHECK_RESULT_RETURN(value) \
    if (const VkResult result = value; result != VK_SUCCESS) \
    { \
        return result; \
    } \
    (void)0
#define CHECK_RESULT_THROW(value) \
    if (const VkResult result = value; result != VK_SUCCESS) \
    { \
        throw luna::helpers::VkResultException(result); \
    } \
    (void)0
#define TRY_CATCH_RESULT(expression) \
    try \
    { \
        expression; \
    } catch (const luna::helpers::VkResultException &exception) \
    { \
        return exception.result; \
    } \
    (void)0
