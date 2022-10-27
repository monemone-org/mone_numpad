//mone_keys.c

#include QMK_KEYBOARD_H
#include "mone_keys.h"
#include "user_config.h"

#define MACRO_TIMER 10


#define DEFAULT_PRESUBMIT_WEBPAGE_WAIT_TIME  70  //choose 70ms for Brave on iPad to work
#define DEFAULT_POSTSUBMIT_WEBPAGE_WAIT_TIME  10
uint64_t presubmit_webpage_wait_time = DEFAULT_PRESUBMIT_WEBPAGE_WAIT_TIME;
uint64_t postsubmit_webpage_wait_time = DEFAULT_POSTSUBMIT_WEBPAGE_WAIT_TIME; 
bool submit_webpage_auto_press_enter = false; // iOS safari doesn't like auto enter. It keeps loading a truncated URL.


static void openUrl(const char *url)
{
#ifdef DEBUG_SUBMIT_WEBPAGE_TIMING
    uprintf("KL: pre: %ld\n", (long)presubmit_webpage_wait_time);
    uprintf("KL: post: %ld\n", (long)postsubmit_webpage_wait_time);
    uprintf("KL: auto enter: %d\n", (int)submit_webpage_auto_press_enter);
#endif 

    // set focus to address bar
    if (user_config.is_win_mode) 
    {
        //Ctrl-L
        SEND_STRING(SS_LCTL(SS_TAP(X_L)));
    }
    else 
    {
        // CMD+L
        SEND_STRING(SS_LGUI(SS_TAP(X_L)));
    }
    wait_ms(presubmit_webpage_wait_time);

    // send URL to address bar
    send_string(url);

    // press enter to submit
    if (submit_webpage_auto_press_enter || user_config.is_win_mode)
    {
        wait_ms(postsubmit_webpage_wait_time);
        SEND_STRING(SS_TAP(X_ENTER));    
    }
}


const char* YT_CYCLEVW_MODES[] = {
    "tt",
    "t",
    "f"
};
const int YT_CYCLEVW_MODES_COUNT = sizeof(YT_CYCLEVW_MODES)/sizeof(YT_CYCLEVW_MODES[0]);
int g_current_yt_cyclevew_index = 0;
void next_yt_cyclevw(void)
{
    const char* vw_mode_keys = YT_CYCLEVW_MODES[g_current_yt_cyclevew_index];
    g_current_yt_cyclevew_index = (g_current_yt_cyclevew_index + 1) % YT_CYCLEVW_MODES_COUNT;
    send_string(vw_mode_keys);
}


