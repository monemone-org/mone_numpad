//
//  KBCommService.swift
//  RaceflightControllerHIDExample
//
//  Created by Siu Man Hsieh on 2022-04-11.
//  Copyright © 2022 Artem Hruzd. All rights reserved.
//

import Foundation
import USBDeviceSwift

public protocol KBCommServiceDelegate: AnyObject {
    func deviceConnected(_ device: HIDDevice)
    func deviceDisconnected(_ device: HIDDevice)
    func log(_ msg: String)
}

/**
 public func connect(device: HIDDevice)
 public func disconnect()
 public func onAudioSessionsChanged()
 
 dependency:
 sessionProvider: AudioSessionProvider
 deviceMonitor: HIDDeviceMonitor
    deviceMonitor.delegfate.deviceGone
    deviceMonitor.delegfate.deviceDataRead
    deviceMonitor.write
 */
public class KBCommService: NSObject, HIDDeviceMonitorDelegate
{
    static let dummySessionData: SessionData = {
        let space: CChar = Int8(" ".cString(using: .utf8)![0])
        return SessionData(id: SESSION_ID_NULL.uint8,
                           name: (space, space, space, space, space, space, space, space, space, space, space, space, space, space, space, space, space, space, space, space),
                           has_prev: 0,
                           has_next: 0,
                           volume: VolumeData(unknown: 0, volume: 50, isMuted: 0))
    }();
    
    let sessionProvider: AudioSessionProvider

    public weak var delegate: KBCommServiceDelegate?
    
    public private(set) var connectedDevice: HIDDevice?
    private var deviceMonitor: HIDDeviceMonitor?
    
    static private let default_session_id: UInt8 = (SESSION_ID_OUT).uint8

    // keyboard firmware protocoal version.
    // if it's not compatible to ours, we don't send it messages
    private var kb_protocol_version: UInt16 = UInt16(UNKNOWN_PROTOCOL_VERSION) // unknown
    
    // current audio session ID
    private var curr_session_id: UInt8 = default_session_id //default is Out session
    
    private var sendSessionInfoTimer: Timer?
    
    init(sessionProvider: AudioSessionProvider,
         delegate: KBCommServiceDelegate? = nil)
    {
        self.sessionProvider = sessionProvider
        self.delegate = delegate
    }
    
    //
    // MARK: -- Public Methods --
    //
    public func connect(device: HIDDevice, deviceMonitor: HIDDeviceMonitor)
    {
        if let connectedDevice = self.connectedDevice {
            
            if connectedDevice === device {
                return //already connected
            }
            
            // disconnect previously connected device before
            // making new connection
            self.disconnect()
        }
        
        self.connectedDevice = device
        self.deviceMonitor = deviceMonitor
        self.deviceMonitor?.addDelegate(self)
        self.sendProtocolVersion()
        self.delegate?.deviceConnected(device)
    }
    
    public func disconnect()
    {
        self.stopRefreshingSessionInfoTimer()
        if let device = self.connectedDevice
        {
            self.connectedDevice = nil
            self.delegate?.deviceDisconnected(device)
        }
        self.deviceMonitor?.removeDelegate(self)
        self.deviceMonitor = nil
    }
    
    public func onAudioSessionsChanged()
    {
        let newAudioSessions = self.sessionProvider.sessions
        
        // check if current session no longer exists
        if nil == newAudioSessions.firstIndex(where:{ $0.id == curr_session_id })
        {
            // reset audio session to default Output
            curr_session_id = Self.default_session_id
        }

        sendSessionInfo()
        sendSessionDatas()
    }
    
    //
    // MARK: -- Internal Methods --
    //
    private func is_protocol_compatible() -> Bool
    {
        return kb_protocol_version == MAXMIX_PROTOCOL_VERSION
    }

    //every 2 sec, send session_info
    //session_info also serve as a heartbeat
    private func startRefreshingSessionInfoTimer()
    {
        if self.sendSessionInfoTimer == nil
        {
            //send sessionInfo as heartbeat to keyboard every 2 sec
            self.sendSessionInfoTimer = Timer.scheduledTimer(timeInterval: 2,
                                                             target: self,
                                                             selector: #selector(sendSessionInfo),
                                                             userInfo: nil,
                                                             repeats: true)
        }
    }
    
