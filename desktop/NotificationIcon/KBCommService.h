#pragma once


#include "monenumpad_maxmix/structs.h"
#include "HIDDevice.h"
#include "AudioSessionProvider.h"

class HIDDevice;
class AudioSessionProvider;
class KBCommService;

class KBCommServiceListener
{
public:
	virtual void OnKBDisconnected(KBCommService* pKBCommService) = 0;
};

class KBCommService:
	HIDDeviceListener
{
protected:
	static BYTE DEFAULT_SESSION_ID;
	
	AudioSessionProvider m_audioSessionProvider;

	HIDDevice* m_pConnectedDevice;

	UINT16 m_kb_protocol_version;

	BYTE m_curr_session_id;

	UINT_PTR m_nIDEvent; //WM_TIMER
		
	KBCommServiceListener* m_pListener;

public:
	KBCommService(CMMDeviceController* pMMDeviceController) :
		m_audioSessionProvider(pMMDeviceController),
		m_pConnectedDevice(NULL),
		m_kb_protocol_version(UNKNOWN_PROTOCOL_VERSION),
		m_curr_session_id(DEFAULT_SESSION_ID),
        m_nIDEvent(NULL),
		m_pListener(NULL)
	{
	}

	~KBCommService()
	{
		Disconnect();
	}

	HIDDevice* GetHIDDevice() const {
		return m_pConnectedDevice;
	}

    void SetListener(KBCommServiceListener* pListener)
    {
        m_pListener = pListener;
    }

    void Connect(hid_device_info* dev_info);

    void Disconnect();

	bool isConnected()
    {
		return m_pConnectedDevice != NULL;
	}

    void OnAudioSessionsChanged();

protected:
    //
    // -- Internal Methods --
    //
    bool IsProtocolCompatible() const
    {
        return this->m_kb_protocol_version == MAXMIX_PROTOCOL_VERSION;
    }

    //every 2 sec, send session_info
    //session_info also serve as a heartbeat
    void StartRefreshingSessionInfoTimer();

    void OnTimedEvent();

    void StopRefreshingSessionInfoTimer();

    void SendProtocolVersion();

    void SendSessionInfo();

    // return -1 if there is no prev session
    int PrevSessionIndex(int sessionIndex);

    // return -1 if there is no next session
    int NextSessionIndex(int sessionIndex);

    SessionData MakeSessionData(
        __in  int sessionIndex);

    void SendSessionDatas(bool curr_only = false);

    template<typename StructDataType>
    void SendStructMessage(
        Command command,
        StructDataType msgData);

    void SendUInt16Message(
        Command command,
        UINT16 n);

    void SendMessage(
        Command command,
        BYTE* data,
        size_t cbData);

    void HandleDeviceDataReceived(BYTE* data, size_t cbData);

public:
	//HIDDeviceListener Impl
    virtual void DeviceDataReceived(HIDDevice* dev, BYTE* data, size_t cbData);
    virtual void DeviceDisconnected(HIDDevice* dev);
	

};

