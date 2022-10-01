#include "CMMDevice.h"
#include "NotificationIcon.h"
#include "GetProcessName.h"
#include "AutoPtr.h"

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
    m_pParentDevice(NULL),
    m_state(AudioSessionStateInactive)
{
}

//disable copy constructor
CMMSession::CMMSession(const CMMSession&) :
    m_pParentDevice(NULL),
    m_state(AudioSessionStateInactive)
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

    if (pParentDevice == NULL)
    {
        throw E_INVALIDARG;
    }

    m_pParentDevice = pParentDevice;
    
    m_spSessionControl = pSession;

    CHK_HR( CAudoSessionEvents::CreateInstance(&m_spSessionEvents) );

    CAudoSessionEvents* notifClient = dynamic_cast<CAudoSessionEvents*>(m_spSessionEvents.p);
    notifClient->Initialize(
        [this]/*OnDisplayNameChanged*/(LPCWSTR NewDisplayName, LPCGUID EventContext) throw(HRESULT) {
            PostMainThreadRefreshSession(this);
        },
        [this]/*OnStateChangedFunction*/(AudioSessionState state) throw(HRESULT)  {
            PostMainThreadRefreshSession(this);
        },
        [this]/*OnSessionDisconnectedFunction*/(AudioSessionDisconnectReason DisconnectReason) throw(HRESULT) {
            if (m_pParentDevice == NULL)
                return;
            PostMainThreadRefreshDevice(m_pParentDevice);
        });

    CHK_HR( m_spSessionControl->RegisterAudioSessionNotification(m_spSessionEvents) );

    // Get process name
    CComQIPtr<IAudioSessionControl2> spSessionControl2 = m_spSessionControl;
    if (spSessionControl2 != NULL)
    {
        DWORD processId = 0;
        CHK_HR(spSessionControl2->GetProcessId(&processId));
        this->m_sProcessName = GetProcessNameFromID(processId).c_str();
    }

    FetchProperties();
}

void CMMSession::Uninitialize()
{
    m_sDisplayName.clear();
    m_sProcessName.clear();
    m_state = AudioSessionStateInactive;

    if (m_spSessionControl != NULL && m_spSessionEvents != NULL)
    {
        m_spSessionControl->UnregisterAudioSessionNotification(m_spSessionEvents);
    }
    m_spSessionEvents = NULL;
    m_spSessionControl = NULL;

    m_pParentDevice = NULL;
}


bool CMMSession::IsActive() const {
    return (m_state == AudioSessionStateActive);
}

LPCWSTR CMMSession::GetDisplayName() const {
    if (m_sDisplayName.empty())
    {
        return this->m_sProcessName.c_str();
    }
    return m_sDisplayName.c_str();
}

void CMMSession::FetchProperties() throw (HRESULT)
{
    HRESULT hr = S_OK;

    if (m_spSessionControl == NULL)
    {
        return;
    }

    m_sDisplayName.clear();

    CComHeapPtr<WCHAR> sDisplayName;
    CHK_HR(m_spSessionControl->GetDisplayName(&sDisplayName));
    m_sDisplayName = sDisplayName;

    AudioSessionState state = AudioSessionStateInactive;
    CHK_HR( m_spSessionControl->GetState(&m_state) );

}

void CMMSession::dump() const {
    ATLTRACE(TEXT("Session \"%s\"\n"), this->GetDisplayName());
    ATLTRACE(TEXT("        IsActive: \"%d\"\n"), (this->IsActive() ? 1 : 0));
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
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        CMMSession* session = *iter;
        delete session;
    }
    m_sessions.clear();

    m_sID.clear();
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
    HRESULT hr = S_OK;

    Uninitialize();

    m_spDevice = pDevice;

    ATLASSERT(m_spSessionMgr == NULL);
    CHK_HR( m_spDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&m_spSessionMgr) );

    ATLASSERT(m_spSessionNotif == NULL);
    CHK_HR(CAudioSessionNotification::CreateInstance(&m_spSessionNotif));

    CAudioSessionNotification* sessionNotif = dynamic_cast<CAudioSessionNotification*>(m_spSessionNotif.p);
    sessionNotif->Initialize(
        [this]/*OnSessionCreated*/(IAudioSessionControl* pSessionControl) throw(HRESULT) {
            try
            {
                if (m_spDevice == NULL)
                    return;
                PostMainThreadRefreshDevice(this);
            }
            catch (HRESULT)
            {
            }
        }
    );

    FetchProperties();

    FetchSessions();
}


void CMMDevice::FetchProperties() throw(HRESULT)
{
    HRESULT hr = S_OK;

    m_sID.clear();

    CComHeapPtr<WCHAR> sID;
    CHK_HR( m_spDevice->GetId(&sID) );
    m_sID = sID;

    CComPtr<IPropertyStore> spProperties;
    CHK_HR(m_spDevice->OpenPropertyStore(STGM_READ, &spProperties) );

    CPROPVARIANT propVariant;
    CHK_HR(spProperties->GetValue(PKEY_Device_FriendlyName, &propVariant));
    m_sDisplayName = propVariant.pwszVal;
}

void CMMDevice::FetchSessions() throw(HRESULT)
{
    HRESULT hr = S_OK;

    CComPtr< IAudioSessionEnumerator> spSessionEnum;
    CHK_HR(m_spSessionMgr->GetSessionEnumerator(&spSessionEnum));

    int nCount = 0;
    CHK_HR(spSessionEnum->GetCount(&nCount));

    for (int i = 0; i < nCount; ++i)
    {
        CComPtr< IAudioSessionControl> spSessionControl;
        CHK_HR(spSessionEnum->GetSession(i, &spSessionControl));

        CComQIPtr< IAudioSessionControl2> spSessionControl2 = spSessionControl;
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
 
void CMMDevice::dump() const 
{
    ATLTRACE(TEXT("Device \"%s\"\n"), this->GetDisplayName());
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        CMMSession* pSession = *iter;
        pSession->dump();
    }
}






