//
// Created by NBT22 on 11/1/26.
//

#pragma once

#include <cstdint>
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>
#include <vulkan/vulkan_core.h>

namespace luna::helpers
{
namespace detail
{
    template<typename T> static inline constexpr uint32_t FEATURE_COUNT = 0;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceVulkan11Features> = 12;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceVulkan12Features> = 47;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceVulkan13Features> = 15;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceVulkan14Features> = 21;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceRayQueryFeaturesKHR> = 1;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceRayTracingPipelineFeaturesKHR> = 5;
    template<> inline constexpr uint32_t FEATURE_COUNT<VkPhysicalDeviceAccelerationStructureFeaturesKHR> = 5;

    template<typename T> requires(FEATURE_COUNT<T> != 0) static inline VkBool32 *boolPtr(T *ptr)
    {
        return ptr == nullptr ? nullptr
                              : reinterpret_cast<VkBool32 *>(reinterpret_cast<char *>(ptr) + sizeof(VkBaseInStructure));
    }
    template<typename T> requires(FEATURE_COUNT<T> != 0) static inline const VkBool32 *boolPtr(const T *ptr)
    {
        return ptr == nullptr ? nullptr
                              : reinterpret_cast<const VkBool32 *>(reinterpret_cast<const char *>(ptr) +
                                                                   sizeof(VkBaseInStructure));
    }

    bool checkFeatureSupport(const VkBool32 *requiredFeatures, const VkBool32 *supportedFeatures, const uint32_t count)
    {
        if (requiredFeatures == nullptr)
        {
            return true;
        }
        if (supportedFeatures == nullptr)
        {
            return false;
        }
        for (uint32_t i = 0; i < count; i++)
        {
            if (requiredFeatures[i] != VK_FALSE && supportedFeatures[i] == VK_FALSE)
            {
                return false;
            }
        }
        return true;
    }

