using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace monenumpad_desktop.Marshal
{
    public static class FieldMarshalFunc
    {
        // Byte
        public static byte ToByte(byte[] data)
        {
            Debug.Assert(data.Length >= 1);
            return data[0];
        }

        public static byte[] FromByte(byte value)
        {
            return new byte[] { value };
        }


        // UInt16
        public static UInt16 ToUInt16(byte[] data)
        {
            if (Platform.BigEndian)
            {
                return ToBigEndianUInt16(data);
            }
            else
            {
                return ToLittleEndianUInt16(data);
            }
        }

        public static byte[] FromUInt16(UInt16 value)
        {
            if (Platform.BigEndian)
            {
                return FromBigEndianUInt16(value);
            }
            else
            {
                return FromLittleEndianUInt16(value);
            }
        }

        public static UInt16 ToBigEndianUInt16(byte[] data)
        {
            Debug.Assert(data.Length >= 2);
            UInt16 b0 = data[0];
            UInt16 b1 = data[1];

            UInt16 value = ((UInt16)((b0 << 8) | b1));
            return value;
        }

        public static byte[] FromBigEndianUInt16(UInt16 value)
        {
            byte b0 = (byte)(value >> 8);
            byte b1 = (byte)(value & 0xFF);
            var bytes = new byte[] { b0, b1 };
            return bytes;
        }

        public static UInt16 ToLittleEndianUInt16(byte[] data)
        {
            Debug.Assert(data.Length >= 2);
            UInt16 b0 = data[0];
            UInt16 b1 = data[1];
            UInt16 value = ((UInt16)((b1 << 8) | b0));
            return value;
        }

        public static byte[] FromLittleEndianUInt16(UInt16 value)
        {
            byte b0 = (byte)(value & 0xFF);
            byte b1 = (byte)(value >> 8);
            var bytes = new byte[] { b0, b1 };
            return bytes;
        }


        // Int32
        public static Int32 ToInt32(byte[] data)
        {
            if (Platform.BigEndian)
            {
                return ToBigEndianInt32(data);
            }
            else
            {
                return ToLittleEndianInt32(data);
            }
        }

        public static byte[] FromInt32(Int32 value)
        {
            if (Platform.BigEndian)
            {
                return FromBigEndianInt32(value);
            }
            else
            {
                return FromLittleEndianInt32(value);
            }
        }

        public static Int32 ToBigEndianInt32(byte[] data)
        {
            Debug.Assert(data.Length >= 4);
            Int32 b0 = data[0];
            Int32 b1 = data[1];
            Int32 b2 = data[2];
            Int32 b3 = data[3];
            Int32 value = ((Int32)((b0 << 24) | (b1 << 16) | (b2 << 8) | b3));
            return value;
        }

        public static byte[] FromBigEndianInt32(Int32 value)
        {
            byte b0 = (byte)(value >> 24);
            byte b1 = (byte)(value >> 16);
            byte b2 = (byte)(value >> 8);
            byte b3 = (byte)(value & 0xFF);
            var bytes = new byte[] { b0, b1, b2, b3 };
            return bytes;
        }

        public static Int32 ToLittleEndianInt32(byte[] data)
        {
            Debug.Assert(data.Length >= 4);
            Int32 b0 = data[0];
            Int32 b1 = data[1];
            Int32 b2 = data[2];
            Int32 b3 = data[3];
            Int32 value = ((Int32)((b3 << 24) | (b2 << 16) | (b1 << 8) | b0));
            return value;
        }

        public static byte[] FromLittleEndianInt32(Int32 value)
        {
            byte b0 = (byte)(value >> 24);
            byte b1 = (byte)(value >> 16);
            byte b2 = (byte)(value >> 8);
            byte b3 = (byte)(value & 0xFF);
            var bytes = new byte[] { b3, b2, b1, b0 };
            return bytes;
        }


        // UInt64
        public static UInt64 ToUInt64(byte[] data)
        {
            if (Platform.BigEndian)
            {
                return ToBigEndianUInt64(data);
            }
            else
            {
                return ToLittleEndianUInt64(data);
            }
        }

        public static byte[] FromUInt64(UInt64 value)
        {
            if (Platform.BigEndian)
            {
                return FromBigEndianUInt64(value);
            }
            else
            {
                return FromLittleEndianUInt64(value);
            }
        }

        public static UInt64 ToBigEndianUInt64(byte[] data)
        {
            Debug.Assert(data.Length >= 8);
            UInt64 b0 = data[0];
            UInt64 b1 = data[1];
            UInt64 b2 = data[2];
            UInt64 b3 = data[3];
            UInt64 b4 = data[4];
            UInt64 b5 = data[5];
            UInt64 b6 = data[6];
            UInt64 b7 = data[7];
            UInt64 value = ((UInt64)((b0 << 56) | (b1 << 48) | (b2 << 40) | (b3 << 32) | (b4 << 24) | (b5 << 16) | (b6 << 8) | b7));
            return value;
        }

        public static byte[] FromBigEndianUInt64(UInt64 value)
        {
            byte b0 = (byte)(value >> 56);
            byte b1 = (byte)(value >> 48);
            byte b2 = (byte)(value >> 40);
            byte b3 = (byte)(value >> 32);
            byte b4 = (byte)(value >> 24);
            byte b5 = (byte)(value >> 16);
            byte b6 = (byte)(value >> 8);
            byte b7 = (byte)(value & 0xFF);
            var bytes = new byte[] { b0, b1, b2, b3, b4, b5, b6, b7 };
            return bytes;
        }

        public static UInt64 ToLittleEndianUInt64(byte[] data)
        {
            Debug.Assert(data.Length >= 8);
            UInt64 b0 = data[0];
            UInt64 b1 = data[1];
            UInt64 b2 = data[2];
            UInt64 b3 = data[3];
            UInt64 b4 = data[4];
            UInt64 b5 = data[5];
            UInt64 b6 = data[6];
            UInt64 b7 = data[7];
            UInt64 value = ((UInt64)((b7 << 56) | (b6 << 48) | (b5 << 40) | (b4 << 32) | (b3 << 24) | (b2 << 16) | (b1 << 8) | b0));
            return value;
        }

        public static byte[] FromLittleEndianUInt64(UInt64 value)
        {
            byte b0 = (byte)(value >> 56);
            byte b1 = (byte)(value >> 48);
            byte b2 = (byte)(value >> 40);
            byte b3 = (byte)(value >> 32);
            byte b4 = (byte)(value >> 24);
            byte b5 = (byte)(value >> 16);
            byte b6 = (byte)(value >> 8);
            byte b7 = (byte)(value & 0xFF);
            var bytes = new byte[] { b7, b6, b5, b4, b3, b2, b1, b0 };
            return bytes;
        }


        // Check first bit
        public static bool ToBool(byte[] data)
        {
            Debug.Assert(data.Length >= 1);
            return (data[0] & 0b1000_0000) != 0;
        }

        public static byte[] FromBool(bool value)
        {
            return new byte[] { (byte)(value ? 0b1000_0000 : 0) };
        }

        // char* to string
        public static string ToString(byte[] data)
        {
            return ToString(data, Encoding.ASCII);
        }
        public static string ToString(byte[] data, Encoding encoding)
        {
            return encoding.GetString(data);
        }
        public static byte[] FromString(string value)
        {
            return FromString(value, Encoding.ASCII);
        }
        public static byte[] FromString(string value, Encoding encoding)
        {
            return encoding.GetBytes(value);
        }

        // Convert a wchar_t* IntPtr to string
        //
        public static string PtrToStringWChar_t(IntPtr ptr)
        {
            int i = 0;
            var bytes = new List<byte>();
            byte b0;
            byte b1;
            byte b2;
            byte b3;
            bool isNullWChar() {
                return b0 == 0 && b1 == 0 && b2 == 0 && b3 == 0;
            };
            do
            {
                b0 = System.Runtime.InteropServices.Marshal.ReadByte(ptr, i);
                b1 = System.Runtime.InteropServices.Marshal.ReadByte(ptr, i + 1);
                b2 = System.Runtime.InteropServices.Marshal.ReadByte(ptr, i + 2);
                b3 = System.Runtime.InteropServices.Marshal.ReadByte(ptr, i + 3);
                i += 4;

                bytes.Add(b0);
                bytes.Add(b1);
                bytes.Add(b2);
                bytes.Add(b3);
                // read till we have 4 null byte
            }
            while (!isNullWChar());

            return BytesToStringWChar_t(bytes.ToArray());
        }

        public static string BytesToStringWChar_t(byte[] bytes)
        {
            if (Platform.Is64Bit)
            {
                return Encoding.UTF32.GetString(bytes).Trim('\0');
            }
            else
            {
                return Encoding.Unicode.GetString(bytes).Trim('\0');
            }
        }

        // IntPtr
        public static IntPtr ToIntPtr(byte[] data)
        {
            if (Platform.POINTER_SIZE == 64)
            {
                Int64 ptrAddress = (Int64)ToUInt64(data);
                return new IntPtr(ptrAddress);
            }
            else
            {
                Int32 ptrAddress = ToInt32(data);
                return new IntPtr(ptrAddress);
            }
        }


        //public static byte[] FromIntPtr(IntPtr value)
        //{
        //    return new byte[] { (byte)(value ? 0b1000_0000 : 0) };
        //}

    }
} // namespace
