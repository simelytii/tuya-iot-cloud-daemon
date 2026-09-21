#include <string.h>
#include <syslog.h>

#include <libubus.h>
#include <libubox/blobmsg.h>

#include "ubus_utils.h"

static void ubus_call_cb(struct ubus_request *req,
			 int type,
			 struct blob_attr *msg)
{
	struct ubus_call_result *result = req->priv;

	(void)type;

	if (msg == NULL)
		return;

	blob_buf_init(&result->storage, 0);

	blob_put_raw(&result->storage,
		     blob_data(msg),
		     blob_len(msg));

	result->data = result->storage.head;
	result->got_data = 1;
}

int ubus_call_sync(struct ubus_context *ctx,
		   const char *path,
		   const char *method,
		   struct blob_attr *args,
		   struct ubus_call_result *result)
{
	uint32_t obj_id;
	int ret;

	memset(result, 0, sizeof(*result));

	if (ctx == NULL) {
		syslog(LOG_ERR,
		       "ubus_utils: ubus context is NULL");
		return -1;
	}

	ret = ubus_lookup_id(ctx, path, &obj_id);
	if (ret != 0) {
		syslog(LOG_WARNING,
		       "ubus_utils: object '%s' not found (%d)",
		       path, ret);
		return -1;
	}

	ret = ubus_invoke(ctx,
			  obj_id,
			  method,
			  args,
			  ubus_call_cb,
			  result,
			  2000);

	if (ret != 0) {
		syslog(LOG_WARNING,
		       "ubus_utils: call %s->%s failed (%d)",
		       path,
		       method,
		       ret);
		return -1;
	}

	if (!result->got_data)
		return -1;

	return 0;
}

void ubus_call_result_free(struct ubus_call_result *result)
{
	if (result == NULL)
		return;

	if (result->got_data)
		blob_buf_free(&result->storage);

	memset(result, 0, sizeof(*result));
}