    private func stopRefreshingSessionInfoTimer()
    {
        self.sendSessionInfoTimer?.invalidate()
        self.sendSessionInfoTimer = nil
    }
    
    private func sendProtocolVersion()
    {
        self.sendUInt16Message(command: PROTOCOL_VERSION_EXCHANGE, data: UInt16(MAXMIX_PROTOCOL_VERSION))
    }
    
    @objc func sendSessionInfo()
    {
        let session_count = UInt8(min(Int(UInt8.max), self.sessionProvider.sessions.count))
        let session_info = SessionInfo(count: session_count)
        self.sendStructMessage(command: SESSION_INFO, data: session_info)
    }
    
    private func prevSessionIndex(index sessionIndex: Int) -> Int?
    {
        if (sessionIndex > 0)
        {
            let prevIndex = sessionIndex - 1
            if self.sessionProvider.sessions.count > prevIndex
            {
                return prevIndex
            }
        }

        return nil
    }

    private func nextSessionIndex(index sessionIndex: Int) -> Int?
    {
        let nextIndex = sessionIndex + 1
        if self.sessionProvider.sessions.count > nextIndex
        {
            return nextIndex
        }

        return nil
    }

    private func makeSessionData(index sessionIndex: Int) -> SessionData?
    {
        let session: AudioSession?
        if sessionIndex >= self.sessionProvider.sessions.count {
            session = self.sessionProvider.sessions.last
        }
        else if sessionIndex >= 0 {
            session = self.sessionProvider.sessions[sessionIndex]
        }
        else {
            session = self.sessionProvider.sessions.first
        }
        
        guard let session = session else {
            return nil
        }

        var utf8Name = session.name.utf8CString
            // pack bytes to the size of SessionData.name
        while utf8Name.count < sessionDataNameLen {
            utf8Name.append(0)
        }
        let namePointer: SessionDataName = utf8Name.withUnsafeBytes { bytes in
            return bytes.bindMemory(to: SessionDataName.self)[0]
        }
        
        let sessionData = SessionData(id: session.id,
                                      name: namePointer,
                                      has_prev: prevSessionIndex(index:sessionIndex) != nil ? 1: 0,
                                      has_next: nextSessionIndex(index:sessionIndex) != nil ? 1 : 0,
                                      volume: VolumeData(unknown: 0, volume: 50, isMuted: 0))
        
        return sessionData;
    }
    
    
    public func sendSessionDatas()
    {
        let curr_session_index = self.sessionProvider.sessions.firstIndex(where: { session in
            session.id == self.curr_session_id
        }) ?? 0 // use Output session as default
         
        let curr_session_data: SessionData = makeSessionData(index: curr_session_index) ?? Self.dummySessionData
        self.sendStructMessage(command: CURRENT_SESSION, data: curr_session_data)
        
        if let prev_session_index = prevSessionIndex(index:curr_session_index),
           let prev_session_data = makeSessionData(index: prev_session_index)
        {
            self.sendStructMessage(command: PREVIOUS_SESSION, data: prev_session_data)
        }
        
        if let next_session_index = nextSessionIndex(index:curr_session_index),
           let next_session_data = makeSessionData(index: next_session_index)
        {
            self.sendStructMessage(command: NEXT_SESSION, data: next_session_data)
        }
    }
    
    private func sendStructMessage<StructDataType>(
        command: Command,
        data msgData: StructDataType)
    {
        sendMessage(command: command, data: msgData, serializer: StructSerializer<StructDataType>())
    }

    private func sendUInt16Message(
        command: Command,
        data msgData: UInt16)
    {
        sendMessage(command: command, data: msgData, serializer: UInt16Serializer())
    }

