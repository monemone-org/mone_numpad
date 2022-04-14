//
//  misc.swift
//  RaceflightControllerHIDExample
//
//  Created by Siu Man Hsieh on 2022-04-11.
//  Copyright © 2022 Artem Hruzd. All rights reserved.
//

import Foundation

extension FixedWidthInteger {
    var uint8: UInt8 {
        assert(self < UInt8.max)
        return UInt8(self)
    }
}


extension Data {
    // Additional: convertion bytes to specific string, removing garbage etc.
    func convertByteDataToString() -> String {
        
        var str = ""
        
        for b in self
        {
            let b_str = String(format: "%02X ", b)
            str.append(b_str)
            
        }

        return str
    }
}
