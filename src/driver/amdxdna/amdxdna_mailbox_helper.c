// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024, Advanced Micro Devices, Inc.
 */

#include "amdxdna_mailbox_helper.h"

int xdna_msg_noresp_cb(void *handle, void __iomem *data, size_t size)
{
	struct amdxdna_dev *xdna = handle;
	u32 status;

	if (data)
		memcpy_fromio(&status, data, sizeof(status));

	XDNA_DBG(xdna, "Got dummy resp, status 0x%x", status);
	return 0;
}

int xdna_msg_cb(void *handle, void __iomem *data, size_t size)
{
	struct xdna_notify *cb_arg = handle;
	int ret;

	if (unlikely(!data))
		goto out;

	/*
	 * Firmware error replies are sometimes shorter than the request
	 * struct's response shape implies. For example, an unrecognized
	 * opcode is rejected with just a 4-byte AIE2_STATUS_INVALID_COMMAND
	 * status word, even though the matching get_*_resp struct includes
	 * additional fields meaningful only on the success path.
	 *
	 * Accept any response that fits in the caller's buffer; copy what
	 * was sent and leave the remaining bytes at the values set by
	 * DECLARE_XDNA_MSG_COMMON (status = sentinel, others = 0). Callers
	 * read the status field and act on it. Oversized responses remain
	 * an error -- they would overrun the caller's buffer.
	 */
	if (unlikely(size > cb_arg->size)) {
		cb_arg->error = -EMSGSIZE;
		goto out;
	}

	memcpy_fromio(cb_arg->data, data, size);
	print_hex_dump_debug("resp data: ", DUMP_PREFIX_OFFSET,
			     16, 4, cb_arg->data, size, false);
out:
	ret = cb_arg->error;
	complete(&cb_arg->comp);
	return ret;
}

int xdna_send_msg_wait(struct amdxdna_dev *xdna, struct mailbox_channel *chann,
		       struct xdna_mailbox_msg *msg)
{
	struct xdna_notify *hdl = msg->handle;
	int ret;

	ret = xdna_mailbox_send_msg(chann, msg, TX_TIMEOUT);
	if (ret) {
		XDNA_ERR(xdna, "Send message failed, ret %d", ret);
		return ret;
	}

	ret = wait_for_completion_timeout(&hdl->comp,
					  msecs_to_jiffies(RX_TIMEOUT));
	if (!ret) {
		XDNA_ERR(xdna, "Wait for completion timeout");
		return -ETIME;
	}

	return hdl->error;
}
