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
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
//Socket stuff
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
//IP Address stuf
#include <arpa/inet.h>
#include <netinet/in.h>
//Datastructs
#include "libdatastruct/linkedlist.h"
#include "libdatastruct/hashtable.h"
//Threading
#include <pthread.h>
//Local headers
#include "udpserv.h"

#define MAGIK_NUM 0x8DCA
#define MAGIK_ACK 0x8DCB
#define error(...) {fprintf(stderr, "UDPSERV.C: ");fprintf(stderr, __VA_ARGS__);abort();}

#define CLEANUP_INTERVAL 3
#define RETRANSMIT_TIMEOUT 1
#define RETRY_COUNT 5
#define NAME_CACHE_TTL 3600

#define CONCURRENT_PACKETS 1

static time_t _getTime() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec;
}

static char* _getHostFromAddr(struct addrinfo* info) {
    char name[NI_MAXHOST];
    char port[NI_MAXSERV];
    getnameinfo(info->ai_addr, info->ai_addrlen, name, sizeof(name), NULL, 0, NI_NUMERICHOST);
    return strdup(name);
}

static int _getPortFromAddr(struct addrinfo* info) {
    char port[NI_MAXSERV];
    getnameinfo(info->ai_addr, info->ai_addrlen, NULL, 0, port, sizeof(port), NI_NUMERICSERV);
    return atoi(port);
}

static struct addrinfo* _resolveNames(const char* host, int port) {
    char portname[6];
    sprintf(portname, "%d", (uint16_t)port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo* res = NULL;
    int err = getaddrinfo(host, portname, &hints, &res);
    if (err!=0) {
        fprintf(stderr, "Unable to resolve address %s\n",host);
        return NULL;
    }
    return res;
}

struct udp_server_t {
    HT* packet_list_map;
    time_t last_cleanup_time;
    int socket; //Right now we only deal with one socket
    pthread_mutex_t id_mutex;
    uint32_t id;
    HT* resolve_mtu_cache; //Cache the name resolutions and path mtus for host;
    //Server has a thread for recv and send
    pthread_t send_thread;
    pthread_t recv_thread;

    LL* recv_queue;
    LL* send_queue;
    HT* send_state_tracker;
    HT* thread_waiting_tracker;
};

static uint32_t _getId(udp_server* serv) {
    pthread_mutex_lock(&(serv->id_mutex));
    uint32_t id = serv->id++;
    if (id == 0) {
        id = serv->id++;
    }
    pthread_mutex_unlock(&(serv->id_mutex));
    return id;
}

typedef struct payload_t {
    int size;
    char* buffer;
    char* host;
    uint32_t id;
    int port;
} payload;

static payload* _initPayload(const char* buffer, int size, const char* host, int port, uint32_t id) {
    payload* p = mallocAndCheck(sizeof(payload));
    p->buffer = mallocAndCheck(size);
    memcpy(p->buffer, buffer, size);
    p->host = strdup(host);
    p->port = port;
    p->size = size;
    p->id = id;
    return p;
}



void _freePayload(payload* p) {
    free(p->buffer);
    free(p->host);
    free(p);
}

typedef struct waiting_tracker_t {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int ack;
    int waiting_on_payload;
    payload* p;
} waiting_tracker;

static waiting_tracker* _createTracker(int waiting_on_payload) {
    waiting_tracker* w = mallocAndCheck(sizeof(waiting_tracker));
    pthread_mutex_init(&(w->mutex), NULL);
    pthread_cond_init(&(w->condition), NULL);
    w->p = NULL;
    w->ack = 0;
    w->waiting_on_payload = waiting_on_payload;
    return w;
}

static payload* _waitTrackerPayload(waiting_tracker* w, int timeout_ms) {
    pthread_mutex_lock(&(w->mutex));
    payload* p;
    while (w->p == NULL) {
        if (timeout_ms > 0) {
            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time);
            //TODO add ms resolution
            time.tv_sec += timeout_ms / 1000;
            int err = pthread_cond_timedwait(&(w->condition), &(w->mutex), &time);
            if (err == ETIMEDOUT) {
                p = NULL;
                goto cleanup;
            }
        }
        else {
            pthread_cond_wait(&(w->condition), &(w->mutex));
        }
        p = w->p;
    }
cleanup:
    pthread_mutex_unlock(&(w->mutex));
    return p;
}

