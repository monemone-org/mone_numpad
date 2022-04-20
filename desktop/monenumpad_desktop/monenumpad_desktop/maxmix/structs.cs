using System;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Collections.Generic;
using monenumpad_desktop.Marshal;

namespace monenumpad_desktop.Maxmix
{
    namespace structs
    {

        public static class Constants
        {
            public const UInt16 UNKNOWN_PROTOCOL_VERSION = 0x0000;
            public const UInt16 MAXMIX_PROTOCOL_VERSION = 0x0001;

            public const UInt16 MONENUMPAD_VENDOR_ID = 0x05ac;
            public const UInt16 MONENUMPAD_PRODUCT_ID = 0x029c;
            public const UInt16 MONENUMPAD_USAGE_PAGE = 0xFF60;
            public const byte MONENUMPAD_USAGE = 0x61;


            //id_mone_prefix
            public const byte MSG_ID_PREFIX = 0xFD;

            // Default session ID for default system OUT and system IN.
            public const byte SESSION_ID_NULL = 0;
            public const byte SESSION_ID_OUT = 1;
            public const byte SESSION_ID_IN = 2;
            public const byte SESSION_ID_APP_FIRST = 3;

            public const String SESSION_NAME_OUT = "Output";
            public const String SESSION_NAME_IN = "Input";


        }


        enum Command //: int8_t
        {
            CMD_ERR = 0,
            CMD_OK = 1,

            // First message to be sent from desktop client app to keyboard.
            // desktop client app sends this and keyboard reply back to client
            // its protocal version number (i.e. MAXMIX_PROTOCOL_VERSION)
            PROTOCOL_VERSION_EXCHANGE = 3,  // data: uint16_t version number

            //PC -> keyboard commands
            SESSION_INFO = 10,  // data: SessionInfo    
            CURRENT_SESSION,    // data: SessionData
            PREVIOUS_SESSION,   // data: SessionData
            NEXT_SESSION,       // data: SessionData

            //keyboard -> PC commands
            CURRENT_SESSION_CHANGED = 20,    // data: uint8_t new current session_id
            VOLUME_UP,          // data: uint8_t session_id
            VOLUME_DOWN,        // data: uint8_t session_id
            TOGGLE_MUTE,        // data: uint8_t session_id

            CMD_DEBUG = 50//DEBUG
        };

        //session_info also serve as a heartbeat
        //[StructLayout(LayoutKind.Explicit, Size = 1/*SessionInfo_Size*/, CharSet = CharSet.Ansi)]
        public struct SessionInfo
        {
            public byte count;           // 8 bits, total count of session
        };



        public struct VolumeData
        {
            public readonly static int VolumeData_Size = 2; //in bytes
            public enum FieldIndex
            {
                Unknown,
                Volume,
                IsMuted
            };
            public readonly static PackedStructLayout<FieldIndex> Layout;

            static VolumeData()
            {
                Layout = new PackedStructLayout<FieldIndex>(new FieldDesc[]{
                        new FieldDesc("unknown", 1), //in bits
                        new FieldDesc("volume", 7), //in bits
                        new FieldDesc("isMuted", 1) //in bits
                    });
                Debug.Assert(Layout.StructSize == VolumeData_Size);
            }


            public bool unknown; // 1 bit
            public byte volume; // 7 bits
            public bool isMuted; // 1 bit
            // 9 bits - 2 bytes

            public VolumeData(
                bool unknown,
                byte volume,
                bool isMuted)
            {
                this.unknown = unknown;
                this.volume = volume;
                this.isMuted = isMuted;
            }

            public VolumeData(BitVector bitVector)
            {
                this.unknown = Layout.GetFieldValue(bitVector, FieldIndex.Unknown, FieldMarshalFunc.ToBool);
                this.volume = Layout.GetFieldValue(bitVector, FieldIndex.Volume, FieldMarshalFunc.ToByte);
                this.isMuted = Layout.GetFieldValue(bitVector, FieldIndex.IsMuted, FieldMarshalFunc.ToBool);
            }

