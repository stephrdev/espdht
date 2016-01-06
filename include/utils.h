#ifndef utils_h
#define utils_h


void ICACHE_FLASH_ATTR get_identifier(char* prefix, char* identifier);

void ICACHE_FLASH_ATTR post_sensors(void);
static void ICACHE_FLASH_ATTR post_value(const char *name, uint8_t value);
static void ICACHE_FLASH_ATTR post_callback(char *resp, int status, char *full_resp);

#endif
