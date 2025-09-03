/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 * Copyright (c) 2022, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <common/desc_image_load.h>

#include <drivers/measured_boot/rse/rse_measured_boot.h>

#if PLAT_RSE_COMMS_USE_SERIAL != 0
/*
 * Platform specific table with image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rse_mboot_metadata qemu_rse_mboot_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.slot = U(8),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_FW_CONFIG_STRING,
		.lock_measurement = false
	},
	{
		.id = TB_FW_CONFIG_ID,
		.slot = U(9),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_TB_FW_CONFIG_STRING,
		.lock_measurement = false
	},
	{
		.id = BL2_IMAGE_ID,
		.slot = U(10),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL2_IMAGE_STRING,
		.lock_measurement = false
	},
	{
		.id = RSE_MBOOT_INVALID_ID
	}
};
#endif

/*
 * Add dummy functions for measured boot for BL1.
 * In most of the SoC's, ROM/BL1 code is pre-built. So we are assumimg that
 * it doesn't have the capability to do measurements and extend eventlog.
 * hence these are dummy functions.
 */
void bl1_plat_mboot_init(void)
{
#if PLAT_RSE_COMMS_USE_SERIAL != 0
	rse_measured_boot_init(qemu_rse_mboot_metadata);
#endif
}

void bl1_plat_mboot_finish(void)
{
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
#if PLAT_RSE_COMMS_USE_SERIAL != 0
	/* Calculate image hash and record data in RSE */
	int err = rse_mboot_measure_and_record(qemu_rse_mboot_metadata,
	                                       image_data->image_base,
	                                       image_data->image_size,
	                                       image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record in RSE", image_id, err);
		return err;
	}
#endif

	return 0;
}

int plat_mboot_measure_key(const void *pk_oid, const void *pk_ptr,
			   size_t pk_len)
{
	return 0;
}
