#include <string.h>
#include <syslog.h>
#include <time.h>

#include <libubus.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>

#include "ubus_api.h"
#include "tuya_client.h"
#include "telemetry.h"

static time_t   last_report_time  = 0;
static int      last_report_ok    = 0;

void ubus_api_note_report_result(int ok)
{
	last_report_time = time(NULL);
	last_report_ok = ok;
}

static int method_status(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg)
{
	struct blob_buf b = {0};

	(void)obj;
	(void)method;
	(void)msg;

	blob_buf_init(&b, 0);

	blobmsg_add_u8(&b, "connected", tuya_client_connected() ? 1 : 0);
	blobmsg_add_u8(&b, "last_report_ok", last_report_ok);
	blobmsg_add_u32(&b, "last_report_unixtime", (uint32_t)last_report_time);

	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}

static int method_report_now(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req, const char *method,
			      struct blob_attr *msg)
{
	struct blob_buf b = {0};

	(void)obj;
	(void)method;
	(void)msg;

	syslog(LOG_INFO, "ubus_api: report_now requested via ubus");
	telemetry_report();

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "triggered", 1);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}

static const struct ubus_method tuya_daemon_methods[] = {
	UBUS_METHOD_NOARG("status", method_status),
	UBUS_METHOD_NOARG("report_now", method_report_now),
};

static struct ubus_object_type tuya_daemon_object_type =
	UBUS_OBJECT_TYPE("tuya_daemon", tuya_daemon_methods);

static struct ubus_object tuya_daemon_object = {
	.name = "tuya_daemon",
	.type = &tuya_daemon_object_type,
	.methods = tuya_daemon_methods,
	.n_methods = ARRAY_SIZE(tuya_daemon_methods),
};

int ubus_api_register(struct ubus_context *ctx)
{
	int ret;

	ret = ubus_add_object(ctx, &tuya_daemon_object);
	if (ret != 0) {
		syslog(LOG_ERR, "ubus_api: failed to register ubus object: %d",
		       ret);
		return -1;
	}

	syslog(LOG_INFO, "ubus_api: 'tuya_daemon' ubus object registered "
			 "(status, report_now)");
	return 0;
}

void ubus_api_unregister(struct ubus_context *ctx)
{
	ubus_remove_object(ctx, &tuya_daemon_object);
}
