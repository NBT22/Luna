//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/lunaTypes.h>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace luna
{
class GraphicsPipeline
{
    public:
        static bool isDestroyed(const GraphicsPipeline &graphicsPipeline);

        GraphicsPipeline() = default;
        explicit GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);
        explicit GraphicsPipeline(const LunaGraphicsPipelineUsingReflectionCreationInfo &creationInfo);

        void destroy();

        [[nodiscard]] VkResult pushConstants() const;
        [[nodiscard]] VkResult bind(const LunaGraphicsPipelineBindInfo &bindInfo) const;

        [[nodiscard]] VkPipelineLayout layout() const;

    private:
        bool isDestroyed_{true};
        VkPipeline pipeline_{};
        VkPipelineLayout layout_{};
        std::vector<LunaPushConstantsRange> pushConstantsRanges_{};
        std::unordered_map<std::string, uint32_t> namedBindingsMap_{};
        uint32_t inputRateVertexBindingIndex_{-1u};
        uint32_t inputRateInstanceBindingIndex_{-1u};
};
} // namespace luna

#pragma region Implementation

namespace luna
{
inline bool GraphicsPipeline::isDestroyed(const GraphicsPipeline &graphicsPipeline)
{
    return graphicsPipeline.isDestroyed_;
}

inline VkPipelineLayout GraphicsPipeline::layout() const
{
    return layout_;
}
} // namespace luna

#pragma endregion Implementation
