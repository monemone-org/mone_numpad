// maxmix.c
#include QMK_KEYBOARD_H

//#include <alloca.h>
#include <string.h>
#include "maxmix.h"
#include "via.h"
#include "raw_hid.h"

#ifdef CONSOLE_ENABLE
#define DEBUG_MAXMIX
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
uprintf("KL: inc_curr_session_volumne(curr_session_data.id=%d)\n", (int)curr_session_data.id);
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
uprintf("KL: dec_curr_session_volumne(curr_session_data.id=%d)\n", (int)curr_session_data.id);
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
uprintf("KL: toggle_curr_session_volumne(curr_session_data.id=%d)\n", (int)curr_session_data.id);
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
			uprintf("KL: send_maxmix_command - command_id=%d, curr_session_data.id=%d.\n",
					(int)command_id, (int)curr_session_data.id);
			#endif 
			raw_hid_send(data, length);
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: send_maxmix_command - ASSERT ERROR command_data_len < 1.\n");
			#endif 
    	}
    	break;

    default:
		#ifdef DEBUG_MAXMIX
    	uprintf("KL: send_maxmix_command - ASSERT ERROR unknown command: %d.\n", (int)command_id);
		#endif 
    	break;
	};		
}




void handle_maxmix_command(uint8_t *data, uint8_t length)
{
	uint8_t *command_id = &data[0];
	uint8_t *command_data = &data[1];
	uint8_t command_data_len = length - 1;

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
    		timer_received_session_info = timer_read();
			#ifdef DEBUG_MAXMIX
			uprintf("KL: handle_maxmix_command(SESSION_INFO), session_info(count=%d).\n", 
					(int)session_info.count);
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
    		memcpy(&curr_session_data, command_data, sizeof(curr_session_data));
    		*command_id = CMD_OK;
			#ifdef DEBUG_MAXMIX
			uprintf("KL: handle_maxmix_command(CURRENT_SESSION), curr_session_data(id=%d, name=%s).\n", 
					(int)curr_session_data.id, curr_session_data.name);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(CURRENT_SESSION) - ERROR command_data_len < sizeof(curr_session_data).\n");
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
			uprintf("KL: handle_maxmix_command(PREVIOUS_SESSION), prev_session_data(id=%d, name=%s).\n", 
					(int)prev_session_data.id, prev_session_data.name);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(PREVIOUS_SESSION) - ERROR command_data_len < sizeof(prev_session_data).\n");
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
			uprintf("KL: handle_maxmix_command(NEXT_SESSION), next_session_data(id=%d, name=%s).\n", 
					(int)next_session_data.id, next_session_data.name);
			#endif 
    	}
    	else 
    	{
			#ifdef DEBUG_MAXMIX
        	uprintf("KL: handle_maxmix_command(NEXT_SESSION) - ERROR command_data_len < sizeof(next_session_data).\n");
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

