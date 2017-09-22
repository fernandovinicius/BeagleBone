INCLUDE_DIR=include
SOURCE_DIR=source
OBJ_DIR=obj

CC=gcc
CFLAGS=-I$(INCLUDE_DIR)/ -Wall

LIBS=-lpthread

_OBJ=main.o ads1256.o spi_interface.o gpio_interface.o
OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

_SOURCE=main.c ads1256.c spi_interface.c gpio_interface.c
SOURCE=$(patsubst %,$(SOURCE_DIR)/%,$(_SOURCE))

TARGET=main

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) 

.PHONY: clean

clean: 
	rm -f $(OBJ_DIR)/*.o $(TARGET)


