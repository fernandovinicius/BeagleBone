CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv

all: pru_adc.bin host_adc

clean:
		rm -f host_adc *.o *.bin

pru_adc.bin: pru_adc.p
		pasm -b $^

host_adc: host_adc.o
