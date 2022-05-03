using System;
namespace monenumpad_desktop.Maxmix
{
    public struct AudioSession
    {
        public byte id;
        public String name;

        public AudioSession(byte id, string name)
        {
            this.id = id;
            this.name = name;
        }
    }

    public interface IAudioSessionProvider
    {
        AudioSession[] sessions { get; }
        int getSessionVolume(byte id);
        void setSessionVolume(byte id, int vol);
        bool isSessionMuted(byte id);
        bool toggleMuteSession(byte id);
    }
}

