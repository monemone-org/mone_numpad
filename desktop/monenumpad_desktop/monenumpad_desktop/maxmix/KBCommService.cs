using System;
using System.Timers;
using hidapi;
using monenumpad_desktop.Marshal;
using monenumpad_desktop.Maxmix.structs;

namespace monenumpad_desktop.Maxmix
{

    public interface IKBCommService
    { }

    public class KBCommService : IKBCommService
    {
        private static SessionData dummySessionData = new SessionData(
                                structs.Constants.SESSION_ID_NULL,
                                " ",
                                false,
                                false,
                                new VolumeData(false, false, 50));

        private IAudioSessionProvider sessionProvider;

        private hidapi.Device _connectedDevice;
        public hidapi.Device connectedDevice => _connectedDevice;

        static private byte default_session_id = structs.Constants.SESSION_ID_OUT;

        // keyboard firmware protocoal version.
        // if it's not compatible to ours, we don't send it messages
        private UInt16 kb_protocol_version = structs.Constants.UNKNOWN_PROTOCOL_VERSION; // unknown

        // current audio session ID
        private byte curr_session_id = default_session_id; //default is Out session

        private Timer sendSessionInfoTimer;


        public delegate void KBCommServiceNotifyDelegate(KBCommService kBCommService);
        public event KBCommServiceNotifyDelegate DeviceDisconnected;

        private class OutputOnlySessionProvider : IAudioSessionProvider
        {
            private AudioSession[] _sessions = new AudioSession[] {
                new AudioSession(id: KBCommService.dummySessionData.id,
                                 name: KBCommService.dummySessionData.name)
                
            };

            private int vol = 50;
            private bool isMuted = false;

            AudioSession[] IAudioSessionProvider.sessions => _sessions;

            public int getSessionVolume(byte id)
            {
                return vol;
            }

            public void setSessionVolume(byte id, int vol)
            {
                this.vol = vol;
            }

            public bool isSessionMuted(byte id)
            {
                return this.isMuted;
            }

            public bool toggleMuteSession(byte id)
            {
                this.isMuted = !this.isMuted;
                return this.isMuted;
            }
        }

        public KBCommService(IAudioSessionProvider sessionProvider)
        {
            this.sessionProvider = sessionProvider ?? new OutputOnlySessionProvider();
        }

        //
        // MARK: -- Public Methods --
        //
        public void connect(hidapi.hid_device_info dev)
        {
            if (this.connectedDevice != null) {

                if (this.connectedDevice.id.Value == dev.device_id)
                {
                    return; //already connected
                }

                // disconnect previously connected device before
                // making new connection
                this.disconnect();
            }

            this._connectedDevice = new hidapi.Device();
            this._connectedDevice.Open(dev);
            this._connectedDevice.DataReceived += Device_DataReceived;
            this._connectedDevice.Disconnected += Device_Disconnected;
            this.sendProtocolVersion();
        }

        public void disconnect()
        {
            this.stopRefreshingSessionInfoTimer();
            if (this.connectedDevice != null)
            {
                var device = this.connectedDevice;
                this._connectedDevice.Close();
                this._connectedDevice = null;
                this.DeviceDisconnected?.Invoke(this);
            }
        }

        public bool isConnected {
            get {
                return this._connectedDevice != null;
            }
        }

        public void onAudioSessionsChanged()
        {
            var newAudioSessions = this.sessionProvider.sessions;

            // check if current session no longer exists
            bool currSessionAlive = Array.Exists(newAudioSessions, session => session.id == this.curr_session_id);

            if (!currSessionAlive)
            {
                // reset audio session to default Output
                curr_session_id = KBCommService.default_session_id;
            }

            this.sendSessionInfo();
            this.sendSessionDatas();
        }

        //
        // MARK: -- Internal Methods --
        //
        private bool is_protocol_compatible()
        {
            return this.kb_protocol_version == structs.Constants.MAXMIX_PROTOCOL_VERSION;
        }

        //every 2 sec, send session_info
        //session_info also serve as a heartbeat
        private void startRefreshingSessionInfoTimer()
        {
            if (this.sendSessionInfoTimer == null)
            {
                //send sessionInfo as heartbeat to keyboard every 2 sec
                this.sendSessionInfoTimer = new Timer(2000);
                this.sendSessionInfoTimer.Elapsed += OnTimedEvent;
                this.sendSessionInfoTimer.AutoReset = false;
                this.sendSessionInfoTimer.Enabled = true;
            }
        }

        private void OnTimedEvent(Object source, ElapsedEventArgs e)
        {
            // accessing sendSessionInfoTimer needs to be thread safe
            // because OnTimedEvent is called on a different thread.
            Timer timer = null;
            lock (this)
            {
                timer = this.sendSessionInfoTimer;
            }

            if (timer != null)
            {
                this.sendSessionInfo();
                timer.Enabled = true;
            }
        }

