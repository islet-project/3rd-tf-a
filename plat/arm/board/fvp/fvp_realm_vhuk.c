
/*
 * Copyright (c) 2024, Samsung Electronics Co., Ltd All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <vhuk.h>
#include <plat/common/platform.h>
#include <psa/error.h>
#include <services/islet_svc.h>

#if PLAT_RSE_COMMS_USE_SERIAL == 0
static uint8_t sample_vhuk_a_key[RMM_ISLET_VHUK_SIZE] = {
	0x66, 0xae, 0x72, 0xda, 0x49, 0x6f, 0x8b, 0x06, 0xcb, 0x6a, 0x98, 0xc3,
	0xf9, 0xc6, 0x8c, 0x22, 0x13, 0x42, 0xbc, 0xdf, 0x70, 0x0f, 0x22, 0x42,
	0x0f, 0x1d, 0xa2, 0xe7, 0x5e, 0x4c, 0x01, 0xc2
};

static uint8_t sample_vhuk_m_key[RMM_ISLET_VHUK_SIZE] = {
	0x2d, 0x0f, 0xe5, 0xa0, 0x06, 0x4f, 0x42, 0xc3, 0xdf, 0xf2, 0xc9, 0xc1,
	0x7c, 0xf7, 0xc1, 0xc3, 0xbe, 0xab, 0xbd, 0x45, 0xd4, 0x54, 0xa7, 0x65,
	0xdd, 0xd0, 0xb5, 0xa5, 0x22, 0x45, 0x0f, 0x96
};
#endif

int plat_rmmd_get_vhuk(uintptr_t buf, size_t len, unsigned int key_id)
{
#if PLAT_RSE_COMMS_USE_SERIAL != 0
	return rse_get_vhuk((uint8_t *)buf, len, key_id);
#else
	if (len != RMM_ISLET_VHUK_SIZE) {
		return -EINVAL;
	}
	switch (key_id) {
		case 0x1:
			(void)memcpy((void *)buf, (const void *)sample_vhuk_a_key,
						 sizeof(sample_vhuk_a_key));
			break;
		case 0x2:
			(void)memcpy((void *)buf, (const void *)sample_vhuk_m_key,
						 sizeof(sample_vhuk_m_key));
			break;
		default:
			return -EINVAL;
	}
	return 0;
#endif
}
