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
        // static bool isDestroyed(const ComputePipeline &computePipeline);

        ComputePipeline() = default;
        explicit ComputePipeline(const LunaComputePipelineCreationInfo &creationInfo);

        ~ComputePipeline();

        [[nodiscard]] VkResult bind(const LunaDescriptorSetBindInfo &descriptorSetBindInfo) const;

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
