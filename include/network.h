#ifndef network_h
#define network_h

#include "ets_sys.h"
#include "ip_addr.h"
#include "user_interface.h"


void ICACHE_FLASH_ATTR network_setup(void);
static void ICACHE_FLASH_ATTR network_wifi_callback(System_Event_t *event);
static void ICACHE_FLASH_ATTR network_dns_callback(const char *name, ip_addr_t *ip, void *arg);

#endif
