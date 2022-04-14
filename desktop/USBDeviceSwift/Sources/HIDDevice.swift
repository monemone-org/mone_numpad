//
//  HIDDevice.swift
//  USBDeviceSwift
//
//  Created by Artem Hruzd on 6/14/17.
//  Copyright © 2017 Artem Hruzd. All rights reserved.
//

import Cocoa
import Foundation
import IOKit.hid

public extension Notification.Name {
    static let HIDDeviceDataReceived = Notification.Name("HIDDeviceDataReceived")
    static let HIDDeviceConnected = Notification.Name("HIDDeviceConnected")
    static let HIDDeviceDisconnected = Notification.Name("HIDDeviceDisconnected")
}


/**
 Page ID    Page Name
 0x01           Generic Desktop Controls
 0x05           Game Controls
 0x08           LEDs
 0x09           Button
 */
public struct UsagePage: Equatable, Hashable
{
    public let rawValue: Int32
    
    static let genericControls = UsagePage(rawValue: 0x01)
    static let gameControls = UsagePage(rawValue: 0x05)
    static let leds = UsagePage(rawValue: 0x08)
    static let button = UsagePage(rawValue: 0x09)
    
    var hexString: String {
        return String(format:"0x%04X", self.rawValue)
    }
    
    var displayName: String {
        get {
            switch (self)
            {
                case .genericControls:
                    return "Generic Desktop Controls"
                case .gameControls:
                    return "Game Controls"
                case .leds:
                    return "LEDs"
                case .button:
                    return "Button"
                default:
                    return "Unknown Usage Page(\(self.rawValue))"
            }
        }
    }
}

/**
 Usage ID    Usage Name
 0x01    Pointer
 0x02    Mouse
 0x04    Joystick
 0x05    Game Pad
 0x06    Keyboard
 0x07    Keypad
 0x08    Multi-axis Controller
 */
public struct Usage: Equatable, Hashable
{
    public let rawValue: Int32
    
    static let pointer = Usage(rawValue: 0x01)
    static let mouse = Usage(rawValue: 0x02)
    static let joystick = Usage(rawValue: 0x04)
    static let gamePad = Usage(rawValue: 0x05)
    static let keyboard = Usage(rawValue: 0x06)
    static let keypad = Usage(rawValue: 0x07)
    static let multiAxisController = Usage(rawValue: 0x08)
    
    var hexString: String {
        return String(format:"0x%02X", self.rawValue)
    }

    var displayName: String {
        get {
            switch (self)
            {
                case .pointer:
                    return "Pointer"
                case .mouse:
                    return "Mouse"
                case .joystick:
                    return "Joystick"
                case .gamePad:
                    return "Game Pad"
                case .keyboard:
                    return "Keyboard"
                case .keypad:
                    return "Keypad"
                case .multiAxisController:
                    return "Multi-axis Controller"
                default:
                    return "Unknown Usage(\(self.rawValue))"
            }
        }
    }
}



public struct HIDMonitorData {
    public let vendorId:Int
    public let productId:Int
    public var usagePage:Int?
    public var usage:Int?

    public init (vendorId:Int, productId:Int) {
        self.vendorId = vendorId
        self.productId = productId
    }

    public init (vendorId:Int, productId:Int, usagePage:Int?, usage:Int?) {
        self.vendorId = vendorId
        self.productId = productId
        self.usagePage = usagePage
        self.usage = usage
    }
}

public class HIDDevice {
    
    private var _id:String?
    private var _vendorId:Int?
    private var _productId:Int?
    private var _reportSize:Int?
    private var _name:String?

    public let device:IOHIDDevice
    public var id:String {
        get {
            if let ret = _id {
                return ret
            }
            
            self._id = String(format:"v=0x%04X p=0x%04X up=%@ u=%@",
                              self.vendorId, self.productId,
                              self.usagePages.map{ $0.hexString }.joined(),
                              self.usages.map{ $0.hexString }.joined())
            return _id!
        }
    }
    
    public var vendorId:Int{
        get {
            if let ret = _vendorId {
                return ret
            }
            
            self._vendorId = IOHIDDeviceGetProperty(self.device, kIOHIDVendorIDKey as CFString) as? Int ?? 0
            return _vendorId!
        }
    }
    
    public var productId:Int{
        get {
            if let ret = _productId {
                return ret
            }
            
            self._productId = IOHIDDeviceGetProperty(self.device, kIOHIDProductIDKey as CFString) as? Int ?? 0
            return _productId!
        }
    }
    
    public var reportSize:Int{
        get {
            if let ret = _reportSize {
                return ret
            }
            
            self._reportSize = IOHIDDeviceGetProperty(self.device, kIOHIDMaxInputReportSizeKey as CFString) as? Int ?? 0
            return _reportSize!
        }
    }
    
    public var name:String{
        get {
            if let ret = _name {
                return ret
            }
            
            self._name = IOHIDDeviceGetProperty(self.device, kIOHIDProductKey as CFString) as? String ?? ""
            return _name!
        }
    }
    
    private var _usagePages: [UsagePage]?
    public var usagePages: [UsagePage] {
        get {
            let usagePages = _usagePages ?? { () -> [UsagePage] in
                guard let usagePairs = IOHIDDeviceGetProperty(self.device, kIOHIDDeviceUsagePairsKey as CFString) as? Array<Dictionary<String,Any>> else {
                    return []
                }

                self._usagePages = usagePairs.compactMap{ (usagePair) -> UsagePage? in
                           if let usagePageNum = usagePair[kIOHIDDeviceUsagePageKey] as? Int32
                           {
                               return UsagePage(rawValue: usagePageNum)
                           }
                           return nil
                       }
                return self._usagePages!
            }()

            return usagePages
        }
    }

    private var _usages: [Usage]?
    public var usages: [Usage] {
        get {
            let usages = _usages ?? { () -> [Usage] in
                guard let usagePairs = IOHIDDeviceGetProperty(self.device, kIOHIDDeviceUsagePairsKey as CFString) as? Array<Dictionary<String,Any>> else {
                    return []
                }

                self._usages = usagePairs.compactMap{ (usagePair) -> Usage? in
                            if let usageNum = usagePair[kIOHIDDeviceUsageKey] as? Int32
                            {
                                return Usage(rawValue: usageNum)
                                
                            }
                            return nil
                       }
                return self._usages!
            }()

            return usages
        }
    }
    
    public init(device:IOHIDDevice) {
        self.device = device
    }
    

}
