#include "osapi.h"
#include "os_type.h"
#include "gpio.h"

#include "dht.h"


void ICACHE_FLASH_ATTR dht_setup(void) {
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
	GPIO_DIS_OUTPUT(2);
}

uint8_t ICACHE_FLASH_ATTR dht_fetch(struct dht_result *result) {
	uint16_t readings;
	uint8_t last_reading = 1;
	uint16_t pulse_width = 0;

	uint8_t bits = 0;
	uint8_t checksum = 0;
	uint8_t data[5] = {0, 0, 0, 0, 0};

	os_printf("DHT: Reading sensor.\n");
	GPIO_OUTPUT_SET(2, 1);
	os_delay_us(250 * 1000);
	GPIO_OUTPUT_SET(2, 0);
	os_delay_us(20 * 1000);
	GPIO_DIS_OUTPUT(2);
	os_delay_us(40);

	for (readings = 0; readings <= 100; readings++) {
		// Check for low signal.
		if (GPIO_INPUT_GET(2) == 0) { break; }

		// Limit reached, fail.
		if (readings >= 100) { return 0; }

		os_delay_us(1);
	}

	for (readings = 0; readings <= 10000; readings++) {
		pulse_width = 0;

		// Measure pulse width.
		while (GPIO_INPUT_GET(2) == last_reading && pulse_width < 1000) {
			pulse_width++;
			os_delay_us(1);
		}

		if(pulse_width > 1000) {
			// Pulse to long, skip.
			break;
		}

		last_reading = GPIO_INPUT_GET(2);

		// Skip some readings.
		if ((readings > 3) && (readings % 2 == 0)) {
			data[bits / 8] <<= 1;
			if (pulse_width > 32) {
				data[bits / 8] |= 1;
			}
			bits++;

			if (bits >= 40) {
				break;
			}
		}
	}

	if (bits != 40) {
		os_printf("DHT: Invalid bit count: %d\n", bits);
		return 0;
	}

	checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;

	os_printf(
		"DHT: Received %02x %02x %02x %02x [%02x] CS: %02x\n",
		data[0],
		data[1],
		data[2],
		data[3],
		data[4],
		checksum
	);

	if (data[4] != checksum) {
		os_printf("DHT: Invalid checksum.\n");
		return 0;
	}

	os_printf("DHT: Temperature: %d, humidity: %d\n", data[2], data[0]);

	result->temperature = data[2];
	result->humidity = data[0];
	return 1;
}

uint8_t ICACHE_FLASH_ATTR dht_fetch_avg(uint8_t count, struct dht_result *result) {
	struct dht_result temp_result;
	uint8_t i;

	result->temperature = 0;
	result->humidity = 0;

	for (i = 0; i < count; i++) {
		if (dht_fetch(&temp_result) != 1) {
			os_printf("DHT: Error while fetching result #%d\n", i + 1);
			return 0;
		}

		result->temperature += temp_result.temperature;
		result->humidity += temp_result.humidity;

		if (i > 0) {
			result->temperature /= 2;
			result->humidity /= 2;
		}
	}

	os_printf(
		"DHT: Average after %d results - temperature: %d / humidity: %d\n",
		i,
		result->temperature,
		result->humidity
	);

	return 1;
}
