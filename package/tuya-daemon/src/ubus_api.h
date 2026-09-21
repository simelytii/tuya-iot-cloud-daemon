#ifndef UBUS_API_H
#define UBUS_API_H

#include <libubus.h>

int ubus_api_register(struct ubus_context *ctx);
void ubus_api_unregister(struct ubus_context *ctx);

#endif
