/*
  WiFiUdp.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define LWIP_INTERNAL

extern "C" 
{
    #include "include/wl_definitions.h"
    #include "osapi.h"
    #include "ets_sys.h"
}

#include "debug.h"
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"
#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "include/UdpContext.h"

/* Constructor */
WiFiUDP::WiFiUDP() : _ctx(0) {}

WiFiUDP::WiFiUDP(const WiFiUDP& other)
{
    _ctx = other._ctx;
    if (_ctx)
        _ctx->ref();
}

WiFiUDP& WiFiUDP::operator=(const WiFiUDP& rhs)
{
    _ctx = rhs._ctx;
    if (_ctx)
        _ctx->ref();
}

WiFiUDP::~WiFiUDP()
{
    if (_ctx)
        _ctx->unref();
}

/* Start WiFiUDP socket, listening at local port */
uint8_t WiFiUDP::begin(uint16_t port) 
{
    if (_ctx)
    {
        _ctx->unref();
    }
    _ctx = new UdpContext;
    ip_addr_t addr;
    addr.addr = INADDR_ANY;
    return (_ctx->listen(addr, port)) ? 1 : 0;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiUDP::available() {
    if (!_ctx)
        return 0;
    return static_cast<int>(_ctx->getSize());
}

/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
    if (_ctx)
        _ctx->disconnect();
    _ctx->unref();
    _ctx = 0;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
    IPAddress remote_addr;
    if (WiFi.hostByName(host, remote_addr))
    {
        return beginPacket(remote_addr, port);
    }
    return 0;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
    ip_addr_t addr;
    addr.addr = ip;

    if (_ctx)
        _ctx->unref();
    _ctx = new UdpContext;
    return (_ctx->connect(addr, port)) ? 1 : 0;
}

int WiFiUDP::endPacket()
{
    if (!_ctx)
        return 0;

    _ctx->send();
}

size_t WiFiUDP::write(uint8_t byte)
{
    return write(&byte, 1);
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
    if (!_ctx)
        return 0;

    return _ctx->append(reinterpret_cast<const char*>(buffer), size);
}

int WiFiUDP::parsePacket()
{
    if (!_ctx)
        return 0;
    if (!_ctx->next())
        return 0;
    return _ctx->getSize();
}

int WiFiUDP::read()
{
    if (!_ctx)
        return -1;
  
    return _ctx->read();  
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
    if (!_ctx)
        return 0;

    return _ctx->read(reinterpret_cast<char*>(buffer), len);
}

int WiFiUDP::peek()
{
    if (!_ctx)
        return 0;

    return _ctx->peek();
}

void WiFiUDP::flush()
{
    if (_ctx)
        _ctx->flush();
}

IPAddress  WiFiUDP::remoteIP()
{
    uint8_t _remoteIp[4] = {0};
    uint8_t _remotePort[2] = {0};

    // WiFiDrv::getRemoteData(_sock, _remoteIp, _remotePort);
    IPAddress ip(_remoteIp);
    return ip;
}

uint16_t  WiFiUDP::remotePort()
{
    uint8_t _remoteIp[4] = {0};
    uint8_t _remotePort[2] = {0};

    // WiFiDrv::getRemoteData(_sock, _remoteIp, _remotePort);
    uint16_t port = (_remotePort[0]<<8)+_remotePort[1];
    return port;
}

