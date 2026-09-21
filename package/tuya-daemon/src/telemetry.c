#include <syslog.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#include "telemetry.h"
#include "telemetry_json.h"
#include "system_info.h"
#include "network_info.h"
#include "cpu_info.h"
#include "tuya_client.h"

static struct ubus_context *ubus_ctx;

extern void ubus_api_note_report_result(int ok);

void telemetry_init(struct ubus_context *ctx)
{
	ubus_ctx = ctx;
}

void telemetry_report(void)
{
	struct system_info system;
	struct network_info interfaces[MAX_INTERFACES];
	int interface_count;
	double cpu_usage;
	cJSON *root;
	char *json;
	int ret;

	if (ubus_ctx == NULL) {
		syslog(LOG_ERR,
		       "Telemetry: ubus context is NULL");
		ubus_api_note_report_result(0);
		return;
	}

	if (collect_system_info(ubus_ctx, &system) != 0) {
		syslog(LOG_ERR,
		       "Failed to collect system information via ubus");
		ubus_api_note_report_result(0);
		return;
	}

	interface_count = collect_network_info(ubus_ctx,
					       interfaces,
					       MAX_INTERFACES);

	if (interface_count < 0) {
		syslog(LOG_ERR,
		       "Failed to collect network information via ubus");
		ubus_api_note_report_result(0);
		return;
	}

	if (get_cpu_usage(&cpu_usage) != 0) {
		syslog(LOG_WARNING,
		       "Failed to get CPU usage, reporting 0 for this cycle");
		cpu_usage = 0.0;
	}

	root = build_telemetry_json(&system,
				    interfaces,
				    interface_count,
				    cpu_usage);

	if (root == NULL) {
		ubus_api_note_report_result(0);
		return;
	}

	json = cJSON_PrintUnformatted(root);

	if (json == NULL) {
		syslog(LOG_ERR,
		       "Failed to create JSON string");

		cJSON_Delete(root);
		ubus_api_note_report_result(0);
		return;
	}

	syslog(LOG_INFO,
	       "Reporting data: %s",
	       json);

	if (!tuya_client_connected()) {
		syslog(LOG_WARNING,
		       "Not connected to Tuya Cloud, skipping report");

		free(json);
		cJSON_Delete(root);
		ubus_api_note_report_result(0);
		return;
	}

	ret = tuya_client_report(json);

	if (ret < 0) {
		syslog(LOG_ERR,
		       "Failed to report data to Tuya Cloud: %d",
		       ret);

		ubus_api_note_report_result(0);
	} else {
		syslog(LOG_INFO,
		       "Data reported successfully, message ID: %d",
		       ret);

		ubus_api_note_report_result(1);
	}

	free(json);
	cJSON_Delete(root);
}