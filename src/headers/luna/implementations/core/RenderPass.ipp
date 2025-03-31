//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <luna/core/Instance.hpp>

namespace luna::core
{
inline bool RenderPass::isDestroyed(const RenderPass &renderPass)
{
	return renderPass.isDestroyed_;
}

inline VkRenderPass RenderPass::renderPass() const
{
	return renderPass_;
}
inline const RenderPassSubpassIndex *RenderPass::getFirstSubpass() const
{
	assert(!subpassIndices_.empty());
	return subpassIndices_.data();
}
inline const RenderPassSubpassIndex *RenderPass::getSubpassIndexByName(const std::string &name) const
{
	assert(!subpassMap_.empty());
	try
	{
		return &subpassIndices_.at(subpassMap_.at(name));
	} catch (const std::out_of_range &)
	{
		return &subpassIndices_.at(0);
	}
}
} // namespace luna::core
