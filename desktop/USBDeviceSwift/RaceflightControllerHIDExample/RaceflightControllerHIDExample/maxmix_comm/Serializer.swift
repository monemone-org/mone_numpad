//
//  StructSerializer.swift
//  RaceflightControllerHIDExample
//
//  Created by Siu Man Hsieh on 2022-04-13.
//  Copyright © 2022 Artem Hruzd. All rights reserved.
//

import Foundation

protocol IStructSerializer
{
    associatedtype MsgDataT
    func serialize(data: MsgDataT, into msg: inout Data)
}

protocol IStructDeserializer
{
    associatedtype MsgDataT
    
    // index: inout param.
    //          in: the starting index of byte in msg to read from
    //          out: the index of byte after the content of MsgDataT
    func deserialize(from msg: Data, index: inout Data.Index) throws -> MsgDataT
}

enum SerializationError: Error
{
    case InsufficiantData
}

struct StructSerializer<StructT>: IStructSerializer
{
    func serialize(data: StructT, into msg: inout Data)
    {
        let structPointer = UnsafeMutablePointer<StructT>.allocate(capacity: 1)
        defer {
            structPointer.deallocate()
        }
        
        structPointer.initialize(repeating: data, count: 1)
        let structSize = MemoryLayout<StructT>.size
        let structBytesPointer: UnsafePointer<UInt8> = UnsafeRawPointer(structPointer).bindMemory(to: UInt8.self, capacity: structSize)
        
        msg.append(structBytesPointer, count: structSize)
    }
}

struct UInt8Serializer: IStructSerializer, IStructDeserializer
{
    func serialize(data: UInt8, into msg: inout Data)
    {
        msg.append(data)
    }

    func deserialize(from msg: Data, index: inout Data.Index) throws -> UInt8 {
        let dataSize = 1 // 1 byte
        if (msg.count - index) < dataSize {
            throw SerializationError.InsufficiantData
        }
        
        let data = msg[index];
        index += dataSize
        
        return data
    }
}

struct UInt16Serializer: IStructSerializer, IStructDeserializer
{
    func serialize(data: UInt16, into msg: inout Data)
    {
        //in big-endian
        let bytes: [UInt8] = [ UInt8(data >> 8), UInt8(data & 0xFF) ]
        msg.append(contentsOf: bytes)
    }

    func deserialize(from msg: Data, index: inout Data.Index) throws -> UInt16 {
        let dataSize = 2 // 2 bytes long
        if (msg.count - index) < dataSize {
            throw SerializationError.InsufficiantData
        }
        
        let data = (UInt16(msg[index]) << 8) | UInt16(msg[index + 1]);
        index += dataSize
        
        return data
    }
}



