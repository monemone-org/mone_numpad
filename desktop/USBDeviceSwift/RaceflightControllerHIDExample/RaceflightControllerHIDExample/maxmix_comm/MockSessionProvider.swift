    //
    //  MockSessionProvider.swift
    //  RaceflightControllerHIDExample
    //
    //  Created by Siu Man Hsieh on 2022-04-11.
    //  Copyright © 2022 Artem Hruzd. All rights reserved.
    //

import Foundation
import USBDeviceSwift

class MockSessionProvider: AudioSessionProvider
{
    let sessions: [AudioSession] = [
        AudioSession(id: SESSION_ID_OUT.uint8,
                     name: SESSION_NAME_OUT),
        AudioSession(id: SESSION_ID_IN.uint8,
                     name: SESSION_NAME_IN),
        AudioSession(id: SESSION_ID_APP_FIRST.uint8,
                     name: "Game"),
        AudioSession(id: (SESSION_ID_APP_FIRST+1).uint8,
                     name: "Music"),
        AudioSession(id: (SESSION_ID_APP_FIRST+2).uint8,
                     name: "Discord")
    ]
    
}


