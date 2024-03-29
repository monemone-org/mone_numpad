#include "CMMDeviceController.h"
#include "CMMSession.h"

void CMMDeviceController::Initialize() throw (HRESULT)
{
    HRESULT hr = S_OK;

    Uninitialize();

    CHK_HR(m_cs.Init());

    CHK_HR(CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spDeviceEnumerator)));

    ATLASSERT(m_spClientNotif == NULL);
    CHK_HR(CMMNotificationClient::CreateInstance(&m_spClientNotif));

    CMMNotificationClient* clientNotif = dynamic_cast<CMMNotificationClient*>(m_spClientNotif.p);
    clientNotif->Initialize(
        []/*OnDeviceStateChanged*/(
            _In_  LPCWSTR pwstrDeviceId,
            _In_  DWORD dwNewState) throw(HRESULT)
        {
            MMDeviceID deviceID(pwstrDeviceId);
            PostMainThreadRefreshDeviceProperties(deviceID, L"CMMNotificationClient::OnDeviceStateChanged()");
        },
        []/*OnDeviceAdded*/(
            _In_  LPCWSTR pwstrDeviceId) throw(HRESULT)
        {
        },
        []/*OnDeviceRemoved*/(
            _In_  LPCWSTR pwstrDeviceId) throw(HRESULT)
        {
        },
        []/*OnDefaultDeviceChanged*/(
            _In_  EDataFlow flow,
            _In_  ERole role,
            _In_  LPCWSTR pwstrDefaultDeviceId) throw(HRESULT)
        {
            ATLTRACE(TEXT("OnDefaultDeviceChanged(%s)\n"), pwstrDefaultDeviceId);
            if (flow == DEFAULT_OUT_DATAFLOW && role == DEFAULT_OUT_ROLE)
            {
                //changed default OUT device
                PostMainThreadRefreshAudioController(L"OnDefaultDeviceChanged(DEFAULT_OUT_DATAFLOW, DEFAULT_OUT_ROLE)");
            }
            else if (flow == DEFAULT_IN_DATAFLOW && role == DEFAULT_IN_ROLE)
            {
                //changed default IN device
                PostMainThreadRefreshAudioController(L"OnDefaultDeviceChanged(DEFAULT_IN_DATAFLOW, DEFAULT_IN_ROLE)");
            }
        },
        []/*OnPropertyValueChanged*/(
            _In_  LPCWSTR pwstrDeviceId,
            _In_  const PROPERTYKEY key) throw(HRESULT)
        {
            MMDeviceID deviceID(pwstrDeviceId);
            PostMainThreadRefreshDeviceProperties(deviceID, L"CMMNotificationClient::OnPropertyValueChanged");
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

void CMMDeviceController::RefreshSession(const MMSessionID& sessionID)
{
    ATLTRACE(TEXT("CMMDeviceController::RefreshSession(id=%s)\n"),
        sessionID.ID.c_str());

    CMMSession* pSession = this->FindSessionByID(sessionID);
    if (pSession)
    {
        pSession->Refresh();
    }

}

void CMMDeviceController::RefreshDeviceSessions(const MMDeviceID& deviceID)
{
    ATLTRACE(TEXT("CMMDeviceController::RefreshDevice(id=%s)\n"),
        deviceID.ID.c_str());

    CMMDevice* pDevice = this->FindDeviceByID(deviceID);
    if (pDevice)
    {
        pDevice->RefreshSessions();
        this->FireOnMMSessionsAddedRemoved(); //MMSession(s) maybe added/removed.
    }
}

void CMMDeviceController::RefreshDeviceProperties(const MMDeviceID& deviceID)
{
    ATLTRACE(TEXT("CMMDeviceController::RefreshDevice(id=%s)\n"),
        deviceID.ID.c_str());

    CMMDevice* pDevice = this->FindDeviceByID(deviceID);
    if (pDevice)
    {
        pDevice->RefreshProperties();
    }
}


void CMMDeviceController::Refresh()
{
    ATLTRACE(TEXT("CMMDeviceController::Refresh()\n"));
    try
    {
        this->LoadDefaultDevices();
        this->FireOnMMSessionsAddedRemoved(); //MMSession(s) maybe added/removed.
        this->dump();
    }
    catch (HRESULT)
    {
    }
}


void CMMDeviceController::FireOnMMSessionsAddedRemoved()
{
    for (auto listenerIter = m_listeners.begin();
        listenerIter != m_listeners.end();
        ++listenerIter)
    {
        CMMDeviceControllerListener* pListener = *listenerIter;
        pListener->OnMMSessionsAddedRemoved();
    }
}

CMMSession* CMMDeviceController::FindSessionByID(const MMSessionID& sessionID) const
{
    CMMDevice* pDevice = FindDeviceByID(sessionID.deviceID);
    if (pDevice == NULL)
    {
        return NULL;
    }

    return pDevice->FindSessionByID(sessionID);
}

CMMDevice* CMMDeviceController::FindDeviceByID(const MMDeviceID& deviceID) const
{
    if (this->m_pDefaultIn->GetID() == deviceID)
    {
        return m_pDefaultIn;
    }
    else if (this->m_pDefaultOut->GetID() == deviceID)
    {
        return m_pDefaultOut;
    }
    return NULL;
}

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
    ATLTRACE(L"<-- Dump CMMDeviceController\n");
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
    ATLTRACE(L"-->\n");
}


