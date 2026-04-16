#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/cassert.h>
#include <drivers/arm/mhu.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>

#include "rse_comms_protocol.h"

/*
 * The serialized_rse_comms_reply_t structure contains
 * the serialized_rse_comms_header_t and rse_embed_reply_t structures for the
 * embed protocol.
 * The rse_embed_reply_t contains a header and a trailer array of length
 * PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE that contains a message payload.
 *
 * NOTE: The current implementation assumes a specific layout of serialized_rse_comms_reply_t
 * and rse_embed_reply_t
 */
#define REPLY_COMMS_HEADER_SIZE (sizeof(struct serialized_rse_comms_header_t))

/* The size of the embed reply header (embed reply without the payload) */
#define REPLY_EMBED_HEADER_SIZE (sizeof(struct rse_embed_reply_t) - PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE)

/* The maximum size of the whole embed reply message containing the comms header */
#define REPLY_EMBED_MESSAGE_MAX_SIZE (sizeof(struct serialized_rse_comms_header_t) + sizeof(struct rse_embed_reply_t))

/*
 * Ensure the actual memory offset of the 'trailer' payload
 * within the overall message. If anyone adds a field before the trailer, this will fail.
 */
CASSERT(offsetof(struct serialized_rse_comms_reply_t, reply.embed.trailer) == REPLY_COMMS_HEADER_SIZE + REPLY_EMBED_HEADER_SIZE,
        assert_reply_header_size_matches_trailer_offset);

/*
 * This verifies that subtracting the max payload size from the total struct size strictly
 * leaves us at the start of the trailer.
 */
CASSERT((sizeof(struct rse_embed_reply_t) - PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE) == offsetof(struct rse_embed_reply_t, trailer),
        assert_embed_reply_trailer_size_logic);

/* Specifies the maximum number of trials while pulling one character from the UART RX FIFO */
#define MAX_RETRY_COUNT 100000000

static console_t data_channel;
static bool data_channel_initialized = false;

/* added in aarch64/pl011_console.S */
int console_pl011_rawputc(int c, console_t *console);

static void serial_lazy_initialize()
{
	if (data_channel_initialized)
		return;

	int rc = console_pl011_register(PLAT_RSE_SERIAL_UART_BASE,
	                                PLAT_RSE_SERIAL_UART_CLK_IN_HZ,
	                                PLAT_RSE_SERIAL_CONSOLE_BAUDRATE,
	                                &data_channel);
	if (rc != 1)
		panic();

	/* unregister the serial, we don't use it as console */
	if (console_is_registered(&data_channel)) {
		console_unregister(&data_channel);
	}
	data_channel.flags = 0;
	data_channel_initialized = true;
	NOTICE("[RSE_SERIAL] Serial initialized\n");
}

static int read_char_blocking()
{
	int c, retry = 0;
	do {
		c = data_channel.getc(&data_channel);
	} while (c < 0 && retry++ < MAX_RETRY_COUNT);
	return c;
}

size_t mhu_get_max_message_size(void)
{
	return MAX(sizeof(struct serialized_rse_comms_msg_t),
	           sizeof(struct serialized_rse_comms_reply_t));
}

enum mhu_error_t mhu_send_data(const uint8_t *send_buffer, size_t size)
{
	size_t i;
	int ret;

	serial_lazy_initialize();

	for (i = 0; i < size; ++i) {
		ret = console_pl011_rawputc(send_buffer[i], &data_channel);
		if (ret < 0) {
			NOTICE("[RSE_SERIAL] serial error: %d\n", ret);
			return MHU_ERR_GENERAL;
		}
	}

	NOTICE("[RSE_SERIAL] sent %lu bytes\n", size);

	return MHU_ERR_NONE;
}

enum mhu_error_t mhu_receive_data(uint8_t *receive_buffer, size_t *size)
{
	size_t i, read = 0;
	int c;

	serial_lazy_initialize();

	/*
	 * We know that the upper layer passess the receive_buffer pointer to
	 * the serialized_rse_comms_reply_t structure and its size.
	 *
	 * In case of embed protocol the serialized_rse_comms_reply_t structure contains:
	 * - fixed size struct serialized_rse_comms_header_t
	 * - the rse_embed_reply_t struct which contains a payload, where the length of the payload
	 *   depends on the out_size field of rse_embed_reply_t
	 *
	 * We need to make sure that the size of receive_buffer is enough to fit the embed protocol
	 * message of maximum possible size.
	 */
	if (*size < REPLY_EMBED_MESSAGE_MAX_SIZE) {
		ERROR("[RSE_SERIAL] The size of receive buffer is too small\n");
		return MHU_ERR_INVALID_ARG;
	}

	/* Lock waiting for the serialized_rse_comms_header_t */
	do {
		c = read_char_blocking();
		if (c < 0) {
			ERROR("[RSE_SERIAL] Timeout while reading from UART (reading the serialized_rse_comms_reply_t)\n");
			return MHU_ERR_GENERAL;
		}
		receive_buffer[read++] = c;
	} while (read < REPLY_COMMS_HEADER_SIZE);

	/* Check whether we got a proper header for embed protocol */
	const struct serialized_rse_comms_reply_t *reply = (const struct serialized_rse_comms_reply_t *)receive_buffer;
	if (reply->header.protocol_ver != RSE_COMMS_PROTOCOL_EMBED) {
		ERROR("[RSE_SERIAL] The protocol version is incompatible\n");
		return MHU_ERR_GENERAL;
	}

	/* Read the header of the rse_embed_reply_t structure */
	do {
		c = read_char_blocking();
		if (c < 0) {
			ERROR("[RSE_SERIAL] Timeout while reading from UART (reading the rse_embed_reply_t header)\n");
			return MHU_ERR_GENERAL;
		}
		receive_buffer[read++] = c;
	} while (read < REPLY_COMMS_HEADER_SIZE + REPLY_EMBED_HEADER_SIZE);

	/* Calculate the rse_embed_reply_t payload size */
	const struct rse_embed_reply_t *embed = &(reply->reply.embed);
	size_t payload_size = 0;
	for (i = 0; i < PSA_MAX_IOVEC; i++) {
		payload_size += embed->out_size[i];
	}

	/* Calculate the expected total message size */
	size_t total_reply_size = REPLY_COMMS_HEADER_SIZE + REPLY_EMBED_HEADER_SIZE + payload_size;
	if (total_reply_size > REPLY_EMBED_MESSAGE_MAX_SIZE) {
		ERROR("[RSE_SERIAL] Receive buffer is too small, expected receiver buffer size: %zu\n", total_reply_size);
		return MHU_ERR_BUFFER_TOO_SMALL;
	}

	/* Read the payload data if there is any */
	while (read < total_reply_size) {
		c = read_char_blocking();
		if (c < 0) {
			ERROR("[RSE_SERIAL] Timeout while reading from UART (reading the payload)\n");
			return MHU_ERR_GENERAL;
		}
		receive_buffer[read++] = c;
	}

	*size = read;
	NOTICE("[RSE_SERIAL] read %lu bytes\n", read);

	return MHU_ERR_NONE;
}
