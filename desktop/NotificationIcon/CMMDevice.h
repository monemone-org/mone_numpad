#pragma once

#include "framework.h"
#include <Mmdeviceapi.h>
#include <audiopolicy.h>
#include <Functiondiscoverykeys_devpkey.h.>
#include <vector>
#include <list>
#include <algorithm>
#include "TMMNotificationClient.h"

#pragma warning( disable : 4290 )


class CMMDevice;
class CMMSession;


class CMMSession
{
private:
    //disable copy constructor
    CMMSession(const CMMSession&);
    CMMSession& operator=(const CMMSession&) { return *this; }

protected:
    CMMDevice* m_pParentDevice;

    CComPtr<IAudioSessionControl> m_spSessionControl;
    CComPtr<IAudioSessionEvents> m_spSessionEvents;

    std::wstring m_sDisplayName;
    AudioSessionState m_state;
    std::wstring m_sProcessName;

public:
    static CMMSession* CreateObject(CMMDevice* pParentDevice, IAudioSessionControl* pSessionControl) throw (HRESULT);

    ~CMMSession();

    bool IsActive() const;
    LPCWSTR GetDisplayName() const;

    void dump() const;

    void Refresh() throw () {
        try
        {
            this->FetchProperties();
        }
        catch (HRESULT)
        {
        }
    }

protected:
    CMMSession();

    void Uninitialize();
    void Initialize(CMMDevice* pParentDevice, IAudioSessionControl* pSession) throw (HRESULT);

    void FetchProperties() throw (HRESULT);
};


class CMMDevice
{
protected:
    CComPtr<IMMDevice> m_spDevice;
    CComPtr<IAudioSessionManager2> m_spSessionMgr;
    CComPtr<IAudioSessionNotification> m_spSessionNotif;

    std::wstring m_sID;
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

    LPCWSTR GetID() const {
        return m_sID.c_str();
    }

    bool HasDeviceID(LPCWSTR pszDeviceID) const 
    {
        if (pszDeviceID == NULL)
            return false;
        if (GetID() == NULL)
            return false;

        return (wcscmp(pszDeviceID, this->GetID()) == 0);
    }

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


    void dump() const;

protected:
    CMMDevice();

    void Uninitialize();
    void Initialize(IMMDevice* pDevice) throw (HRESULT);
    
    void FetchProperties() throw (HRESULT);
    void FetchSessions() throw (HRESULT);
};

