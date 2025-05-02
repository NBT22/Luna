//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <luna/core/CommandPool.hpp>
#include <luna/luna.h>
#include <vector>
#include <vk_mem_alloc.h>

namespace luna::core
{
template<typename T> struct FamilyValues
{
        T graphics;
        T transfer;
        T presentation;
};

class Device
{
    public:
        friend VkResult(::lunaBeginRenderPass(LunaRenderPass, const LunaRenderPassBeginInfo *beginInfo));

        Device() = default;
        explicit Device(const LunaDeviceCreationInfo2 &creationInfo);

        void destroy();

        VkResult addShaderModule(const VkShaderModuleCreateInfo *creationInfo, VkShaderModule *shaderModule);

        /// A getter for the @c physicalDevice_ value
        /// @return The Vulkan handle for the physical device described in this instance
        [[nodiscard]] VkPhysicalDevice physicalDevice() const;
        /// A getter for the @c logicalDevice_ value
        /// @return The Vulkan handle for the physical device described in this instance
        [[nodiscard]] VkDevice logicalDevice() const;
        [[nodiscard]] VkSharingMode sharingMode() const;
        /// A getter for the @c familyCount_ value
        /// @return The total count of unique families
        [[nodiscard]] uint32_t familyCount() const;
        [[nodiscard]] const uint32_t *queueFamilyIndices() const;
        [[nodiscard]] VmaAllocator allocator() const;
        [[nodiscard]] const FamilyValues<VkQueue> &familyQueues() const;
        [[nodiscard]] FamilyValues<CommandPool> &commandPools();
        [[nodiscard]] const FamilyValues<CommandPool> &commandPools() const;
        [[nodiscard]] const VkSemaphore &imageAvailableSemaphore() const;
        [[nodiscard]] const VkSemaphore &renderFinishedSemaphore() const;

    private:
        VkResult findQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        void initQueueFamilyIndices();
        [[nodiscard]] bool checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const;
        [[nodiscard]] bool checkFeatureSupport(const VkBool32 *requiredFeatures) const;
        [[nodiscard]] bool checkUsability(VkPhysicalDevice device, VkSurfaceKHR surface);
        VkResult createCommandPools();
        VkResult createSemaphores();

        bool isDestroyed_{true};
        VkPhysicalDevice physicalDevice_{};
        VkDevice logicalDevice_{};
        VkPhysicalDeviceVulkan14Features vulkan14Features_{};
        VkPhysicalDeviceVulkan13Features vulkan13Features_{};
        VkPhysicalDeviceVulkan12Features vulkan12Features_{};
        VkPhysicalDeviceVulkan11Features vulkan11Features_{};
        VkPhysicalDeviceFeatures2 features_{};
        VkPhysicalDeviceProperties properties_{};
        VkPhysicalDeviceMemoryProperties memoryProperties_{};
        VmaAllocator allocator_{};
        uint32_t familyCount_{};
        std::vector<uint32_t> queueFamilyIndices_{};
        FamilyValues<bool> hasFamily_{};
        FamilyValues<VkQueue> familyQueues_{};
        FamilyValues<uint32_t> familyIndices_{};
        FamilyValues<CommandPool> commandPools_{};
        VkSemaphore imageAvailableSemaphore_{};
        VkSemaphore renderFinishedSemaphore_{};

        // TODO: Fix pipelines so that shader modules can be created and destroyed in a cleaner fashion
        std::vector<VkShaderModule> shaderModules_{};
};
} // namespace luna::core

#include <luna/implementations/core/Device.ipp>
