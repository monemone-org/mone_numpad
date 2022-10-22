#include "KBCommService.h"

#include "AudioSessionProvider.h"
#include "CMMDeviceController.h"
#include "CMMSession.h"
#include <limits>
#include <functional>
#include "MarshalFunc.h"
#include "NotificationIcon.h"


BYTE KBCommService::DEFAULT_SESSION_ID = SESSION_ID_OUT;

#define MAX_VOLUME 100

std::wstring ToString(BYTE* data, size_t cbData);


void on_added_monenumpad(struct hid_device_info* dev_info, void* user_data)
{
    KBCommService* pKBCommService = (KBCommService * )user_data;
    pKBCommService->OpenMoneNumPad(dev_info);
}


KBCommService::KBCommService(CMMDeviceController* pMMDeviceController) :
    m_audioSessionProvider(pMMDeviceController),
    m_pConnectedDevice(NULL),
    m_kb_protocol_version(UNKNOWN_PROTOCOL_VERSION),
    m_curr_session_id(DEFAULT_SESSION_ID),
    m_nIDEvent(NULL)
    //m_pListener(NULL)
{
}

bool KBCommService::Start()
{
    m_audioSessionProvider.RefreshSessions();

    on_added_device_callback_entry on_added_callback = {
        .on_added_device = on_added_monenumpad,
        .user_data = NULL
    };
    hid_device_info* dev_info = hid_enumerate_ex(
        MONENUMPAD_VENDOR_ID,
        MONENUMPAD_PRODUCT_ID,
        MONENUMPAD_USAGE_PAGE,
        MONENUMPAD_USAGE,
        on_added_callback);
    if (dev_info)
    {
        OpenMoneNumPad(dev_info);
    }

    return true;
}

void KBCommService::Stop()
{
    CloseMoneNumPad();
    hid_stop_enumerate_on_added_device_callback();
}

void KBCommService::OpenMoneNumPad(hid_device_info* dev_info)
{
    if (m_pConnectedDevice != NULL) {

        // disconnect previously connected device before
        // making new connection
        CloseMoneNumPad();
    }

    m_pConnectedDevice = new HIDDevice();
    if (m_pConnectedDevice)
    {
        try
        {
            m_pConnectedDevice->Open(dev_info);
            m_pConnectedDevice->SetListener(this);
            this->SendProtocolVersion();
        }
        catch (HRESULT hr_)
        {
            ATLTRACE(L"KBCommService::OpenMoneNumPad failed. hr=%08x", hr_);
            delete m_pConnectedDevice;
            m_pConnectedDevice = NULL;
        }
    }

}

void KBCommService::CloseMoneNumPad()
{
    StopRefreshingSessionInfoTimer();

    if (m_pConnectedDevice)
    {
        try
        {
            m_pConnectedDevice->SetListener(NULL);
            m_pConnectedDevice->Close();
        }
        catch (HRESULT hr_)
        {
            ATLTRACE(L"KBCommService::CloseMoneNumPad failed. hr=%08x", hr_);
        }
        delete m_pConnectedDevice;
        m_pConnectedDevice = NULL;
    }

    m_curr_session_id = DEFAULT_SESSION_ID;
    m_kb_protocol_version = UNKNOWN_PROTOCOL_VERSION;

}

// AudioSessionProviderListener impl
void KBCommService::OnAudioSessionsRefreshed()
{
    if (!IsConnected())
    {
        return;
    }

    const std::vector<AudioSession>& newAudioSessions = m_audioSessionProvider.GetSessions();
    
    // check if current session no longer exists
    if (IsNull(m_audioSessionProvider.GetSessionByID(m_curr_session_id)))
    {
        // reset audio session to default Output
        this->m_curr_session_id = DEFAULT_SESSION_ID;
    }

    SendSessionInfo();
    SendSessionDatas();
}

//
// -- Internal Methods --
//
// 

//every 2 sec, send session_info
//session_info also serve as a heartbeat
void KBCommService::StartRefreshingSessionInfoTimer()
{
    WMAPPMessageHandler handler = {
        .id = (DWORD_PTR)this, //handlerID
        .handler = [this](UINT message, WPARAM wparam, LPARAM lparam) -> bool {
            if (message == WM_TIMER)
            {
                if (this->m_nIDEvent == wparam)
                {
                    this->OnTimedEvent();
                    return true;
                }
            }

            return false;
        }
    };

    AddWMAPPMessageHander(handler);

    if (m_nIDEvent == NULL)
    {
        //send sessionInfo as heartbeat to keyboard every 2 sec
        m_nIDEvent = (UINT_PTR)this;
        SetTimer(GetAppHWND(), m_nIDEvent, 2000, NULL);
    }
}

