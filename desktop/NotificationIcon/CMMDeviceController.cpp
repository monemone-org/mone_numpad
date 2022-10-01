#include "CMMDeviceController.h"


void CMMDeviceController::Initialize() throw (HRESULT)
{
    HRESULT hr = S_OK;

    Uninitialize();

    m_ID = MMDeviceControllerID( CreateGUIDString().c_str() );

    CHK_HR(m_cs.Init());

    CHK_HR(CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spDeviceEnumerator)));

    ATLASSERT(m_spClientNotif == NULL);
    CHK_HR(CMMNotificationClient::CreateInstance(&m_spClientNotif));

    MMDeviceControllerID ID = m_ID;
    CMMNotificationClient* clientNotif = dynamic_cast<CMMNotificationClient*>(m_spClientNotif.p);
    clientNotif->Initialize(
        [ID]/*OnDeviceStateChanged*/(
            _In_  LPCWSTR pwstrDeviceId,
            _In_  DWORD dwNewState) throw(HRESULT)
        {
            MMDeviceID deviceID(pwstrDeviceId);
            PostMainThreadRefreshDevice(deviceID);
        },
        []/*OnDeviceAdded*/(
            _In_  LPCWSTR pwstrDeviceId) throw(HRESULT)
        {
        },
        []/*OnDeviceRemoved*/(
            _In_  LPCWSTR pwstrDeviceId) throw(HRESULT)
        {
        },
        [ID]/*OnDefaultDeviceChanged*/(
            _In_  EDataFlow flow,
            _In_  ERole role,
            _In_  LPCWSTR pwstrDefaultDeviceId) throw(HRESULT)
        {
            ATLTRACE(TEXT("OnDefaultDeviceChanged(%s)\n"), pwstrDefaultDeviceId);
            if (flow == DEFAULT_OUT_DATAFLOW && role == DEFAULT_OUT_ROLE)
            {
                //changed default OUT device
                PostMainThreadRefreshAudioController(ID);
            }
            else if (flow == DEFAULT_IN_DATAFLOW && role == DEFAULT_IN_ROLE)
            {
                //changed default IN device
                PostMainThreadRefreshAudioController(ID);
            }
        },
        []/*OnPropertyValueChanged*/(
            _In_  LPCWSTR pwstrDeviceId,
            _In_  const PROPERTYKEY key) throw(HRESULT)
        {
            MMDeviceID deviceID(pwstrDeviceId);
            PostMainThreadRefreshDevice(deviceID);
        }
    );
    CHK_HR(m_spDeviceEnumerator->RegisterEndpointNotificationCallback(m_spClientNotif));

    LoadDefaultDevices();
}

void CMMDeviceController::Uninitialize()
{
    if (m_spClientNotif != NULL && m_spDeviceEnumerator != NULL)
    {
        m_spDeviceEnumerator->UnregisterEndpointNotificationCallback(m_spClientNotif);
    }
    m_spClientNotif = NULL;
    m_spDeviceEnumerator = NULL;

    if (m_pDefaultOut)
    {
        delete m_pDefaultOut;
        m_pDefaultOut = NULL;
    }

    if (m_pDefaultIn)
    {
        delete m_pDefaultIn;
        m_pDefaultIn = NULL;
    }

    m_cs.Term();
}

void CMMDeviceController::LoadDefaultDevices() throw (HRESULT)
{
    HRESULT hr = S_OK;

    delete m_pDefaultOut;
    delete m_pDefaultIn;
    m_pDefaultOut = NULL;
    m_pDefaultIn = NULL;

    CComPtr<IMMDevice> spDefaultOutDevice;
    hr = m_spDeviceEnumerator->GetDefaultAudioEndpoint(DEFAULT_OUT_DATAFLOW, DEFAULT_OUT_ROLE, &spDefaultOutDevice);
    if (FAILED(hr) && hr != ERROR_NOT_FOUND) {
        OutputDebugString(TEXT("GetDefaultAudioEndpoint Output failed."));
        throw hr;
    }

    //Default Input: e.g. microphone 
    CComPtr<IMMDevice> spDefaultInDevice;
    hr = m_spDeviceEnumerator->GetDefaultAudioEndpoint(DEFAULT_IN_DATAFLOW, eCommunications, &spDefaultInDevice);
    if (FAILED(hr) && hr != ERROR_NOT_FOUND) {
        OutputDebugString(TEXT("GetDefaultAudioEndpoint Input failed."));
        throw hr;
    }

    if (spDefaultOutDevice != NULL)
    {
        m_pDefaultOut = CMMDevice::CreateObject(spDefaultOutDevice);
    }

    if (spDefaultInDevice != NULL)
    {
        m_pDefaultIn = CMMDevice::CreateObject(spDefaultInDevice);
    }
}

//CComPtr<IMMDevice> CMMDeviceController::GetMMDeviceByID(LPCWSTR pwszDeviceID) throw (HRESULT)
//{
//    CComPtr<IMMDevice> spMMDevice;
//    HRESULT hr = this->m_spDeviceEnumerator->GetDevice(pwszDeviceID, &spMMDevice);
//    if (FAILED(hr))
//    {
//        throw hr;
//    }
//
//    return spMMDevice;
//}

//CMMDevice* CMMDeviceController::FindDeviceByID(LPCWSTR pwszDeviceID)
//{
//    if (this->m_pDefaultIn->HasDeviceID(pwszDeviceID))
//    {
//        return m_pDefaultIn;
//    }
//    else if (this->m_pDefaultOut->HasDeviceID(pwszDeviceID))
//    {
//        return m_pDefaultOut;
//    }
//
//    //auto found_iter = std::find_if(m_devices.begin(), m_devices.end(), [=](CMMDevice* pDevice) -> bool {
//    //    return (0 == wcscmp(pDevice->GetID(), pwszDeviceID));
//    //    });
//    //if (found_iter == m_devices.end())
//    //{
//    //    return NULL;
//    //}
//    //CMMDevice* pDisconnectedDevice = *found_iter;
//    //return pDisconnectedDevice;
//
//    return NULL;
//}


//thread safe version
void CMMDeviceController::TS_GetDefaultDeviceIDs(
    _Out_ std::wstring* pDefaultOutID,
    _Out_ std::wstring* pDefaultInID)
{
    ATLASSERT(pDefaultOutID && pDefaultInID);

    pDefaultOutID->clear();
    pDefaultInID->clear();

    CComCritSecLock<CComCriticalSection> lock(m_cs);
    if (m_pDefaultOut)
    {
        *pDefaultOutID = m_pDefaultOut->GetID().ID;
    }

    if (m_pDefaultIn)
    {
        *pDefaultInID = m_pDefaultIn->GetID().ID;
    }
}

bool CMMDeviceController::TS_IsDefaultDeviceID(LPCWSTR pszDeviceID)
{
    if (pszDeviceID == NULL) {
        return false;
    }

    std::wstring sDefaultOutID, sDefaultInID;
    TS_GetDefaultDeviceIDs(&sDefaultOutID, &sDefaultInID);

    return (0 == wcscmp(sDefaultOutID.c_str(), pszDeviceID))
        || (0 == wcscmp(sDefaultInID.c_str(), pszDeviceID));
}

void CMMDeviceController::dump() const {
    if (m_pDefaultOut)
    {
        ATLTRACE(TEXT("Default Out Device\n"));
        m_pDefaultOut->dump();
    }

    if (m_pDefaultIn)
    {
        ATLTRACE(TEXT("Default In Device\n"));
        m_pDefaultIn->dump();
    }
}