static int _waitTrackerAck(waiting_tracker* w, int timeout_ms) {
    pthread_mutex_lock(&(w->mutex));
    int ret = 1;
    while (w->ack == 0) {
        if (timeout_ms > 0) {
            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time);
            //TODO add ms resolution
            time.tv_sec += timeout_ms / 1000;
            int err = pthread_cond_timedwait(&(w->condition), &(w->mutex), &time);
            if (err == ETIMEDOUT) {
                ret = 0;
                goto cleanup;
            }
        }
        else {
            pthread_cond_wait(&(w->condition), &(w->mutex));
        }
    }
cleanup:
    pthread_mutex_unlock(&(w->mutex));
    return ret;
}


static void _signalTrackerPayload(waiting_tracker* w, payload* p) {
    if (!w)
        return;
    pthread_mutex_lock(&(w->mutex));
    w->p = p;
    pthread_cond_signal(&(w->condition));
    pthread_mutex_unlock(&(w->mutex));
}

static void _signalTrackerAck(waiting_tracker* w) {
    if (!w)
        return;
    pthread_mutex_lock(&(w->mutex));
    w->ack = 1;
    pthread_cond_signal(&(w->condition));
    pthread_mutex_unlock(&(w->mutex));
}



typedef struct piece_t {
    int sent;
    int ack;
    uint16_t size;
    char* piece;
} piece;

static piece* _initPiece(int mtu, char* buffer, int buffer_size, int which) {
    fprintf(stderr, "buffer_size_init_piece: %u\n", buffer_size);
    piece* p = mallocAndCheck(sizeof(piece));
    if (((which + 1) * mtu) > buffer_size)
        p->size = buffer_size % mtu;
    else {
        p->size = mtu;
    }
//    fprintf(stderr, "size: %d\n", p->size);
    p->piece = mallocAndCheck(p->size);
    memcpy(p->piece, &(buffer[mtu * which]), p->size);
    p->sent = 0;
    p->ack = 0;
    return p;
}

static void _freePiece(piece* p) {
    free(p->piece);
    free(p);
}

typedef struct send_state_t {
    struct addrinfo* info;
    int socket;
    uint32_t id;
    uint32_t finished_point;
    uint32_t num_pieces;
    uint32_t num_sent;
    uint32_t num_ack;
    int last_recv_time;
    int retry_count;
    piece** pieces;
} send_state;

static send_state* _initSendState(struct addrinfo* info, int socket, int mtu, char* buffer, int buffer_size, int id) {
    send_state* s = mallocAndCheck(sizeof(send_state));
    s->socket = socket;
    s->info = info;
    s->id = id;
    s->finished_point = 0;
    s->num_sent = 0;
    s->num_ack = 0;
    s->num_pieces = (buffer_size / mtu) + 1;
    s->last_recv_time = _getTime();
    s->retry_count = 0;
    s->pieces = mallocAndCheck(sizeof(piece*) * s->num_pieces);
    for (unsigned int i = 0; i < s->num_pieces; i++) {
        s->pieces[i] = _initPiece(mtu, buffer, buffer_size, i);
    }
    return s;
}

static void _freeSendState(key_ptr key, send_state* s) {
    for (unsigned int i = 0; i < s->num_pieces; i++) {
        _freePiece(s->pieces[i]);
    }
    free(s->pieces);
    free(s);
}

struct __attribute__((packed)) packet_header {
    uint16_t magik;
    uint32_t seq;
    uint32_t id;
    uint32_t full_seq;
    uint16_t size;
};

struct __attribute__((packed)) packet_ack {
    uint16_t magik;
    uint32_t seq;
    uint32_t id;
};

#define MAX_PAYLOAD_SIZE (65535 - 9 - 20 - sizeof(struct packet_header))
#define DEFAULT_MTU MAX_PAYLOAD_SIZE
//#define DEFAULT_MTU (1500 - 9 - 20 - sizeof(struct packet_header))

