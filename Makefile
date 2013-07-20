## Oven controller Makefile

MCU=atmega8515
LIBS=

PROGRAMMER_MCU=m8515
AVRDUDE_PROGRAMMER=usbasp

PROJECT_NAME=ovencontroller

OPTLEVEL=s


# compiler
CFLAGS=-I. $(INC) -g -mmcu=$(MCU) -O$(OPTLEVEL) \
	-fpack-struct -fshort-enums             \
	-funsigned-bitfields -funsigned-char    \
	-Wall -Wstrict-prototypes               \
	-Wa,-ahlms=$(firstword                  \
	$(filter %.lst, $(<:.c=.lst)))

PRJSRC= src/main.c src/spi.c src/uart.c \
	src/temp.c src/lcd.c src/controller.c

#  C
CFILES=$(filter %.c, $(PRJSRC))


SOURCEDIRS=./src
INC=-I./src

TARGET=$(PROJECT_NAME).out

CFLAGS += ${addprefix -I,$(SOURCEDIRS)}
CFLAGS += -mmcu=$(MCU)

# linker
LDFLAGS=-Wl,-Map,$(TARGET).map -mmcu=$(MCU) \
	-lm $(LIBS)

OBJDEPS=$(CFILES:.c=.o)

CC=avr-gcc
OBJCOPY=avr-objcopy

HEXFORMAT=ihex

.SUFFIXES : .c .cc .cpp .C .o .out .s .S \
	.hex .ee.hex .h .hh .hpp

.PHONY : flash


all: $(TARGET)
	


$(TARGET): $(OBJDEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJDEPS)

$(PROJECT_NAME).out.hex: $(PROJECT_NAME).out
	$(OBJCOPY) -j .text                    \
		-j .data                       \
		-O $(HEXFORMAT) $< $@


# object from C
.c.o: 
	$(CC) $(CFLAGS) -c $< -o $@

flash:
	@avrdude -p $(PROGRAMMER_MCU) -c $(AVRDUDE_PROGRAMMER) -U flash:w:$(PROJECT_NAME).out.hex


