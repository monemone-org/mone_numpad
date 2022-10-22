#include "CMMDevice.h"
#include "NotificationIcon.h"
#include "AutoPtr.h"
#include "CMMSession.h"
#include "CMMDeviceController.h"

std::wstring CreateGUIDString()
{
    GUID guid = { 0 };
    CHK_HR(CoCreateGuid(&guid));
    CHeapPtr<TCHAR, CCoTaskAllocator> spszGuid;
    (void)StringFromCLSID(guid, &spszGuid);
    return std::wstring(spszGuid);       
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

bool CMMDevice::IsInput() const throw(HRESULT)
{
    CComQIPtr<IMMEndpoint> spMMEndPoint = m_spDevice.p;
    if (spMMEndPoint)
    {
        EDataFlow dataFlow = eRender;
        CHK_HR(spMMEndPoint->GetDataFlow(&dataFlow));
        return (dataFlow == CMMDeviceController::DEFAULT_IN_DATAFLOW);
    }
    else
    {
        CHK_HR(E_NOINTERFACE);
    }
    return false;
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






