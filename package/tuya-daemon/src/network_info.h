#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#include <libubus.h>

#define MAX_INTERFACES 8

struct network_info {
	char name[16];
	char ip_address[46];
	char netmask[46];
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
};

int collect_network_info(struct ubus_context *ctx,
			 struct network_info *interfaces,
			 int max_interfaces);

#endif