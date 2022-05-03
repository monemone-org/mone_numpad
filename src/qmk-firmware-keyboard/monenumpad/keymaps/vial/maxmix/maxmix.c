// maxmix.c
#include QMK_KEYBOARD_H

//#include <alloca.h>
#include <string.h>
#include "maxmix.h"
#include "via.h"
#include "raw_hid.h"

#ifdef CONSOLE_ENABLE
//#define DEBUG_MAXMIX
//void print_byte_array(uint8_t* bytes, uint8_t cb);
#endif


uint16_t desktop_client_protocol_version;
uint16_t timer_received_session_info;
SessionInfo session_info;
SessionData curr_session_data;
SessionData prev_session_data;
SessionData next_session_data;


void initialize_maxmix()
{
	desktop_client_protocol_version = 0;
	timer_received_session_info = 0;
	session_info = makeSessionInfo();
	curr_session_data = makeOutSessionData();
	prev_session_data = makeSessionData();
	next_session_data = makeSessionData();
	assert_maxmix_struct_preconditions();
}

bool is_desktop_version_compatible(void)
{
	return desktop_client_protocol_version == MAXMIX_PROTOCOL_VERSION;
}


bool maxmix_is_running(void) 
{
	// if we haven't received an update of session_info for more than
	// 2 sec, then consider maxmix client app is dead
	return 
		is_desktop_version_compatible() &&
		!isNullSession(curr_session_data) &&
		timer_elapsed(timer_received_session_info) <= 2000;
}

void edit_next_session() 
{
	if (!curr_session_data.has_next) {
#ifdef DEBUG_MAXMIX
		uprintf("KL: edit_next_session returns - !curr.has_next\n");
#endif 
		return;
	}

	prev_session_data = curr_session_data;
	curr_session_data = next_session_data;
	next_session_data = makeSessionData();
	send_maxmix_command(CURRENT_SESSION_CHANGED);
}

void edit_prev_session()
{
	if (!curr_session_data.has_prev) {
#ifdef DEBUG_MAXMIX
		uprintf("KL: edit_prev_session returns - !curr.has_prev\n");
#endif 
		return;
	}

	next_session_data = curr_session_data;;
	curr_session_data = prev_session_data;
	prev_session_data = makeSessionData();
	send_maxmix_command(CURRENT_SESSION_CHANGED);
}


void inc_curr_session_volumne()
{
#ifdef DEBUG_MAXMIX
uprintf("KL: inc_curr_session_volumne(curr.id=%d)\n", (int)curr_session_data.id);
#endif 

	if (maxmix_is_running())
	{
		send_maxmix_command(VOLUME_UP);
	}
	else
	{
		SEND_STRING(SS_TAP(X_VOLU));
	}
}

void dec_curr_session_volumne()
{
#ifdef DEBUG_MAXMIX
uprintf("KL: dec_curr_session_volumne(curr.id=%d)\n", (int)curr_session_data.id);
#endif 
	if (maxmix_is_running())
	{
		send_maxmix_command(VOLUME_DOWN);
	}
	else
	{
		SEND_STRING(SS_TAP(X_VOLD));
	}
}

void toggle_curr_session_mute()
{
#ifdef DEBUG_MAXMIX
uprintf("KL: toggle_curr_session_volumne(curr.id=%d)\n", (int)curr_session_data.id);
#endif 
	send_maxmix_command(TOGGLE_MUTE);
}

void send_maxmix_command(uint8_t command_id)
{
	uint8_t data[32] = {0};
	uint8_t length = sizeof(data);

	data[0] = id_mone_prefix;
	data[1] = command_id;

	uint8_t *command_data = &data[2];
	uint8_t command_data_len = length - 2;

	switch ((int)command_id)
	{
	case CURRENT_SESSION_CHANGED:	// data: uint8_t new current session_id
    case VOLUME_UP:          // data: uint8_t session_id
    case VOLUME_DOWN:        // data: uint8_t session_id
    case TOGGLE_MUTE:        // data: uint8_t session_id
    	if (command_data_len >= 1)
    	{
    		*command_data = curr_session_data.id;
			#ifdef DEBUG_MAXMIX
			uprintf("KL: send_maxmix_command - command_id=%d, curr.id=%d.\n",
					(int)command_id, (int)curr_session_data.id);
			#endif 
			raw_hid_send(data, length);
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: send_maxmix_command - ASSERT command_data_len < 1.\n");
			#endif 
    	}
    	break;

    default:
		#ifdef DEBUG_MAXMIX
    	uprintf("KL: send_maxmix_command - ASSERT unknown command: %d.\n", (int)command_id);
		#endif 
    	break;
	};		
}




