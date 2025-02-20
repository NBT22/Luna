//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNADEVICE_H
#define LUNADEVICE_H

#include <luna/lunaTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

void lunaAddNewDevice(const LunaDeviceCreationInfo *creationInfo);
void lunaAddNewDevice2(const LunaDeviceCreationInfo2 *creationInfo);

#ifdef __cplusplus
}
#endif

#endif //LUNADEVICE_H
