//
//  HIDDeviceInputReportContext.swift
//  USBDeviceSwift
//
//  Created by Siu Man Hsieh on 2022-04-12.
//  Copyright © 2022 Artem Hruzd. All rights reserved.
//

import Foundation

class HIDDeviceInputReportContext: Hashable, Identifiable
{
    weak var monitor: HIDDeviceMonitor?
    var device: HIDDevice
    var report: UnsafeMutablePointer<UInt8>
    
    init(monitor: HIDDeviceMonitor,
         device: HIDDevice,
         report: UnsafeMutablePointer<UInt8>)
    {
        self.monitor = monitor
        self.device = device
        self.report = report
    }
    
    func deallocate() {
        self.report.deallocate()
    }
    
    var id: String {
        return device.id
    }
    
    public func hash(into hasher: inout Hasher) {
        hasher.combine(device.id)
    }

    static func == (lhs: HIDDeviceInputReportContext, rhs: HIDDeviceInputReportContext) -> Bool {
        return
            (lhs.monitor != nil && rhs.monitor != nil && lhs.monitor === rhs.monitor)
            && (lhs.device.id == rhs.device.id)
    }

}


