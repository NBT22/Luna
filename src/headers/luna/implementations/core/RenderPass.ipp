//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <luna/core/Instance.hpp>

namespace luna::core
{
inline uint32_t RenderPass::getInfoIndexByName(const char *name) const
{
	return infoMap_.at(name);
}

inline void RenderPass::fillMap(const char **names, const uint32_t count)
{
	if (names)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			if (names[i])
			{
				infoMap_[names[i]] = i;
			}
		}
	}
}
} // namespace luna::core
