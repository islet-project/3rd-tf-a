/*
 * Copyright (c) 2024, Samsung. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* This file describes the VHUK API */

#ifndef VHUK_H
#define VHUK_H

#include <stddef.h>
#include <stdint.h>

#include "psa/error.h"

/* RSE VHUK message types that distinguish its services. */
#define RSE_VHUK_GET_KEY		1006U

psa_status_t
rse_get_vhuk(uint8_t  *key_buf,
			 size_t    key_buf_size,
			 uint8_t   key_id);

#endif /* VHUK_H */
