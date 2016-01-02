#include "ets_sys.h"
#include "osapi.h"

#include "dht.h"
#include "network.h"

#include "user_config.h"


void ICACHE_FLASH_ATTR user_init() {
	uart_div_modify(0, UART_CLK_FREQ / 115200);

	gpio_init();
	dht_setup();
	network_setup();
}
