#pragma once

#include "framework.h"
#include <Mmdeviceapi.h>
#include <audiopolicy.h>
#include <Functiondiscoverykeys_devpkey.h.>
#include <vector>
#include <list>
#include <algorithm>
#include "TMMNotificationClient.h"
#include "MMDeviceID.h"
#include "CMMSession.h"

#pragma warning( disable : 4290 )

std::wstring CreateGUIDString();

class CMMDevice;
class CMMSession;

class CMMDevice
{
protected:
    CComPtr<IMMDevice> m_spDevice;
    CComPtr<IAudioSessionManager2> m_spSessionMgr;
    CComPtr<IAudioSessionNotification> m_spSessionNotif;

    MMDeviceID m_ID;
    std::wstring m_sDisplayName;
    bool m_bIsInput;
    std::list<CMMSession*> m_sessions;

private:
    //disable copy constructor
    CMMDevice(const CMMDevice&);
    CMMDevice& operator=(const CMMDevice&) { return *this; }

public:
    static CMMDevice* CreateObject(IMMDevice* pMMDevice) throw (HRESULT);
    ~CMMDevice();

    LPCWSTR GetDisplayName() const {
        return m_sDisplayName.c_str();
    }

    const MMDeviceID& GetID() const {
        return m_ID;
    }

    bool IsInput() const throw(HRESULT);

    const std::list<CMMSession*>& GetSessions() const {
        return m_sessions;
    }

    void Refresh() throw () {
        try
        {
            this->FetchProperties();
            this->FetchSessions();
        }
        catch (HRESULT)
        {
        }
    }

    CMMSession* FindSessionByID(const MMSessionID& sessionID) const;

    void dump() const;

protected:
    CMMDevice();

    void Uninitialize();
    void Initialize(IMMDevice* pDevice) throw (HRESULT);
    
    void FetchProperties() throw (HRESULT);
    void FetchSessions() throw (HRESULT);
    void ClearSessions();
};