typedef struct __attribute__((packed)) packet_t {
    uint16_t magik;
    uint32_t seq;
    uint32_t id;
    uint32_t full_seq;
    uint16_t size;
    char packet_payload[MAX_PAYLOAD_SIZE];
} packet;


static int _createListenSocket(const char* bind_ip, int port) {
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    int server_socket, err;
    if (strlen(bind_ip) == 0) {
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = port;
        server_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_socket < 0) {
            error("Cannot create socket: %s", strerror(errno));
        }
        err = bind(server_socket, (struct sockaddr*)&server, sizeof(server));
    }
    else {
        struct addrinfo *info,*next;
        info = _resolveNames(bind_ip, port);
        for(next = info; next != NULL; next = next->ai_next) {
            server_socket = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
            if (server_socket == -1)
                continue;
            if ((err = bind(server_socket, next->ai_addr, next->ai_addrlen)) == 0)
                break;
        }
        freeaddrinfo(info);
    }
    if (err < 0) {
        error("Unable to bind socket\n");
        close(server_socket);
        return -1;
    }
    return server_socket;
}

typedef struct packet_list_t {
    time_t time;
    unsigned int count;
    unsigned int size;
    unsigned int total_bytes;
    packet** packets;
} packet_list;

static char* _checkAndDoAssembly(packet_list* pl) {
//    fprintf(stderr, "pl: %d, %d\n", pl->count, pl->size);
    if (pl->count < pl->size)
        return NULL;
    //Check if a continous sequence exist
    for (unsigned int i = 0; i < pl->size; i++) {
        if (pl->packets[i] == NULL) {
            fprintf(stderr, "missing: %u\n", i);
            return NULL;
        }
    }
    //We are good and can assemble the packet
    char* buffer = mallocAndCheck(pl->total_bytes);
    int buf_offset = 0;
    for (unsigned int i = 0; i < pl->size; i++) {
        memcpy(&(buffer[buf_offset]), pl->packets[i]->packet_payload, pl->packets[i]->size);
        buf_offset += pl->packets[i]->size;
        free(pl->packets[i]);
    }
    return buffer;
}

static void _deletePacketList(const key_ptr key, value_ptr value) {
    packet_list* pl = (packet_list*) value;
    free(pl->packets);
    free(pl);
}



static packet_list* _createPacketList(unsigned int size) {
    packet_list* pl = mallocAndCheck(sizeof(packet_list));
    pl->time = _getTime();
    pl->count = 0;
    pl->size = size;
    pl->total_bytes = 0;
    pl->packets = mallocAndCheck(sizeof(packet*) * size);
    for (unsigned int i = 0; i < size; i++)
        pl->packets[i] = NULL;
    return pl;
}


static void _cleanupPacketList(const key_ptr key, value_ptr value, void* params) {
    packet_list* pl = (packet_list*)value;
    time_t time = _getTime();
    udp_server* serv = params;
    if (time - pl->time > 10) {
        fprintf(stderr, "Payload from %s timed out", key);
        htRemove(serv->packet_list_map, key);
    }
}

static void _cleanupPacketListTable(udp_server* serv) {
    htForEach(serv->packet_list_map, _cleanupPacketList, (void*)&time);
}

static void _signalAck(udp_server* serv) {
    llLock(serv->send_queue);
    llPushBack(serv->send_queue, NULL);
    llUnlock(serv->send_queue);
}