// return true if qmk should continue processing the pressed key 
bool process_mone_key(uint16_t keycode)
{
    if (!IS_MONE_CODE(keycode))
    {
        return true;   
    }

    bool handled = true;

    //Example:
    //  SEND_STRING(SS_LALT(SS_TAP(X_V)SS_TAP(X_ENTER)));
    switch (keycode)
    {
        case MK_YT_REWIND:
            SEND_STRING(SS_TAP(X_LEFT));
            break;
        case MK_YT_FASTFORWD:
            SEND_STRING(SS_TAP(X_RIGHT));
            break;
        case MK_YT_SPEEDUP:
            SEND_STRING(SS_LSFT(">"));
            break;
        case MK_YT_SPEEDDOWN:
            SEND_STRING(SS_LSFT("<"));
            break;
        case MK_YT_NEXTCH:
            if (user_config.is_win_mode) 
            {
                SEND_STRING(SS_RCTL(SS_TAP(X_RIGHT)));
            }
            else 
            {
                SEND_STRING(SS_LALT(SS_TAP(X_RIGHT)));
            }
            break;
        case MK_YT_PREVCH:
            if (user_config.is_win_mode) 
            {
                SEND_STRING(SS_RCTL(SS_TAP(X_LEFT)));
            }
            else {
                SEND_STRING(SS_LALT(SS_TAP(X_LEFT)));
            }
            break;
        case MK_YT_NEXTVID:
            SEND_STRING(SS_LSFT(SS_TAP(X_N)));
            break;
        case MK_YT_PREVVID:
            if (user_config.is_win_mode) 
            {
                // Windows Edge: Alt + Left arrow
                SEND_STRING(SS_LALT(SS_TAP(X_LEFT)));
            }
            else {   
                // Mac Safari: CMD + [
                SEND_STRING(SS_LGUI("["));
            }
            break;
        case MK_YT_FULLSCVW:
            SEND_STRING(SS_TAP(X_F));
            break;
        case MK_YT_CYCLEVW:
            next_yt_cyclevw();
            break;
        case MK_YT_THEATERVW:
            SEND_STRING(SS_TAP(X_T));
            break;
        case MK_YT_PLAY:
            SEND_STRING(SS_TAP(X_SPACE));
            break;

        case MK_YT_HOME:
#ifdef DEBUG_SUBMIT_WEBPAGE_TIMING
            //testing reset
            presubmit_webpage_wait_time = DEFAULT_PRESUBMIT_WEBPAGE_WAIT_TIME;
            postsubmit_webpage_wait_time = DEFAULT_POSTSUBMIT_WEBPAGE_WAIT_TIME; 
#endif            
            openUrl("https://www.youtube.com");
            break;

        case MK_YT_SUBSCRIPTNS:
            openUrl("https://www.youtube.com/feed/subscriptions");
            break;

        case MK_YT_WATCHLATER:
            openUrl("https://www.youtube.com/playlist?list=WL");
#ifdef DEBUG_SUBMIT_WEBPAGE_TIMING
            //testing
            presubmit_webpage_wait_time += MACRO_TIMER;
#endif
            break;

        case MK_YT_HISTORY:
            openUrl("https://www.youtube.com/feed/history");
#ifdef DEBUG_SUBMIT_WEBPAGE_TIMING            
            //testing
            postsubmit_webpage_wait_time += MACRO_TIMER;
#endif
            break;


        case MK_IOS_HOME: // FN+H
            register_code16(KC_APPLE_FN);
            tap_code(KC_H);
            unregister_code16(KC_APPLE_FN);
            break;

        // case MK_IOS_SHOWKEED:
        //     break;

        case MK_YT_NEXTVID_PLAYLIST:
            SEND_STRING(SS_LSFT(SS_TAP(X_N)));
            break;

        case MK_YT_PREVVID_PLAYLIST:
            SEND_STRING(SS_LSFT(SS_TAP(X_P)));
            break;

        case MK_IOS_PREVTRACK:  //iOS previous track  KC_MEDIA_PREV_TRACK
            SEND_STRING(SS_TAP(X_MEDIA_PREV_TRACK));
            break;

        case MK_IOS_NEXTTRACK: //iOS next track  KC_MEDIA_NEXT_TRACK
            SEND_STRING(SS_TAP(X_MEDIA_NEXT_TRACK));
            break;

        case MK_IOS_PLAY: // iOS Play/pause media  KC_MEDIA_PLAY_PAUSE
            SEND_STRING(SS_TAP(X_MEDIA_PLAY_PAUSE));
            break;

        case MK_IOS_SEARCH: //Spotlight Search. 
            if (user_config.is_win_mode) 
            {
                // Win+S
                SEND_STRING(SS_LGUI(SS_TAP(X_S)));
            }
            else {
                // Cmd+Space
                SEND_STRING(SS_LGUI(SS_TAP(X_SPACE)));
            }
            break;

        case MK_IOS_DOCK: //Show Dock, World+A
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_A));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_APPLIB: //Show App Lib, Shift+World+A
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_LSFT(SS_TAP(X_A)));
            unregister_code16(KC_APPLE_FN);
           break;

        case MK_IOS_QUICKNOTE:  //Quick Note, World+Q
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_Q));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_CONTROLCENTER: //Control Center
            if (user_config.is_win_mode) 
            {
                // Win+N
                SEND_STRING(SS_LGUI(SS_TAP(X_A)));
            }
            else 
            {
                //World+C
                register_code16(KC_APPLE_FN);
                SEND_STRING(SS_TAP(X_C));
                unregister_code16(KC_APPLE_FN);
            }
            break;

        case MK_IOS_NOTIFICATION: //Notification Center
            if (user_config.is_win_mode) 
            {
                // Win+N
                SEND_STRING(SS_LGUI(SS_TAP(X_N)));
            }
            else {
                // World+N
                register_code16(KC_APPLE_FN);
                SEND_STRING(SS_TAP(X_N));
                unregister_code16(KC_APPLE_FN);
            }
            break;

        case MK_IOS_APPSWITCHER: //App Switcher, World+Up
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_UP));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_PREVAPP: //Previous App, World+Left
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_LEFT));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_NEXTAPP: //Next App, World+Right
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_RIGHT));
            unregister_code16(KC_APPLE_FN);
            break;

        default:
            handled = false;
            break;

    } //switch (keycode) 

    return !handled;
}




