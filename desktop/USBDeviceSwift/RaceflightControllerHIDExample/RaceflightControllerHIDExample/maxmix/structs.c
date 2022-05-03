//structs.c
//#include QMK_KEYBOARD_H

#include <stdio.h>
#include <stddef.h>
#include "structs.h"
#include "assert_precond.h"
#include <string.h>

#define STR(s) #s

#define SHOW_STRUCT_VOLUME_DATA(t)                                             \
  do {                                                                         \
    printf(STR(t) " is size %zd, align %zd\n", sizeof(struct t),               \
           _Alignof(struct t));                                                \
    printf("  unknown is at offset %zd\n", offsetof(struct t, unknown));                   \
    printf("  volume is at offset %zd\n", offsetof(struct t, volume));                   \
    printf("  isMuted is at offset %zd\n", offsetof(struct t, isMuted));                   \
  } while (0)

#define SHOW_STRUCT_SESSION_DATA(t)                                            \
  do {                                                                         \
    printf(STR(t) " is size %zd, align %zd\n", sizeof(struct t),               \
           _Alignof(struct t));                                                \
    printf("  id is at offset %zd\n", offsetof(struct t, id));                   \
    printf("  name is at offset %zd\n", offsetof(struct t, name));                   \
    printf("  has_prev is at offset %zd\n", offsetof(struct t, has_prev));                   \
    printf("  has_next is at offset %zd\n", offsetof(struct t, has_next));                   \
    printf("  volume is at offset %zd\n", offsetof(struct t, volume));                   \
  } while (0)

void print_byte_array(uint8_t* bytes, uint8_t cb);

SessionInfo makeSessionInfo(void) {
    SessionInfo o = { 
        .count = (0)
    };
    return o;
}


VolumeData makeVolumeData() {
    VolumeData o = {
        .unknown = (1),
        .volume = (0), 
        .isMuted = (0)
 	};
 	return o;
}

SessionData makeSessionData(void)
{
    // name & data use { } initializers
    SessionData o = { 
    	.id = (SESSION_ID_NULL),
    	.name = {0},
    	.has_prev = 0,
    	.has_next = 0,
    	.volume = makeVolumeData()
    };
    print_byte_array( (uint8_t *)&o, sizeof(SessionData) );
    
//    
//    //SHOW_STRUCT_VOLUME_DATA(_VolumeData);
//    //SHOW_STRUCT_VOLUME_DATA(_SessionData);
//    
//    int offset = offsetof(struct _SessionData, volume);
//    
//    SessionData o1 = {
//        .id = 0,
//        .name = {0},
//        .has_prev = 0,
//        .has_next = 0,
//        .volume = {0}
//    };
//    print_byte_array( (uint8_t *)&o1, sizeof(SessionData) );
//    
//    printf("o1.id = 4\n");
//    o1.id = 4;
//    print_byte_array( (uint8_t *)&o1, sizeof(SessionData) );
//
//    printf("o1.name = Music\n");
//    strcpy(o1.name, "Music");
//    print_byte_array( (uint8_t *)&o1, sizeof(SessionData) );
//
//    printf("o1.has_prev = 1\n");
//    o1.has_prev = 1;
//    print_byte_array( (uint8_t *)&o1, sizeof(SessionData) );
//
//    printf("o1.has_next = 1\n");
//    o1.has_next = 1;
//    print_byte_array( (uint8_t *)&o1, sizeof(SessionData) );

//    VolumeData v1 = makeVolumeData();
//    v1.unknown = 1;
//    printf("v1.unknown = 1\n");
//    print_byte_array( (uint8_t *)&v1, sizeof(VolumeData) );
//
//    v1.isMuted = 1;
//    printf("v1.isMuted = 1\n");
//    print_byte_array( (uint8_t *)&v1, sizeof(VolumeData) );
//
//    v1.volume = 127;
//    printf("v1.volume = 127\n");
//    print_byte_array( (uint8_t *)&v1, sizeof(VolumeData) );

    return o;
}

SessionData makeOutSessionData(void)
{
    SessionData o = { 
    	.id = (SESSION_ID_OUT),
    	.name = (SESSION_NAME_OUT),
    	.has_prev = 0,
    	.has_next = 0,
    	.volume = makeVolumeData()
    };
    return o;
}

uint8_t isNullSession(SessionData session) 
{
	return session.id == SESSION_ID_NULL;
}

// Color makeColor(void) {
// 	Color c = {
// 		 .r = (0),
// 		 .g = (0), 
// 		 .b = (0)
// 	};
// 	return c;
// }
// Color makeColorWithRGB(uint8_t r, uint8_t g, uint8_t b) {
// 	Color c = {
// 		 .r = (r),
// 		 .g = (g), 
// 		 .b = (b)
// 	};
// 	return c;
// }

// DeviceSettings makeDeviceSettings(void){
// 	DeviceSettings o = {
// 		.sleepAfterSeconds = (5), 
// 		.accelerationPercentage = (60), 
// 		.continuousScroll = (true),
//     	.volumeMinColor = makeColorWithRGB(0, 0, 255),
//     	.volumeMaxColor = makeColorWithRGB(255, 0, 0), 
//     	.mixChannelAColor = makeColorWithRGB(0, 0, 255), 
//     	.mixChannelBColor = makeColorWithRGB(255, 0, 255)
// 	};
// 	return o;
// }
    

// ModeStates makeModeStates(void)
// {
//     ModeStates o = { 
//     	.states = {0, 1, 1, 0, 0} 
//     };
//     return o;
// }



void assert_maxmix_struct_preconditions(void) {
//	assert_precond(sizeof(SessionInfo) == SessionInfo_Size, "Invalid Expected SessionInfo Size");
//	assert_precond(sizeof(VolumeData) == VolumeData_Size, "Invalid Expected Message Size");
//	assert_precond(sizeof(SessionData) == SessionData_Size, "Invalid Expected SessionData Size");
	// assert_precond(sizeof(Color) == 3, "Invalid Expected Message Size");
	// assert_precond(sizeof(DeviceSettings) == 14, "Invalid Expected Message Size");
	//assert_precond(sizeof(ModeStates) == 5, "Invalid Expected Message Size");


}


static uint8_t bitFlags[8] = {
    1 << 7,
    1 << 6,
    1 << 5,
    1 << 4,
    1 << 3,
    1 << 2,
    1 << 1,
    1
};

void print_byte_array(uint8_t* bytes, uint8_t cb)
{
    for (int i=0; i<cb; ++i)
    {
        char strByte[9] = {};
        uint8_t b = bytes[i];
        for (int j=0; j<8; ++j)
        {
            if (b & bitFlags[j])
            {
                strByte[j] = '1';
            }
            else
            {
                strByte[j] = '0';
            }
        }
        printf("%s ", strByte );
    }
    printf("\n\n");

}

