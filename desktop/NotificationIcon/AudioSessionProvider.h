#pragma once

#include <list>
#include "CMMDevice.h"
#include "CMMDeviceController.h"
#include <map>
#include "monenumpad_maxmix/structs.h"


struct AudioSession
{
	uint8_t id; // session id to communicate with monenum_pad
	CMMSession* pMMSession;
};

static const AudioSession NullAudioSession = { 0 };
bool IsNull(const AudioSession& audioSession);

class AudioSessionProvider: CMMDeviceControllerListener
{
protected:
	CMMDeviceController* m_mmdeviceController;
	uint8_t m_nextNumPadID;
	std::map<std::wstring, uint8_t> m_mmSessionIDToNumPadSessionIDMap;
	std::vector<AudioSession> m_sessions;

public:
	AudioSessionProvider(CMMDeviceController* mmdeviceController)
	{
		m_mmdeviceController = mmdeviceController;
		m_nextNumPadID = SESSION_ID_APP_FIRST;
	}

	const std::vector<AudioSession>& GetSessions() const {
		return m_sessions;
	}

	// returns NullAudioSession if not found
	const AudioSession& GetSessionByID(uint8_t idToFind) const {
		auto iterFound = std::find_if(
			m_sessions.begin(), 
			m_sessions.end(),
			[idToFind](const AudioSession& audioSession) {
				return audioSession.id == idToFind;
			}
		);

		if (iterFound != m_sessions.end())
		{
			return *iterFound;
		}
		else
		{
			return NullAudioSession;
		}
	}

	void RefreshSessions();

	// impl CMMDeviceControllerListener
	virtual void OnAudioSessionsRefreshed()
	{
		RefreshSessions();
	}

protected:
	uint8_t AppSessionIDForMMSession(const CMMSession* pMMSession);
	void UpdateSessionIDsMap();

	AudioSession CreateAudioSession(CMMSession* pMMSession, uint8_t id);
};

