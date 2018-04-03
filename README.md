This library provides UUID generation functions, to generate UUIDs 
compliant with RFC4122.
 
  - Version 1 : Date-time and MAC address
  - Version 2 : DCE security                 (no support planned)
  - Version 3 : MD5 Hash and Namespace
  - Version 4 : Random           
  - Version 5 : SHA1 Hash and Namespace
  - Version 6 : Non-standard variant
 
Note that the bulk of this library is NOT re-entrant. It is the 
application's responsibility to ensure that a uuid generation function 
is not called while a previous one is still in process.

@see <https://en.wikipedia.org/wiki/Universally_unique_identifier>
@see <https://tools.ietf.org/html/rfc4122>

Support for versions 1 and 6
----------------------------

Date-time is utilized from the EBS time library, which must be included in 
the application. 

@warning If you intend to use version 1 or 6 UUIDs, you should ensure that 
the time library is initialized, the systick timer is installed and started, 
and time is synchronized to real time - either from an RTC or by the Host 
before the v1 UUID is generated. This library does nothing to make sure 
that the used system time makes any kind of sense. 

@warning While the EBS time library allows for some flexibility of the 
epoch, this library assumes system time is provided against the UNIX epoch, 
and offsets are hard coded accordingly. Similarly, the TIME_TICKS_PER_SECOND 
preprocessor definiton of the time library provides the applicable 
conversion factor which cannot be changed at link time or run time.

@see <https://github.com/chintal/ebs-lib-time>

Clock sequence is simply generated randomly using the rand_byte() function,
which should be available at link time. For better compliance with RFC4122, 
the clock sequence should be written to some form of non-volatile storage. 
Such an implementation is left to the application.

The 'node' or 'MAC address' used is platfom (and application) dependent. 
This implementation will simply use the first 6 bytes of whatever unique 
identifier is available on the platform via the uC HAL's id_read().
Ideally, the node should be constructed using a hash of the entire 
identifier, but that is not done here to keep code relatively small. This
may be changed in the future if v1 or v6 UUIDs are used often. 

UUID version 6 is a non-standard variant of the version 1 UUID which allows for 
chronological sorting when treated as an arbitrary string of bytes.

@see <https://bradleypeabody.github.io/uuidv6/>

Support for versions 3 and 5
----------------------------

For version 3 and 5 UUID generation, the namespace should be a UUID in of 
itself, and can either be generated using one of the functions here or 
pre-installed into a UUID type variable. MD5 and SHA1 hashes are generated 
using the EBS repackaged version of AVR Crypto Lib. Note that due to the 
dependency on cryptolib, which is in turn a derivative AVR Crypto Lib, this 
library is necessarily licensed under the GPLv3 without any linking 
exceptions. Applications using this library must also, consequently, be 
distributed under the terms of the GPLv3 license.

@note The v5 implementation may be switched to SHA256 instead of SHA1 in 
the future. since SHA256 is more likely to be used in the application 
otherwise, including as part of the PRNG (entropium). This will make the
implementation less compliant with the letter of the RFC4122.

@see <https://github.com/chintal/cryptolib>

Support for version 4
---------------------

Random (v4) UUID generation in this library depends on the presense of a 
rand_byte() function at link time. In order to be fully compliant with 
the UUID spec, the preferable random generator would be the entropium 
CSPRNG. Some of the simpler PRBS generators may be fine for less demanding 
applications.

Note that this library does not attempt to initialize the random number
generator. The application should ensure it is correctly configured and 
initialized before UUID v4 is generated.

@see <https://github.com/chintal/ebs-lib-prbs>
@see <https://github.com/chintal/cryptolib>

Note on version 2
-----------------

DCE Security (v2) UUID does not seem to have immediate applications in 
constrained embedded systems. If there is an application that calls for it, 
support may be added to this library based on the context of the example 
application. The DCE/CDE spec for v2 UUIDs by itself seems exceptionally 
dense.

@see <http://pubs.opengroup.org/onlinepubs/9629399/apdxa.htm>
@see <http://pubs.opengroup.org/onlinepubs/9696989899/chap5.htm#tagcjh_08_02_01_01>
