#pragma once

#include "framework.h"

//#include "CMMDevice.h"
//#include "CMMDeviceController.h"

struct MMDeviceControllerID;

void PostMainThreadRefreshSession(const MMSessionID& sessionID) throw ();
void PostMainThreadRefreshDevice(const MMDeviceID& deviceID) throw ();
void PostMainThreadRefreshAudioController(const MMDeviceControllerID& controllerID) throw ();


