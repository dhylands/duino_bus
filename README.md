# duino_bus

[TOC]

Arduino Library for parsing/encoding packets and sending them over a bus (i.e. serial or socket)

## Packet

This class describes the in-memory format of the packet.

## PacketEncoder

Encodes a packet from it's in-memory format to it's over-the-wire format.

## PacketDecoder

Decodes a packet from it's over-the-wire format into it's in-memory format.

## Packer

Helper class for packing data into a packet.

## Unpacker

Helper class for unpacking data from a packet.

## IBus

Abstract base class for implementing a bus, which sends/receives packets over a bus.

## IPacketHandler

Abstract base class for implementing a packet handler.
