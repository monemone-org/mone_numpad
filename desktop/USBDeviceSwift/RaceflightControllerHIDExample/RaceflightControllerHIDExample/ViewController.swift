//
//  ViewController.swift
//  RaceflightControllerHIDExample
//
//  Created by Artem Hruzd on 6/14/17.
//  Copyright © 2017 Artem Hruzd. All rights reserved.
//

import Cocoa
import USBDeviceSwift

class ViewController: NSViewController, NSComboBoxDataSource, KBCommServiceDelegate, HIDDeviceMonitorDelegate
{
    
    //make sure that rfDeviceMonitor always exist
    let rfDeviceMonitor = HIDDeviceMonitor(
        [
            //HIDMonitorData(vendorId: 0x0483, productId: 0x5742)
            // Monenumpad
            HIDMonitorData(vendorId: Int(MONENUMPAD_VENDOR_ID),
                           productId: Int(MONENUMPAD_PRODUCT_ID),
                           usagePage: Int(MONENUMPAD_USAGE_PAGE),
                           usage: Int(MONENUMPAD_USAGE))
        ],
        reportSize: 64)
    
    let commService: KBCommService = KBCommService(
        sessionProvider: MockSessionProvider())
    
    @IBOutlet weak var devicesComboBox: NSComboBox!
    @IBOutlet weak var connectButton: NSButton!
    @IBOutlet weak var connectedDeviceLabel: NSTextField!
    @IBOutlet weak var rfDeviceView: NSView!
    
    @IBOutlet weak var outputTextFied: NSTextField!
    @IBOutlet weak var sendButton: NSButton!
    
    @IBOutlet var inputTextView: NSTextView!
    @IBOutlet weak var clearLogButton: NSButton!
    
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    @IBAction func sendOutputData(_ sender:AnyObject) {
    }
    
    @IBAction func clearLog(sender: AnyObject) {
        self.inputTextView?.string = ""
        self.inputTextView?.scrollToBeginningOfDocument(self)
    }
    
    // HIDDeviceMonitorDelegate
    func deviceFound(_ device: HIDDevice)
    {
        self.devicesComboBox.reloadData()
    }
    
    func deviceGone(_ device: HIDDevice)
    {
        self.devicesComboBox.reloadData()
    }
    
    func deviceDataRead(_ device: HIDDevice, data: Data, reportId: UInt32)
    {
    }

    // KBCommServiceDelegate implementation
    func deviceConnected(_ device: HIDDevice) {
        let connectedDevice = self.commService.connectedDevice
        self.connectButton.title = "Disconnect"
        self.connectedDeviceLabel.isHidden = false
        self.connectedDeviceLabel.stringValue = "Connected device: \(connectedDevice!.name) (\(connectedDevice!.vendorId), \(connectedDevice!.productId))"
        self.rfDeviceView.isHidden = false
        self.devicesComboBox.isEnabled = false
    }
    func deviceDisconnected(_ device: HIDDevice)
    {
        self.connectButton.title = "Connect"
        self.connectedDeviceLabel.isHidden = true
        self.devicesComboBox.isEnabled = true
        self.rfDeviceView.isHidden = true
        self.devicesComboBox.reloadData()
    }
    
    func log(_ msg: String) {
        self.appendToInputTextView(msg)
    }
    
    @IBAction func connectDevice(_ sender: Any) {
        if (self.rfDeviceMonitor.discoveredDevices.count > 0) {
            if (self.commService.connectedDevice != nil) {
                self.commService.disconnect()
            } else {
                let deviceInfo = self.rfDeviceMonitor.discoveredDevices[self.devicesComboBox.integerValue]
                self.commService.connect(device: deviceInfo, deviceMonitor: self.rfDeviceMonitor)
            }
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.commService.delegate = self
        
        // Do any additional setup after loading the view.
        if (!self.rfDeviceMonitor.isRunning)
        {
            self.rfDeviceMonitor.addDelegate(self)
            self.rfDeviceMonitor.run()
        }

        self.devicesComboBox.isEditable = false
        self.devicesComboBox.completes = false
        self.rfDeviceView.isHidden = true
        self.connectedDeviceLabel.isHidden = true
        self.devicesComboBox.reloadData()
    }
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }
    
    func numberOfItems(in comboBox: NSComboBox) -> Int {
        return self.rfDeviceMonitor.discoveredDevices.count
    }
    
    func comboBox(_ comboBox: NSComboBox, objectValueForItemAt index: Int) -> Any? {
        let device = self.rfDeviceMonitor.discoveredDevices[index]
        return "\(device.name) \(device.id))"
    }
    
    func appendToInputTextView(_ str: String)
    {
        DispatchQueue.main.async {
            self.inputTextView?.string = "\(self.inputTextView!.string)\(str)\n"
            self.inputTextView?.scrollToEndOfDocument(self)
        }
    }
        

    
}


