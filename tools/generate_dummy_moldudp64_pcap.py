#!/usr/bin/env python3
from scapy.all import *
import struct

# MoldUDP64 constants
SESSION_ID = b"SESSION1"  # 10 bytes
UDP_SPORT = 12345
UDP_DPORT = 9000

# ITCH AddOrder message constants (sizes from your C++ struct)
ADD_ORDER_SIZE = 37  # bytes
MESSAGE_TYPE_ADD_ORDER = b"A"  # just using char 'A'

def make_add_order(order_id, orderbook_id, side, orderbook_pos, quantity, price):
    """
    Pack AddOrder into binary format exactly like your C++ struct
    """
    nanoseconds = 123456789  # dummy
    order_attributes = 0
    lot_type = 0

    # Pack using big-endian for network (like be32toh / be64toh in C++)
    # struct: type(1), nanoseconds(4), order_id(8), orderbook_id(4), side(1),
    # orderbook_position(4), quantity(8), price(4), order_attributes(2), lot_type(1)
    return struct.pack(
        ">cI Q I c I Q I H B",
        MESSAGE_TYPE_ADD_ORDER,
        nanoseconds,
        order_id,
        orderbook_id,
        side.encode(),
        orderbook_pos,
        quantity,
        price,
        order_attributes,
        lot_type,
    )

def make_moldudp64_packet(sequence_number, messages):
    """
    MoldUDP64 header + message blocks
    """
    session = SESSION_ID.ljust(10, b"\x00")  # pad to 10 bytes
    msg_count = len(messages)
    header = session + struct.pack(">Q", sequence_number) + struct.pack(">H", msg_count)

    payload = b""
    for msg in messages:
        msg_len = struct.pack(">H", len(msg))
        payload += msg_len + msg
    return header + payload

def main():
    packets = []

    # create 3 dummy AddOrder messages
    add_orders = [
        make_add_order(1001, 1, "B", 1, 100, 1000),
        make_add_order(1002, 1, "S", 2, 200, 1010),
        make_add_order(1003, 1, "B", 3, 150, 1020),
    ]

    # single MoldUDP64 packet with 3 messages
    mold_packet = make_moldudp64_packet(sequence_number=1, messages=add_orders)
    pkt = (
        Ether() /
        IP(src="192.168.0.1", dst="239.255.0.1") /
        UDP(sport=UDP_SPORT, dport=UDP_DPORT) /
        Raw(load=mold_packet)
    )
    packets.append(pkt)

    # save to PCAP
    wrpcap("dummy_add_order.pcap", packets)
    print("dummy_add_order.pcap generated with 1 MoldUDP64 packet containing 3 AddOrder messages.")

if __name__ == "__main__":
    main()
