/*
 * Copyright (c) 2024, Samsung Electronics Co., Ltd All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ISLET_SVC_H
#define ISLET_SVC_H

#define RMI_ISLET_REALM_SET_METADATA	0xC7000150
#define RMM_ISLET_GET_VHUK				0xC70001B0

/* Virtual Hardware Unique Key identifiers */
#define RMM_ISLET_VHUK_A				0x1
#define RMM_ISLET_VHUK_M				0x2

#define RMM_ISLET_VHUK_SIZE				32 /* size in bytes (256 bits) */

#endif
