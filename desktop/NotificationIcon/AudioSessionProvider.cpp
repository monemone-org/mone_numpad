#include "framework.h"
#include "AudioSessionProvider.h"
#include "CMMDeviceController.h"
#include "CMMSession.h"
#include "monenumpad_maxmix/structs.h"

const AudioSession NullAudioSession = { 0 };

bool IsNull(const AudioSession& audioSession)
{
	return audioSession.id == 0;
}

void AudioSession::dump() const
{
	ATLTRACE(TEXT("<-- Dump AudioSession\n"));
	ATLTRACE(TEXT("        id: \"%d\"\n"), (this->id));
	ATLTRACE(TEXT("        name: \"%s\"\n"), this->pVolControl->GetDisplayName());
	ATLTRACE(TEXT("-->\n"));
}

/*
* class AudioSessionProvider
*	provide monenumpad with a collection of audio sessions
*/
void AudioSessionProvider::RefreshSessions()
{
	m_sessions.clear();

	CMMDevice *pOutDevice = m_mmdeviceController->GetDefaultOut();
	const std::list<CMMSession*> defaultOutSessions = pOutDevice->GetSessions();

	CMMDevice* pInDevice = m_mmdeviceController->GetDefaultIn();
	const std::list<CMMSession*> defaultInSessions = pInDevice->GetSessions();

	auto addSystemInOutFunc = [this](CMMDevice* pMMDevice)
	{
		LPCSTR name = NULL;
		uint8_t id = 0;
		CW2A aName = pMMDevice->GetDisplayName();
		if (pMMDevice->IsInput())
		{
			id = SESSION_ID_IN;
		}
		else
		{
			id = SESSION_ID_OUT;
		}
		m_sessions.push_back(CreateAudioSession(pMMDevice, aName, id));
	};

	auto addAppsInOutFunc = [this](const std::list<CMMSession*> sessions)
	{
		for (auto iter = sessions.begin(); iter != sessions.end(); iter++)
		{
			CMMSession* pMMSession = *iter;
			if (!pMMSession->IsSystemInOut())
			{
				uint8_t id = AppSessionIDForMMSession(pMMSession);
				CW2A aName = pMMSession->GetDisplayName();
				m_sessions.push_back(CreateAudioSession(pMMSession, aName, id));
			}
		}
	};

	//return sessions in the following order
	//[ 
	//	default out
	//	default in
	//  output apps ...
	//  //input apps ...
	//]
	addSystemInOutFunc(pOutDevice);
	addSystemInOutFunc(pInDevice);
	addAppsInOutFunc(defaultOutSessions);
	//addAppsInOutFunc(defaultInSessions);

	//clean up m_mmSessionIDToNumPadSessionIDMap
	UpdateSessionIDsMap();

	if (m_pListener)
	{
		m_pListener->OnAudioSessionsRefreshed();
	}
}

AudioSession AudioSessionProvider::CreateAudioSession(IMMVolumeControl *pVolControl, LPCSTR pszName, uint8_t id)
{
	AudioSession audioSession =
	{
		.id = id,
		.name = pszName,
		.pVolControl = pVolControl
	};
	return audioSession;
}

void AudioSessionProvider::UpdateSessionIDsMap()
{
	// clean up m_mmSessionIDToNumPadSessionIDMap
	// for sessions that no longer exists
	std::map<std::wstring, uint8_t> newMap;

	for (auto iter = m_sessions.begin();
		iter != m_sessions.end();
		++iter)
	{
		const AudioSession& audioSession = *iter;
		newMap[audioSession.pVolControl->GetVolControlID()] = audioSession.id;
	}

	m_mmVolControlIDToNumPadSessionIDMap = newMap;
}

uint8_t AudioSessionProvider::AppSessionIDForMMSession(IMMVolumeControl *pVolControl)
{
	uint8_t sessionID;

	std::wstring volControlID = pVolControl->GetVolControlID();

	auto iter = m_mmVolControlIDToNumPadSessionIDMap.find(volControlID);
	if (iter == m_mmVolControlIDToNumPadSessionIDMap.end())
	{
		// assign new uint8_t session ID 
		sessionID = this->m_nextNumPadID;
		m_mmVolControlIDToNumPadSessionIDMap[volControlID] = sessionID;
		this->m_nextNumPadID += 1;
		if (this->m_nextNumPadID == 0)
		{
			this->m_nextNumPadID = SESSION_ID_APP_FIRST; //overflow back to SESSION_ID_APP_FIRST
		}
	}
	else
	{
		sessionID = iter->second;
	}

	return sessionID;
}

void AudioSessionProvider::dump() const
{
	ATLTRACE(L"<-- Dump AudioSessionProvider\n");
	for (auto sessionIter = m_sessions.begin();
		sessionIter != m_sessions.end();
		++sessionIter)
	{
		sessionIter->dump();
	}
	ATLTRACE(L"-->\n");
}

