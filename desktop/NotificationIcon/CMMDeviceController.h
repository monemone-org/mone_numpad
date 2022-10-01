#pragma once

#include "CMMDevice.h"
#include "NotificationIcon.h"


class CMMDeviceControllerListener
{
public:
    virtual void OnDeviceChanged() = 0;
    virtual void OnAudioSessionChanged() = 0;
};


class CMMDeviceController
{
protected:
    CComPtr<IMMDeviceEnumerator> m_spDeviceEnumerator;
    CComPtr<IMMNotificationClient> m_spClientNotif;

    CMMDevice* m_pDefaultOut;
    CMMDevice* m_pDefaultIn;

    // for thread-safe operations
    // CMMNotificationClient is notified in a background thread.
    // but we should only access CMMDeviceController and its data in
    // the main UI thread.
    CComCriticalSection m_cs;

private:
    CMMDeviceController(const CMMDeviceController&) :
        m_pDefaultOut(NULL),
        m_pDefaultIn(NULL)
    {
        ATLASSERT(false);
    };
    CMMDeviceController& operator=(const CMMDeviceController&) {
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

    void Refresh() throw () {
        try
        {
            this->LoadDefaultDevices();
        }
        catch (HRESULT)
        {
        }
    }

protected:
    CMMDeviceController() :
        m_pDefaultOut(NULL),
        m_pDefaultIn(NULL)
    {}

    static const EDataFlow DEFAULT_OUT_DATAFLOW = eRender;
    static const ERole DEFAULT_OUT_ROLE = eConsole;

    static const EDataFlow DEFAULT_IN_DATAFLOW = eCapture;
    static const ERole DEFAULT_IN_ROLE = eCommunications;

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

    CMMDevice* FindDeviceByID(LPCWSTR pwszDeviceID)
    {
        if (this->m_pDefaultIn->HasDeviceID(pwszDeviceID))
        {
            return m_pDefaultIn;
        }
        else if (this->m_pDefaultOut->HasDeviceID(pwszDeviceID))
        {
            return m_pDefaultOut;
        }

        //auto found_iter = std::find_if(m_devices.begin(), m_devices.end(), [=](CMMDevice* pDevice) -> bool {
        //    return (0 == wcscmp(pDevice->GetID(), pwszDeviceID));
        //    });
        //if (found_iter == m_devices.end())
        //{
        //    return NULL;
        //}
        //CMMDevice* pDisconnectedDevice = *found_iter;
        //return pDisconnectedDevice;

        return NULL;
    }

public:


    //thread safe version
    void TS_GetDefaultDeviceIDs(
        _Out_ std::wstring* pDefaultOutID,
        _Out_ std::wstring* pDefaultInID);
    bool TS_IsDefaultDeviceID(LPCWSTR pszDeviceID);

};
