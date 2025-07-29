//
// Created by NBT22 on 2/25/25.
//

#pragma once

#include <luna/luna.h>
#include <vector>

namespace luna::core
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
} // namespace luna::core

#include <luna/implementations/core/GraphicsPipeline.ipp>
