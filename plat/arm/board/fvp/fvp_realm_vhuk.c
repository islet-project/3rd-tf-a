
/*
 * Copyright (c) 2024, Samsung Electronics Co., Ltd All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <vhuk.h>
#include <plat/common/platform.h>
#include <psa/error.h>

int plat_rmmd_get_vhuk(uintptr_t buf, size_t len, unsigned int key_id)
{
#if PLAT_RSE_COMMS_USE_SERIAL != 0
	return rse_get_vhuk((uint8_t *)buf, len, key_id);
#else
	return PSA_ERROR_NOT_SUPPORTED;
#endif
}
