/*
 * Copyright (c) 2024, Samsung Electronics Co., Ltd All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <services/rmmd_svc.h>
#include <services/islet_svc.h>

/*
 * This file contains an implementation of a dedicated service for Islet.
 * Curently, the only purpose of this service is to route a vendor specific
 * RMI/EL3 SMCs to proper handlers implemented by the RMMD service.
 */

static int32_t islet_svc_setup(void)
{
	return 0;
}

static uintptr_t islet_svc_smc_handler(uint32_t smc_fid,
			     u_register_t x1,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	switch (smc_fid) {
		case RMI_ISLET_REALM_SET_METADATA:
			return rmmd_rmi_handler(smc_fid, x1, x2, x3, x4, cookie,
		 						    handle, flags);
		case RMM_ISLET_GET_VHUK:
			return rmmd_rmm_el3_handler(smc_fid, x1, x2, x3, x4, cookie,
						    handle, flags);
		default:
			VERBOSE("Unimplemented Vendor Service Call for Islet: 0x%x \n", smc_fid);
			SMC_RET1(handle, SMC_UNK);
	}
}

DECLARE_RT_SVC(
		islet_svc,

		OEN_VEN_EL3_START,
		OEN_VEN_EL3_END,
		SMC_TYPE_FAST,

		islet_svc_setup,
		islet_svc_smc_handler
);