    private func sendMessage<S: IStructSerializer, DataType>(
        command: Command,
        data msgData: DataType,
        serializer: S)
    where DataType == S.MsgDataT
    {
        if (!Thread.isMainThread)
        {
            DispatchQueue.main.async {
                self.sendMessage(command: command,
                                 data: msgData,
                                 serializer: serializer)
            }
            return
        }
        
        assert(Thread.isMainThread)

        guard let device = self.connectedDevice else
        {
            return
        }
        
        var data: Data = Data()
        data.append(0xFD)
        data.append(command.to_command_id())
        serializer.serialize(data: msgData, into: &data)
        
        let str = """
                  Sending cmd=\(command.displayName)
                  """;
        self.delegate?.log(str)
        
        self.deviceMonitor?.write(data, to: device, reportId: 0)
    }
    

    //
    // MARK: -- HIDDeviceMonitorDelegate --
    //
    public func deviceFound(_ device: HIDDevice) {
    }
    
    public func deviceGone(_ device: HIDDevice) {
        if (device.id == self.connectedDevice?.id)
        {
            self.disconnect()
        }
    }

    public func deviceDataRead(_ device: HIDDevice, data: Data, reportId: UInt32) {
        if (device.id != self.connectedDevice?.id) {
            return
        }

        do
        {
            let uint8Deserializer = UInt8Serializer()
            
            // check if the first byte == MSG_ID_PREFIX
            var dataIndex = 0
            let msgPrefix = try uint8Deserializer.deserialize(from: data, index: &dataIndex)
            if msgPrefix == MSG_ID_PREFIX
            {
                let command_id: Command = Command(rawValue: UInt32(try uint8Deserializer.deserialize(from: data, index: &dataIndex)))
                let command_str: String = command_id.displayName
                
                if command_id == PROTOCOL_VERSION_EXCHANGE
                {
                    kb_protocol_version = try UInt16Serializer().deserialize(from: data, index: &dataIndex)
                    if kb_protocol_version != MAXMIX_PROTOCOL_VERSION
                    {
                        self.delegate?.log("Incompatible protocol version. Disconnect.")
                        self.disconnect()
                    }
                    else
                    {
                        self.sendSessionInfo()
                        self.sendSessionDatas()
                        self.startRefreshingSessionInfoTimer()
                    }
                }
                else if is_protocol_compatible()
                {
                    var new_curr_session_id: UInt8 = self.curr_session_id
                    
                    var msgRead: Bool = true
                    switch (command_id)
                    {
                        case CURRENT_SESSION_CHANGED:
                            new_curr_session_id = try uint8Deserializer.deserialize(from: data, index: &dataIndex)
                            break
                        case VOLUME_UP:
                            new_curr_session_id = try uint8Deserializer.deserialize(from: data, index: &dataIndex)
                            break
                        case VOLUME_DOWN:
                            new_curr_session_id = try uint8Deserializer.deserialize(from: data, index: &dataIndex)
                            break
                        case TOGGLE_MUTE:
                            new_curr_session_id = try uint8Deserializer.deserialize(from: data, index: &dataIndex)
                            break
                        case CMD_OK, CMD_ERR:
                            break
                        default:
                            msgRead = false
                            break
                    }

                    if msgRead
                    {
                        if command_id != CMD_OK
                        {
                            let str = """
                                      Reading cmd=\(command_str) curr_session_id=\(String(new_curr_session_id))
                                      """;
                            self.delegate?.log(str)
                        }
                        
                        if (new_curr_session_id != self.curr_session_id)
                        {
                            self.curr_session_id = new_curr_session_id
                            
                            // curr_session_id is changed, so send update
                            // SessionData to keyboard
                            self.sendSessionDatas()
                        }
                    }
                    else // !msgRead
                    {
                        let str = "Unknown message cmd=\(command_str)";
                        self.delegate?.log(str)
                    }
                        
                } // else if is_protocol_compatible()
            }
            else
            {
                let data_str = data.convertByteDataToString()
                let str = """
                          Reading unrecognized data=\(data_str)
                          """;
                self.delegate?.log(str)
                
            }
        }
        catch
        {
            self.delegate?.log("\(error)")
        }

    }
}
