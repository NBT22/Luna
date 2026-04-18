//
// Created by NBT22 on 10/18/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace luna
{
class ComputePipeline
{
    public:
        ComputePipeline() = default;
        explicit ComputePipeline(VkDevice device, const LunaComputePipelineCreationInfo &creationInfo);

        void destroy(VkDevice device);

        [[nodiscard]] VkResult bind(VkCommandBuffer commandBuffer,
                                    const LunaDescriptorSetBindInfo &descriptorSetBindInfo) const;

        [[nodiscard]] VkPipelineLayout layout() const;

    private:
        VkPipeline pipeline_{};
        VkPipelineLayout layout_{};
        std::vector<LunaPushConstantsRange> pushConstantsRanges_{};
};
} // namespace luna

#pragma region Implementation

namespace luna
{
inline VkPipelineLayout ComputePipeline::layout() const
{
    return layout_;
}
} // namespace luna

#pragma endregion Implementation