void KBCommService::StopRefreshingSessionInfoTimer()
{
    if (m_nIDEvent)
    {
        KillTimer(GetAppHWND(), m_nIDEvent);
        m_nIDEvent = NULL;
    }

    DWORD_PTR handlerID = (DWORD_PTR)this;
    RemoveWMAPPMessageHanderByID(handlerID);
}



void KBCommService::OnTimedEvent()
{
    if (m_nIDEvent != NULL)
    {
        SendSessionInfo();
    }
}

void KBCommService::SendProtocolVersion()
{
    SendUInt16Message(PROTOCOL_VERSION_EXCHANGE, MAXMIX_PROTOCOL_VERSION);
}

void KBCommService::SendSessionInfo()
{
    uint8_t session_count = (uint8_t)min(_UI8_MAX, m_audioSessionProvider.GetSessions().size());
    SessionInfo session_info = {
        .count = session_count
    };        
    SendStructMessage(SESSION_INFO, session_info);
}

//returns -1 if there is no previous session
int KBCommService::PrevSessionIndex(int sessionIndex)
{
    if (sessionIndex > 0)
    {
        int prevIndex = sessionIndex - 1;
        if (m_audioSessionProvider.GetSessions().size() > prevIndex)
        {
            return prevIndex;
        }
    }

    return -1;
}

// return -1 if there is no next session
int KBCommService::NextSessionIndex(int sessionIndex)
{
    int nextIndex = sessionIndex + 1;
    if (m_audioSessionProvider.GetSessions().size() > nextIndex)
    {
        return nextIndex;
    }

    return -1;
}

bool IsNull(const SessionData& sessionData)
{
    return (sessionData.id == 0);
}

SessionData KBCommService::MakeSessionData(
    __in  int sessionIndex)
{
    const std::vector<AudioSession>& sessions = m_audioSessionProvider.GetSessions();
    
    AudioSession audioSession = NullAudioSession;
    if (sessions.size() > 0)
    {
        if (sessionIndex >= sessions.size())
        {
            audioSession = *(sessions.rbegin());
        }
        else if (sessionIndex >= 0)
        {
            audioSession = sessions[sessionIndex];
        }
        else 
        {
            audioSession = sessions[0];
        }
    }

    if (!IsNull(audioSession))
    {
        int vol = max(0, (int)(audioSession.pMMSession->GetVolume() * MAX_VOLUME));
        vol = min(MAX_VOLUME, vol);
        bool muted = audioSession.pMMSession->IsMute();

        SessionData sessionData = {
            .id = audioSession.id,
            .name = {0},
            .has_prev = PrevSessionIndex(sessionIndex) != -1,
            .has_next = NextSessionIndex(sessionIndex) != -1,
            .volume = {
                0,
                (muted ? (uint8_t)1 : (uint8_t)0),
                (uint8_t)vol
                }
        };

        USES_CONVERSION;
        std::wstring sSessionName = audioSession.pMMSession->GetDisplayName();
        char* pszSessionName = CW2A(sSessionName.c_str());
        if (pszSessionName)
        {
            strncpy_s(sessionData.name, pszSessionName, SessionData_Name_Size - 1);
        }

        return sessionData;
    }
    else
    {
        VolumeData v = {
            .unknown = (true),
            .isMuted = (false),
            .volume = (0)
        };
        SessionData o = {
            .id = (SESSION_ID_NULL),
            .name = {0},
            .has_prev = false,
            .has_next = false,
            .volume = v
        };
        return o; //makeSessionData(); //return an empty SessionData
    }
}


