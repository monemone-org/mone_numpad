//structs.c
#ifdef VIA_ENABLE
#include QMK_KEYBOARD_H
#endif


#ifdef _MSC_VER
#include <stdbool.h>
#endif


#include "structs.h"
#include "assert_precond.h"

SessionInfo makeSessionInfo(void) {
    SessionInfo o = { 
        .count = (0)
    };
    return o;
}


VolumeData makeVolumeData() {
    VolumeData o = {
        .unknown = (true),
        .volume = (0), 
        .isMuted = (false)
 	};
 	return o;
}

SessionData makeSessionData(void)
{
    // name & data use { } initializers
    SessionData o = { 
    	.id = (SESSION_ID_NULL),
    	.name = {0},
    	.has_prev = false,
    	.has_next = false,
    	.volume = makeVolumeData()
    };
    return o;
}

SessionData makeOutSessionData(void)
{
    SessionData o = { 
    	.id = (SESSION_ID_OUT),
    	.name = (SESSION_NAME_OUT),
    	.has_prev = false,
    	.has_next = false,
    	.volume = makeVolumeData()
    };
    o.volume.unknown = true; //we don't know the volumne
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

#ifdef CONSOLE_ENABLE
    uprint("KL: assert_maxmix_struct_preconditions()\n");
#endif

	assert_precond(sizeof(SessionInfo) == SessionInfo_Size, "Invalid Expected SessionInfo Size");
	assert_precond(sizeof(VolumeData) == VolumeData_Size, "Invalid Expected Message Size");
	assert_precond(sizeof(SessionData) == SessionData_Size, "Invalid Expected SessionData Size");
	// assert_precond(sizeof(Color) == 3, "Invalid Expected Message Size");
	// assert_precond(sizeof(DeviceSettings) == 14, "Invalid Expected Message Size");
	//assert_precond(sizeof(ModeStates) == 5, "Invalid Expected Message Size");


}

