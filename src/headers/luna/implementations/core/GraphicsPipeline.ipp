//
// Created by NBT22 on 3/18/25.
//

#pragma once

namespace luna::core
{
inline bool GraphicsPipeline::isDestroyed(const GraphicsPipeline &graphicsPipeline)
{
	return graphicsPipeline.isDestroyed_;
}

inline void GraphicsPipeline::unbind()
{
	bound_ = false;
}

} // namespace luna::core
