#include <syslog.h>
#include <stdlib.h>
#include <math.h>
#include <cjson/cJSON.h>

#include "telemetry_json.h"

static double round2(double value)
{
	return round(value * 100.0) / 100.0;
}

static int add_number_item(cJSON *root, const char *name, double value)
{
	cJSON *item;

	item = cJSON_CreateObject();
	if (item == NULL) {
		syslog(LOG_ERR, "Failed to create JSON item: %s", name);
		return -1;
	}

	cJSON_AddNumberToObject(item, "value", value);
	cJSON_AddItemToObject(root, name, item);

	return 0;
}

static int add_array_item(cJSON *root, const char *name, cJSON *array)
{
	cJSON *item;

	if (array == NULL) {
		syslog(LOG_ERR, "Array item is NULL: %s", name);
		return -1;
	}

	item = cJSON_CreateObject();
	if (item == NULL) {
		syslog(LOG_ERR, "Failed to create JSON item: %s", name);
		cJSON_Delete(array);
		return -1;
	}

	cJSON_AddItemToObject(item, "value", array);
	cJSON_AddItemToObject(root, name, item);

	return 0;
}

static cJSON *build_network_interface_json(const struct network_info *interface)
{
	cJSON *object;
	char *json_string;
	cJSON *string_item;

	if (interface == NULL) {
		syslog(LOG_ERR, "Network interface is NULL");
		return NULL;
	}

	object = cJSON_CreateObject();
	if (object == NULL) {
		syslog(LOG_ERR, "Failed to create network interface object");
		return NULL;
	}

	cJSON_AddStringToObject(object, "name", interface->name);
	cJSON_AddStringToObject(object, "ip_addr", interface->ip_address);
	cJSON_AddStringToObject(object, "netmask", interface->netmask);

	cJSON_AddNumberToObject(
		object, "tx_mb",
		round2((double)interface->tx_bytes / (1024.0 * 1024.0)));

	cJSON_AddNumberToObject(
		object, "rx_mb",
		round2((double)interface->rx_bytes / (1024.0 * 1024.0)));

	json_string = cJSON_PrintUnformatted(object);
	cJSON_Delete(object);

	if (json_string == NULL) {
		syslog(LOG_ERR, "Failed to convert network interface to string");
		return NULL;
	}

	string_item = cJSON_CreateString(json_string);
	free(json_string);

	if (string_item == NULL) {
		syslog(LOG_ERR, "Failed to create network interface string");
		return NULL;
	}

	return string_item;
}

static cJSON *build_network_array_json(const struct network_info *interfaces,
					int interface_count)
{
	cJSON *array;
	cJSON *interface_string;
	int i;

	array = cJSON_CreateArray();
	if (array == NULL) {
		syslog(LOG_ERR, "Failed to create network array");
		return NULL;
	}

	for (i = 0; i < interface_count; i++) {
		interface_string = build_network_interface_json(&interfaces[i]);

		if (interface_string == NULL) {
			cJSON_Delete(array);
			return NULL;
		}

		cJSON_AddItemToArray(array, interface_string);
	}

	return array;
}

cJSON *build_telemetry_json(const struct system_info *system,
			    const struct network_info *interfaces,
			    int interface_count, double cpu_usage)
{
	cJSON *root;
	cJSON *network_array;

	if (system == NULL || interface_count < 0) {
		syslog(LOG_ERR, "Invalid telemetry data");
		return NULL;
	}

	if (interface_count > 0 && interfaces == NULL) {
		syslog(LOG_ERR, "Network interfaces are NULL");
		return NULL;
	}

	root = cJSON_CreateObject();
	if (root == NULL) {
		syslog(LOG_ERR, "Failed to create JSON object");
		return NULL;
	}

	if (add_number_item(root, "total_ram",
			    round2(system->total_ram / (1024.0 * 1024.0))) != 0) {
		cJSON_Delete(root);
		return NULL;
	}

	if (add_number_item(root, "free_ram",
			    round2(system->free_ram / (1024.0 * 1024.0))) != 0) {
		cJSON_Delete(root);
		return NULL;
	}

	if (add_number_item(root, "system_uptime", system->uptime) != 0) {
		cJSON_Delete(root);
		return NULL;
	}

	if (add_number_item(root, "cpu_usage", round2(cpu_usage)) != 0) {
		cJSON_Delete(root);
		return NULL;
	}

	network_array = build_network_array_json(interfaces, interface_count);
	if (network_array == NULL) {
		cJSON_Delete(root);
		return NULL;
	}

	if (add_array_item(root, "interfaces", network_array) != 0) {
		cJSON_Delete(root);
		return NULL;
	}

	return root;
}