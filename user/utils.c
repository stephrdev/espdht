#include "osapi.h"
#include "user_interface.h"

#include "dht.h"
#include "utils.h"


#define POST_TEMPERATURE 1
#define POST_HUMIDITY 2
#define POST_VOLTAGE 3

struct dht_result sensor_result;
uint8_t current_post = 0;


void ICACHE_FLASH_ATTR get_identifier(char* prefix, char* identifier) {
	uint8_t mac[os_strlen(prefix) + 7];

	wifi_get_macaddr(0, mac);

	os_sprintf(identifier, "%s%02x%02x%02x", prefix, mac[3], mac[4], mac[5]);
}

void ICACHE_FLASH_ATTR post_sensors(void) {
	if (current_post == POST_TEMPERATURE) {
		current_post = POST_HUMIDITY;
		post_value("humidity", sensor_result.humidity);
		return;
	}
	
	if (current_post == POST_HUMIDITY) {
		current_post = POST_VOLTAGE;
		post_value("voltage", (uint8_t)system_system_get_vdd33());
		return;
	}
	
	if (current_post == POST_VOLTAGE) {
		os_printf("UTILS: Posting done, disconnect.\n");
		wifi_station_disconnect();
		return;
	}

	uint8_t retries = 0;
	while(retries < 3) {
		if (dht_fetch_avg(3, &sensor_result) == 1) {
			current_post = POST_TEMPERATURE;
			post_value("temperature", sensor_result.temperature);
			return;
		}
		retries++;
	}

	os_printf("UTILS: Sensor failed after %d retries!\n", retries);
	wifi_station_disconnect();
}

static void ICACHE_FLASH_ATTR post_value(const char *name, uint8_t value) {
	char headers[128];
	char post_data[64];
	char identifier[6];

	os_printf("UTILS: Posting %s: %d\n", name, value);
	get_identifier("", identifier);
	os_sprintf(
		headers,
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Authorization: Token %s\r\n",
		API_SECRET
	);
	os_sprintf(post_data, "metric=%s-%s&value=%d", identifier, name, value);
	http_post(API_ENDPOINT, headers, post_data, post_callback);
}

static void ICACHE_FLASH_ATTR post_callback(char *resp, int status, char *full_resp) {
	post_sensors();
}
