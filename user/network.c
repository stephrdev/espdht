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
}

static void ICACHE_FLASH_ATTR network_wifi_callback(System_Event_t *event) {
	if (event->event == EVENT_STAMODE_GOT_IP) {
		os_printf(
			"WIFI: Got IP " IPSTR "/" IPSTR " (" IPSTR ")\n",
			IP2STR(&event->event_info.got_ip.ip),
			IP2STR(&event->event_info.got_ip.mask),
			IP2STR(&event->event_info.got_ip.gw)
		);

		struct espconn api_conn;
		ip_addr_t api_ip;
		espconn_gethostbyname(&api_conn, API_HOST, &api_ip, network_dns_callback);

		// struct dht_result sensor;
		// uint8_t result = dht_fetch_avg(3, &sensor);

		// os_printf(
		// 	"%d/TEMP: %d HUM: %d\n",
		// 	result,
		// 	sensor.temperature,
		// 	sensor.humidity
		// );
	}
}

static void ICACHE_FLASH_ATTR network_dns_callback(const char *name, ip_addr_t *ip, void *arg) {
	struct espconn *api_conn = (struct espconn *)arg;

	if(ip == NULL) {
		os_printf("WIFI: DNS Lookup failed: %s\n", name);
		return;
	}

	os_printf("WIFI: Connecting to " IPSTR "\n", IP2STR(&ip->addr));
}