static payload* _getPayload(udp_server* serv) {
    if (_getTime() - serv->last_cleanup_time > CLEANUP_INTERVAL) {
        _cleanupPacketListTable(serv);
    }

    packet p;
    struct sockaddr_storage src_addr;
//    struct sockaddr* addr = (struct sockaddr*)&src_addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    for(;;) {
        ssize_t size = recvfrom(serv->socket, &p, sizeof(packet), 0, (struct sockaddr*)&src_addr, &addrlen);
        if (size < 0)
            error("%s", strerror(errno));
        char host[NI_MAXHOST];
        char port[NI_MAXSERV];
        getnameinfo((struct sockaddr*)&src_addr, addrlen, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
        if (p.magik == MAGIK_ACK) {
           uintptr_t id = p.id;
           uint32_t seq = p.seq;

           htLock(serv->send_state_tracker);
           send_state* s = htSearch(serv->send_state_tracker, (value_ptr)id);
           if (s) {
               if (seq >= s->num_pieces)
                   fprintf(stderr, "ack for non existent packet seq %d id %u\n", seq, (uint32_t) id);
               else {
                   piece* pc = s->pieces[seq];
                   pc->ack = 1;
                   s->last_recv_time = _getTime();
                   s->num_ack++;
                   if (s->finished_point == seq-1)
                       s->finished_point++;
               }
           }
           else
               fprintf(stderr, "ack for non existent packet id %u seq %u\n", (uint32_t)id, seq );
           htUnlock(serv->send_state_tracker);
           _signalAck(serv);
           continue;
        }
        else if (p.magik != MAGIK_NUM) {
            fprintf(stderr, "Spurious packet from %s:%s\n", host, port);
            continue;
        }
//        fprintf(stderr, "packet from %s port %s id: %d seq %d:\n", host, port, p.id, p.seq);
        struct packet_ack ack;
        ack.magik = MAGIK_ACK;
        ack.id = p.id;
        ack.seq = p.seq;
        int ack_size = sizeof(struct packet_ack);
        int ack_sent = sendto(serv->socket, &ack, ack_size, 0, (struct sockaddr*)&src_addr, addrlen);
        if (ack_sent == -1)
            error("%s", strerror(errno));
        if (ack_sent != ack_size)
            error("Expected number of bytes not sent: %d of %d sent\n", ack_sent, ack_size);
        char index[NI_MAXHOST + NI_MAXSERV + 10];
        sprintf(index, "%s:%s,%d", host, port, p.id);
        htLock(serv->packet_list_map);
        packet_list* pl = htSearch(serv->packet_list_map, index);
        if (pl == NULL) {
            pl = _createPacketList(p.full_seq);
            htInsert(serv->packet_list_map, index, pl);
        }
        packet* save_p = mallocAndCheck(sizeof(struct packet_header) + p.size);

        save_p = memcpy(save_p, &p, sizeof(struct packet_header) + p.size);
        if (p.seq >= pl->size) {
            fprintf(stderr, "seq %u is bigger than max_seq %u\n", p.seq, pl->size);
        } else {
            if (!pl->packets[p.seq]) {
                pl->count++;
                pl->total_bytes += p.size;
                pl->packets[p.seq] =  save_p;
            }
            else {
                fprintf(stderr, "dup packet seq %u id %u\n", p.seq, p.id);
                free(save_p);
            }
        }
        char* payload_buffer = _checkAndDoAssembly(pl);
        if (payload_buffer) {
            payload* ret = mallocAndCheck(sizeof(payload));
            ret->port = atoi(port);
            ret->host = strdup(host);
            ret->buffer = payload_buffer;
            ret->id = p.id;
            ret->size = pl->total_bytes;
            htRemove(serv->packet_list_map, index);
            htUnlock(serv->packet_list_map);
            return ret;
        }
        htUnlock(serv->packet_list_map);
    }
}



typedef struct addr_mtu_info_t {
    struct addrinfo* info;
    int mtu;
    time_t _time;
} addr_mtu_info;

static addr_mtu_info* _getAddrAndMtu(udp_server* serv, const char* host, int port) {
    char index[NI_MAXHOST + NI_MAXSERV + 2];
    sprintf(index, "%s:%d", host, port);
    htLock(serv->resolve_mtu_cache);
    addr_mtu_info* n = htSearch(serv->resolve_mtu_cache, index);
    time_t  time = _getTime();
    if (!n) {
        n = mallocAndCheck(sizeof(addr_mtu_info));
        n->mtu = DEFAULT_MTU;
        n->_time = 0;
        n->info = NULL;
        htInsert(serv->resolve_mtu_cache, index, n);
    }
    if (time - n->_time > NAME_CACHE_TTL) {
       if (n->info)
           freeaddrinfo(n->info);
           //TODO This could be an unsafe situation as the addrinfo could exist in some send state, however this is highly unlikely
       n->info = _resolveNames(host, port);
       n->_time = time;
    }
    htUnlock(serv->resolve_mtu_cache);
    return n;
}