            public BitVector ToBitVector()
            {
                var bitVector = new BitVector();
                Layout.SetFieldValue(bitVector, FieldIndex.Unknown, this.unknown, FieldMarshalFunc.FromBool);
                Layout.SetFieldValue(bitVector, FieldIndex.Volume, this.volume, FieldMarshalFunc.FromByte);
                Layout.SetFieldValue(bitVector, FieldIndex.IsMuted, this.isMuted, FieldMarshalFunc.FromBool);
                return bitVector;
            }

        };

        public struct SessionData
        {
            private readonly static int SessionData_Name_Size = 20; //in bytes
            private readonly static int SessionData_Size = 23; //in bytes
            enum FieldIndex
            {
                ID,
                Name,
                HasPrev,
                HasNext,
                Volume
            };
            private readonly static PackedStructLayout<FieldIndex> Layout;

            static SessionData()
            {
                Layout = new PackedStructLayout<FieldIndex>(new FieldDesc[]{
                        new FieldDesc("ID", 8), //in bits
                        new FieldDesc("Name", SessionData_Name_Size * 8), //in bits
                        new FieldDesc("HasPrev", 1), //in bits
                        new FieldDesc("HasNext", 1), //in bits
                        new FieldDesc("Volume", VolumeData.Layout.StructSize * 8), //in bits
                    });
                Debug.Assert(Layout.StructSize == SessionData_Size);
            }

            public byte id;    // 8 bits, session id
            public string name; // 20 bytes - 160 bits
            public bool has_prev; // : 1; // 1 bit
            public bool has_next; // : 1; // 1 bit
            public VolumeData volume; // 9 bits 

            public SessionData(
                byte id,
                string name,
                bool has_prev,
                bool has_next,
                VolumeData volume)
            {
                this.id = id;
                this.name = name;
                this.has_prev = has_prev;
                this.has_next = has_next;
                this.volume = volume;
            }

            public SessionData(BitVector bitVector)
            {
                this.id = Layout.GetFieldValue(bitVector, FieldIndex.ID, FieldMarshalFunc.ToByte);
                this.name = Layout.GetFieldValue(bitVector, FieldIndex.Name, FieldMarshalFunc.ToString);
                this.has_prev = Layout.GetFieldValue(bitVector, FieldIndex.HasPrev, FieldMarshalFunc.ToBool);
                this.has_next = Layout.GetFieldValue(bitVector, FieldIndex.HasNext, FieldMarshalFunc.ToBool);
                this.volume = Layout.GetFieldValue(bitVector, FieldIndex.Volume, MarshaStructFunc.ToVolumeData);
            }

            public BitVector ToBitVector()
            {
                var bitVector = new BitVector();
                Layout.SetFieldValue(bitVector, FieldIndex.ID, this.id, FieldMarshalFunc.FromByte);
                Layout.SetFieldValue(bitVector, FieldIndex.Name, this.name, FieldMarshalFunc.FromString);
                Layout.SetFieldValue(bitVector, FieldIndex.HasPrev, this.has_prev, FieldMarshalFunc.FromBool);
                Layout.SetFieldValue(bitVector, FieldIndex.HasNext, this.has_next, FieldMarshalFunc.FromBool);
                Layout.SetFieldValue(bitVector, FieldIndex.Volume, this.volume, MarshaStructFunc.FromVolumeData);
                return bitVector;
            }
        } // class SessionData

        public static class MarshaStructFunc
        {
            public static VolumeData ToVolumeData(byte[] data)
            {
                var bitVector = new BitVector(data);
                return new VolumeData(bitVector);
            }

            public static byte[] FromVolumeData(VolumeData value)
            {
                return value.ToBitVector().ToBytes();
            }

            public static SessionData ToSessionData(byte[] data)
            {
                var bitVector = new BitVector(data);
                return new SessionData(bitVector);
            }

            public static byte[] FromSessionData(SessionData value)
            {
                return value.ToBitVector().ToBytes();
            }
        }


    } // namespace structs
} // namespace monenumpad_desktop.maxmix
