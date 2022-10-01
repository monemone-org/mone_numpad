#include "CMMDevice.h"
#include "NotificationIcon.h"
#include "GetProcessName.h"
#include "AutoPtr.h"

std::wstring CreateGUIDString()
{
    GUID guid = { 0 };
    CHK_HR(CoCreateGuid(&guid));
    CHeapPtr<TCHAR, CCoTaskAllocator> spszGuid;
    StringFromCLSID(guid, &spszGuid);    
    return std::wstring(spszGuid);       
}

/// <summary>
/// Class CMMSession
/// </summary>
CMMSession* CMMSession::CreateObject(CMMDevice* pParentDevice, IAudioSessionControl* pSessionControl) throw(HRESULT)
{
    CMMSession* pSession = new CMMSession();
    if (pSession == NULL)
    {
        throw (E_OUTOFMEMORY);
    }

    pSession->Initialize(pParentDevice, pSessionControl);
    return pSession;
}

CMMSession::CMMSession() :
    m_pParentDevice(NULL)
{
}

//disable copy constructor
CMMSession::CMMSession(const CMMSession&) :
    m_pParentDevice(NULL)
{
    ATLASSERT(false);
}

CMMSession::~CMMSession()
{
    Uninitialize();
    m_pParentDevice = NULL;
}


void CMMSession::Initialize(CMMDevice* pParentDevice, IAudioSessionControl* pSession) throw(HRESULT)
{
    HRESULT hr = S_OK;

    Uninitialize();

    if (pParentDevice == NULL || pSession == NULL)
    {
        throw E_INVALIDARG;
    }

    CComQIPtr<IAudioSessionControl2> spSessionControl2 = pSession;
    if (spSessionControl2 == NULL)
    {
        throw E_INVALIDARG;
    }

    m_pParentDevice = pParentDevice;
    
    m_spSessionControl = pSession;

    // Get sessionIdentifier and process name
    CHeapPtr<WCHAR, CCoTaskAllocator> sessionIdentifier;
    spSessionControl2->GetSessionIdentifier(&sessionIdentifier);

    m_ID = MMSessionID(pParentDevice->GetID(), sessionIdentifier);

    DWORD processId = 0;
    CHK_HR(spSessionControl2->GetProcessId(&processId));
    this->m_sProcessName = GetProcessNameFromID(processId).c_str();
    
    CHK_HR( CAudoSessionEvents::CreateInstance(&m_spSessionEvents) );

    // Create a variable copy for passing to the lamda functions below, so they
    // don't rely on [this] pointer which could be deleted in a multi-thread environment.
    MMDeviceID deviceID = m_pParentDevice->GetID();
    MMSessionID id = this->m_ID;
    CAudoSessionEvents* notifClient = dynamic_cast<CAudoSessionEvents*>(m_spSessionEvents.p);
    notifClient->Initialize(
        [id]/*OnDisplayNameChanged*/(LPCWSTR NewDisplayName, LPCGUID EventContext) throw(HRESULT) {
            PostMainThreadRefreshSession(id);
        },
        [id]/*OnStateChangedFunction*/(AudioSessionState state) throw(HRESULT)  {
            PostMainThreadRefreshSession(id);
        },
        [deviceID]/*OnSessionDisconnectedFunction*/(AudioSessionDisconnectReason DisconnectReason) throw(HRESULT) {
            PostMainThreadRefreshDevice(deviceID);
        });
    CHK_HR( m_spSessionControl->RegisterAudioSessionNotification(m_spSessionEvents) );


    FetchProperties();
}

void CMMSession::Uninitialize()
{
    m_sDisplayName.clear();
    m_sProcessName.clear();

    if (m_spSessionControl != NULL && m_spSessionEvents != NULL)
    {
        m_spSessionControl->UnregisterAudioSessionNotification(m_spSessionEvents);
    }
    m_spSessionEvents = NULL;
    m_spSessionControl = NULL;

    m_pParentDevice = NULL;
}


void CMMSession::FetchProperties() throw (HRESULT)
{
    HRESULT hr = S_OK;

    if (m_spSessionControl == NULL)
    {
        return;
    }

    CComHeapPtr<WCHAR> sDisplayName;
    CHK_HR(m_spSessionControl->GetDisplayName(&sDisplayName));
    m_sDisplayName = sDisplayName;

}

float CMMSession::GetVolume() const throw(HRESULT)
{
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl;
    if (spAudioVol == NULL)
    {
        throw E_UNEXPECTED;
    }

    float vol = 1.0;
    CHK_HR(spAudioVol->GetMasterVolume(&vol));
    return vol;
    
}

void CMMSession::SetVolume(float vol) throw(HRESULT)
{
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl;
    if (spAudioVol == NULL)
    {
        throw E_UNEXPECTED;
    }

    CHK_HR(spAudioVol->SetMasterVolume(vol, NULL));
}

AudioSessionState CMMSession::GetState() const throw (HRESULT)
{
    AudioSessionState state = AudioSessionStateInactive;
    CHK_HR(m_spSessionControl->GetState(&state));
    return state;
}

bool CMMSession::IsMute() const throw(HRESULT)
{
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl;
    if (spAudioVol == NULL)
    {
        throw E_UNEXPECTED;
    }

    BOOL mute = FALSE;
    CHK_HR(spAudioVol->GetMute(&mute));
    return (mute != FALSE);
}

void CMMSession::SetMute(bool mute) throw(HRESULT)
{
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl;
    if (spAudioVol == NULL)
    {
        throw E_UNEXPECTED;
    }
    CHK_HR(spAudioVol->SetMute(mute, NULL));
}