static void _createPayloadState(udp_server* serv, payload* p) {
    intptr_t id = p->id;
    htLock(serv->send_state_tracker);
    send_state* s = htSearch(serv->send_state_tracker, (value_ptr)id);
    if (s != NULL) {
        error("Send state tracker corrupted: id %u\n", (unsigned int)id);
    }
    addr_mtu_info* info = _getAddrAndMtu(serv, p->host, p->port);
    s = _initSendState(info->info, serv->socket, info->mtu, p->buffer, p->size, p->id);
    htInsert(serv->send_state_tracker, (key_ptr)id, s);
    fprintf(stderr, "%d\n", s->id);
    htUnlock(serv->send_state_tracker);
}

static void _sendPiece(send_state* s, piece* p, int seq) {
    packet pak;
    pak.magik = MAGIK_NUM;
    pak.size = p->size;
    pak.id = s->id;
    pak.seq = seq;
    pak.full_seq = s->num_pieces;
    memcpy(pak.packet_payload, p->piece, p->size);
    int size = sizeof(struct packet_header) + pak.size;
    char* host = _getHostFromAddr(s->info);
    int port = _getPortFromAddr(s->info);
//    fprintf(stderr, "sending packet of id %d size %d seq %d to %s port %d\n", s->id, pak.size, pak.seq, host, port);
    free(host);
    int sent = sendto(s->socket, &pak, size, 0, s->info->ai_addr, s->info->ai_addrlen);
    if (sent == -1)
        error("%s", strerror(errno));
    if (sent != size)
        error("Expected number of bytes not sent: %d of %d sent\n", sent, size);
    p->sent = 1;
}


static void _processSingleSendState(const key_ptr key, value_ptr value, void* params) {
    send_state* s = value;
    int diff = s->num_sent - s->num_ack;
    udp_server* serv = (udp_server*)params;
    time_t time = _getTime();
    if (diff < 0)
        diff = 0;
    if (diff <= CONCURRENT_PACKETS) {
        unsigned int current = s->finished_point;
        for (unsigned int i = diff; i < CONCURRENT_PACKETS && current < s->num_pieces; i++) {
            piece* p;
            do {
                p = s->pieces[current];
                current++;
            } while(p->ack && current < s->num_pieces);
            if (!p->sent)
                _sendPiece(s, p, current-1);
        }
    }
    if ((time - s->last_recv_time) > RETRANSMIT_TIMEOUT) {
        if (s->retry_count >= RETRY_COUNT) {
            char* host =  _getHostFromAddr(s->info);
            fprintf(stderr, "Timed out transmitting to %s id %d\n", host, s->id);
            free(host);
            htQueueRemove(serv->send_state_tracker, key);
        }
        for (unsigned int i = 0; i < s->num_pieces; i++) {
           piece* p = s->pieces[i];
           if (p->sent && !p->ack) {
               fprintf(stderr, "retransmit %d\n", i);
               _sendPiece(s, p, i);
           }
        }
        s->last_recv_time = time;
        s->retry_count++;
    }
    if (s->num_ack == s->num_pieces) {
        //We push an ack because it has all been sent
        htLock(serv->thread_waiting_tracker);
        uintptr_t id = s->id;
        waiting_tracker* w = htSearch(serv->thread_waiting_tracker, (value_ptr)id);
        _signalTrackerAck(w);
        htUnlock(serv->thread_waiting_tracker);
        htQueueRemove(serv->send_state_tracker, key);
    }
}

static void _processSendState(udp_server* serv) {
    htLock(serv->send_state_tracker);
    htForEach(serv->send_state_tracker, _processSingleSendState, serv);
    htRemoveQueued(serv->send_state_tracker);
    htUnlock(serv->send_state_tracker);
}

