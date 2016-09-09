/* 
 *   Copyright (c)
 *     (c) 2015-2016 Chintalagiri Shashank, Quazar Technologies Pvt. Ltd.
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
#include <printf/printf.h>

#if UUID_SUPPORT_v1
    #include <ucdm/ucdm.h>
    #include <time/time.h>
#endif

#if UUID_SUPPORT_v3
    #include <crypto/md5/md5.h>
#endif

#if UUID_SUPPORT_v5
    #include <crypto/sha1/sha1.h>
#endif


uint8_t hashstage[64];
md5_ctx_t md5ctx;

sha1_ctx_t sha1ctx;
sha1_hash_t sha1hash;

uint16_t uuid_init(uint16_t ucdm_next_address){
    #if UUID_SUPPORT_v1
        ucdm_redirect_regw_ptr(ucdm_next_address, &uuid_clk_seq);
        ucdm_next_address += 1;
    #endif
    return ucdm_next_address;
}


void uuid_clear(uuid_t * out)
{
    memset((void *) out, 0, sizeof(uuid_t));
}

void uuid_sprintf(char * bufp, uuid_t * uuid){
    for (uint8_t i=0; i<16; i++){
        sprintf(bufp, "%x", uuid->b[i]);
        bufp += 2;
        if (i == 3 || i == 5 || i == 7 || i == 9){
            *(bufp++) = '-';
        }
    }
}

static void uuid_setversion(uuid_t * out, uint8_t ver_m);

static void uuid_setversion(uuid_t * out, uint8_t ver_m){
    out->b[8] &= ~0xC0;
    out->b[8] |= 0x80;
    out->b[6] &= ~0xF0;
    out->b[6] |= ver_m;
}

#if UUID_SUPPORT_v1
uint16_t uuid_clk_seq;

void uuid1(uuid_t * out){
    uuid_setversion(out, (1<<4));
}
#endif

#if UUID_SUPPORT_v3
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
    uuid_setversion(out, (3<<4));
}
#endif 

#if UUID_SUPPORT_v4
void uuid4(uuid_t * out){
    uuid_setversion(out, (4<<4));
}
#endif

#if UUID_SUPPORT_v5
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
                name_p += MD5_BLOCK_BYTES;
                len -= MD5_BLOCK_BYTES;
            }
        }
    }
    sha1_ctx2hash(&sha1hash, &sha1ctx);
    memcpy((void *)out, (void *)(&sha1hash), 16);
    uuid_setversion(out, (5<<4));
}
#endif
