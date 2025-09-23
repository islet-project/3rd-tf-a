/*
 * Copyright (c) 2024, Samsung Electronics Co., Ltd All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include "rmmd_private.h"
#include <services/rmmd_svc.h>
#include <services/islet_svc.h>

extern spinlock_t rmmd_lock;

int rmmd_islet_get_vhuk(uint64_t buf_pa, uint64_t buf_size, uint64_t key_id)
{
	int err;

	if (key_id != RMM_ISLET_VHUK_A && key_id != RMM_ISLET_VHUK_M) {
		ERROR("Invalid VHUK key id\n");
		return E_RMM_INVAL;
	}

	if (buf_size != RMM_ISLET_VHUK_SIZE) {
		ERROR("Invalid VHUK key size\n");
		return E_RMM_INVAL;
	}

	spin_lock(&rmmd_lock);

	/* Get the Realm VHUK key. */
	err = plat_rmmd_get_vhuk((uintptr_t)buf_pa, buf_size,
						 (unsigned int)key_id);
	if (err != 0) {
		ERROR("Failed to get attestation key: %d.\n", err);
		err =  E_RMM_UNK;
	}

	spin_unlock(&rmmd_lock);

	return err;
}
