/* 
 *   Copyright (c)
 *     (c) 2015-2018 Chintalagiri Shashank, Quazar Technologies Pvt. Ltd.
 *   
 *   This file is part of
 *   Embedded bootstraps : uuid library
 *   
 *   This library is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

/**
 * @file uuid.c
 * @brief Implementation of the UUID generation library.
 */

#include "uuid.h"
#include <string.h>
#include <ucdm/descriptor.h>
#include <printf/printf.h>

// v1 or v6 
#include <time/time.h>
static uuid_t uuid1_template;
static struct UUID1_STATE{
    uint64_t ts_last;
    uint16_t uuids_this_tick;
}uuid1_state = {0,0};
static void _uuid1_template_init(void);

// v3
#include <crypto/md5/md5.h>
static md5_ctx_t md5ctx;
    
// v5
#include <crypto/sha1/sha1.h>
static sha1_ctx_t sha1ctx;
static sha1_hash_t sha1hash;

// v3 or v5
static uint8_t hashstage[64];

/** @brief UUID Library Version Descriptor */
static descriptor_custom_t uuid_descriptor = {NULL, DESCRIPTOR_TAG_LIBVERSION,
    sizeof(UUID_VERSION), DESCRIPTOR_ACCTYPE_PTR, {UUID_VERSION}};

    
void uuid_init(void){
    _uuid1_template_init();
    return;
}

void uuid_install_descriptor(void)
{
    descriptor_install(&uuid_descriptor);
}


void uuid_clear(uuid_t * uuid)
{
    memset((void *) uuid, 0, sizeof(uuid_t));
}


void uuid_sprintf(char * bufp, uuid_t * uuid){
    for (uint8_t i=0; i<16; i++){
        sprintf(bufp, "%02x", uuid->b[i]);
        bufp += 2;
        if (i == 3 || i == 5 || i == 7 || i == 9){
            *(bufp++) = '-';
        }
    }
}


static void _uuid_setversion(uuid_t * out, uint8_t ver_m){
    out->b[8] &= ~0xC0;
    out->b[8] |= 0x80;
    out->b[6] &= ~0xF0;
    out->b[6] |= ver_m;
}


static void _uuid1_template_init(void){
    // Write node
    id_read(6, (void *)(&uuid1_template.s.node[0]));
    // Set multicast bit
    uuid1_template.s.node[5] |= (uuid1_template.s.node[5] & 0x01);
    // Write clkseq
    uuid1_template.s.clock_seq_low = rand_byte();
    uuid1_template.s.clock_seq_hi_and_reserved = rand_byte();
}


static void _uuid1(uuid_t * out, void (*ts_writer)(uuid_t *, uint8_t *)){
    memcpy(out, &uuid1_template, sizeof(uuid_t));
    // Write time
    tm_system_t stime;
    union TS{
        uint64_t i;
        uint8_t b[8];
    }ts;
    tm_current_time(&stime);
    ts.i = stime.seconds * 10000000 + stime.frac * TIME_TICKS_PER_SECOND * 10 + UUID_EPOCH_OFFSET;
    if (ts.i == uuid1_state.ts_last){
        uuid1_state.uuids_this_tick++;
    }
    else{
        uuid1_state.ts_last = ts.i;
        uuid1_state.uuids_this_tick = 0;
    }
    ts.i += uuid1_state.uuids_this_tick;
    ts_writer(out, &(ts.b[0]));
}


static void _uuid1_ts_writer1(uuid_t * out, uint8_t * tsb){
    // time_low
    out->b[0] = tsb[3];
    out->b[1] = tsb[2];
    out->b[2] = tsb[1];
    out->b[3] = tsb[0];
    // time_mid
    out->b[4] = tsb[5];
    out->b[5] = tsb[4];
    // time_high
    out->b[6] = tsb[7];
    out->b[7] = tsb[6];
    _uuid_setversion(out, (1<<4));
}

void uuid1(uuid_t * out){
    _uuid1(out, &_uuid1_ts_writer1);
}


static void _uuid1_ts_writer6(uuid_t * out, uint8_t * tsb){
    // time_low
    for (uint8_t i=0, j=7; i<6; i++, j--){
        out->b[i] = (tsb[j] << 4) | ((tsb[j-1] & 0xF0) >> 4);
    }
    out->b[6] = tsb[1] & 0x0F;
    out->b[7] = tsb[0];
    _uuid_setversion(out, (6<<4));
}

void uuid6(uuid_t * out){
    _uuid1(out, &_uuid1_ts_writer6);
}


void uuid3(uuid_t * out, uuid_t * ns, uint8_t * name_p, uint8_t len){
    md5_init(&md5ctx);
    uint8_t t;
    
    memcpy((void *)&(hashstage[0]), ns, sizeof(uuid_t));
    t = MD5_BLOCK_BYTES - sizeof(uuid_t);
    if (len <= t){
        t = len;
    }
    memcpy((void *)&(hashstage[sizeof(uuid_t)]), name_p, t);
    name_p += t;
    len -= t;
    if (!len){
        md5_lastBlock(&md5ctx, &(hashstage[0]), (sizeof(uuid_t) + t) * 8);
    }
    else{
        md5_nextBlock(&md5ctx, &(hashstage[0]));
        while (len){
            if (len <= MD5_BLOCK_BYTES){
                md5_lastBlock(&md5ctx, name_p, len * 8);
                len = 0;
            }
            else{
                md5_nextBlock(&md5ctx, name_p);
                name_p += MD5_BLOCK_BYTES;
                len -= MD5_BLOCK_BYTES;
            }
        }
    }
    md5_ctx2hash((md5_hash_t *)&(out->b), &md5ctx);
    _uuid_setversion(out, (3<<4));
}


void uuid4(uuid_t * out){
    uint8_t i;
    for (i = 0; i < 16; i++){
        out->b[i] = rand_byte();
    }
    _uuid_setversion(out, (4<<4));
}


void uuid5(uuid_t * out, uuid_t * ns, uint8_t * name_p, uint8_t len){
    sha1_init(&sha1ctx);
    uint8_t t;
    
    memcpy((void *)&(hashstage[0]), ns, sizeof(uuid_t));
    t = SHA1_BLOCK_BYTES - sizeof(uuid_t);
    if (len <= t){
        t = len;
    }
    memcpy((void *)&(hashstage[sizeof(uuid_t)]), name_p, t);
    name_p += t;
    len -= t;
    if (!len){
        sha1_lastBlock(&sha1ctx, &(hashstage[0]), (sizeof(uuid_t) + t) * 8);
    }
    else{
        sha1_nextBlock(&sha1ctx, &(hashstage[0]));
        while (len){
            if (len <= SHA1_BLOCK_BYTES){
                sha1_lastBlock(&sha1ctx, name_p, len * 8);
                len = 0;
            }
            else{
                sha1_nextBlock(&sha1ctx, name_p);
                name_p += SHA1_BLOCK_BYTES;
                len -= SHA1_BLOCK_BYTES;
            }
        }
    }
    sha1_ctx2hash(&sha1hash, &sha1ctx);
    memcpy((void *)out, (void *)(&sha1hash), 16);
    _uuid_setversion(out, (5<<4));
}
