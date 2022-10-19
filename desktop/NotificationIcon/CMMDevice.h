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

std::wstring CreateGUIDString();

class CMMDevice;
class CMMSession;


typedef struct MMDeviceID
{
    std::wstring ID;

    MMDeviceID()
    {}

    MMDeviceID(LPCWSTR pszID) :
        ID(pszID)
    {
    }

    MMDeviceID(const MMDeviceID& copy)
    {
        this->ID = copy.ID;
    }

    MMDeviceID& operator=(const MMDeviceID& copy)
    {
        this->ID = copy.ID;
        return *this;
    }

    bool operator==(const MMDeviceID& copy) const
    {
        return this->ID == copy.ID;
    }

    bool operator!=(const MMDeviceID& copy) const
    {
        return !(*this == copy);
    }

    void clear()
    {
        this->ID.clear();
    }

}MMDeviceID;


typedef struct MMSessionID
{
    MMDeviceID deviceID;
    std::wstring ID;

    MMSessionID()
    {}

    MMSessionID(const MMDeviceID& deviceID_, LPCWSTR id_):
        deviceID(deviceID_),
        ID(id_)
    {}

    MMSessionID(const MMSessionID& copy)
    {
        this->ID = copy.ID;
        this->deviceID = copy.deviceID;
    }

    MMSessionID& operator=(const MMSessionID& copy)
    {
        this->ID = copy.ID;
        this->deviceID = copy.deviceID;
        return *this;
    }

    bool operator==(const MMSessionID& copy) const
    {
        return this->ID == copy.ID
            && this->deviceID == copy.deviceID;
    }

    bool operator!=(const MMSessionID& copy) const
    {
        return !(*this == copy);
    }

    void clear()
    {
        this->ID.clear();
        this->deviceID.clear();
    }

}MMSessionID;



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

    MMSessionID m_ID;
    std::wstring m_sDisplayName;
    std::wstring m_sProcessName;
    bool m_bIsSystemInOut;

public:
    static CMMSession* CreateObject(CMMDevice* pParentDevice, IAudioSessionControl* pSessionControl) throw (HRESULT);

    ~CMMSession();

    const MMSessionID& GetID() const {
        return m_ID;
    }

    LPCWSTR GetDisplayName() const
    {
        if (m_sDisplayName.empty())
        {
            return this->m_sProcessName.c_str();
        }
        return m_sDisplayName.c_str();
    }

    bool IsSystemInOut() const {
        return m_bIsSystemInOut;
    }

    bool IsActive() const
    {
        return (GetState() == AudioSessionStateActive);
    }

    AudioSessionState GetState() const throw (HRESULT);

    // vol is between 0 .. 1
    float GetVolume() const throw (HRESULT);
    void SetVolume(float vol) throw (HRESULT);

    bool IsMute() const throw (HRESULT);
    void SetMute(bool mute) throw (HRESULT);
    bool toggleMute() throw (HRESULT);

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