        private void stopRefreshingSessionInfoTimer()
        {
            // accessing sendSessionInfoTimer needs to be thread safe
            // because OnTimedEvent is called on a different thread.
            Timer timer = null;
            lock (this)
            {
                timer = this.sendSessionInfoTimer;
                this.sendSessionInfoTimer = null;
            }

            if (timer != null)
            {
                timer.Stop();
                timer.Dispose();
            }
        }

        private void sendProtocolVersion()
        {
            this.sendUInt16Message(Command.PROTOCOL_VERSION_EXCHANGE, structs.Constants.MAXMIX_PROTOCOL_VERSION);
        }

        private void sendSessionInfo()
        {
            var session_count = Math.Min(Byte.MaxValue, (byte)this.sessionProvider.sessions.Length);
            var session_info = new SessionInfo(session_count);
            this.sendStructMessage(Command.SESSION_INFO, session_info);
        }

        private int? prevSessionIndex(int sessionIndex)
        {
            if (sessionIndex > 0)
            {
                var prevIndex = sessionIndex - 1;
                if (this.sessionProvider.sessions.Length > prevIndex)
                {
                    return prevIndex;
                }
            }

            return null;
        }

        private int? nextSessionIndex(int sessionIndex)
        {
            var nextIndex = sessionIndex + 1;
            if (this.sessionProvider.sessions.Length > nextIndex)
            {
                return nextIndex;
            }

            return null;
        }

        private SessionData? makeSessionData(int sessionIndex)
        {
            AudioSession? session = null;
            if (sessionIndex >= this.sessionProvider.sessions.Length)
            {
                session = this.sessionProvider.sessions.Last();
            }
            else if (sessionIndex >= 0)
            {
                session = this.sessionProvider.sessions[sessionIndex];
            }
            else
            {
                session = this.sessionProvider.sessions.First();
            }

            if (session is AudioSession session1)
            {
                int vol = Math.Max(0, this.sessionProvider.getSessionVolume(session1.id));
                vol = Math.Min(100, vol);
                bool muted = this.sessionProvider.isSessionMuted(session1.id);

                var sessionData = new SessionData(
                                          session1.id,
                                          session1.name,
                                          prevSessionIndex(sessionIndex) != null,
                                          nextSessionIndex(sessionIndex) != null,
                                          new VolumeData(false, muted, (byte)vol));
                return sessionData;
            }
            else
            {
                return null;
            }

        }


        public void sendSessionDatas(bool curr_only = false)
        {
            var curr_session_index = Array.FindIndex(this.sessionProvider.sessions, s => s.id == this.curr_session_id);
            if (curr_session_index == -1)
            {
                curr_session_index = 0;
            }

            SessionData curr_session_data = makeSessionData(curr_session_index) ?? KBCommService.dummySessionData;
            this.log("sendSessionData(curr = {0}). has_prev={1}. has_next={2}.", curr_session_data.name,
                curr_session_data.has_prev, curr_session_data.has_next);
            this.sendStructMessage(Command.CURRENT_SESSION, curr_session_data);
            //this.log(curr_session_data.ToBitVector().ToString());

            if (!curr_only)
            {
                var prev_session_index_nullable = prevSessionIndex(curr_session_index);
                if (prev_session_index_nullable is int prev_session_index)
                {
                    var prev_session_data_nullable = makeSessionData(prev_session_index);
                    if (prev_session_data_nullable is SessionData prev_session_data)
                    {
                        this.log("sendSessionData(prev = {0})", prev_session_data.name);
                        this.sendStructMessage(Command.PREVIOUS_SESSION, prev_session_data);
                    }
                }

                var next_session_index_nullable = nextSessionIndex(curr_session_index);
                if (next_session_index_nullable is int next_session_index)
                {
                    var next_session_data_nullable = makeSessionData(next_session_index);
                    if (next_session_data_nullable is SessionData next_session_data)
                    {
                        this.log("sendSessionData(next = {0})", next_session_data.name);
                        this.sendStructMessage(Command.NEXT_SESSION, next_session_data);
                    }

                }
            } // !curr_only
        }

        private void sendStructMessage<StructDataType>(
            Command command,
            StructDataType msgData) where StructDataType : StructMarshallable
        {
            var bitVector = msgData.ToBitVector();
            var data = bitVector.ToBytes();
            sendMessage(command: command, data: data);
        }

        private void sendUInt16Message(
            Command command,
            UInt16 n)
        {
            byte[] data = FieldMarshalFunc.FromBigEndianUInt16(n);
            sendMessage(command, data);
        }

        private void sendMessage(
            Command command,
            byte[] data)
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

            if (this.connectedDevice == null)
            {
                return;
            }

            if (command != Command.SESSION_INFO)
            {
                this.log("Sending cmd={0}", command);
            }

            // 1 : MSG_ID_PREFIX 0xFD
            // 1 : command id
            // data
            var msgBytes = new byte[1 + 1 + data.Length];
            msgBytes[0] = structs.Constants.MSG_ID_PREFIX;
            msgBytes[1] = (byte)command;
            data.CopyTo(msgBytes, 2);

            this.connectedDevice.Write(msgBytes, msgBytes.Length);
        }

        void log(string format, params object[] args)
        {
            //TODO
            var msg = String.Format(format, args);
            Console.WriteLine(msg);
        }