void KBCommService::SendSessionDatas(bool curr_only /*= false*/)
{
    const std::vector<AudioSession>& audioSessions = this->m_audioSessionProvider.GetSessions();

    auto sessionIter = std::find_if(
        audioSessions.begin(), 
        audioSessions.end(), 
        [this](const AudioSession& audioSession) -> bool {
            return audioSession.id == this->m_curr_session_id;
        }
    );

    int curr_session_index = 0;
    if (sessionIter != audioSessions.end())
    {
        curr_session_index = (int)(audioSessions.end() - sessionIter);
    }

    bool bSucceeded = false;
    SessionData curr_session_data = MakeSessionData(curr_session_index);
    {
        USES_CONVERSION;
        ATLTRACE(TEXT("sendSessionData(curr = \"%s\"). has_prev=%d. has_next=%d."), static_cast<LPCWSTR>(CA2W(curr_session_data.name)),
            curr_session_data.has_prev, curr_session_data.has_next);
    }
    SendStructMessage(CURRENT_SESSION, curr_session_data);

    if (!curr_only)
    {
        int prev_session_index = PrevSessionIndex(curr_session_index);
        if (prev_session_index != -1)
        {
            SessionData prev_session_data = MakeSessionData(prev_session_index);
            if (!IsNull(prev_session_data))
            {
                {
                    USES_CONVERSION;
                    ATLTRACE(TEXT("sendSessionData(prev = \"%s\")"), static_cast<LPCWSTR>(CA2W(prev_session_data.name)));
                }
                SendStructMessage(PREVIOUS_SESSION, prev_session_data);
            }
        }

        int next_session_index = NextSessionIndex(curr_session_index);
        if (next_session_index != -1)
        {
            SessionData next_session_data = MakeSessionData(next_session_index);
            if (!IsNull(next_session_data))
            {
                {
                    USES_CONVERSION;
                    ATLTRACE(TEXT("sendSessionData(next = \"%s\")"), static_cast<LPCWSTR>(CA2W(next_session_data.name)));
                }
                SendStructMessage(NEXT_SESSION, next_session_data);
            }
        }
    } // !curr_only
}

template<typename StructDataType>
void KBCommService::SendStructMessage(
    Command command,
    StructDataType msgData)
{
    BYTE* data = (BYTE*)&msgData;
    size_t cbData = sizeof(msgData);
    SendMessage(command, data, cbData);
}

void KBCommService::SendUInt16Message(
    Command command,
    UINT16 n)
{    
    BYTE b0 = (byte)(n >> 8);
    BYTE b1 = (byte)(n & 0xFF);
    // network order (big-endian)
    byte data[2] = { b0, b1 };
    size_t cbData = sizeof(data);
    SendMessage(command, data, cbData);
}


void KBCommService::SendMessage(
    Command command,
    BYTE* data,
    size_t cbData)
{
    //if (!Thread.isMainThread)
    //{
    //    DispatchQueue.main.async {
    //        this.sendMessage(command: command,
    //                                data: msgData,
    //                                serializer: serializer)
    //                    };
    //    return;
    //}
    //
    //assert(Thread.isMainThread)

    if (m_pConnectedDevice == NULL)
    {
        return;
    }

    if (command != SESSION_INFO)
    {
        ATLTRACE(TEXT("Sending cmd=%d"), (int)command);
    }

    // 1 : MSG_ID_PREFIX 0xFD
    // 1 : command id
    // data

    size_t cbMsgBytes = static_cast<size_t>(1) + 1 + cbData;
    BYTE* msgBytes = (BYTE*)malloc(cbMsgBytes);
    if (msgBytes)
    {
#pragma warning(push)
#pragma warning(disable: 6386)
        msgBytes[0] = MSG_ID_PREFIX;
        msgBytes[1] = (byte)command;
        memcpy(msgBytes + 2, data, cbData);
#pragma warning(pop)

        try
        {
            m_pConnectedDevice->Write(msgBytes, cbMsgBytes);
        }
        catch (HRESULT hr_)
        {
            ATLTRACE(L"m_pConnectedDevice->Write failed. hr=%08x", hr_);
        }

        free(msgBytes);
    }
}