void CMMSession::dump() const {
    try
    {
        ATLTRACE(TEXT("Session \"%s\"\n"), this->m_sProcessName.c_str());
        ATLTRACE(TEXT("        state: \"%d\"\n"), (int)(this->GetState()));
        ATLTRACE(TEXT("        Volume: \"%f\"\n"), this->GetVolume());
        ATLTRACE(TEXT("        Mute: \"%d\"\n"), (this->IsMute() ? 1 : 0));
    }
    catch (HRESULT)
    {
    }
}


/// <summary>
/// Class CMMDevice
/// </summary>

CMMDevice* CMMDevice::CreateObject(IMMDevice* pMMDevice) throw(HRESULT)
{
    if (pMMDevice == NULL)
    {
        throw E_INVALIDARG;
    }

    CMMDevice* pDevice = new CMMDevice();
    if (pDevice == NULL)
    {
        throw (E_OUTOFMEMORY);
    }

    pDevice->Initialize(pMMDevice);
    return pDevice;
}

CMMDevice::CMMDevice() :
    m_bIsInput(false)
{
}

//disable copy constructor
CMMDevice::CMMDevice(const CMMDevice& that) :
    m_bIsInput(that.m_bIsInput)
{
    ATLASSERT(false);
}

CMMDevice::~CMMDevice()
{
    Uninitialize();
}

void CMMDevice::Uninitialize() 
{
    ClearSessions();

    m_ID.clear();
    m_sDisplayName.clear();
    m_bIsInput = false;

    if (m_spSessionMgr != NULL && m_spSessionNotif != NULL) {
        m_spSessionMgr->UnregisterSessionNotification(m_spSessionNotif);
    }
    m_spSessionMgr = NULL;
    m_spSessionNotif = NULL;

    m_spDevice = NULL;
}

void CMMDevice::Initialize(IMMDevice* pDevice) throw(HRESULT)
{
    ATLASSERT(pDevice != NULL);

    HRESULT hr = S_OK;

    Uninitialize();

    m_spDevice = pDevice;

    CComHeapPtr<WCHAR> sID;
    CHK_HR(m_spDevice->GetId(&sID));
    m_ID = MMDeviceID(sID.operator LPWSTR());

    ATLASSERT(m_spSessionMgr == NULL);
    CHK_HR( m_spDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&m_spSessionMgr) );

    ATLASSERT(m_spSessionNotif == NULL);
    CHK_HR(CAudioSessionNotification::CreateInstance(&m_spSessionNotif));

    // Create a variable copy for passing to the lamda functions below, so they
    // don't rely on [this] pointer which could be deleted in a multi-thread environment.
    MMDeviceID ID = m_ID;
    CAudioSessionNotification* sessionNotif = dynamic_cast<CAudioSessionNotification*>(m_spSessionNotif.p);
    sessionNotif->Initialize(
        [ID]/*OnSessionCreated*/(IAudioSessionControl* pSessionControl) throw(HRESULT) {
            try
            {
                PostMainThreadRefreshDevice(ID);
            }
            catch (HRESULT)
            {
            }
        }
    );
    m_spSessionMgr->RegisterSessionNotification(m_spSessionNotif);    

    FetchProperties();

    FetchSessions();
}


void CMMDevice::FetchProperties() throw(HRESULT)
{
    HRESULT hr = S_OK;

    CComPtr<IPropertyStore> spProperties;
    CHK_HR(m_spDevice->OpenPropertyStore(STGM_READ, &spProperties) );

    CPROPVARIANT propVariant;
    CHK_HR(spProperties->GetValue(PKEY_Device_FriendlyName, &propVariant));
    m_sDisplayName = propVariant.pwszVal;
}

void CMMDevice::ClearSessions()
{
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        CMMSession* session = *iter;
        delete session;
    }
    m_sessions.clear();
}

void CMMDevice::FetchSessions() throw(HRESULT)
{
    HRESULT hr = S_OK;

    ClearSessions();

    CComPtr< IAudioSessionEnumerator> spSessionEnum;
    CHK_HR(m_spSessionMgr->GetSessionEnumerator(&spSessionEnum));

    int nCount = 0;
    CHK_HR(spSessionEnum->GetCount(&nCount));

    for (int i = 0; i < nCount; ++i)
    {
        CComPtr< IAudioSessionControl> spSessionControl;
        CHK_HR(spSessionEnum->GetSession(i, &spSessionControl));

        CComQIPtr< IAudioSessionControl2> spSessionControl2 = spSessionControl;
        if (spSessionControl2 == NULL)
        {
            throw E_UNEXPECTED;
        }
        DWORD processId = 0;
        spSessionControl2->GetProcessId(&processId);
        if (processId == 0)
        {
            //skip process 
            continue;
        }

        CMMSession* pSession = CMMSession::CreateObject(this, spSessionControl);
        if (pSession == NULL)
        {
            throw MSSIPOTF_E_OUTOFMEMRANGE;
        }

        m_sessions.push_back(pSession);
    }
}


CMMSession* CMMDevice::FindSessionByID(const MMSessionID& sessionID) const
{
    if (m_ID != sessionID.deviceID) {
        return NULL;
    }

    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        if ((*iter)->GetID() == sessionID)
        {
            return *iter;
        }
    }

    return NULL;
}

 
void CMMDevice::dump() const 
{
    ATLTRACE(TEXT("Device \"%s\"\n"), this->GetDisplayName());
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        CMMSession* pSession = *iter;
        pSession->dump();
    }
}