        public enum FieldIndex
        {
            MsgIDPrefix, //a byte
            MsgCommand, //a byte
            MsgData
        };
        public readonly static PackedStructLayout<FieldIndex> Layout;

        static KBCommService()
        {
            Layout = new PackedStructLayout<FieldIndex>(new FieldDesc[]{
                        new FieldDesc("MsgIDPrefix", 8), // in bits, 
                        new FieldDesc("MsgCommand", 8), //in bits
                        new FieldDesc("MsgData", 0) //in bits, 0 because we don't know how big the msg data body is
                    });
        }

        //
        // MARK: -- hidapi.Device EventHandler --
        //
        public void Device_Disconnected(object sender, EventArgs e)
        {
            this.log("Device is disconnected.");
            this.disconnect();
        }

        public void Device_DataReceived(object sender, hidapi.DataEventArgs e)
        {
            try
            {
                var data = e.Data;
                var bitVector = new BitVector(data);

                // check if the first byte == MSG_ID_PREFIX
                var msgPrefix = Layout.GetFieldValue(bitVector, FieldIndex.MsgIDPrefix, FieldMarshalFunc.ToByte);
                if (msgPrefix != structs.Constants.MSG_ID_PREFIX) {
                    return;
                }

                var command_id = (Command)Layout.GetFieldValue(bitVector, FieldIndex.MsgCommand, FieldMarshalFunc.ToByte);
                if (command_id == Command.CMD_OK)
                {
                    return;
                }

                this.log("Device_DataReceived({0})", command_id.ToString());

                if (command_id == Command.PROTOCOL_VERSION_EXCHANGE)
                {
                    this.kb_protocol_version = Layout.GetFieldValue(bitVector, FieldIndex.MsgData, FieldMarshalFunc.ToBigEndianUInt16);
                    if (this.kb_protocol_version != structs.Constants.MAXMIX_PROTOCOL_VERSION)
                    {
                        this.log("Incompatible protocol version. Disconnect.");
                        this.disconnect();
                    }
                    else
                    {
                        this.sendSessionInfo();
                        this.sendSessionDatas();
                        this.startRefreshingSessionInfoTimer();
                    }
                }
                else if (is_protocol_compatible())
                {
                    byte new_curr_session_id = this.curr_session_id;
                    bool volChanged = false;

                    bool msgRead = true;
                    switch (command_id)
                    {
                        case Command.CURRENT_SESSION_CHANGED:
                            {
                                new_curr_session_id = Layout.GetFieldValue(bitVector, FieldIndex.MsgData, FieldMarshalFunc.ToByte);
                                break;
                            }
                        case Command.VOLUME_UP:
                            {
                                new_curr_session_id = Layout.GetFieldValue(bitVector, FieldIndex.MsgData, FieldMarshalFunc.ToByte);
                                int new_vol = Math.Min(100, this.sessionProvider.getSessionVolume(new_curr_session_id) + 5);
                                this.sessionProvider.setSessionVolume(new_curr_session_id, new_vol);
                                volChanged = true;
                                break;
                            }
                        case Command.VOLUME_DOWN:
                            {
                                new_curr_session_id = Layout.GetFieldValue(bitVector, FieldIndex.MsgData, FieldMarshalFunc.ToByte);
                                int new_vol = Math.Max(0, this.sessionProvider.getSessionVolume(new_curr_session_id) - 5);
                                this.sessionProvider.setSessionVolume(new_curr_session_id, new_vol);
                                volChanged = true;
                                break;
                            }
                        case Command.TOGGLE_MUTE:
                            {
                                new_curr_session_id = Layout.GetFieldValue(bitVector, FieldIndex.MsgData, FieldMarshalFunc.ToByte);
                                this.sessionProvider.toggleMuteSession(new_curr_session_id);
                                volChanged = true;
                                break;
                            }
                        case Command.CMD_OK:
                        case Command.CMD_ERR:
                            break;
                        default:
                            msgRead = false;
                            break;
                    }

                    if (msgRead)
                    {
                        if (command_id != Command.CMD_OK)
                        {
                            this.log("Reading cmd ={0} curr_session_id ={1}",
                                command_id.ToString(), new_curr_session_id);
                        }

                        if (new_curr_session_id != this.curr_session_id)
                        {
                            this.curr_session_id = new_curr_session_id;

                            // curr_session_id is changed, so send update
                            // SessionData to keyboard
                            this.sendSessionDatas();
                        }
                        else if (volChanged)
                        {
                            this.sendSessionDatas( true );
                        }
                    }
                    else // !msgRead
                    {
                        this.log("Unknown message cmd={0}", command_id.ToString());
                    }

                } // else if is_protocol_compatible()                    
                else
                {
                    var data_str = bitVector.ToString();
                    this.log("Reading unrecognized data ={0}", data_str);
                }
            }
            catch (Exception error)
            {
                this.log("Error: {0}\n", error.Message);
                this.log("Error: {0}\n", error.StackTrace);
            } // try
        } // void deviceDataRead

    } // class KBCommService
} //namespace