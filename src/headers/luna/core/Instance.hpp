//
// Created by NBT22 on 2/13/25.
//

#pragma once
#include <luna/lunaInstance.h>

namespace luna::core
{
extern VkInstance instance;

struct Instance
{};

LunaInstance createInstance(const LunaApplicationInfo &applicationInfo,
							const LunaInstanceExtensionInfo &extensionInfo,
							const LunaInstanceLayerInfo &layerInfo);
} // namespace luna::core

struct LunaInstanceStruct : luna::core::Instance
{};
