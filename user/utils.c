#include "osapi.h"
#include "user_interface.h"


void ICACHE_FLASH_ATTR get_identifier(char* prefix, char* identifier) {
	uint8_t mac[os_strlen(prefix) + 7];

	wifi_get_macaddr(0, mac);

	os_sprintf(identifier, "%s%02x%02x%02x", prefix, mac[3], mac[4], mac[5]);
}