void handle_maxmix_command(uint8_t *data, uint8_t length)
{
	uint8_t *command_id = &data[0];
	uint8_t *command_data = &data[1];
	uint8_t command_data_len = length - 1;

	timer_received_session_info = timer_read();

	switch ((int)*command_id)
	{
	case PROTOCOL_VERSION_EXCHANGE:
		{
			//reply big-endian
			desktop_client_protocol_version = (command_data[0] << 8) | command_data[1];

			#ifdef DEBUG_MAXMIX
			uprintf("KL: handle_maxmix_command(PROTOCOL_VERSION_EXCHANGE)=0x%04X.\n", 
					(int)desktop_client_protocol_version);
			#endif 

			//reply big-endian
            command_data[0] = MAXMIX_PROTOCOL_VERSION >> 8;
            command_data[1] = MAXMIX_PROTOCOL_VERSION & 0xFF;
            break;
		}

    case SESSION_INFO:
    	if (command_data_len >= sizeof(session_info))
    	{
    		memcpy(&session_info, command_data, sizeof(session_info));
    		*command_id = CMD_OK;
			#ifdef DEBUG_MAXMIX
			// uprintf("KL: handle_maxmix_command(SESSION_INFO), session_info(count=%d).\n", 
			// 		(int)session_info.count);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(SESSION_INFO) - ERROR command_data_len < sizeof(session_info).\n");
			#endif 
			*command_id = CMD_ERR;
    	}
    	break;    

    case CURRENT_SESSION:    // data: SessionData
    	if (command_data_len >= sizeof(curr_session_data))
    	{
// #ifdef DEBUG_MAXMIX
//     		SessionData testingData;
//     		memset( (uint8_t *)&testingData, 0, sizeof(testingData) );
//     		strcpy(testingData.name, "Music");
// 		    testingData.id  = 4;
// 		    testingData.has_prev = 1;
// 		    testingData.has_next = 1;
// 		    testingData.volume.unknown = 0;
// 		    testingData.volume.isMuted = 0;
// 		    testingData.volume.volume = 50;
// 		    print_byte_array( (uint8_t *)&testingData, sizeof(testingData) );
// #endif

    		memcpy(&curr_session_data, command_data, sizeof(curr_session_data));
    		*command_id = CMD_OK;
			#ifdef DEBUG_MAXMIX
			uprintf("KL: handle_maxmix_command(CURRENT_SESSION), curr(id=%d, name=%s, vol=%d, isMuted=%d).\n", 
					(int)curr_session_data.id, curr_session_data.name,
					(int)curr_session_data.volume.volume, (int)curr_session_data.volume.isMuted);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(CURRENT_SESSION) - ERROR command_data_len < sizeof(SessionData).\n");
			#endif 
			*command_id = CMD_ERR;
    	}
    	break;

    case PREVIOUS_SESSION:   // data: SessionData
    	if (command_data_len >= sizeof(prev_session_data))
    	{
    		memcpy(&prev_session_data, command_data, sizeof(prev_session_data));
    		*command_id = CMD_OK;
			#ifdef DEBUG_MAXMIX
			uprintf("KL: handle_maxmix_command(PREVIOUS_SESSION), prev(id=%d, name=%s).\n", 
					(int)prev_session_data.id, prev_session_data.name);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(PREVIOUS_SESSION) - ERROR command_data_len < sizeof(SessionData).\n");
			#endif 
			*command_id = CMD_ERR;
    	}
    	break;

    case NEXT_SESSION:       // data: SessionData
    	if (command_data_len >= sizeof(next_session_data))
    	{
    		memcpy(&next_session_data, command_data, sizeof(next_session_data));
    		*command_id = CMD_OK;
			#ifdef DEBUG_MAXMIX
			uprintf("KL: handle_maxmix_command(NEXT_SESSION), next(id=%d, name=%s).\n", 
					(int)next_session_data.id, next_session_data.name);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(NEXT_SESSION) - ERROR command_data_len < sizeof(SessionData).\n");
			#endif 
			*command_id = CMD_ERR;
    	}
    	break;

    default:
		#ifdef DEBUG_MAXMIX
    	uprintf("KL: handle_maxmix_command - ERROR unknown cmd: %d.\n", (int)command_id);
		#endif 
		*command_id = CMD_ERR;
    	break;
	};

}

#ifdef CONSOLE_ENABLE
// static uint8_t bitFlags[8] = {
// 	1 << 7,
// 	1 << 6,
// 	1 << 5,
// 	1 << 4,
// 	1 << 3,
// 	1 << 2,
// 	1 << 1,
// 	1
// };

// void print_byte_array(uint8_t* bytes, uint8_t cb)
// {
// 	uprintf("KL: print_byte_array <----");
// 	for (int i=0; i<cb; ++i)
// 	{
// 		char strByte[9] = {};
// 		uint8_t b = bytes[i];
// 		for (int j=0; j<8; ++j)
// 		{
// 			if (b & bitFlags[j])
// 			{
// 				strByte[j] = '1';
// 			}
// 			else
// 			{
// 				strByte[j] = '0';
// 			}
// 		}
// 		uprintf("%s ", strByte );
// 	}
// 	uprintf("---->");

// }

#endif



