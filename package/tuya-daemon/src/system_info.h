#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <libubus.h>

struct system_info {
	unsigned long long total_ram;
	unsigned long long free_ram;
	long uptime;
};

int collect_system_info(struct ubus_context *ctx,
			struct system_info *info);

#endif