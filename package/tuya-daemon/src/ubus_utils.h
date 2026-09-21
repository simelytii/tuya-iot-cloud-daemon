#ifndef UBUS_UTILS_H
#define UBUS_UTILS_H

#include <libubus.h>
#include <libubox/blob.h>

struct ubus_call_result {
	struct blob_attr *data;
	struct blob_buf storage;
	int got_data;
};

int ubus_call_sync(struct ubus_context *ctx,
		   const char *path,
		   const char *method,
		   struct blob_attr *args,
		   struct ubus_call_result *result);

void ubus_call_result_free(struct ubus_call_result *result);

#endif