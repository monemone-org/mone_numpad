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
#include "IMMVolumeControl.h"
#include <endpointvolume.h>

#pragma warning( disable : 4290 )

std::wstring CreateGUIDString();

class CMMDevice;
class CMMSession;

class CMMDevice: 
    public IMMVolumeControl
{
protected:
    CComPtr<IMMDevice> m_spDevice;
    CComPtr<IAudioSessionManager2> m_spSessionMgr;
    CComPtr<IAudioSessionNotification> m_spSessionNotif;    
    CComPtr<IAudioEndpointVolume> m_spEndpointVolume;

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
            this->dump();
        }
        catch (HRESULT)
        {
        }
    }

    void RefreshSessions() throw () {
        try
        {
            this->FetchSessions();
            this->dump();
        }
        catch (HRESULT)
        {
        }
    }

    void RefreshProperties() throw () {
        try
        {
            this->FetchProperties();
        }
        catch (HRESULT)
        {
        }
    }

    CMMSession* FindSessionByID(const MMSessionID& sessionID) const;

    // --- interface IMMVolumeControl
    // vol is between 0 .. 1
    float GetVolume() const throw (HRESULT);
    void SetVolume(float vol) throw (HRESULT);
    bool IsMute() const throw (HRESULT);
    void SetMute(bool mute) throw (HRESULT);
    void toggleMute() throw (HRESULT);
    virtual LPCWSTR GetVolControlID() const {
        return m_ID.ID.c_str();
    }

    void dump() const;

protected:
    CMMDevice();

    void Uninitialize();
    void Initialize(IMMDevice* pDevice) throw (HRESULT);
    
    void FetchProperties() throw (HRESULT);
    void FetchSessions() throw (HRESULT);
    void ClearSessions();
};

