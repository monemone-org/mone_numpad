#include "framework.h"
#include "CMMSession.h"
//#include <Mmdeviceapi.h>
#include "CMMDevice.h"
#include <audiopolicy.h>
#include "GetProcessName.h"
#include "AutoPtr.h"
#include "NotificationIcon.h"


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
    m_ID(),
    m_bIsSystemInOut(false),
    m_bIsInput(false)
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

    m_bIsInput = pParentDevice->IsInput();

    DWORD processId = 0;
    CHK_HR(spSessionControl2->GetProcessId(&processId));
    this->m_bIsSystemInOut = (spSessionControl2->IsSystemSoundsSession() == S_OK);
    this->m_sProcessName = GetProcessNameFromID(processId).c_str();

    CHK_HR(CAudoSessionEvents::CreateInstance(&m_spSessionEvents));

    // Create a variable copy for passing to the lamda functions below, so they
    // don't rely on [this] pointer which could be deleted in a multi-thread environment.
    MMDeviceID deviceID = m_pParentDevice->GetID();
    MMSessionID id = this->m_ID;
    CAudoSessionEvents* notifClient = dynamic_cast<CAudoSessionEvents*>(m_spSessionEvents.p);
    notifClient->Initialize(
        [id]/*OnDisplayNameChanged*/(LPCWSTR NewDisplayName, LPCGUID EventContext) throw(HRESULT) {
            PostMainThreadRefreshSession(id);
        },
        [id]/*OnStateChangedFunction*/(AudioSessionState state) throw(HRESULT) {
            PostMainThreadRefreshSession(id);
        },
            [deviceID]/*OnSessionDisconnectedFunction*/(AudioSessionDisconnectReason DisconnectReason) throw(HRESULT) {
            PostMainThreadRefreshDevice(deviceID);
        });
    CHK_HR(m_spSessionControl->RegisterAudioSessionNotification(m_spSessionEvents));


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
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl.p;
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
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl.p;
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
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl.p;
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
    CComQIPtr<ISimpleAudioVolume> spAudioVol = m_spSessionControl.p;
    if (spAudioVol == NULL)
    {
        throw E_UNEXPECTED;
    }
    CHK_HR(spAudioVol->SetMute(mute, NULL));
}

bool CMMSession::toggleMute() throw (HRESULT)
{
    bool isMute = IsMute();
    SetMute(!isMute);
    return IsMute();
}


void CMMSession::dump() const {
    try
    {
        ATLTRACE(TEXT("Session \"%s\"\n"), this->m_sProcessName.c_str());
        ATLTRACE(TEXT("        System In/Out: \"%d\"\n"), (this->IsSystemInOut() ? 1 : 0));
        ATLTRACE(TEXT("        State: \"%d\"\n"), (int)(this->GetState()));
        ATLTRACE(TEXT("        Volume: \"%f\"\n"), this->GetVolume());
        ATLTRACE(TEXT("        Mute: \"%d\"\n"), (this->IsMute() ? 1 : 0));
    }
    catch (HRESULT)
    {
    }
}