void KBCommService::HandleDeviceDataReceived(BYTE* data, size_t cbData)
{
    // check if the first byte == MSG_ID_PREFIX
    BYTE msgPrefix = data[0];
    if (msgPrefix != MSG_ID_PREFIX) 
    {
        return;
    }

    BYTE command_id = data[1];
    if (command_id == CMD_OK)
    {
        return;
    }

    ATLTRACE(TEXT("Device_DataReceived(%d)"), (int)command_id);

    BYTE* msgData = data + 2;
    if (command_id == PROTOCOL_VERSION_EXCHANGE)
    {
        m_kb_protocol_version = ToUInt16(msgData);
        if (m_kb_protocol_version != MAXMIX_PROTOCOL_VERSION)
        {
            ATLTRACE(TEXT("Incompatible protocol version. Close device connection."));
            CloseMoneNumPad();
        }
        else
        {
            SendSessionInfo();
            SendSessionDatas();
            StartRefreshingSessionInfoTimer();
        }
    }
    else if (IsProtocolCompatible())
    {
        byte new_curr_session_id = this->m_curr_session_id;
        bool volChanged = false;

        bool msgRead = true;
        switch (command_id)
        {
        case CURRENT_SESSION_CHANGED:
        {
            new_curr_session_id = *msgData;
            break;
        }
        case VOLUME_UP:
        {
            new_curr_session_id = *msgData;
            AudioSession curr_session = m_audioSessionProvider.GetSessionByID(new_curr_session_id);
            if (!IsNull(curr_session))
            {
                float new_vol = min(MAX_VOLUME, curr_session.pMMSession->GetVolume() + ((float)5.0/ MAX_VOLUME));
                curr_session.pMMSession->SetVolume(new_vol);
                volChanged = true;
            }
            break;
        }
        case VOLUME_DOWN:
        {
            new_curr_session_id = *msgData;
            AudioSession curr_session = m_audioSessionProvider.GetSessionByID(new_curr_session_id);
            if (!IsNull(curr_session))
            {
                float new_vol = min(MAX_VOLUME, curr_session.pMMSession->GetVolume() - ((float)5.0 / MAX_VOLUME));
                curr_session.pMMSession->SetVolume(new_vol);
                volChanged = true;
            }
            break;
        }
        case TOGGLE_MUTE:
        {
            new_curr_session_id = *msgData;
            AudioSession curr_session = m_audioSessionProvider.GetSessionByID(new_curr_session_id);
            if (!IsNull(curr_session))
            {
                curr_session.pMMSession->SetMute(!curr_session.pMMSession->IsMute());
                volChanged = true;
            }
            break;
        }
        case CMD_OK:
        case CMD_ERR:
            break;

        default:
            msgRead = false;
            break;
        }

        if (msgRead)
        {
            if (command_id != CMD_OK)
            {
                ATLTRACE(TEXT("Reading cmd=%d curr_session_id =%d"),
                    (int)command_id, (int)new_curr_session_id);
            }

            if (new_curr_session_id != this->m_curr_session_id)
            {
                this->m_curr_session_id = new_curr_session_id;

                // curr_session_id is changed, so send update
                // SessionData to keyboard
                SendSessionDatas();
            }
            else if (volChanged)
            {
                SendSessionDatas(true);
            }
        }
        else // !msgRead
        {
            ATLTRACE("Unknown message cmd=%d", (int)command_id);
        }

    } // else if is_protocol_compatible()                    
    else
    {
        std::wstring data_str = ToString(data, cbData);
        ATLTRACE("Reading unrecognized data=\"%s\"", data_str.c_str());
    }
} // void deviceDataRead


//HIDDeviceListener Impl
void KBCommService::DeviceDataReceived(HIDDevice* dev, BYTE* data, size_t cbData)
{
    if (m_pConnectedDevice == dev)
    {
        this->HandleDeviceDataReceived(data, cbData);
    }
}

void KBCommService::DeviceDisconnected(HIDDevice* dev)
{
    if (m_pConnectedDevice == dev)
    {
        ATLTRACE(TEXT("Device is disconnected."));
        this->CloseMoneNumPad();
    }
}


// util functions
std::wstring ToString(BYTE* data, size_t cbData)
{
    std::wstring result = L"0b";

    const int bitGroupSize = 4;
    const int byteGroupSize = 4;

    for (size_t i = 0; i < cbData; ++i)
    {
        BYTE byte = data[i];
        if (i > 0 && (i % byteGroupSize) == 0)
        {
            result += L"\n";
        }

        BYTE bitMask = 0b1000000;
        for (int j = 0; j < 8; ++j)
        {
            bool value = (byte & bitMask) != 0;
            bitMask = bitMask >> 1;

            size_t bitPos = (i * 8) + j;
            if (bitPos > 0 && (bitPos % bitGroupSize == 0))
            {
                result += L" ";
            }

            result += (value ? L"1" : L"0");

        }
    }

    return result;
}



