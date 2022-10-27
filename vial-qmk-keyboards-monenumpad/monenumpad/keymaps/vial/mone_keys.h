//mone_keys.h

#pragma once

enum mone_keys {
   //YouTube Layer
   MK_FIRST = SAFE_RANGE,

   MK_FN,  //Function key to switch mode

   MK_YT_REWIND = MK_FIRST,  //YouTube Rewind
   MK_YT_FASTFORWD,  //YouTube Fast forward
   MK_YT_SPEEDUP, //YouTube Speed up
   MK_YT_SPEEDDOWN, //YouTube Speed down
   MK_YT_NEXTVID_PLAYLIST, // Move to the next video in playlist
   MK_YT_PREVVID_PLAYLIST,  // Move to the previous video in playlist
   MK_YT_NEXTCH, //YouTube Next Chapter
   MK_YT_PREVCH, //YouTube Previous Chapter
   MK_YT_NEXTVID, //YouTube Next Video
   MK_YT_PREVVID, //YouTube Previous Video
   MK_YT_FULLSCVW, //YouTube Full Screen Player
   MK_YT_CYCLEVW, //YouTube Cycle through different view modes: Normal -> Theater -> Full
   MK_YT_THEATERVW, //YouTube Theater view
   MK_YT_PLAY, //YouTube play/pause

   MK_YT_HOME,  //  //YouTube open www.youtube.com
   MK_YT_WATCHLATER, //YouTube open watch later playlist
   MK_YT_SUBSCRIPTNS, //YouTube open subscription list
   MK_YT_HISTORY,  //YouTube open history list

   //iOS Layer
   MK_IOS_HOME, //iOS home screen, WORD+H
   //MK_IOS_SHOWKEED, //iOS show soft keyboard    
   MK_IOS_PREVTRACK, //iOS previous track  KC_MEDIA_PREV_TRACK
   MK_IOS_NEXTTRACK, //iOS next track  KC_MEDIA_NEXT_TRACK
   MK_IOS_PLAY, // iOS Play/pause media  KC_MEDIA_PLAY_PAUSE
   MK_IOS_SEARCH, //Spotlight Search. Cmd+Space
   MK_IOS_DOCK,   //Show Dock, World+A
   MK_IOS_APPLIB, //Show App Lib, Shift+World+A
   MK_IOS_QUICKNOTE, //Quick Note, World+Q
   MK_IOS_CONTROLCENTER, //Control Center, World+C
   MK_IOS_NOTIFICATION, //Notification Center, World+N
   MK_IOS_APPSWITCHER, //App Switcher, World+Up
   MK_IOS_PREVAPP, //Previous App, World+Left
   MK_IOS_NEXTAPP, //Next App, World+Right

   MK_LAST //exclusive
};

#define IS_MONE_CODE(keycode)     \
    (MK_FIRST <= keycode && keycode < MK_LAST)

// return true if qmk should continue processing the pressed key 
bool process_mone_key(uint16_t keycode);


