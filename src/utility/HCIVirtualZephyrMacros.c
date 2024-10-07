#if defined(__ZEPHYR__)

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/buf.h>
#include <zephyr/bluetooth/hci_vs.h>

static K_FIFO_DEFINE(rx_queue);
struct k_fifo* __rx_queue = &rx_queue;


// from https://github.com/thomasstenersen/sdk-zephyr/commit/df546a0eb9b96b453373f38483959d2363a83cae#diff-217778ef1d0638c89207786f06425396891f987713120e83db724d26b9813eebR331
// referenced by https://devzone.nordicsemi.com/f/nordic-q-a/73310/incompatibility-between-mcumgr-cli-and-hci_usb-when-running-with-bt_ll_softdevice-enabled

void set_public_address(struct k_fifo *rx_queue)
{
	const struct bt_hci_cp_vs_write_bd_addr cmd = {
		.bdaddr.val = {0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA}
	};
	const struct bt_hci_cmd_hdr cmd_header = {
		.opcode = BT_HCI_OP_VS_WRITE_BD_ADDR,
		.param_len = sizeof(cmd)
	};
	struct net_buf *buf;
	int err;
	buf = bt_buf_get_tx(BT_BUF_CMD, K_NO_WAIT,
			    &cmd_header, sizeof(cmd_header));
	net_buf_add_mem(buf, &cmd, sizeof(cmd));
	err = bt_send(buf);
	__ASSERT_NO_MSG(err == 0);
	/* Pull out the command complete. */
	buf = net_buf_get(rx_queue, K_SECONDS(10)); /* 10s == HCI_CMD_TIMEOUT */
	struct bt_hci_evt_hdr *hdr;
	__ASSERT_NO_MSG(buf != NULL);
	__ASSERT_NO_MSG(buf->len >= sizeof(*hdr));
	hdr = net_buf_pull_mem(buf, sizeof(*hdr));
	__ASSERT_NO_MSG(hdr->evt == BT_HCI_EVT_CMD_COMPLETE);
	struct bt_hci_evt_cmd_complete *evt;
	evt = net_buf_pull_mem(buf, sizeof(*evt));
	__ASSERT_NO_MSG(sys_le16_to_cpu(evt->opcode) == BT_HCI_OP_VS_WRITE_BD_ADDR);
	net_buf_unref(buf);
}

#endif