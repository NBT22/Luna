//
// Created by NBT22 on 2/13/25.
//

#ifndef LUNALOGICALDEVICE_H
#define LUNALOGICALDEVICE_H

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

#endif //LUNALOGICALDEVICE_H
