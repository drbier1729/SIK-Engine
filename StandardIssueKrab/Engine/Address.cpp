#include "stdafx.h"

#include "Address.h"

/*
* Create an Ipaddress from 4 unit8 values
*/
Address::Address(Uint8 a, Uint8 b, Uint8 c, Uint8 d, Uint16 _port) {
    ip_addr = (a << 24) |
              (b << 16) |
              (c << 8) |
              d;

    port = _port;
}

/*
* Create an IP address from a full 32bit Uint
*/
Address::Address(Uint32 _ip_addr, Uint16 _port) {
    ip_addr = _ip_addr;
    port = _port;
}

Uint32 Address::GetAddress() const{
    return ip_addr;
}

Uint16 Address::GetPort() const {
    return port;
}

void Address::SetAddress(Uint32 _ip_addr) {
    ip_addr = _ip_addr;
}

void Address::SetPort(Uint16 _port) {
    port = _port;
}


