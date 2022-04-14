//
//  AudioSessionProvider.swift
//  RaceflightControllerHIDExample
//
//  Created by Siu Man Hsieh on 2022-04-13.
//  Copyright © 2022 Artem Hruzd. All rights reserved.
//

import Foundation

public struct AudioSession {
    let id: UInt8
    let name: String
}

public protocol AudioSessionProvider {
    var sessions: [AudioSession] { get }
}

