/* crc32.h

   header file for crc32 checksum
*/

#define CRC32_XINIT 0xFFFFFFFFL		/* initial value */
#define CRC32_XOROT 0xFFFFFFFFL		/* final xor value */

#define MINIMUM_CHECKSUM_LEN	 8
#define MAXIMUM_CHECKSUM_LEN	99

/* NAACCR 6.0 Specifications */
#define NAACCR_60_CHECKSUM_POS	942
#define NAACCR_60_CHECKSUM_LEN	10

/* function prototypes */

#ifdef __cplusplus
extern "C"
{
#endif

unsigned long CalcCRC32(unsigned char *, unsigned long, unsigned long, unsigned long);
int AssignCRC32(unsigned char *, unsigned long, unsigned long, unsigned long);
int CompareCRC32(unsigned char *, unsigned long, unsigned long, unsigned long);

#ifdef __cplusplus
}
#endif
