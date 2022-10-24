#pragma once

inline bool IsPlatformLittleEndian()
{
    int n = 1;
    // little endian if true
    return (*(char*)&n == 1);
}

inline bool IsPlatformBigEndian()
{
    return !IsPlatformLittleEndian();
}

typedef struct UINT16_Bytes
{
    byte data[2];

} UINT16_Bytes;

// in network order (big-endian)
inline UINT16_Bytes UInt16ToBigEndian(UINT16 value)
{
    byte b0 = (byte)(value >> 8);
    byte b1 = (byte)(value & 0xFF);
    UINT16_Bytes bytes = {
       .data = { b0, b1 }
    };
    return bytes;
}

inline UINT16_Bytes UInt16ToLittleEndian(UINT16 value)
{
    byte b0 = (byte)(value >> 8);
    byte b1 = (byte)(value & 0xFF);
    UINT16_Bytes bytes = {
       .data = { b1, b0 }
    };
    return bytes;
}

inline UINT16 ToUInt16(BYTE* data) //data is in big endian format
{
    UINT16 b0 = data[0];
    UINT16 b1 = data[1];
    UINT16 value = (UINT16)((b0<< 8) | b1);
    return value;
}

inline UINT16_Bytes FromUInt16(UINT16 value)
{
    if (IsPlatformBigEndian())
    {
        return UInt16ToBigEndian(value);
    }
    else
    {
        return UInt16ToLittleEndian(value);
    }
}

