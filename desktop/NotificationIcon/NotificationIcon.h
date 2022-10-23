#pragma once

#include "framework.h"
#include <functional>
#include <string>

//#include "CMMDevice.h"
//#include "CMMDeviceController.h"

struct MMSessionID;
struct MMDeviceControllerID;

void PostMainThreadRefreshSession(MMSessionID sessionID, LPCWSTR pszReason) throw ();
void PostMainThreadRefreshDeviceSessions(MMDeviceID deviceID, LPCWSTR pszReason) throw ();
void PostMainThreadRefreshDeviceProperties(MMDeviceID deviceID, LPCWSTR pszReason) throw ();
void PostMainThreadRefreshAudioController(LPCWSTR pszReason) throw ();

//return true if handled message
typedef struct WMAPPMessageHandler
{
	DWORD_PTR id;
	std::function<bool(UINT, WPARAM, LPARAM)> handler;

	friend bool operator<(const WMAPPMessageHandler& l, const WMAPPMessageHandler& r)
	{
		return l.id
			< r.id;
	}

	friend bool operator==(const WMAPPMessageHandler& l, const WMAPPMessageHandler& r)
	{
		return l.id
			== r.id;
	}

} WMAPPMessageHandler;

void AddWMAPPMessageHander(const WMAPPMessageHandler& handler);
void RemoveWMAPPMessageHanderByID(DWORD_PTR handlerID);

HWND GetAppHWND();