/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __CRC_T10DIF_H
#define __CRC_T10DIF_H

#include <linux/types.h>

extern __u16 fio_crc_t10dif_generic(__u16 crc, const unsigned char *buffer,
				    unsigned int len);

#endif
