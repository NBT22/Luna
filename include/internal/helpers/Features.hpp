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
    /// Gets the number of VkBool32 feature fields in the given structure type, or zero if it is an unknown type
    static inline uint32_t featureCount(const VkStructureType structureType)
    {
        // NOLINTBEGIN(*-magic-numbers)
        switch (structureType)
        {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
                return 12;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
                return 47;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
                return 15;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
                return 21;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR:
                return 1;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
                return 5;
            default:
                return 0;
        }
        // NOLINTEND(*-magic-numbers)
    }


    static inline bool checkFeatureSupport(const VkBool32 *requiredFeatures,
                                           const VkBool32 *supportedFeatures,
                                           const uint32_t count)
    {
        if (count == 0)
        {
            // TODO: Somehow warn about unsupported struct usage
            assert(count != 0 && "Use of unsupported feature struct!");
            return false;
        }
        assert(requiredFeatures != nullptr && supportedFeatures != nullptr); // Internal state check
        for (uint32_t i = 0; i < count; i++)
        {
            if (requiredFeatures[i] != VK_FALSE && supportedFeatures[i] == VK_FALSE)
            {
                return false;
            }
        }
        return true;
    }

    static inline bool checkFeatureSupport(const VkBaseInStructure *requiredFeatures,
                                           const VkBaseInStructure *supportedFeatures)
    {
        assert(requiredFeatures != nullptr &&
               supportedFeatures != nullptr &&
               requiredFeatures->sType == supportedFeatures->sType); // Internal state check
        return checkFeatureSupport(reinterpret_cast<const VkBool32 *>(requiredFeatures + 1),
                                   reinterpret_cast<const VkBool32 *>(supportedFeatures + 1),
                                   featureCount(requiredFeatures->sType));
    }


    static inline void getSupportedOptionalFeatures(const VkBool32 *optionalFeatures,
                                                    const VkBool32 *supportedFeatures,
                                                    VkBool32 *supportedOptionalFeatures,
                                                    const uint32_t count)
    {
        if (count == 0)
        {
            // TODO: Somehow warn about unsupported struct usage
            assert(count != 0 && "Use of unsupported feature struct!");
            return;
        }

        assert(optionalFeatures != nullptr &&
               supportedFeatures != nullptr &&
               supportedOptionalFeatures != nullptr); // Internal state check

        for (uint32_t i = 0; i < count; i++)
        {
            if (supportedFeatures == nullptr)
            {
                supportedOptionalFeatures[i] = VK_FALSE;
            } else
            {
                supportedOptionalFeatures[i] = (optionalFeatures[i] != VK_FALSE && supportedFeatures[i] != VK_FALSE)
                                                       ? VK_TRUE
                                                       : VK_FALSE;
            }
        }
    }

    static inline void getSupportedOptionalFeatures(const VkBaseInStructure *optionalFeatures,
                                                    const VkBaseInStructure *supportedFeatures,
                                                    VkBaseOutStructure *supportedOptionalFeatures)
    {
        assert(optionalFeatures != nullptr &&
               supportedFeatures != nullptr &&
               supportedOptionalFeatures != nullptr); // Internal state check
        assert(optionalFeatures->sType == supportedFeatures->sType &&
               optionalFeatures->sType == supportedOptionalFeatures->sType); // Internal state check
        getSupportedOptionalFeatures(reinterpret_cast<const VkBool32 *>(optionalFeatures + 1),
                                     reinterpret_cast<const VkBool32 *>(supportedFeatures + 1),
                                     reinterpret_cast<VkBool32 *>(supportedOptionalFeatures + 1),
                                     featureCount(optionalFeatures->sType));
    }
} // namespace detail

static inline bool checkFeatureSupport(const vku::safe_VkPhysicalDeviceFeatures2 &requiredFeatures,
                                       const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures)
{
    bool allSupported = detail::checkFeatureSupport(reinterpret_cast<const VkBool32 *>(&requiredFeatures.features),
                                                    reinterpret_cast<const VkBool32 *>(&supportedFeatures.features),
                                                    sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    const VkBaseInStructure *required = static_cast<const VkBaseInStructure *>(requiredFeatures.pNext);
    const VkBaseInStructure *supported = static_cast<const VkBaseInStructure *>(supportedFeatures.pNext);
    while (allSupported && required != nullptr && supported != nullptr)
    {
        assert(required->sType == supported->sType); // Internal state check
        allSupported &= detail::checkFeatureSupport(required, supported);
        required = static_cast<const VkBaseInStructure *>(required->pNext);
        supported = static_cast<const VkBaseInStructure *>(supported->pNext);
    }
    return allSupported && required == nullptr && supported == nullptr;
}

static inline void getSupportedOptionalFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &optionalFeatures,
                                                const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures,
                                                vku::safe_VkPhysicalDeviceFeatures2 &supportedOptionalFeatures)
{
    detail::getSupportedOptionalFeatures(reinterpret_cast<const VkBool32 *>(&optionalFeatures.features),
                                         reinterpret_cast<const VkBool32 *>(&supportedFeatures.features),
                                         reinterpret_cast<VkBool32 *>(&supportedOptionalFeatures.features),
                                         sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    const VkBaseInStructure *optional = static_cast<const VkBaseInStructure *>(optionalFeatures.pNext);
    const VkBaseInStructure *supported = static_cast<const VkBaseInStructure *>(supportedFeatures.pNext);
    VkBaseOutStructure *supportedOptional = static_cast<VkBaseOutStructure *>(supportedOptionalFeatures.pNext);
    while (optional != nullptr && supported != nullptr && supportedOptional != nullptr)
    {
        assert(optional->sType == supported->sType &&
               optional->sType == supportedOptional->sType); // Internal state check
        detail::getSupportedOptionalFeatures(optional, supported, supportedOptional);
        optional = static_cast<const VkBaseInStructure *>(optional->pNext);
        supported = static_cast<const VkBaseInStructure *>(supported->pNext);
        supportedOptional = static_cast<VkBaseOutStructure *>(supportedOptional->pNext);
    }
    assert(optional == nullptr && supported == nullptr && supportedOptional == nullptr); // Internal state check
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
