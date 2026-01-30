/********************************************************************************
    This file is part of Syncater.

    Syncater is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Syncater is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Syncater.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************/
#ifndef _UDPSERV_H_
#define _UDPSERV_H_
#include <stdint.h>

typedef struct udp_server_t udp_server;


udp_server* udpCreateServer(const char* bind_ip, int port);

typedef struct recv_payload_t {
    int size;
    uint32_t id;
    char* payload;
    char* remote_ip;
    int remote_port;
} udp_recv_payload;

//Send a payload and block until acknowledged by host
int udpSendIDPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id);
int udpSendPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size);
//Send a payload and block until acknowledged by host, or a timeout occured
int udpSendIDPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id, int timeout_ms);
int udpSendPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, int timeout_ms);
//Send a payload and do not block
int udpSendIDPayloadAsync(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id);
int udpSendPayloadAsync(udp_server* serv, const char* host, int port, const char* payload, int payload_size);
//Wait for payload from any host
udp_recv_payload udpRecvPayload(udp_server* serv);
//Wait for payload from any host without blocking
udp_recv_payload udpRecvPayloadWithTimeout(udp_server* serv, int timeout_ms);
udp_recv_payload udpSendAndRecvIDPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id);
udp_recv_payload udpSendAndRecvPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size);
udp_recv_payload udpSendAndRecvIDPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id, int timeout_ms);
udp_recv_payload udpSendAndRecvPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, int timeout_ms);

void udpDestroyServer(udp_server* serv);


#endif


