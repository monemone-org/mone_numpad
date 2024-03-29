#pragma once

#include <list>
#include "CMMDevice.h"
#include "CMMDeviceController.h"
#include <map>
#include "monenumpad_maxmix/structs.h"


struct AudioSession
{
	uint8_t id; // session id to communicate with monenum_pad
	std::string name;
	IMMVolumeControl *pVolControl;

	void dump() const;
};

extern const AudioSession NullAudioSession;

bool IsNull(const AudioSession& audioSession);

class AudioSessionProviderListener
{
public:
	virtual void OnAudioSessionsRefreshed() = 0;
};


class AudioSessionProvider: CMMDeviceControllerListener
{
protected:
	CMMDeviceController* m_mmdeviceController;
	uint8_t m_nextNumPadID;
	std::map<std::wstring, uint8_t> m_mmVolControlIDToNumPadSessionIDMap;
	std::vector<AudioSession> m_sessions;
	AudioSessionProviderListener* m_pListener;

public:
	AudioSessionProvider(CMMDeviceController* mmdeviceController):
		m_mmdeviceController(mmdeviceController),
		m_pListener(NULL),
		m_nextNumPadID(SESSION_ID_APP_FIRST)
	{
		m_mmdeviceController->AddListener(this);
	}
	~AudioSessionProvider()
	{
		m_mmdeviceController->RemoveListener(this);
	}


	void SetListener(AudioSessionProviderListener* pListener)
	{
		m_pListener = pListener;
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
	virtual void OnMMSessionsAddedRemoved()
	{
		RefreshSessions();
	}

	void dump() const;

protected:
	uint8_t AppSessionIDForMMSession(IMMVolumeControl *pVolControl);
	void UpdateSessionIDsMap();

	AudioSession CreateAudioSession(IMMVolumeControl *pVolControl, LPCSTR pszName, uint8_t id);
};

