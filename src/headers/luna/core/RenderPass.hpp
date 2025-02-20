//
// Created by NBT22 on 2/18/25.
//

#pragma once

#include <luna/helpers/Luna.hpp>
#include <luna/lunaTypes.h>
#include <string>
#include <unordered_map>

namespace luna::core
{
class RenderPass
{
	public:
		RenderPass() = default;
		explicit RenderPass(const LunaRenderPassCreationInfo &creationInfo);

		uint32_t getInfoIndexByName(const char *name) const;

	private:
		void fillMap(const char **names, uint32_t count);

		VkRenderPass renderPass_{};
		std::unordered_map<std::string, uint32_t> infoMap_;
};
} // namespace luna::core

#include <luna/implementations/core/RenderPass.ipp>
