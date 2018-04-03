/* 
 *   Copyright (c)
 *     (c) 2016-2018 Chintalagiri Shashank, Quazar Technologies Pvt. Ltd.
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
 *  - Version 1 : Date-time and MAC address
 *  - Version 2 : DCE security                 (no support planned)
 *  - Version 3 : MD5 Hash and Namespace
 *  - Version 4 : Random           
 *  - Version 5 : SHA1 Hash and Namespace
 *  - Version 6 : Non-standard variant
 * 
 * Note that the bulk of this library is NOT re-entrant. It is the 
 * application's responsibility to ensure that a uuid generation function 
 * is not called while a previous one is still in process.
 * 
 * @see <https://en.wikipedia.org/wiki/Universally_unique_identifier>
 * @see <https://tools.ietf.org/html/rfc4122>
 * 
 * Support for versions 1 and 6
 * ----------------------------
 * 
 * Date-time is utilized from the EBS time library, which must be included in 
 * the application. 
 * 
 * @warning If you intend to use version 1 or 6 UUIDs, you should ensure that 
 * the time library is initialized, the systick timer is installed and started, 
 * and time is synchronized to real time - either from an RTC or by the Host 
 * before the v1 UUID is generated. This library does nothing to make sure 
 * that the used system time makes any kind of sense. 
 * 
 * @warning While the EBS time library allows for some flexibility of the 
 * epoch, this library assumes system time is provided against the UNIX epoch, 
 * and offsets are hard coded accordingly. Similarly, the TIME_TICKS_PER_SECOND 
 * preprocessor definiton of the time library provides the applicable 
 * conversion factor which cannot be changed at link time or run time.
 * 
 * @see <https://github.com/chintal/ebs-lib-time>
 * 
 * Clock sequence is simply generated randomly using the rand_byte() function,
 * which should be available at link time. For better compliance with RFC4122, 
 * the clock sequence should be written to some form of non-volatile storage. 
 * Such an implementation is left to the application.
 * 
 * The 'node' or 'MAC address' used is platfom (and application) dependent. 
 * This implementation will simply use the first 6 bytes of whatever unique 
 * identifier is available on the platform via the uC HAL's id interface.
 * Ideally, the node should be constructed using a hash of the entire 
 * identifier, but that is not done here to keep code relatively small. This
 * may be changed in the future if v1 or v6 UUIDs are used often. 
 * 
 * UUID version 6 is a non-standard variant of the version 1 UUID which allows for 
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
 * dependency on cryptolib, which is in turn a derivative AVR Crypto Lib, this 
 * library is necessarily licensed under the GPLv3 without any linking 
 * exceptions. Applications using this library must also, consequently, be 
 * distributed under the terms of the GPLv3 license.
 * 
 * @note The v5 implementation may be switched to SHA256 instead of SHA1 in 
 * the future. since SHA256 is more likely to be used in the application 
 * otherwise, including as part of the PRNG (entropium). This will make the
 * implementation less compliant with the letter of the RFC4122.
 * 
 * @see <https://github.com/chintal/cryptolib>
 * 
 * Support for version 4
 * ---------------------
 * 
 * Random (v4) UUID generation in this library depends on the presense of a 
 * rand_byte() function at link time. In order to be fully compliant with 
 * the UUID spec, the preferable random generator would be the entropium 
 * CSPRNG. Some of the simpler PRBS generators may be fine for less demanding 
 * applications.
 * 
 * Note that this library does not attempt to initialize the random number
 * generator. The application should ensure it is correctly configured and 
 * initialized before UUID v4 is generated.
 *
 * @see <https://github.com/chintal/ebs-lib-prbs>
 * @see <https://github.com/chintal/cryptolib>
 * 
 * Note on version 2
 * -----------------
 * 
 * DCE Security (v2) UUID does not seem to have immediate applications in 
 * constrained embedded systems. If there is an application that calls for it, 
 * support may be added to this library based on the context of the example 
 * application. The DCE/CDE spec for v2 UUIDs by itself seems exceptionally 
 * dense.
 * 
 * @see <http://pubs.opengroup.org/onlinepubs/9629399/apdxa.htm>
 * @see <http://pubs.opengroup.org/onlinepubs/9696989899/chap5.htm#tagcjh_08_02_01_01>
 * 
 */

#ifndef UUID_H
#define UUID_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdint.h>
#include "config.h"

/**
 * @name UUID Version support status
 */
/**@{*/ 
#define UUID_SUPPORT_v1     1
#define UUID_SUPPORT_v2     0
#define UUID_SUPPORT_v3     1
#define UUID_SUPPORT_v4     1
#define UUID_SUPPORT_v5     1
#define UUID_SUPPORT_v6     1
/**@}*/ 

/** @brief Epoch offset between the UNIX epoch (00:00:00 UTC, 1 January 1970) 
  * and UUID epoch (00:00:00.00 UTC, 15 October 1582) in units of the UUID 
  * timestamp (100ns). This number is taken from page 28 of RFC4122.
  */
#define UUID_EPOCH_OFFSET (uint64_t)(0x01B21DD213814000)

/** @brief Length of the binary representation of a UUID in bytes */
#define UUID_LENGTH_BINARY  16

/** @brief Length of the string representation of a UUID in bytes */
#define UUID_LENGTH_STRING  37

/**
 * @brief UUID Container Type
 */
