#pragma once

#include "CMMDevice.h"
#include "NotificationIcon.h"


class CMMDeviceControllerListener
{
public:
    virtual void OnMMSessionsRefreshed() = 0;
};


typedef struct MMDeviceControllerID
{
    std::wstring ID;

    MMDeviceControllerID() {}

    MMDeviceControllerID(LPCWSTR pszID) :
        ID(pszID)
    {
    }

    MMDeviceControllerID(const MMDeviceControllerID& copy) :
        ID(copy.ID)
    {
    }

    MMDeviceControllerID& operator=(const MMDeviceControllerID& copy)
    {
        ID = copy.ID;
        return *this;
    }

    bool operator==(const MMDeviceControllerID& copy) const
    {
        return this->ID == copy.ID;
    }

    bool operator!=(const MMDeviceControllerID& copy) const
    {
        return !(*this == copy);
    }

    void clear()
    {
        this->ID.clear();
    }

} MMDeviceControllerID;

class CMMDeviceController
{
protected:
    MMDeviceControllerID m_ID;

    CComPtr<IMMDeviceEnumerator> m_spDeviceEnumerator;
    CComPtr<IMMNotificationClient> m_spClientNotif;

    CMMDevice* m_pDefaultOut;
    CMMDevice* m_pDefaultIn;

    // for thread-safe operations
    // CMMNotificationClient is notified in a background thread.
    // but we should only access CMMDeviceController and its data in
    // the main UI thread.
    CComCriticalSection m_cs;

    std::list< CMMDeviceControllerListener*> m_listeners;

public:
    static const EDataFlow DEFAULT_OUT_DATAFLOW = eRender;
    static const ERole DEFAULT_OUT_ROLE = eConsole;

    static const EDataFlow DEFAULT_IN_DATAFLOW = eCapture;
    static const ERole DEFAULT_IN_ROLE = eCommunications;

private:
    CMMDeviceController(const CMMDeviceController&) :
        m_pDefaultOut(NULL),
        m_pDefaultIn(NULL)
    {
        ATLASSERT(false);
    };
    CMMDeviceController& operator=(const CMMDeviceController&) {
        ATLASSERT(false);
        return *this;
    }

public:
    static CMMDeviceController* CreateObject() throw (HRESULT)
    {
        CMMDeviceController* pController = new CMMDeviceController();
        if (pController == NULL)
        {
            throw E_OUTOFMEMORY;
        }

        pController->Initialize();
        return pController;
    }

    ~CMMDeviceController()
    {
        Uninitialize();
    }

    void dump() const;

    const MMDeviceControllerID& GetID() const 
    {
        return this->m_ID;
    }

    CMMDevice* GetDefaultIn() const
    {
        return this->m_pDefaultIn;
    }

    CMMDevice* GetDefaultOut() const
    {
        return this->m_pDefaultOut;
    }

    void AddListener(CMMDeviceControllerListener *pListener)
    {
        m_listeners.push_back(pListener);
    }

    void RemoveListener(CMMDeviceControllerListener* pListener)
    {
        m_listeners.remove(pListener);
    }

    CMMSession* FindSessionByID(const MMSessionID& sessionID) const;
    CMMDevice* FindDeviceByID(const MMDeviceID& deviceID) const;

    void Refresh() throw () {
        try
        {
            this->LoadDefaultDevices();
        }
        catch (HRESULT)
        {
        }
    }

    void OnSessionRefreshed(CMMSession *pSession);
    void OnDeviceRefreshed(CMMDevice* pDevice);

protected:
    CMMDeviceController() :
        m_pDefaultOut(NULL),
        m_pDefaultIn(NULL)
    {}

    void Initialize() throw (HRESULT);
    void Uninitialize();

    void LoadDefaultDevices() throw (HRESULT);

    CComPtr<IMMDevice> GetMMDeviceByID(LPCWSTR pwszDeviceID) throw (HRESULT)
    {
        CComPtr<IMMDevice> spMMDevice;
        HRESULT hr = this->m_spDeviceEnumerator->GetDevice(pwszDeviceID, &spMMDevice);
        if (FAILED(hr))
        {
            throw hr;
        }

        return spMMDevice;
    }

    void FireOnSessionsRefreshed();

public:


    //thread safe version
    void TS_GetDefaultDeviceIDs(
        _Out_ std::wstring* pDefaultOutID,
        _Out_ std::wstring* pDefaultInID);
    bool TS_IsDefaultDeviceID(LPCWSTR pszDeviceID);

};
