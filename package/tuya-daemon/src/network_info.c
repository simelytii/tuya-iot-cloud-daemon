#include <string.h>
#include <stdio.h>
#include <syslog.h>

#include <libubus.h>
#include <libubox/blobmsg.h>

#include "network_info.h"
#include "ubus_utils.h"

enum {
	IF_INTERFACE,
	__IF_MAX,
};

static const struct blobmsg_policy dump_policy[__IF_MAX] = {
	[IF_INTERFACE] = {
		.name = "interface",
		.type = BLOBMSG_TYPE_ARRAY
	},
};

enum {
	ENTRY_INTERFACE,
	ENTRY_DEVICE,
	ENTRY_IPV4,
	__ENTRY_MAX,
};

static const struct blobmsg_policy entry_policy[__ENTRY_MAX] = {
	[ENTRY_INTERFACE] = {
		.name = "interface",
		.type = BLOBMSG_TYPE_STRING
	},
	[ENTRY_DEVICE] = {
		.name = "l3_device",
		.type = BLOBMSG_TYPE_STRING
	},
	[ENTRY_IPV4] = {
		.name = "ipv4-address",
		.type = BLOBMSG_TYPE_ARRAY
	},
};

enum {
	ADDR_ADDRESS,
	ADDR_MASK,
	__ADDR_MAX,
};

static const struct blobmsg_policy addr_policy[__ADDR_MAX] = {
	[ADDR_ADDRESS] = {
		.name = "address",
		.type = BLOBMSG_TYPE_STRING
	},
	[ADDR_MASK] = {
		.name = "mask",
		.type = BLOBMSG_TYPE_INT32
	},
};

enum {
	DEV_STATS,
	__DEV_MAX,
};

static const struct blobmsg_policy dev_policy[__DEV_MAX] = {
	[DEV_STATS] = {
		.name = "statistics",
		.type = BLOBMSG_TYPE_TABLE
	},
};

enum {
	STAT_RX,
	STAT_TX,
	__STAT_MAX,
};

static const struct blobmsg_policy stat_policy[__STAT_MAX] = {
	[STAT_RX] = {
		.name = "rx_bytes",
		.type = BLOBMSG_TYPE_INT64
	},
	[STAT_TX] = {
		.name = "tx_bytes",
		.type = BLOBMSG_TYPE_INT64
	},
};

static void prefix_to_netmask(int prefix,
			      char *out,
			      size_t out_len)
{
	unsigned int mask;

	if (prefix <= 0)
		mask = 0;
	else if (prefix >= 32)
		mask = 0xFFFFFFFFu;
	else
		mask = 0xFFFFFFFFu << (32 - prefix);

	snprintf(out,
		 out_len,
		 "%u.%u.%u.%u",
		 (mask >> 24) & 0xFF,
		 (mask >> 16) & 0xFF,
		 (mask >> 8) & 0xFF,
		 mask & 0xFF);
}

static void fill_device_stats(struct ubus_context *ctx,
			      const char *device_name,
			      struct network_info *entry)
{
	struct ubus_call_result result;
	struct blob_attr *tb[__DEV_MAX];
	struct blob_attr *stb[__STAT_MAX];
	struct blob_buf args = {0};

	blob_buf_init(&args, 0);
	blobmsg_add_string(&args, "name", device_name);

	if (ubus_call_sync(ctx,
			   "network.device",
			   "status",
			   args.head,
			   &result) != 0) {
		syslog(LOG_WARNING,
		       "network_info: could not read device stats for %s",
		       device_name);

		blob_buf_free(&args);
		return;
	}

	blobmsg_parse(dev_policy,
		      __DEV_MAX,
		      tb,
		      blob_data(result.data),
		      blob_len(result.data));

	if (tb[DEV_STATS] != NULL) {
		blobmsg_parse(stat_policy,
			      __STAT_MAX,
			      stb,
			      blobmsg_data(tb[DEV_STATS]),
			      blobmsg_data_len(tb[DEV_STATS]));

		if (stb[STAT_RX] != NULL)
			entry->rx_bytes =
				blobmsg_get_u64(stb[STAT_RX]);

		if (stb[STAT_TX] != NULL)
			entry->tx_bytes =
				blobmsg_get_u64(stb[STAT_TX]);
	}

	ubus_call_result_free(&result);
	blob_buf_free(&args);
}

int collect_network_info(struct ubus_context *ctx,
			 struct network_info *interfaces,
			 int max_interfaces)
{
	struct ubus_call_result result;
	struct blob_attr *tb[__IF_MAX];
	struct blob_attr *cur;
	int rem;
	int count = 0;

	if (interfaces == NULL || max_interfaces <= 0) {
		syslog(LOG_ERR,
		       "network_info: invalid network info arguments");
		return -1;
	}

	memset(interfaces,
	       0,
	       sizeof(*interfaces) * max_interfaces);

	if (ubus_call_sync(ctx,
			   "network.interface",
			   "dump",
			   NULL,
			   &result) != 0) {
		syslog(LOG_WARNING,
		       "network_info: could not dump network interfaces via ubus");
		return -1;
	}

	blobmsg_parse(dump_policy,
		      __IF_MAX,
		      tb,
		      blob_data(result.data),
		      blob_len(result.data));

	if (tb[IF_INTERFACE] == NULL) {
		ubus_call_result_free(&result);
		return 0;
	}

	blobmsg_for_each_attr(cur,
			      tb[IF_INTERFACE],
			      rem) {
		struct blob_attr *etb[__ENTRY_MAX];
		struct network_info *entry;
		const char *iface_name;
		const char *device_name;

		if (count >= max_interfaces)
			break;

		blobmsg_parse(entry_policy,
			      __ENTRY_MAX,
			      etb,
			      blobmsg_data(cur),
			      blobmsg_data_len(cur));

		if (etb[ENTRY_INTERFACE] == NULL ||
		    etb[ENTRY_DEVICE] == NULL)
			continue;

		iface_name = blobmsg_get_string(
			etb[ENTRY_INTERFACE]);

		device_name = blobmsg_get_string(
			etb[ENTRY_DEVICE]);

		/* Skip loopback. */
		if (strcmp(device_name, "lo") == 0)
			continue;

		entry = &interfaces[count];

		strncpy(entry->name,
			iface_name,
			sizeof(entry->name) - 1);

		if (etb[ENTRY_IPV4] != NULL) {
			struct blob_attr *addr_cur;
			int addr_rem;

			blobmsg_for_each_attr(addr_cur,
					      etb[ENTRY_IPV4],
					      addr_rem) {
				struct blob_attr *atb[__ADDR_MAX];

				blobmsg_parse(addr_policy,
					      __ADDR_MAX,
					      atb,
					      blobmsg_data(addr_cur),
					      blobmsg_data_len(addr_cur));

				if (atb[ADDR_ADDRESS] != NULL) {
					strncpy(
						entry->ip_address,
						blobmsg_get_string(
							atb[ADDR_ADDRESS]),
						sizeof(entry->ip_address) - 1);
				}

				if (atb[ADDR_MASK] != NULL) {
					prefix_to_netmask(
						blobmsg_get_u32(
							atb[ADDR_MASK]),
						entry->netmask,
						sizeof(entry->netmask));
				}

				break;
			}
		}

		fill_device_stats(ctx, device_name, entry);

		count++;
	}

	ubus_call_result_free(&result);

	return count;
}