typedef union UUID_t{
    struct {
        uint32_t time_low;                  /**< @brief bytes 0-3 */
        uint16_t time_mid;                  /**< @brief bytes 4-5 */
        uint16_t time_hi_and_version;       /**< @brief bytes 6-7 */
        uint8_t clock_seq_hi_and_reserved;  /**< @brief byte 8 */
        uint8_t clock_seq_low;              /**< @brief byte 9 */
        uint8_t node[6];                    /**< @brief bytes 10-15 */
    } s;            /**< @brief UUID type as per the structure defined in RFC4122. */
    uint8_t b[16];  /**< @brief UUID type as an array of bytes. */
}uuid_t;

/**
 * @name UUID Application Dependent External API Functions
 * 
 * These functions related to hardware or application-specific interfaces and 
 * must be provided by the application. They should be available at link time. 
 */
/**@{*/ 

/**
 * @brief Read the uC's unique identification.
 * @param maxlen Maximum length of the identifier to write to the buffer. 
 * @param buffer Pointer to a buffer into which the ID is to be written.
 * @return Number of bytes of the ID which were written to the buffer.
 * 
 * Obtain the best available identification information for the uC and write 
 * it into the provided buffer. This library will ask for 6 bytes (48 bits) of
 * identification. The platform should be able to provide a 48-bit ID of 
 * sufficient quality. Within the EBS ecosystem, this function would typically 
 * be available from bsp/hal/uc/id.h
 */
extern uint8_t id_read(uint8_t maxlen, void * buffer);    

/**
 * @brief Get one byte's worth of entropy.
 * @return One byte of entropy.
 * 
 * Return exactly one byte of entropy. Within the EBS ecosystem, this function 
 * would typically be available from sys/rand.h, and would depend on a PRNG 
 * seeded by a hardware entropy source.
 */
extern uint8_t rand_byte(void);
/**@}*/ 

/**
 * @name UUID Init and Maintenance Functions
 * 
 */
/**@{*/ 

/**
 * @brief Intialize the UUID library and elements needed for UUID generation.
 * 
 * This function currently initializes a template for UUID v1 and v6 with the 
 * node ID and clockseq. This function should be called before any attempt to 
 * generate v1 or v6 UUIDs. If v1 or v6 UUIDs are not required by the 
 * application, this function need not and should not be called. 
 */
void uuid_init(void);
/**@}*/ 

/**
 * @name UUID Primary API Functions
 * 
 * See the library level documentation and RFC4122 for details of each UUID 
 * version, its dependencies and its construction.
 * 
 */
/**@{*/ 

/**
 * @brief Generate a Version 1 UUID (Date-Time and MAC Address)
 * @param out An existing UUID container to write the generated UUID into.
 */
void uuid1(uuid_t * out);

/**
 * @brief Generate a Version 3 UUID (MD5 Hash and Namespace)
 * @param out An existing UUID container to write the generated UUID into.
 * @param ns Pointer to the UUID of the namespace.
 * @param name_p A pointer to the name for which the UUID is to be generated.
 * @param len Length of the name for which the UUID is to be generated.
 */
void uuid3(uuid_t * out, uuid_t * ns, uint8_t * name_p, uint8_t len);

/**
 * @brief Generate a Version 4 UUID (Random)
 * @param out An existing UUID container to write the generated UUID into.
 */
void uuid4(uuid_t * out);

/**
 * @brief Generate a Version 5 UUID (SHA1 Hash and Namespace)
 * @param out An existing UUID container to write the generated UUID into.
 * @param ns Pointer to the UUID of the namespace.
 * @param name_p A pointer to the name for which the UUID is to be generated.
 * @param len Length of the name for which the UUID is to be generated.
 */
void uuid5(uuid_t * out, uuid_t * ns, uint8_t * name_p, uint8_t len);

/**
 * @brief Generate a Version 6 UUID (Version 1 based non-standard variant)
 * @param out An existing UUID container to write the generated UUID into.
 */
void uuid6(uuid_t * out);

/**
 * @brief Print a formatted UUID to a character array.
 * @param bufp Pointer to the buffer to write the formatted UUID into.
 * @param uuid Pointer to the UUID container to print.  
 * 
 * Print a UUID into a string, formatted as per the standard format for UUIDs.
 * This function should be provided a buffer that is long enough to contain a
 * UUID string, i.e., 37 characters. The generated string will be null 
 * terminated. 
 * 
 * The format of the output is as per RFC4122 : 
 * 
 *                   +---------------------- Version
 *                   | 
 *     xxxxxxxx-xxxx-Vxxx-xxxx-xxxxxxxxxxxx
 *     |        |    |    | |  |
 *     |        |    |    | |  +------------ [node]  
 *     |        |    |    | +--------------- [clock-seq-low]
 *     |        |    |    +----------------- [clock-seq-and-reserved]
 *     |        |    +---------------------- [time-high-and-version]
 *     |        +--------------------------- [time-mid]
 *     +------------------------------------ [time-low]
 * 
 * This function pulls in the printf library. In most instances, it is 
 * preferable to avoid using this. Transact with the host using the binary 
 * format of the UUID instead. This will avoid the significant overhead of 
 * the underlying sprintf call.
 */
void uuid_sprintf(char * bufp, uuid_t * uuid);

/**
 * @brief Clear a UUID container.
 * @param uuid Pointer to the UUID container to clear. 
 * 
 * Clear a UUID container by writing 0 to all fields. This function is usually 
 * not needed. Most UUID generation functions will rewrite the entire container
 * anyway.
 */
void uuid_clear(uuid_t * uuid);
/**@}*/ 

#ifdef __cplusplus
}
#endif

#endif
