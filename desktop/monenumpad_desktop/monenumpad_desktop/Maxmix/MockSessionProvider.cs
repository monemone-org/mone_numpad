using System;
using System.Collections.Generic;
using monenumpad_desktop.Maxmix.structs;

namespace monenumpad_desktop.Maxmix
{
    public class MockSessionProvider : IAudioSessionProvider
    {
        public MockSessionProvider()
        {
        }

        private AudioSession[] _sessions = new AudioSession[]{
            new AudioSession(id: Constants.SESSION_ID_OUT,
                         name: Constants.SESSION_NAME_OUT),
            new AudioSession(id: Constants.SESSION_ID_IN,
                         name: Constants.SESSION_NAME_IN),
            new AudioSession(id: Constants.SESSION_ID_APP_FIRST,
                         name: "Game"),
            new AudioSession(id: (Constants.SESSION_ID_APP_FIRST + 1),
                         name: "Music"),
            new AudioSession(id: (Constants.SESSION_ID_APP_FIRST + 2),
                         name: "Discord")
        };

        AudioSession[] IAudioSessionProvider.sessions => _sessions;

        struct VolumeState {
            public int volume; // 0 to 100
            public bool isMuted;

            public VolumeState(int volume, bool isMuted)
            {
                this.volume = volume;
                this.isMuted = isMuted;
            }
        }

        private Dictionary<byte, VolumeState> _sessionsValueStates = new Dictionary<byte, VolumeState> ();

        private VolumeState GetVolumeState(byte id)
        {
            if (_sessionsValueStates.ContainsKey(id))
            {
                return _sessionsValueStates[id];
            }
            else
            {
                return new VolumeState(volume: 50, isMuted: false);
            }
        }

        private void SetVolumeState(byte id, VolumeState volumeState)
        {
            _sessionsValueStates[id] = volumeState;
        }

        int IAudioSessionProvider.getSessionVolume(byte id)
        {
            return GetVolumeState(id).volume;
        }

        void IAudioSessionProvider.setSessionVolume(byte id, int vol)
        {
            var volState = GetVolumeState(id);
            volState.volume = vol;
            SetVolumeState(id, volState);
        }

        bool IAudioSessionProvider.isSessionMuted(byte id)
        {
            var volState = GetVolumeState(id);
            return volState.isMuted;
        }

        bool IAudioSessionProvider.toggleMuteSession(byte id)
        {
            var volState = GetVolumeState(id);
            volState.isMuted = !volState.isMuted;
            SetVolumeState(id, volState);
            return volState.isMuted;
        }
    }
}
