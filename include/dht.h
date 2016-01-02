#ifndef dht_h
#define dht_h

#include "ets_sys.h"


struct dht_result {
	uint8_t temperature;
	uint8_t humidity;
};

void ICACHE_FLASH_ATTR dht_setup(void);
uint8_t ICACHE_FLASH_ATTR dht_fetch(struct dht_result *result);
uint8_t ICACHE_FLASH_ATTR dht_fetch_avg(uint8_t count, struct dht_result *result);

#endif