    template<typename T>
    bool checkFeatureSupport(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                             const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures) = delete;
    template<>
    bool checkFeatureSupport<VkPhysicalDeviceFeatures>(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                                                       const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures)
    {
        return checkFeatureSupport(reinterpret_cast<const VkBool32 *>(&requiredFeatures.features),
                                   reinterpret_cast<const VkBool32 *>(&supportedFeatures.features),
                                   sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    }
    template<typename T> requires(detail::FEATURE_COUNT<T> != 0)
    bool checkFeatureSupport(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                             const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures)
    {
        return checkFeatureSupport(detail::boolPtr(vku::FindStructInPNextChain<T>(requiredFeatures.ptr())),
                                   detail::boolPtr(vku::FindStructInPNextChain<T>(supportedFeatures.ptr())),
                                   detail::FEATURE_COUNT<T>);
    }

    void getSupportedOptionalFeatures(const VkBool32 *optionalFeatures,
                                      const VkBool32 *supportedFeatures,
                                      VkBool32 *supportedOptionalFeatures,
                                      const uint32_t count)
    {
        if (optionalFeatures == nullptr)
        {
            return;
        }
        for (uint32_t i = 0; i < count; i++)
        {
            if (supportedFeatures == nullptr)
            {
                supportedOptionalFeatures[i] = 0;
            } else
            {
                assert(supportedOptionalFeatures != nullptr); // Internal state check.
                supportedOptionalFeatures[i] = optionalFeatures[i] != 0 && supportedFeatures[i] != 0 ? VK_TRUE
                                                                                                     : VK_FALSE;
            }
        }
    }

    template<typename T>
    void getSupportedOptionalFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &optionalFeatures,
                                      const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures,
                                      vku::safe_VkPhysicalDeviceFeatures2 &supportedOptionalFeatures) = delete;
    template<> void getSupportedOptionalFeatures<
            VkPhysicalDeviceFeatures>(const vku::safe_VkPhysicalDeviceFeatures2 &optionalFeatures,
                                      const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures,
                                      vku::safe_VkPhysicalDeviceFeatures2 &supportedOptionalFeatures)
    {
        getSupportedOptionalFeatures(reinterpret_cast<const VkBool32 *>(&optionalFeatures.features),
                                     reinterpret_cast<const VkBool32 *>(&supportedFeatures.features),
                                     reinterpret_cast<VkBool32 *>(&supportedOptionalFeatures.features),
                                     sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    }
    template<typename T> requires(FEATURE_COUNT<T> != 0)
    void getSupportedOptionalFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &optionalFeatures,
                                      const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures,
                                      vku::safe_VkPhysicalDeviceFeatures2 &supportedOptionalFeatures)
    {
        getSupportedOptionalFeatures(detail::boolPtr(vku::FindStructInPNextChain<T>(optionalFeatures.ptr())),
                                     detail::boolPtr(vku::FindStructInPNextChain<T>(supportedFeatures.ptr())),
                                     detail::boolPtr(vku::FindStructInPNextChain<T>(supportedOptionalFeatures.ptr())),
                                     detail::FEATURE_COUNT<T>);
    }
} // namespace detail

template<typename... T>
static inline bool checkFeatureSupport(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                                       const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures)
{
    return (detail::checkFeatureSupport<T>(requiredFeatures, supportedFeatures) && ...);
}
static inline bool checkFeatureSupport(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                                       const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures)
{
    return checkFeatureSupport<VkPhysicalDeviceFeatures,
                               VkPhysicalDeviceVulkan11Features,
                               VkPhysicalDeviceVulkan12Features,
                               VkPhysicalDeviceVulkan13Features,
                               VkPhysicalDeviceVulkan14Features,
                               VkPhysicalDeviceRayQueryFeaturesKHR,
                               VkPhysicalDeviceRayTracingPipelineFeaturesKHR,
                               VkPhysicalDeviceAccelerationStructureFeaturesKHR>(requiredFeatures, supportedFeatures);
}

template<typename... T>
static inline vku::safe_VkPhysicalDeviceFeatures2 getSupportedOptionalFeatures(const vku::safe_VkPhysicalDeviceFeatures2
                                                                                       &optionalFeatures,
                                                                               const vku::safe_VkPhysicalDeviceFeatures2
                                                                                       &supportedFeatures)
{
    vku::safe_VkPhysicalDeviceFeatures2 supportedOptionalFeatures{optionalFeatures};
    (detail::getSupportedOptionalFeatures<T>(optionalFeatures, supportedFeatures, supportedOptionalFeatures), ...);
    return supportedOptionalFeatures;
}
static inline vku::safe_VkPhysicalDeviceFeatures2 getSupportedOptionalFeatures(const vku::safe_VkPhysicalDeviceFeatures2
                                                                                       &optionalFeatures,
                                                                               const vku::safe_VkPhysicalDeviceFeatures2
                                                                                       &supportedFeatures)
{
    return getSupportedOptionalFeatures<VkPhysicalDeviceFeatures,
                                        VkPhysicalDeviceVulkan11Features,
                                        VkPhysicalDeviceVulkan12Features,
                                        VkPhysicalDeviceVulkan13Features,
                                        VkPhysicalDeviceVulkan14Features,
                                        VkPhysicalDeviceRayQueryFeaturesKHR,
                                        VkPhysicalDeviceRayTracingPipelineFeaturesKHR,
                                        VkPhysicalDeviceAccelerationStructureFeaturesKHR>(optionalFeatures,
                                                                                          supportedFeatures);
}

// template<typename... T>
// static inline vku::safe_VkPhysicalDeviceFeatures2 combineFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &a,
//                                                                   const vku::safe_VkPhysicalDeviceFeatures2 &b)
// {}
// static inline vku::safe_VkPhysicalDeviceFeatures2 combineFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &a,
//                                                                   const vku::safe_VkPhysicalDeviceFeatures2 &b)
// {
//     return combineFeatures<VkPhysicalDeviceFeatures,
//                            VkPhysicalDeviceVulkan11Features,
//                            VkPhysicalDeviceVulkan12Features,
//                            VkPhysicalDeviceVulkan13Features,
//                            VkPhysicalDeviceVulkan14Features,
//                            VkPhysicalDeviceRayQueryFeaturesKHR,
//                            VkPhysicalDeviceRayTracingPipelineFeaturesKHR,
//                            VkPhysicalDeviceAccelerationStructureFeaturesKHR>(a, b);
// }
} // namespace luna::helpers
