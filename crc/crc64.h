#ifndef CRC64_H
#define CRC64_H

#include <linux/types.h>

unsigned long long fio_crc64(const unsigned char *, unsigned long);

__u64 fio_crc64_rocksoft_generic(__u64 crc, const void *p, unsigned int len);

#endif