static void* _recvThread(void* serv_ptr) {
    udp_server* serv = (udp_server*) serv_ptr;
    for(;;) {
        payload* p = _getPayload(serv);
        //Check if there is a thread waiting for this payload
        //lock the thread tracker map
        htLock(serv->thread_waiting_tracker);
        uintptr_t id = p->id;
        //grab the tracker
        waiting_tracker* w = htSearch(serv->thread_waiting_tracker, (value_ptr)id);
        int waiting_on_payload = 0;
        //If there is a tracker registered, see if it is for waiting for an ack or a payload
        if (w)
            waiting_on_payload = w->waiting_on_payload;
        //If it is waiting on a payload, then signal the tracker with the payload
        if (w && waiting_on_payload)
            _signalTrackerPayload(w, p);
        //unlock the tracker map
        htUnlock(serv->thread_waiting_tracker);

        //If there is no tracker or the tracker is for an ack, then push the payload on the general
        //recv queue
        if (!w || !waiting_on_payload) {
            llLock(serv->recv_queue);
            llPushBack(serv->recv_queue, p);
            llUnlock(serv->recv_queue);
        }
    }
    return NULL;
}

static void* _sendThread(void* serv_ptr) {
    udp_server* serv = (udp_server*) serv_ptr;
    for(;;) {
        llLock(serv->send_queue);
        while(llGetCount(serv->send_queue) > 0) {
            payload* p = llGetFront(serv->send_queue);
            llDeleteFront(serv->send_queue);
            //fprintf(stderr, "process payload %d\n", p->id);
            if (p) {
                _createPayloadState(serv, p);
                _freePayload(p);
            }
            _processSendState(serv);
        }
        llWaitForChangeWithTimeout(serv->send_queue, 1000);
        //Whether we have new data lets check for timeouts.
        _processSendState(serv);
        llUnlock(serv->send_queue);
    }
    return NULL;
}


udp_server* udpCreateServer(const char* bind_ip, int port) {
    int socket = _createListenSocket(bind_ip, port);
    if (!socket)
        return NULL;
    udp_server* serv = mallocAndCheck(sizeof(udp_server));
    serv->socket = socket;
    serv->id = 1;
    pthread_mutex_init(&(serv->id_mutex), NULL);
    serv->packet_list_map = htCreateWithStringHash();
    serv->resolve_mtu_cache = htCreateWithStringHash();
    serv->last_cleanup_time = _getTime();
    //init recv queue
    serv->recv_queue = llCreate();
    //init send queue
    serv->send_queue = llCreate();
    //init ack
    serv->send_state_tracker = htCreateWithIntHash();
    htSetOnremoveCallback(serv->send_state_tracker, (ht_fn_onremove)_freeSendState);
    serv->thread_waiting_tracker = htCreateWithIntHash();
    pthread_create(&(serv->recv_thread), NULL, _recvThread, (void*) serv);
    pthread_create(&(serv->send_thread), NULL, _sendThread, (void*) serv);
    return serv;
}

static int _sendPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id, int block, int timeout_ms) {
    waiting_tracker* w = NULL;
    uintptr_t id = response_id;
    if (id == 0) {
        id = _getId(serv);
    }
    int retval = 1;
    if (block) {
        htLock(serv->thread_waiting_tracker);
        w = _createTracker(0);
        htInsert(serv->thread_waiting_tracker, (key_ptr)id, w);
        htUnlock(serv->thread_waiting_tracker);
    }
    llLock(serv->send_queue);
    llPushBack(serv->send_queue, _initPayload(payload, payload_size, host, port, id));
    llUnlock(serv->send_queue);
    if (block) {
        retval = _waitTrackerAck(w, timeout_ms);
        htLock(serv->thread_waiting_tracker);
        htRemove(serv->thread_waiting_tracker, (key_ptr)id);
        htUnlock(serv->thread_waiting_tracker);
    }
    return retval;
}

int udpSendIDPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id) {
    return _sendPayload(serv, host, port, payload, payload_size, response_id, 1, 0);
}

int udpSendPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size) {
    return _sendPayload(serv, host, port, payload, payload_size, 0, 1, 0);
}

int udpSendIDPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id, int timeout_ms) {
    return _sendPayload(serv, host, port, payload, payload_size, response_id, 1, timeout_ms);
}

int udpSendPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, int timeout_ms) {
    return _sendPayload(serv, host, port, payload, payload_size, 0, 1, timeout_ms);
}

int udpSendIDPayloadAsync(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id) {
    return _sendPayload(serv, host, port, payload, payload_size, response_id, 0, 0);

}

int udpSendPayloadAsync(udp_server* serv, const char* host, int port, const char* payload, int payload_size) {
    return _sendPayload(serv, host, port, payload, payload_size, 0, 0, 0);
}


static payload* _recvPayload(udp_server* serv, int timeout_ms) {
    llLock(serv->recv_queue);
    while(llGetCount(serv->recv_queue) == 0) {
        if (timeout_ms > 0) {
            llWaitForChangeWithTimeout(serv->recv_queue, timeout_ms);
        }
        else
            llWaitForChange(serv->recv_queue);
    }
    payload* p = llGetFront(serv->recv_queue);
    llDeleteFront(serv->recv_queue);
    llUnlock(serv->recv_queue);
    return p;
}

static payload* _sendAndRecvPayloadId(udp_server* serv, const char* host, int port, const char* payload_buffer, int payload_size, int timeout_ms, uint32_t id) {
    //If we don't specify an ID, generate a new one
    if (id == 0) {
        id = _getId(serv);
    }
    //Create a new payload recieved tracker
    //lock the tracker map
    htLock(serv->thread_waiting_tracker);
    //create the tracker (1) means it's a payload tracker
    waiting_tracker* w = _createTracker(1);
    //Insert on the id we want to track
    intptr_t id_index = id;
    htInsert(serv->thread_waiting_tracker, (key_ptr)id_index, w);
    //unlock the tracker map
    htUnlock(serv->thread_waiting_tracker);

    //Send the payload without blocking for ack
    _sendPayload(serv, host, port, payload_buffer, payload_size, id, 0, 0);

    //Wait for tracker
    payload* p = _waitTrackerPayload(w, timeout_ms);

    //Remove the tracker from the tracker map, and free it
    htLock(serv->thread_waiting_tracker);
    htRemove(serv->thread_waiting_tracker, (key_ptr)id_index);
    free(w);
    htUnlock(serv->thread_waiting_tracker);
    return p;
}

static udp_recv_payload _convertPayloadToRetval(payload* p) {
    udp_recv_payload pld;
    if (p) {
        pld.remote_ip = p->host;
        pld.remote_port = p->port;
        pld.payload = p->buffer;
        pld.size = p->size;
        pld.id = p->id;
        free(p);
    }
    else {
        pld.remote_ip = NULL;
        pld.remote_port = -1;
        pld.payload = NULL;
        pld.size = p->size;
        pld.id = 0;
    }
    return pld;
}

udp_recv_payload udpRecvPayload(udp_server* serv) {
    return _convertPayloadToRetval(_recvPayload(serv, 0));
}

udp_recv_payload udpRecvPayloadWithTimeout(udp_server* serv, int timeout_ms) {
    return _convertPayloadToRetval(_recvPayload(serv, timeout_ms));
}

udp_recv_payload udpSendAndRecvIDPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id) {
    return _convertPayloadToRetval(_sendAndRecvPayloadId(serv, host, port, payload, payload_size, response_id, 0));
}

udp_recv_payload udpSendAndRecvPayload(udp_server* serv, const char* host, int port, const char* payload, int payload_size) {
    return _convertPayloadToRetval(_sendAndRecvPayloadId(serv, host, port, payload, payload_size, 0, 0));
}

udp_recv_payload udpSendAndRecvIDPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, uint32_t response_id, int timeout_ms) {
    return _convertPayloadToRetval(_sendAndRecvPayloadId(serv, host, port, payload, payload_size, response_id, timeout_ms));
}

udp_recv_payload udpSendAndRecvPayloadWithTimeout(udp_server* serv, const char* host, int port, const char* payload, int payload_size, int timeout_ms) {
    return _convertPayloadToRetval(_sendAndRecvPayloadId(serv, host, port, payload, payload_size, 0, timeout_ms));

}
















