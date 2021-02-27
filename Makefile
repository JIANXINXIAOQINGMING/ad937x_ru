PWD := $(shell pwd)
OUTPUT_ROOT=$(PWD)/output
BIN_DIR=$(OUTPUT_ROOT)/bin
USR_CPPFLAGS=-I$(PWD)/include

ENV= /home/lxl/ENV/gcc-linaro-7.2.1
LINARO_PATH := $(ENV)/bin/

CROSS_TARGET=arm-linux-gnueabihf-

AR      := $(LINARO_PATH)$(CROSS_TARGET)ar
AS      := $(LINARO_PATH)$(CROSS_TARGET)as
LD      := $(LINARO_PATH)$(CROSS_TARGET)ld
NM      := $(LINARO_PATH)$(CROSS_TARGET)nm
CC      := $(LINARO_PATH)$(CROSS_TARGET)gcc
GCC     := $(LINARO_PATH)$(CROSS_TARGET)gcc
CPP     := $(LINARO_PATH)$(CROSS_TARGET)cpp
CXX     := $(LINARO_PATH)$(CROSS_TARGET)g++
FC      := $(LINARO_PATH)$(CROSS_TARGET)gfortran
F77     := $(LINARO_PATH)$(CROSS_TARGET)gfortran
RANLIB  := $(LINARO_PATH)$(CROSS_TARGET)ranlib
READELF := $(LINARO_PATH)$(CROSS_TARGET)readelf
STRIP   := $(LINARO_PATH)$(CROSS_TARGET)strip
OBJCOPY := $(LINARO_PATH)$(CROSS_TARGET)objcopy
OBJDUMP := $(LINARO_PATH)$(CROSS_TARGET)objdump

PATH:=$(PATH):$(RAU_TOOLCHAINS_PATH)

#OBJDUMP := $(if $(SDK_PATH),$(SDK_PATH)$(TARGET)objdump, $(TARGET)objdump)

CAPTURE_NAME = capture
CAPTURE_DIR = $(PWD)/capture_common
CAPTURE_C = capture_main.c
CAPTURE_O = capture_common.o

MAKE=make

HOST=arm-linux-gnueabihf

all: clean init capture

init:
	mkdir $(OUTPUT_ROOT)
	mkdir $(BIN_DIR)

capture:
	cd $(CAPTURE_DIR) && \
	$(CC) -c -o $(CAPTURE_O) $(CAPTURE_C) $(USR_CPPFLAGS) &&\
	$(CC) -o $(CAPTURE_NAME) $(CAPTURE_O) $(USR_CPPFLAGS) && \
	mv $(CAPTURE_NAME) $(BIN_DIR)


clean: init_clean capture_clean

init_clean:
	rm -rf $(OUTPUT_ROOT)

capture_clean:
	cd $(CAPTURE_DIR)
	rm -rf *.o
