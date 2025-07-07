//
// Created by NBT22 on 2/13/25.
//

#pragma once

#include <list>
#include <luna/core/CommandPool.hpp>
#include <luna/luna.h>
#include <vector>
#include <vk_mem_alloc.h>

namespace luna::core
{
template<typename T> struct FamilyValues
{
        T graphics{};
        T transfer{};
        T presentation{};
};

class Device
{
    public:
        friend VkResult(::lunaBeginRenderPass(LunaRenderPass, const LunaRenderPassBeginInfo *beginInfo));

        Device() = default;
        explicit Device(const LunaDeviceCreationInfo2 &creationInfo);

        operator const VkPhysicalDevice &() const;
        operator const VkDevice &() const;

        void destroy();

        VkResult addShaderModule(const VkShaderModuleCreateInfo *creationInfo, LunaShaderModule *shaderModule);
        VkResult createSemaphores(uint32_t imageCount);
        VkResult addApplicationCommandPool(const LunaCommandPoolCreationInfo &creationInfo,
                                           LunaCommandPool *commandPool);

        [[nodiscard]] VkSharingMode sharingMode() const;
        /// A getter for the @c familyCount_ value
        /// @return The total count of unique families
        [[nodiscard]] uint32_t familyCount() const;
        [[nodiscard]] const uint32_t *queueFamilyIndices() const;
        [[nodiscard]] VmaAllocator allocator() const;
        [[nodiscard]] const FamilyValues<VkQueue> &familyQueues() const;
        [[nodiscard]] FamilyValues<CommandPool> &commandPools();
        [[nodiscard]] const FamilyValues<CommandPool> &commandPools() const;
        [[nodiscard]] Semaphore &renderFinishedSemaphore(uint32_t imageIndex);
        [[nodiscard]] VkShaderModule shaderModule(LunaShaderModule shaderModule) const;

    private:
        VkResult findQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        void initQueueFamilyIndices();
        [[nodiscard]] bool checkFeatureSupport(const VkPhysicalDeviceFeatures2 &requiredFeatures) const;
        [[nodiscard]] bool checkFeatureSupport(const VkBool32 *requiredFeatures) const;
        [[nodiscard]] bool checkUsability(VkPhysicalDevice device, VkSurfaceKHR surface);
        VkResult createCommandPools();

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
        FamilyValues<CommandPool> internalCommandPools_{};
        std::vector<CommandPool> applicationCommandPools_{};
        std::list<uint32_t> applicationCommandPoolIndices_{};
        std::vector<Semaphore> renderFinishedSemaphores_{};

        std::vector<VkShaderModule> shaderModules_{};
        std::list<uint32_t> shaderModuleIndices_{};
};
} // namespace luna::core

#include <luna/implementations/core/Device.ipp>
