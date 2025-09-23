/*
 * Copyright (c) 2024, Samsung. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <vhuk.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>

psa_status_t
rse_get_vhuk(uint8_t  *key_buf,
			 size_t    key_buf_size,
			 uint8_t   key_id)
{
	psa_status_t status;
	psa_invec in_vec[] = {
		{&key_id, sizeof(key_id)}
	};
	psa_outvec out_vec[] = {
		{key_buf, key_buf_size}
	};

	status = psa_call(RSE_VHUK_SERVICE_HANDLE,
			  RSE_VHUK_GET_KEY,
			  in_vec,  IOVEC_LEN(in_vec),
			  out_vec, IOVEC_LEN(out_vec));

	return status;
}
