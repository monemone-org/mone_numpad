#pragma once

#include <stdint.h>
#include <assert.h>

#define UNKNOWN_PROTOCOL_VERSION 0x0000
#define MAXMIX_PROTOCOL_VERSION  0x0001

#define MONENUMPAD_VENDOR_ID    0x05ac
#define MONENUMPAD_PRODUCT_ID   0x029c
#define MONENUMPAD_USAGE_PAGE   0xFF60
#define MONENUMPAD_USAGE        0x61


//id_mone_prefix
#define MSG_ID_PREFIX       0xFD

// Default session ID for default system OUT and system IN.
#define SESSION_ID_NULL         0
#define SESSION_ID_OUT          1
#define SESSION_ID_IN           2
#define SESSION_ID_APP_FIRST    3

#define SESSION_NAME_OUT    "Output"
#define SESSION_NAME_IN     "Input"

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
#define Command_t int8_t

#if defined(_MSC_VER)
#define STRUCT_ATTR_PACKED 
#pragma pack(push, 1)
#else
#define STRUCT_ATTR_PACKED			__attribute__((__packed__))
#endif

//
// Mone: note USB data is in big endian format.
// 

//session_info also serve as a heartbeat
typedef struct STRUCT_ATTR_PACKED
{
    uint8_t count;           // 8 bits, total count of session
    // 8 bits - 1 byte
} SessionInfo;
#define SessionInfo_Size       1

extern SessionInfo makeSessionInfo(void);


typedef struct STRUCT_ATTR_PACKED
{
    uint8_t unknown: 1;    // 1 bit.  if true, then we don't have the correct values of isMuted and volume.
    uint8_t isMuted : 1;   // 1 bit
    uint8_t volume;        // 8 bits
    // 10 bits -> 2 bytes
} VolumeData;
#define VolumeData_Size       2


extern VolumeData makeVolumeData(void);

#define SessionData_Name_Size    25
typedef  struct STRUCT_ATTR_PACKED
{
    uint8_t id;    // 8 bits, session id
    char name[SessionData_Name_Size]; // 25 bytes - 200 bits
    uint8_t has_prev : 1; // 1 bit
    uint8_t has_next : 1; // 1 bit
    VolumeData volume; // VolumeData_Size * 8 bits = 16 bits

    // 226 bits -> 24 bytes
} SessionData;
#define SessionData_Size        29

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#define EXTERNAL
#else
#define EXTERNAL extern
#endif
EXTERNAL SessionData makeSessionData(void);
EXTERNAL SessionData makeOutSessionData(void);
EXTERNAL uint8_t isNullSession(SessionData session);

EXTERNAL void assert_maxmix_struct_preconditions(void);




