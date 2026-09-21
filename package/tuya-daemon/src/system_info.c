#include <string.h>
#include <syslog.h>

#include <libubus.h>
#include <libubox/blobmsg.h>

#include "system_info.h"
#include "ubus_utils.h"

enum {
	SYS_INFO_UPTIME,
	SYS_INFO_MEMORY,
	__SYS_INFO_MAX,
};

static const struct blobmsg_policy sys_info_policy[__SYS_INFO_MAX] = {
	[SYS_INFO_UPTIME] = {
		.name = "uptime",
		.type = BLOBMSG_TYPE_INT32
	},
	[SYS_INFO_MEMORY] = {
		.name = "memory",
		.type = BLOBMSG_TYPE_TABLE
	},
};

enum {
	MEM_TOTAL,
	MEM_FREE,
	__MEM_MAX,
};

static const struct blobmsg_policy mem_policy[__MEM_MAX] = {
	[MEM_TOTAL] = {
		.name = "total",
		.type = BLOBMSG_TYPE_INT64
	},
	[MEM_FREE] = {
		.name = "free",
		.type = BLOBMSG_TYPE_INT64
	},
};

int collect_system_info(struct ubus_context *ctx,
			struct system_info *info)
{
	struct ubus_call_result result;
	struct blob_attr *tb[__SYS_INFO_MAX];
	struct blob_attr *mtb[__MEM_MAX];

	if (info == NULL) {
		syslog(LOG_ERR,
		       "system_info: output pointer is NULL");
		return -1;
	}

	memset(info, 0, sizeof(*info));

	if (ubus_call_sync(ctx,
			   "system",
			   "info",
			   NULL,
			   &result) != 0) {
		syslog(LOG_WARNING,
		       "system_info: could not read system info via ubus");
		return -1;
	}

	blobmsg_parse(sys_info_policy,
		      __SYS_INFO_MAX,
		      tb,
		      blob_data(result.data),
		      blob_len(result.data));

	if (tb[SYS_INFO_UPTIME] != NULL)
		info->uptime = blobmsg_get_u32(tb[SYS_INFO_UPTIME]);

	if (tb[SYS_INFO_MEMORY] != NULL) {
		blobmsg_parse(mem_policy,
			      __MEM_MAX,
			      mtb,
			      blobmsg_data(tb[SYS_INFO_MEMORY]),
			      blobmsg_data_len(tb[SYS_INFO_MEMORY]));

		if (mtb[MEM_TOTAL] != NULL)
			info->total_ram =
				blobmsg_get_u64(mtb[MEM_TOTAL]);

		if (mtb[MEM_FREE] != NULL)
			info->free_ram =
				blobmsg_get_u64(mtb[MEM_FREE]);
	}

	ubus_call_result_free(&result);

	return 0;
}