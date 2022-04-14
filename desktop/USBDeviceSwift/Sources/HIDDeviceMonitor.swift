//
//  HIDDeviceMonitor.swift
//  USBDeviceSwift
//
//  Created by Artem Hruzd on 6/14/17.
//  Copyright © 2017 Artem Hruzd. All rights reserved.
//

import Foundation
import IOKit.hid


public protocol HIDDeviceMonitorDelegate: AnyObject {
    func deviceFound(_ device: HIDDevice)
    func deviceGone(_ device: HIDDevice)
    func deviceDataRead(_ device: HIDDevice, data: Data, reportId: UInt32)
}

class WeakDelegateRef
{
    private(set) weak var value: HIDDeviceMonitorDelegate?

    init(_ value: HIDDeviceMonitorDelegate?) {
        self.value = value
    }
}

open class HIDDeviceMonitor {
    public let vp:[HIDMonitorData]
    public let reportSize:Int
    
    // Main thread properties
    public private(set) var discoveredDevices: [HIDDevice] = []
    
    private(set) var delegates: [WeakDelegateRef] = []
    
    private var rfDeviceDaemon: Thread?
    
    // Daemon thread properties
    // map of Device ID (String) to Device (HIDDevice)
    private var connectedDeviceContexts: Set<HIDDeviceInputReportContext> = Set()
    
    public init(_ vp:[HIDMonitorData], reportSize:Int) {
        self.vp = vp
        self.reportSize = reportSize
    }

    // main thread function
    public func isDeviceDiscovered(
                    vendorId:Int,
                    productId:Int) -> Bool
    {
        assert(Thread.isMainThread)
        return self.discoveredDevices.contains { device in
            return device.vendorId == vendorId &&
            device.productId == productId
        }
    }

    // main thread function
    public func isDeviceDiscovered(
                    id deviceId:String) -> Bool
    {
        assert(Thread.isMainThread)
        return self.discoveredDevices.contains { device in
            return device.id == deviceId
        }
    }

    
    // main thread function
    public func isDeviceDiscovered(
                    _ device:HIDDevice) -> Bool
    {
        assert(Thread.isMainThread)
        return isDeviceDiscovered(id: device.id)
    }

    public var isRunning: Bool {
        return rfDeviceDaemon != nil
    }
    
