#pragma once

#include "framework.h"
using namespace ATL;

#include <Mmdeviceapi.h>
#include <audiopolicy.h>
#include <functional>


class CAudoSessionEvents;
class CAudioSessionNotification;
class CMMNotificationClient;


class CAudoSessionEvents :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass< CAudoSessionEvents, &CLSID_NULL>,
    public IAudioSessionEvents
{
public:
    typedef std::function<void(LPCWSTR, LPCGUID)> OnDisplayNameChangedFunction; //throw(HRESULT) 
    //void(__cdecl* OnDisplayNameChangedFunction)(LPCWSTR NewDisplayName, LPCGUID EventContext) throw(HRESULT);

    typedef std::function<void(AudioSessionState)> OnStateChangedFunction; //throw(HRESULT) 
    //void(__cdecl* OnStateChangedFunction)(AudioSessionState) throw(HRESULT);

    typedef std::function<void(AudioSessionDisconnectReason)> OnSessionDisconnectedFunction; //throw(HRESULT) 
    //void(__cdecl* OnSessionDisconnectedFunction)(AudioSessionDisconnectReason) throw(HRESULT);

protected:
    OnDisplayNameChangedFunction m_onDisplayNameChangedFunc;
    OnStateChangedFunction m_onStateChangedFunc;
    OnSessionDisconnectedFunction m_onSessionDisconnectedFunc;


public:
    CAudoSessionEvents()
    {
        m_onDisplayNameChangedFunc = NULL;
        m_onStateChangedFunc = NULL;
        m_onSessionDisconnectedFunc = NULL;
    }

    void Initialize(
        OnDisplayNameChangedFunction onDisplayNameChangedFunc,
        OnStateChangedFunction onStateChangedFunc,
        OnSessionDisconnectedFunction onSessionDisconnectedFunc
    ) 
    {
        m_onDisplayNameChangedFunc = onDisplayNameChangedFunc;
        m_onStateChangedFunc = onStateChangedFunc;
        m_onSessionDisconnectedFunc = onSessionDisconnectedFunc;
    }

    BEGIN_COM_MAP(CAudoSessionEvents)
        COM_INTERFACE_ENTRY(IAudioSessionEvents)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:
    // IAudioSessionEvents
    HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(
        _In_  LPCWSTR NewDisplayName,
        _In_  LPCGUID EventContext)
    {
        ATLTRACE(_T("CMMSession::OnDisplayNameChanged(%s)\n"), NewDisplayName);

        try
        {
            if (m_onDisplayNameChangedFunc != NULL)
            {
                m_onDisplayNameChangedFunc(NewDisplayName, EventContext);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }

    HRESULT STDMETHODCALLTYPE OnIconPathChanged(
        _In_  LPCWSTR NewIconPath,
        _In_  LPCGUID EventContext)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(
        _In_  float NewVolume,
        _In_  BOOL NewMute,
        _In_  LPCGUID EventContext)
    {
        ATLTRACE(_T("CMMSession::OnSimpleVolumeChanged(vol=%f, muted=%d)\n"), NewVolume, (int)NewMute);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(
        _In_  DWORD ChannelCount,
        _In_reads_(ChannelCount)  float NewChannelVolumeArray[],
        _In_  DWORD ChangedChannel,
        _In_  LPCGUID EventContext)
    {
        ATLTRACE(_T("CMMSession::OnChannelVolumeChanged(ChannelCount=%ld, ChangedChannel=%ld)\n"), ChannelCount, ChangedChannel);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(
        _In_  LPCGUID NewGroupingParam,
        _In_  LPCGUID EventContext)
    {
        ATLTRACE(_T("CMMSession::OnGroupingParamChanged()\n"));
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnStateChanged(
        _In_  AudioSessionState NewState)
    {
        ATLTRACE(_T("CMMSession::OnStateChanged()\n"));
        try
        {
            if (m_onStateChangedFunc != NULL)
            {
                m_onStateChangedFunc(NewState);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }

    HRESULT STDMETHODCALLTYPE OnSessionDisconnected(
        _In_  AudioSessionDisconnectReason DisconnectReason)
    {
        ATLTRACE(_T("CMMSession::OnSessionDisconnected()\n"));

        try
        {
            if (m_onSessionDisconnectedFunc != NULL)
            {
                m_onSessionDisconnectedFunc(DisconnectReason);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }
};



class CAudioSessionNotification :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass< CAudioSessionNotification, &CLSID_NULL>,
    public IAudioSessionNotification
{
public:
    typedef std::function<void(IAudioSessionControl*)> OnSessionCreatedFunction; //throw(HRESULT) 
    //void(__cdecl* OnSessionCreatedFunction)(IAudioSessionControl* NewSession) throw(HRESULT);

protected:
    OnSessionCreatedFunction m_onSessionCreatedFunc;

public:
    CAudioSessionNotification()
    {
        m_onSessionCreatedFunc = NULL;
    }

    void Initialize(OnSessionCreatedFunction onSessionCreatedFunc)
    {
        m_onSessionCreatedFunc = onSessionCreatedFunc;
    }
    
    BEGIN_COM_MAP(CAudioSessionNotification)
        COM_INTERFACE_ENTRY(IAudioSessionNotification)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:

    // IAudioSessionNotification 
    HRESULT STDMETHODCALLTYPE OnSessionCreated(
        /* [in] */ IAudioSessionControl* NewSession)
    {
        ATLTRACE(_T("CAudioSessionNotification(CMMDevice)::OnSessionCreated()\n"));

        try
        {
            if (m_onSessionCreatedFunc != NULL)
            {
                m_onSessionCreatedFunc(NewSession);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }

};



class CMMNotificationClient :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass< CMMNotificationClient, &CLSID_NULL>,
	public IMMNotificationClient
{
public:
    typedef std::function<void(LPCWSTR, DWORD)> OnDeviceStateChangedFunction; // throw(HRESULT) 
    //typedef int(__cdecl* OnDeviceStateChangedFunction)(
    //    _In_  LPCWSTR pwstrDeviceId,
    //    _In_  DWORD dwNewState) throw(HRESULT) ;

    typedef std::function<void(LPCWSTR)> OnDeviceAddedFunction; // throw(HRESULT) 
    //typedef int(__cdecl* OnDeviceAddedFunction)(
    //    _In_  LPCWSTR pwstrDeviceId) throw(HRESULT) ;

    typedef std::function<void(LPCWSTR)> OnDeviceRemovedFunction; // throw(HRESULT) 
    //typedef int(__cdecl* OnDeviceRemovedFunction)(
    //    _In_  LPCWSTR pwstrDeviceId) throw(HRESULT) ;

    typedef std::function<void(EDataFlow, ERole, LPCWSTR)> OnDefaultDeviceChangedFunction; // throw(HRESULT) 
    //typedef int(__cdecl* OnDefaultDeviceChangedFunction)(
    //    _In_  EDataFlow flow,
    //    _In_  ERole role,
    //    _In_  LPCWSTR pwstrDefaultDeviceId) throw(HRESULT) ;

    typedef std::function<void(LPCWSTR, const PROPERTYKEY)> OnPropertyValueChangedFunction; // throw(HRESULT) 
    //typedef int(__cdecl* OnPropertyValueChangedFunction)(
    //    _In_  LPCWSTR pwstrDeviceId,
    //    _In_  const PROPERTYKEY key) throw(HRESULT) ;


protected:
    OnDeviceStateChangedFunction m_onDeviceStateChangedFunc;
    OnDeviceAddedFunction m_onDeviceAddedFunc;
    OnDeviceRemovedFunction m_onDeviceRemovedFunc;
    OnDefaultDeviceChangedFunction m_onDefaultDeviceChangedFunc;
    OnPropertyValueChangedFunction m_onPropertyValueChangedFunc;

public:
	CMMNotificationClient() :
		m_onDeviceStateChangedFunc(NULL),
		m_onDeviceAddedFunc(NULL),
		m_onDeviceRemovedFunc(NULL),
		m_onDefaultDeviceChangedFunc(NULL),
		m_onPropertyValueChangedFunc(NULL)
	{
	}

	void Initialize(
        OnDeviceStateChangedFunction onDeviceStateChangedFunc,
		OnDeviceAddedFunction onDeviceAddedFunc,
		OnDeviceRemovedFunction onDeviceRemovedFunc,
		OnDefaultDeviceChangedFunction onDefaultDeviceChangedFunc,
		OnPropertyValueChangedFunction onPropertyValueChangedFunc)
    {
        m_onDeviceStateChangedFunc = onDeviceStateChangedFunc;
        m_onDeviceAddedFunc = onDeviceAddedFunc;
        m_onDeviceRemovedFunc = onDeviceRemovedFunc;
        m_onDefaultDeviceChangedFunc = onDefaultDeviceChangedFunc;
        m_onPropertyValueChangedFunc = onPropertyValueChangedFunc;
    }


	BEGIN_COM_MAP(CMMNotificationClient)
		COM_INTERFACE_ENTRY(IMMNotificationClient)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
    // 
    // IMMNotificationClient
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
        /* [annotation][in] */
        _In_  LPCWSTR pwstrDeviceId,
        /* [annotation][in] */
        _In_  DWORD dwNewState)
    {
        ATLTRACE(_T("CMMNotificationClient::OnDeviceStateChanged()\n"));

        try
        {
            if (m_onDeviceStateChangedFunc) {
                m_onDeviceStateChangedFunc(pwstrDeviceId, dwNewState);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(
        /* [annotation][in] */
        _In_  LPCWSTR pwstrDeviceId)
    {
        ATLTRACE(_T("CMMNotificationClient::OnDeviceAdded()\n"));

        try
        {
            if (m_onDeviceAddedFunc) {
                m_onDeviceAddedFunc(pwstrDeviceId);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(
        /* [annotation][in] */
        _In_  LPCWSTR pwstrDeviceId)
    {
        ATLTRACE(_T("CMMNotificationClient::OnDeviceRemoved()\n"));

        try
        {
            if (m_onDeviceRemovedFunc) {
                m_onDeviceRemovedFunc(pwstrDeviceId);
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }


    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
        /* [annotation][in] */
        _In_  EDataFlow flow,
        /* [annotation][in] */
        _In_  ERole role,
        /* [annotation][in] */
        _In_  LPCWSTR pwstrDefaultDeviceId)
    {
        ATLTRACE(_T("CMMNotificationClient::OnDefaultDeviceChanged()\n"));
        try
        {
            if (m_onDefaultDeviceChangedFunc) {
                m_onDefaultDeviceChangedFunc(
                    flow, role, pwstrDefaultDeviceId
                );
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
        /* [annotation][in] */
        _In_  LPCWSTR pwstrDeviceId,
        /* [annotation][in] */
        _In_  const PROPERTYKEY key)
    {
        ATLTRACE(_T("CMMNotificationClient::OnPropertyValueChanged()\n"));
        try
        {
            if (m_onPropertyValueChangedFunc)
            {
                m_onPropertyValueChangedFunc( pwstrDeviceId, key );
            }
            return S_OK;
        }
        catch (HRESULT hr)
        {
            return hr;
        }
    }


};

