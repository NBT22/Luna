//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <luna/core/Instance.hpp>

namespace luna::core
{
inline VkRenderPass RenderPass::renderPass() const
{
	return renderPass_;
}
inline const RenderPassSubpassIndex *RenderPass::getFirstSubpass() const
{
	assert(subpassIndices_.size() > 0);
	return &subpassIndices_.front();
}
inline const RenderPassSubpassIndex *RenderPass::getSubpassIndexByName(const std::string &name) const
{
	assert(subpassMap_.size() > 0);
	try
	{
		return &subpassIndices_.at(subpassMap_.at(name));
	} catch (const std::out_of_range &)
	{
		return &subpassIndices_.at(0);
	}
}
} // namespace luna::core
