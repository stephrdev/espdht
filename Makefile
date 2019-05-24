# Makefile for ESP8266 projects based on
# https://github.com/esp8266/source-code-examples/blob/master/example.Makefile
#
# Ensure ESP_SDK_DIR is set!


TARGET = app
MODULES = user lib/esphttpclient
LIBS = c gcc hal pp phy net80211 lwip wpa ssl main


# Changes below are not needed most of the time.
XTENSA_TOOLS_DIR ?= $(ESP_SDK_DIR)/../xtensa-lx106-elf/bin

EXTRA_INC_DIR = include
BUILD_BASE_DIR = build
FIRMWARE_BASE_DIR = firmware
FIRMWARE_FILE_1_ADDR = 0x00000
FIRMWARE_FILE_2_ADDR = 0x40000

CFLAGS = -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static
LD_SCRIPT = eagle.app.v6.ld

SDK_LIBDIR = lib
SDK_LDDIR = ld
SDK_INCDIR = include include/json

CC := $(XTENSA_TOOLS_DIR)/xtensa-lx106-elf-gcc
AR := $(XTENSA_TOOLS_DIR)/xtensa-lx106-elf-ar
LD := $(XTENSA_TOOLS_DIR)/xtensa-lx106-elf-gcc

ESPTOOL := python2 $(XTENSA_TOOLS_DIR)/esptool.py
ESPTERM ?= picocom
ESPPORT ?= /dev/ttyUSB0
ESPBAUD ?= 38400
ESPTERM_OPTIONS ?= --baud $(ESPBAUD) $(ESPPORT)

# DONT CHANGE STUFF BELOW.
SRC_DIR := $(MODULES)
BUILD_DIR := $(addprefix $(BUILD_BASE_DIR)/,$(MODULES))

SDK_LIBDIR := $(addprefix $(ESP_SDK_DIR)/,$(SDK_LIBDIR))
SDK_INCDIR := $(addprefix -I$(ESP_SDK_DIR)/,$(SDK_INCDIR))

SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ := $(patsubst %.c,$(BUILD_BASE_DIR)/%.o,$(SRC))
LIBS := $(addprefix -l,$(LIBS))
APP_AR := $(addprefix $(BUILD_BASE_DIR)/,$(TARGET)_app.a)
TARGET_OUT := $(addprefix $(BUILD_BASE_DIR)/,$(TARGET).out)
LD_SCRIPT := $(addprefix -T$(ESP_SDK_DIR)/$(SDK_LDDIR)/,$(LD_SCRIPT))
INCDIR := $(addprefix -I,$(SRC_DIR))
EXTRA_INC_DIR := $(addprefix -I,$(EXTRA_INC_DIR))
MODULE_INCDIR := $(addsuffix /include,$(INCDIR))

FIRMWARE_FILE_1 := $(addprefix $(FIRMWARE_BASE_DIR)/,$(FIRMWARE_FILE_1_ADDR).bin)
FIRMWARE_FILE_2 := $(addprefix $(FIRMWARE_BASE_DIR)/,$(FIRMWARE_FILE_2_ADDR).bin)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INC_DIR) $(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@
endef

.PHONY: all checkdirs flash serial clean

all: checkdirs $(TARGET_OUT) $(FIRMWARE_FILE_1) $(FIRMWARE_FILE_2)

$(FIRMWARE_BASE_DIR)/%.bin: $(TARGET_OUT) | $(FIRMWARE_BASE_DIR)
	$(vecho) "FW $(FIRMWARE_BASE_DIR)/"
	$(Q) $(ESPTOOL) elf2image -o $(FIRMWARE_BASE_DIR)/ $(TARGET_OUT)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FIRMWARE_BASE_DIR)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FIRMWARE_BASE_DIR):
	$(Q) mkdir -p $@

flash: $(FIRMWARE_FILE_1) $(FIRMWARE_FILE_2)
	$(Q) $(ESPTOOL) --port $(ESPPORT) write_flash $(FIRMWARE_FILE_1_ADDR) $(FIRMWARE_FILE_1) $(FIRMWARE_FILE_2_ADDR) $(FIRMWARE_FILE_2)

clean:
	$(Q) rm -rf $(FIRMWARE_BASE_DIR) $(BUILD_BASE_DIR)

serial:
	$(Q) $(ESPTERM) $(ESPTERM_OPTIONS)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
