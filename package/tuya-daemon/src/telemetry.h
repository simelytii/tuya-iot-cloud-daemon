#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <libubus.h>

void telemetry_init(struct ubus_context *ctx);
void telemetry_report(void);

#endif