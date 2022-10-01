#pragma once

#include "framework.h"

class CMMDeviceController;

void PostMainThreadRefreshSession(CMMSession* pSession);
void PostMainThreadRefreshDevice(CMMDevice* pDevice);
void PostMainThreadRefreshAudioController(CMMDeviceController* pDeviceController);


