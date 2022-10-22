#pragma once


#include "monenumpad_maxmix/structs.h"
#include "HIDDevice.h"
#include "AudioSessionProvider.h"

class HIDDevice;
class AudioSessionProvider;
class KBCommService;

//class KBCommServiceListener
//{
//public:
//	virtual void OnKBDisconnected(KBCommService* pKBCommService) = 0;
//};

class KBCommService:
	HIDDeviceListener, // monitor if monenumpad is connected
    AudioSessionProviderListener // monitor if audio sessions are changed.
{
protected:
	static BYTE DEFAULT_SESSION_ID;
	
	AudioSessionProvider m_audioSessionProvider;

    //KBCommServiceListener* m_pListener;

    HIDDevice* m_pConnectedDevice;

	UINT16 m_kb_protocol_version;

	BYTE m_curr_session_id;

	UINT_PTR m_nIDEvent; //WM_TIMER
		
public:
    KBCommService(CMMDeviceController* pMMDeviceController);

	~KBCommService()
	{
		CloseMoneNumPad();
        Stop();
	}

    //void SetListener(KBCommServiceListener* pListener)
    //{
    //    m_pListener = pListener;
    //}

    bool Start();
    void Stop();

    HIDDevice* GetMoneNumpad() const {
        return m_pConnectedDevice;
    }

    bool IsConnected()
    {
		return m_pConnectedDevice != NULL;
	}


protected:
    friend void on_added_monenumpad(struct hid_device_info* dev_info, void* user_data);

    void OpenMoneNumPad(hid_device_info* dev_info);
    void CloseMoneNumPad();

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
	
    // AudioSessionProviderListener
    virtual void OnAudioSessionsRefreshed();



};