    public func run() {
        if rfDeviceDaemon == nil
        {
            rfDeviceDaemon = Thread(target: self, selector:#selector(self.daemonThreadStart), object: nil)
            rfDeviceDaemon?.start()
        }
    }
    
    // run under rfDeviceDaemon
    @objc open func daemonThreadStart() {
        
        let managerRef = IOHIDManagerCreate(kCFAllocatorDefault, IOOptionBits(kIOHIDOptionsTypeNone))
        var deviceMatches:[[String:Any]] = []
        for vp in self.vp {
            var match = [kIOHIDProductIDKey: vp.productId, kIOHIDVendorIDKey: vp.vendorId]
            if let usagePage = vp.usagePage {
                match[kIOHIDDeviceUsagePageKey] = usagePage
            }
            if let usage = vp.usage {
                match[kIOHIDDeviceUsageKey] = usage
            }
            deviceMatches.append(match)
        }
        IOHIDManagerSetDeviceMatchingMultiple(managerRef, deviceMatches as CFArray)
        IOHIDManagerScheduleWithRunLoop(managerRef, CFRunLoopGetCurrent(), CFRunLoopMode.defaultMode.rawValue);
        IOHIDManagerOpen(managerRef, IOOptionBits(kIOHIDOptionsTypeNone));
        
        let matchingCallback:IOHIDDeviceCallback = { inContext, inResult, inSender, inIOHIDDeviceRef in
            let this:HIDDeviceMonitor = unsafeBitCast(inContext, to: HIDDeviceMonitor.self)
            this.rawDeviceAdded(inResult, inSender: inSender!, inIOHIDDeviceRef: inIOHIDDeviceRef)
        }
        
        let removalCallback:IOHIDDeviceCallback = { inContext, inResult, inSender, inIOHIDDeviceRef in
            let this:HIDDeviceMonitor = unsafeBitCast(inContext, to: HIDDeviceMonitor.self)
            this.rawDeviceRemoved(inResult, inSender: inSender!, inIOHIDDeviceRef: inIOHIDDeviceRef)
        }
        IOHIDManagerRegisterDeviceMatchingCallback(managerRef, matchingCallback, unsafeBitCast(self, to: UnsafeMutableRawPointer.self))
        IOHIDManagerRegisterDeviceRemovalCallback(managerRef, removalCallback, unsafeBitCast(self, to: UnsafeMutableRawPointer.self))
        
        RunLoop.current.run()
    }
    
    open func rawDeviceAdded(_ inResult: IOReturn, inSender: UnsafeMutableRawPointer, inIOHIDDeviceRef: IOHIDDevice!) {
        // It would be better to look up the report size and create a chunk of memory of that size
        let device = HIDDevice(device:inIOHIDDeviceRef)
        
        let report = UnsafeMutablePointer<UInt8>.allocate(capacity: reportSize)
        let context = HIDDeviceInputReportContext(monitor: self, device: device, report: report)
        self.connectedDeviceContexts.insert(context)
        
        let inputCallback : IOHIDReportCallback = { inContext, inResult, inSender, type, reportId, report, reportLength in
            let context: HIDDeviceInputReportContext = unsafeBitCast(inContext, to: HIDDeviceInputReportContext.self)
            if let monitor: HIDDeviceMonitor = context.monitor
            {
                monitor.read(inResult,
                             device: context.device,
                             inSender: inSender!,
                             type: type,
                             reportId: reportId,
                             report: report,
                             reportLength: reportLength)
            }
        }
        
        //Hook up inputcallback
        IOHIDDeviceRegisterInputReportCallback(inIOHIDDeviceRef!, report, reportSize, inputCallback, unsafeBitCast(context, to: UnsafeMutableRawPointer.self))
        
        DispatchQueue.main.async {
            NotificationCenter.default.post(name: .HIDDeviceConnected, object: self, userInfo: ["device": device])

            self.discoveredDevices.append(device)
            for delegate in self.delegates {
                delegate.value?.deviceFound(device)
            }
        }

    }
    
    open func rawDeviceRemoved(_ inResult: IOReturn, inSender: UnsafeMutableRawPointer, inIOHIDDeviceRef: IOHIDDevice!) {
        let device = HIDDevice(device:inIOHIDDeviceRef)
        
        if let context = self.connectedDeviceContexts.first(where:{ $0.device.id == device.id })
        {
            self.connectedDeviceContexts.remove(context)
            context.deallocate()
        }
        
        DispatchQueue.main.async {
            if let index = self.discoveredDevices.firstIndex(where: { $0.id == device.id })
            {
                self.discoveredDevices.remove(at: index)
            }
            
            for delegate in self.delegates
            {
                delegate.value?.deviceGone(device)
            }

            NotificationCenter.default.post(name: .HIDDeviceDisconnected, object: self, userInfo: [
                "device": device
            ])
            
        }

    }
    
    open func read(_ inResult: IOReturn,
                   device: HIDDevice,
                   inSender: UnsafeMutableRawPointer,
                   type: IOHIDReportType,
                   reportId: UInt32,
                   report: UnsafeMutablePointer<UInt8>,
                   reportLength: CFIndex)
    {
        let data = Data(bytes: UnsafePointer<UInt8>(report), count: reportLength)
        
        DispatchQueue.main.async {
            
            for delegate in self.delegates
            {
                delegate.value?.deviceDataRead(device, data: data, reportId: reportId)
            }

            NotificationCenter.default.post(name: .HIDDeviceDataReceived,
                                            object: self,
                                            userInfo: [
                                                "device": device,
                                                "reportId": reportId,
                                                "data": data
                                            ])
        }
    }
    
    public func write(_ data: Data,
                      to device: HIDDevice,
                      reportId: UInt8 = 2)
    {
        let bytesArray = [UInt8](data)

        if (bytesArray.count > device.reportSize) {
            print("Output data too large for USB report")
            return
        }
        
        let correctData = Data(bytes: bytesArray, count: device.reportSize)
        
        IOHIDDeviceSetReport(
            device.device,
            kIOHIDReportTypeOutput,
            CFIndex(reportId),
            (correctData as NSData).bytes.bindMemory(to: UInt8.self, capacity: correctData.count),
            correctData.count
        )
    }


    //
    // MARK: Delegates
    //
    public func addDelegate(_ delegate: HIDDeviceMonitorDelegate) {
        assert(Thread.isMainThread)
        self.delegates.append(WeakDelegateRef(delegate))
    }
    
    public func removeDelegate(_ delegate: HIDDeviceMonitorDelegate) {
        assert(Thread.isMainThread)
        if let index = delegates.firstIndex(where: { $0.value === delegate})
        {
            self.delegates.remove(at: index)
        }
    }
}
