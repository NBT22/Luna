//
// Created by NBT22 on 11/1/26.
//

#pragma once

#include <cstdint>
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_safe_struct_utils.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>
#include <vulkan/vulkan_core.h>

namespace vku
{
template<typename Base> void AddToPnext(Base &base, void *pNext)
{
    assert(base.ptr()); // All safe struct have a ptr() method. Prevent use with non-safe structs.
    VkBaseOutStructure **prev = reinterpret_cast<VkBaseOutStructure **>(&base.pNext);
    for (; *prev != nullptr; prev = reinterpret_cast<VkBaseOutStructure **>(&(*prev)->pNext))
    {}
    *prev = reinterpret_cast<VkBaseOutStructure *>(SafePnextCopy(pNext));
}
} // namespace vku

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
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
                return 20;
            default:
                // TODO: Somehow warn about unsupported struct usage
                assert(false && "Use of unsupported feature struct!");
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

    static inline void addCombinedFeaturesToPNext(const VkBaseInStructure *a,
                                                  vku::safe_VkPhysicalDeviceFeatures2 &b,
                                                  vku::safe_VkPhysicalDeviceFeatures2 &combined)
    {
        assert(!vku::RemoveFromPnext(combined, a->sType)); // Internal state check
        std::vector<char> newStructData(sizeof(VkBaseInStructure) + featureCount(a->sType) * sizeof(VkBool32));
        VkBaseOutStructure *newStructBase = reinterpret_cast<VkBaseOutStructure *>(newStructData.data());
        newStructBase->sType = a->sType;
        newStructBase->pNext = nullptr;
        VkBool32 *newStructFeatures = reinterpret_cast<VkBool32 *>(newStructBase + 1);
        for (const VkBaseInStructure *bPtr = static_cast<const VkBaseInStructure *>(b.pNext); bPtr != nullptr;
             bPtr = static_cast<const VkBaseInStructure *>(bPtr->pNext))
        {
            if (bPtr->sType == a->sType)
            {
                const VkBool32 *aFeatures = reinterpret_cast<const VkBool32 *>(a + 1);
                const VkBool32 *bFeatures = reinterpret_cast<const VkBool32 *>(bPtr + 1);
                for (uint32_t i = 0; i < featureCount(a->sType); i++)
                {
                    newStructFeatures[i] = (aFeatures[i] != VK_FALSE || bFeatures[i] != VK_FALSE) ? VK_TRUE : VK_FALSE;
                }

                vku::RemoveFromPnext(b, a->sType);
                vku::AddToPnext(combined, reinterpret_cast<void *>(newStructBase));
                return;
            }
        }
        assert(!vku::RemoveFromPnext(b, a->sType)); // Internal state check
        std::copy_n(reinterpret_cast<const VkBool32 *>(a + 1), featureCount(a->sType), newStructFeatures);
        vku::AddToPnext(combined, reinterpret_cast<void *>(newStructBase));
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

static inline vku::safe_VkPhysicalDeviceFeatures2 getSupportedOptionalFeatures(const vku::safe_VkPhysicalDeviceFeatures2
                                                                                       &optionalFeatures,
                                                                               const vku::safe_VkPhysicalDeviceFeatures2
                                                                                       &supportedFeatures)
{
    vku::safe_VkPhysicalDeviceFeatures2 supportedOptionalFeatures{supportedFeatures};
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
    return supportedOptionalFeatures;
}

static inline bool getSupportedFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &supportedFeatures,
                                        VkPhysicalDeviceFeatures2 &features)
{
    bool allSupported = detail::checkFeatureSupport(reinterpret_cast<const VkBool32 *>(&features.features),
                                                    reinterpret_cast<const VkBool32 *>(&supportedFeatures.features),
                                                    sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    detail::getSupportedOptionalFeatures(reinterpret_cast<const VkBool32 *>(&features.features),
                                         reinterpret_cast<const VkBool32 *>(&supportedFeatures.features),
                                         reinterpret_cast<VkBool32 *>(&features.features),
                                         sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    for (VkBaseOutStructure *out = static_cast<VkBaseOutStructure *>(features.pNext); out != nullptr;
         out = static_cast<VkBaseOutStructure *>(out->pNext))
    {
        VkBool32 *outFeatureArray = reinterpret_cast<VkBool32 *>(out + 1);
        const VkBaseInStructure *supported = static_cast<const VkBaseInStructure *>(supportedFeatures.pNext);
        while (supported != nullptr && supported->sType != out->sType)
        {
            supported = static_cast<const VkBaseInStructure *>(supported->pNext);
        }
        if (supported == nullptr)
        {
            allSupported = false;
        }
        const VkBool32 *supportedFeatureArray = reinterpret_cast<const VkBool32 *>(supported + 1);
        for (uint32_t i = 0; i < detail::featureCount(out->sType); i++)
        {
            if (supported == nullptr)
            {
                outFeatureArray[i] = VK_FALSE;
                continue;
            }
            if (allSupported && outFeatureArray[i] != VK_FALSE && supportedFeatureArray[i] == VK_FALSE)
            {
                allSupported = false;
            }
            outFeatureArray[i] = supportedFeatureArray[i] != VK_FALSE ? VK_TRUE : VK_FALSE;
        }
    }
    return allSupported;
}

static inline vku::safe_VkPhysicalDeviceFeatures2 combineFeatures(const vku::safe_VkPhysicalDeviceFeatures2 &a,
                                                                  vku::safe_VkPhysicalDeviceFeatures2 &b)
{
    vku::safe_VkPhysicalDeviceFeatures2 combined;
    const VkBool32 *aFeatures = reinterpret_cast<const VkBool32 *>(&a.features);
    const VkBool32 *bFeatures = reinterpret_cast<const VkBool32 *>(&b.features);
    VkBool32 *combinedFeatures = reinterpret_cast<VkBool32 *>(&combined.features);
    for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++)
    {
        combinedFeatures[i] = (aFeatures[i] != VK_FALSE || bFeatures[i] != VK_FALSE) ? VK_TRUE : VK_FALSE;
    }
    for (const VkBaseInStructure *aBase = static_cast<const VkBaseInStructure *>(a.pNext); aBase != nullptr;
         aBase = static_cast<const VkBaseInStructure *>(aBase->pNext))
    {
        detail::addCombinedFeaturesToPNext(aBase, b, combined);
    }
    vku::AddToPnext(combined, b);
    vku::RemoveFromPnext(combined, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
    return combined;
}
} // namespace luna::helpers
