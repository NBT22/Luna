//
// Created by NBT22 on 3/1/25.
//

#include <luna/helpers/Luna.hpp>
#include <luna/luna.h>
#include "luna/core/Instance.hpp"

namespace luna::helpers
{}

VkShaderModule lunaCreateShaderModule(const uint32_t *spirv, const size_t bytes)
{
	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = bytes,
		.pCode = spirv,
	};

	VkShaderModule shaderModule;
	vkCreateShaderModule(luna::core::instance.device().logicalDevice(), &createInfo, nullptr, &shaderModule);
	return shaderModule;
}
