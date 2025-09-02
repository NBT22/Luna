//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/lunaDrawing.h>
#include <luna/lunaTypes.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace luna
{
class GraphicsPipeline
{
    public:
        static bool isDestroyed(const GraphicsPipeline &graphicsPipeline);

        friend VkResult(::lunaPushConstants(LunaGraphicsPipeline pipeline));

        GraphicsPipeline() = default;
        explicit GraphicsPipeline(const LunaGraphicsPipelineCreationInfo &creationInfo);

        void destroy();

        [[nodiscard]] VkResult bind(const LunaGraphicsPipelineBindInfo &bindInfo) const;

        [[nodiscard]] VkPipelineLayout layout() const;

    private:
        bool isDestroyed_{true};
        VkPipeline pipeline_{};
        VkPipelineLayout layout_{};
        std::vector<LunaPushConstantsRange> pushConstantsRanges_{};
};
} // namespace luna

#pragma region "Implmentation"

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

#pragma endregion "Implmentation"
