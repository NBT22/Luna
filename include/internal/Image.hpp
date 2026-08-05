//
// Created by NBT22 on 3/11/25.
//

#pragma once

#include <cstdint>
#include <luna/lunaTypes.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"

namespace luna::helpers
{
// TODO: VkImageViewCreateInfo can be extended with structs in the core spec
VkResult createImageView(VkDevice device,
                         VkImage image,
                         VkFormat format,
                         VkImageAspectFlags aspectMask,
                         uint8_t mipmapLevels,
                         VkImageView *imageView);
} // namespace luna::helpers

namespace luna
{
class CommandBuffer;

// TODO: Support for 1D images
class Image
{
    public:
        Image(Device &device,
              CommandBuffer &commandBuffer,
              const LunaImageCreationInfo &creationInfo,
              uint32_t depth,
              uint32_t arrayLayers,
              VkImageViewType viewType);

        constexpr bool operator==(const Image &other) const;

        void destroy(VkDevice device, const VmaAllocator &allocator);

        [[nodiscard]] VkResult write(Device &device,
                                     CommandBuffer &commandBuffer,
                                     const LunaImageWriteInfo &writeInfo) const;
        void updateDescriptorBinding(VkDevice device,
                                     LunaDescriptorSet descriptorSet,
                                     const char *descriptorLayoutBindingName,
                                     uint32_t descriptorArrayElement) const;

        [[nodiscard]] VkImage image() const;
        [[nodiscard]] VkImageView imageView() const;
        [[nodiscard]] VkImageLayout layout() const;
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

#pragma region Implementation

#include <cassert>
#include <volk.h>
#include "DescriptorSetLayout.hpp"
#include "Luna.hpp"

namespace luna::helpers
{
inline VkResult createImageView(const VkDevice device,
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
    CHECK_RESULT_RETURN(vkCreateImageView(device, &createInfo, nullptr, imageView));
    return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna
{
constexpr bool Image::operator==(const Image &other) const
{
    return image_ == other.image_ &&
           imageView_ == other.imageView_ &&
           allocation_ == other.allocation_ &&
           extent_.width == other.extent_.width &&
           extent_.height == other.extent_.height &&
           extent_.depth == other.extent_.depth &&
           arrayLayers_ == other.arrayLayers_ &&
           aspectMask_ == other.aspectMask_ &&
           layout_ == other.layout_ &&
           sampler_ == other.sampler_;
}

inline void Image::destroy(const VkDevice device, const VmaAllocator &allocator)
{
    vkDestroyImageView(device, imageView_, nullptr);
    vmaDestroyImage(allocator, image_, allocation_);
    imageView_ = VK_NULL_HANDLE;
    image_ = VK_NULL_HANDLE;
}

inline void Image::updateDescriptorBinding(const VkDevice device,
                                           const LunaDescriptorSet descriptorSet,
                                           const char *descriptorLayoutBindingName,
                                           const uint32_t descriptorArrayElement) const
{
    assert(descriptorSet);
    assert(descriptorLayoutBindingName);
    const VkDescriptorImageInfo imageInfo = {
        .sampler = sampler_,
        .imageView = imageView_,
        .imageLayout = layout_,
    };
    const DescriptorSetIndex *descriptorSetIndex = helpers::fromHandle<DescriptorSetIndex>(descriptorSet);
    const char *bindingName = descriptorLayoutBindingName;
    const DescriptorSetLayout::Binding &binding = descriptorSetIndex->layout->binding(bindingName);
    const VkWriteDescriptorSet writeDescriptor = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = *descriptorSetIndex->set,
        .dstBinding = binding.index,
        .dstArrayElement = descriptorArrayElement,
        .descriptorCount = 1,
        .descriptorType = binding.type,
        .pImageInfo = &imageInfo,
    };
    vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, nullptr);
}

inline VkImage Image::image() const
{
    return image_;
}
inline VkImageView Image::imageView() const
{
    return imageView_;
}
inline VkImageLayout Image::layout() const
{
    return layout_;
}
inline VkSampler Image::sampler() const
{
    return sampler_;
}
} // namespace luna

#pragma endregion Implementation
