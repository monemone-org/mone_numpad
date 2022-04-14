//
//  structs+ext.swift
//  RaceflightControllerHIDExample
//
//  Created by Siu Man Hsieh on 2022-04-11.
//  Copyright © 2022 Artem Hruzd. All rights reserved.
//

import Foundation

extension Command
{
    func to_command_id() -> UInt8
    {
        return self.rawValue.uint8
    }
    
    var displayName: String {
        get {
            switch (self) {
                case CMD_ERR:
                    return "CMD_ERR"
                case CMD_OK:
                    return "CMD_OK"
                case PROTOCOL_VERSION_EXCHANGE:
                    return "PROTOCOL_VERSION_EXCHANGE"
                case SESSION_INFO:
                    return "SESSION_INFO"
                case CURRENT_SESSION:
                    return "CURRENT_SESSION"
                case PREVIOUS_SESSION:
                    return "PREVIOUS_SESSION"
                case NEXT_SESSION:
                    return "NEXT_SESSION"
                case CURRENT_SESSION_CHANGED:
                    return "CURRENT_SESSION_CHANGED"
                case VOLUME_UP:
                    return "VOLUME_UP"
                case VOLUME_DOWN:
                    return "VOLUME_DOWN"
                case TOGGLE_MUTE:
                    return "TOGGLE_MUTE"
                case CMD_DEBUG:
                    return "CMD_DEBUG"
                default:
                    return "<Unrecognized>"
            }
        }
    }
}

let sessionDataNameLen = 20
typealias SessionDataName = (CChar, CChar, CChar, CChar, CChar,  CChar, CChar, CChar, CChar, CChar,
                             CChar, CChar, CChar, CChar, CChar,  CChar, CChar, CChar, CChar, CChar)


