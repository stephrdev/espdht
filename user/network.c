#include "ip_addr.h"
#include "espconn.h"
#include "osapi.h"
#include "user_interface.h"

#include "dht.h"
#include "network.h"
#include "utils.h"
#include "user_config.h"


void ICACHE_FLASH_ATTR network_setup(void) {
	static struct station_config config;

	char identifier[13];
	get_identifier("sensor-", identifier);

	os_printf("WIFI: Connecting to %s (%s)\n", SSID, identifier);

	config.bssid_set = 0;
	os_memcpy(&config.ssid, SSID, os_strlen(SSID));
	os_memcpy(&config.password, SSID_PASSWORD, os_strlen(SSID_PASSWORD));

	wifi_set_opmode_current(STATION_MODE);
	wifi_station_set_hostname(identifier);
	wifi_station_set_config(&config);

	wifi_set_event_handler_cb(network_wifi_callback);
	wifi_station_connect();
}

static void ICACHE_FLASH_ATTR network_wifi_callback(System_Event_t *event) {
	os_printf("WIFI: %d", event->event);

	if (event->event == EVENT_STAMODE_GOT_IP) {
		os_printf(
			"WIFI: Got IP " IPSTR "/" IPSTR " (" IPSTR ")\n",
			IP2STR(&event->event_info.got_ip.ip),
			IP2STR(&event->event_info.got_ip.mask),
			IP2STR(&event->event_info.got_ip.gw)
		);

		post_sensors();
	}

	if (event->event == EVENT_STAMODE_DISCONNECTED) {
		os_printf("WIFI: Disconnected. Going to sleep.\n");
		deep_sleep_set_option(0);
		system_deep_sleep(API_INTERVAL * 1000 * 1000);
	}
}
