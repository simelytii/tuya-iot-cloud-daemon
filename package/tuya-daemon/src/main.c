#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>

#include <libubus.h>
#include <libubox/uloop.h>

#include "cli.h"
#include "tuya_client.h"
#include "telemetry.h"
#include "ubus_api.h"

static struct ubus_context *ubus_ctx;
static struct uloop_timeout report_timer;
static struct uloop_timeout tuya_tick_timer;

static int report_interval_sec = 10;

static void report_timer_cb(struct uloop_timeout *t)
{
	telemetry_report();

	uloop_timeout_set(t, report_interval_sec * 1000);
}

static void tuya_tick_cb(struct uloop_timeout *t)
{
	tuya_client_loop();

	uloop_timeout_set(t, 100);
}

static void handle_signal(int signum)
{
	syslog(LOG_INFO,
	       "Received signal %d, shutting down",
	       signum);

	uloop_end();
}

int main(int argc, char **argv)
{
	struct arguments arguments = {0};
	struct sigaction sa;
	int ret;

	openlog("tuya-daemon",
		LOG_PID | LOG_CONS,
		LOG_USER);

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGTERM, &sa, NULL) != 0 ||
	    sigaction(SIGINT, &sa, NULL) != 0 ||
	    sigaction(SIGHUP, &sa, NULL) != 0) {

		syslog(LOG_ERR,
		       "Failed to install signal handlers");

		closelog();
		return EXIT_FAILURE;
	}

	ret = parse_arguments(argc, argv, &arguments);

	if (ret != 0) {
		syslog(LOG_ERR,
		       "Failed to parse arguments");

		closelog();
		return EXIT_FAILURE;
	}

	if (arguments.device_id == NULL ||
	    arguments.device_secret == NULL ||
	    arguments.product_id == NULL) {

		syslog(LOG_ERR,
		       "Missing required arguments "
		       "(device-id/device-secret/product-id)");

		closelog();
		return EXIT_FAILURE;
	}

	uloop_init();

	// Create the single ubus connection used by the daemon.
	ubus_ctx = ubus_connect(NULL);

	if (ubus_ctx == NULL) {
		syslog(LOG_ERR,
		       "Failed to connect to ubus");

		uloop_done();
		closelog();

		return EXIT_FAILURE;
	}

	// Allow ubus to work together with the uloop event loop.
	ubus_add_uloop(ubus_ctx);
	telemetry_init(ubus_ctx);

	// Register the daemon's ubus control API.
	if (ubus_api_register(ubus_ctx) != 0) {

		syslog(LOG_ERR,
		       "Failed to register ubus control API");

		ubus_free(ubus_ctx);
		uloop_done();
		closelog();

		return EXIT_FAILURE;
	}

	ret = tuya_client_init(arguments.device_id,
			       arguments.device_secret);

	if (ret != 0) {

		syslog(LOG_ERR,
		       "Failed to initialize Tuya MQTT: %d",
		       ret);

		ubus_api_unregister(ubus_ctx);
		ubus_free(ubus_ctx);
		uloop_done();
		closelog();

		return EXIT_FAILURE;
	}

	ret = tuya_client_connect();

	if (ret != 0) {

		syslog(LOG_ERR,
		       "Failed to connect to Tuya Cloud: %d",
		       ret);

		tuya_client_deinit();
		ubus_api_unregister(ubus_ctx);
		ubus_free(ubus_ctx);
		uloop_done();
		closelog();

		return EXIT_FAILURE;
	}

	syslog(LOG_INFO,
	       "tuya-daemon started, reporting every %d seconds",
	       report_interval_sec);

	report_timer.cb = report_timer_cb;

	uloop_timeout_set(&report_timer,
			  report_interval_sec * 1000);


	tuya_tick_timer.cb = tuya_tick_cb;

	uloop_timeout_set(&tuya_tick_timer, 100);

	uloop_run();

	syslog(LOG_INFO,
	       "Shutting down, freeing resources");

	uloop_timeout_cancel(&report_timer);
	uloop_timeout_cancel(&tuya_tick_timer);

	tuya_client_deinit();

	ubus_api_unregister(ubus_ctx);

	ubus_free(ubus_ctx);

	uloop_done();

	syslog(LOG_INFO,
	       "Tuya daemon stopped");

	closelog();

	return EXIT_SUCCESS;
}