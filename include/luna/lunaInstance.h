//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNAINSTANCE_H
#define LUNAINSTANCE_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

LunaInstance lunaCreateInstance(const LunaApplicationInfo &applicationInfo,
								const LunaInstanceExtensionInfo &extensionInfo,
								const LunaInstanceLayerInfo &layerInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNAINSTANCE_H
