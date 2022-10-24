//maxmix.h

#pragma once

#include "structs.h"

void initialize_maxmix(void);

bool maxmix_is_running(void);

// loopBack:
//      true: if current session is the last session, goto the 1st session.
//      false: if current session is the last session, stay at the last session.
void edit_next_session(bool loopBack);
void edit_prev_session(void);
void inc_curr_session_volumne(void);
void dec_curr_session_volumne(void);
void toggle_curr_session_mute(void);

void send_maxmix_command(uint8_t command_id);
void handle_maxmix_command(uint8_t *data, uint8_t length);

extern SessionInfo session_info;
extern SessionData curr_session_data;
extern SessionData prev_session_data;
extern SessionData next_session_data;

