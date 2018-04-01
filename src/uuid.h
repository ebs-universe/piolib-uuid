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
 * @file uuid.h
 * @brief Public include header for the UUID generation library.
 * 
 * This library provides UUID generation functions, to generate UUIDs 
 * compliant with RFC4122.
 * 
 * Version 1 : Date-time and MAC address
 * Version 2 : DCE security              (no support planned)
 * Version 3 : MD5 Hash and Namespace
 * Version 4 : Random           
 * Version 5 : SHA1 Hash and Namespace
 * Version 6 : Non-standard variant
 * 
 * Note that the bulk of this library is NOT re-entrant. It is the 
 * application's responsibility to ensure that a uuid generation function 
 * is not called while a previous one is still in process.
 * 
 * Support for version 1
 * ---------------------
 * 
 * Date-time is utilized from the EBS time library, which must be included in 
 * the application. If you intend to use version 1 UUIDs, you should ensure 
 * that the time library is initialized, the systick timer is installed and 
 * started, and time is synchronized to real time - either from an RTC or by 
 * the Host before the v1 UUID is generated. This library does nothing to 
 * make sure that the  used system time makes any kind of sense. 
 * 
 * Clock sequence is simply generated randomly using the rand_byte() function,
 * which should be available at link time. Within the EBS ecosystem, this 
 * function would typically be available from sys/rand.h, and would depend on 
 * a PRNG seeded by a hardware entropy source. For better compliance with 
 * RFC4122, the clock sequence should be written to some form of non 
 * volatile storage. Such an implementation is left to the application.
 * 
 * @see <https://github.com/chintal/ebs-lib-time>
 * 
 * The 'node' or 'MAC address' used is platfom (and application) dependent. 
 * This implementation will simply use the first 6 bytes of whatever unique 
 * identifier is available on the platform via the uC HAL's id interface.
 * Ideally, the node should be constructed using a hash of the entire 
 * identifier, but that is not done here to keep code relatively small. 
 * This may be changed in the future if v1 UUIDs are used often. 
 * 
 * Support for version 6
 * ---------------------
 * 
 * This is a non-standard variant of the version 1 UUID which allows for 
 * chronological sorting when treated as an arbitrary string of bytes.
 * 
 * @see <https://bradleypeabody.github.io/uuidv6/>
 * 
 * Support for versions 3 and 5
 * ----------------------------
 * 
 * For version 3 and 5 UUID generation, the namespace should be a UUID in of 
 * itself, and can either be generated using one of the functions here or 
 * pre-installed into a UUID type variable. MD5 and SHA1 hashes are generated 
 * using the EBS repackaged version of AVR Crypto Lib. Note that due to the 
 * dependency on cryptolib, which is in turn a derivative AVR Crypto Lib, 
 * this library is necessarily licensed under the GPLv3 without any linking 
 * exceptions. Applications using this library must also, consequently, be 
 * distributed under the terms of the GPLv3 license.
 * 
 * It may be noted that the v5 implementation may be switched to SHA256 
 * instead of SHA1, since SHA256 is more likely to be used in the application
 * otherwise, including as part of the PRNG (entropium). This will make the
 * implementation less compliant with the letter of the RFC4122.
 * 
 * @see <https://github.com/chintal/cryptolib>
 * 
 * Support for version 4
 * ---------------------
 * 
 * Random (v4) UUID generation in this library depends on the presense of a 
 * rand_byte() function at link time, which returns exactly one byte of 
 * entropy. Within the EBS ecosystem, this function would typically be 
 * available from sys/rand.h, and would depend on a PRNG seeded by a hardware
 * entropy source. In order to be fully compliant with the UUID spec, the 
 * preferable random generator would be the entropium CSPRNG. Any of the 
 * simpler PRBS generators should be fine in less demanding applications.
 * 
 * Note that this library does not attempt to initialize the random number
 * generator. Tthe application should ensure it is correctly configured and 
 * initialized before UUID4 is generated.
 *
 * @see <https://github.com/chintal/ebs-lib-prbs>
 * @see <https://github.com/chintal/cryptolib>
 * 
 * @see <https://en.wikipedia.org/wiki/Universally_unique_identifier>
 * @see <https://tools.ietf.org/html/rfc4122>
 */

#ifndef UUID_H
#define UUID_H

#include<stdint.h>


typedef union UUID_t{
    struct {
        uint32_t time_low;
        uint16_t time_mid;
        uint16_t time_hi_and_version;
        uint8_t clock_seq_hi_and_reserved;
        uint8_t clock_seq_low;
        uint8_t node[6];
    } s;
    uint8_t b[16];
}uuid_t;


#define UUID_SUPPORT_v1     1
#define UUID_SUPPORT_v2     0
#define UUID_SUPPORT_v3     1
#define UUID_SUPPORT_v4     1
#define UUID_SUPPORT_v5     1
#define UUID_SUPPORT_v6     1

void uuid_init(void);

void uuid_clear(uuid_t * out);

void uuid_sprintf(char * bufp, uuid_t * uuid);

#if UUID_SUPPORT_v1
    void uuid1(uuid_t * out);
#endif

#if UUID_SUPPORT_v3
    void uuid3(uuid_t * out, uuid_t * ns, uint8_t * name_p, uint8_t len);
#endif

#if UUID_SUPPORT_v4    
    void uuid4(uuid_t * out);
#endif

#if UUID_SUPPORT_v5
    void uuid5(uuid_t * out, uuid_t * ns, uint8_t * name_p, uint8_t len);
#endif
    
#if UUID_SUPPORT_v6
    void uuid6(uuid_t * out);
#endif

#if UUID_SUPPORT_v1 + UUID_SUPPORT_v6 > 0
    #define UUID_EPOCH_OFFSET (uint64_t)(0x01B21DD213814000)
    extern uint8_t id_read(uint8_t maxlen, void * buffer);    
#endif
    
#if UUID_SUPPORT_v4 + UUID_SUPPORT_v1 > 0
    extern uint8_t rand_byte(void);
#endif
    
#endif
