//
// Created by NBT22 on 3/11/25.
//

#pragma once

#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "CommandBuffer.hpp"

namespace luna::helpers
{
// TODO: VkImageViewCreateInfo can be extended with structs in the core spec
VkResult createImageView(VkDevice logicalDevice,
                         VkImage image,
                         VkFormat format,
                         VkImageAspectFlags aspectMask,
                         uint8_t mipmapLevels,
                         VkImageView *imageView);
} // namespace luna::helpers

namespace luna
{
// TODO: Support for 1D images
class Image
{
    public:
        Image(const LunaSampledImageCreationInfo &creationInfo, uint32_t depth, uint32_t arrayLayers);

        void destroy() const;
        void erase(std::list<Image>::const_iterator iterator) const;

        [[nodiscard]] VkResult write(const LunaImageWriteInfo &writeInfo) const;
        void updateDescriptorBinding(VkDevice logicalDevice,
                                     LunaDescriptorSet descriptorSet,
                                     const char *descriptorLayoutBindingName) const;

        [[nodiscard]] VkImageView imageView() const;
        [[nodiscard]] VkSampler sampler() const;
        [[nodiscard]] VkSampler sampler(LunaSampler sampler) const;

    private:
        void generateMipmaps_(const CommandBuffer &commandBuffer,
                              VkOffset3D extent,
                              uint32_t mipmapLevels,
                              const LunaImageWriteInfo &writeInfo) const;

        VkImage image_{};
        VkImageView imageView_{};
        VmaAllocation allocation_{};
        VkExtent3D extent_{};
        uint32_t arrayLayers_{};
        VkImageAspectFlags aspectMask_{};
        VkImageLayout layout_{};
        VkSampler sampler_{};
};
} // namespace luna

#pragma region "Implmentation"

#include <cassert>
#include <volk.h>
#include "DescriptorSetLayout.hpp"
#include "Luna.hpp"

namespace luna::helpers
{
inline VkResult createImageView(const VkDevice logicalDevice,
                                const VkImage image,
                                const VkFormat format,
                                const VkImageAspectFlags aspectMask,
                                const uint8_t mipmapLevels,
                                VkImageView *imageView)
{
    constexpr VkComponentMapping componentMapping = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    const VkImageSubresourceRange subresourceRange = {
        .aspectMask = aspectMask,
        .levelCount = mipmapLevels,
        .layerCount = 1,
    };
    const VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = componentMapping,
        .subresourceRange = subresourceRange,
    };
    CHECK_RESULT_RETURN(vkCreateImageView(logicalDevice, &createInfo, nullptr, imageView));
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna
{
inline void Image::updateDescriptorBinding(const VkDevice logicalDevice,
                                           const LunaDescriptorSet descriptorSet,
                                           const char *descriptorLayoutBindingName) const
{
    assert(descriptorSet);
    assert(descriptorLayoutBindingName);
    const VkDescriptorImageInfo imageInfo = {
        .sampler = sampler_,
        .imageView = imageView_,
        .imageLayout = layout_,
    };
    const DescriptorSetIndex *descriptorSetIndex = static_cast<const DescriptorSetIndex *>(descriptorSet);
    const char *bindingName = descriptorLayoutBindingName;
    const DescriptorSetLayout::Binding &binding = descriptorSetIndex->layout->binding(bindingName);
    const VkWriteDescriptorSet writeDescriptor = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = *descriptorSetIndex->set,
        .dstBinding = binding.index,
        .descriptorCount = 1,
        .descriptorType = binding.type,
        .pImageInfo = &imageInfo,
    };
    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptor, 0, nullptr);
}

inline VkImageView Image::imageView() const
{
    return imageView_;
}
inline VkSampler Image::sampler() const
{
    return sampler_;
}
} // namespace luna

#pragma endregion "Implmentation"
