#ifndef _ADS1256_H
#define _ADS1256_H
/***********************************************************************
 * INCLUDES
 **/
#include <stdint.h>
#include <stdbool.h>

/***********************************************************************
 * TYPEDEFS
 **/
typedef struct spi_config_t
{
  uint32_t clk_freq;
  uint8_t  clk_mode;
  uint8_t  endianess;
  uint8_t  bits_per_word;
  uint8_t  cs_active_mode;
} spi_config_t;

/***********************************************************************
 * DEFINES
 **/
/* Endianness */
#define MSB_FIRST 0
#define LSB_FIRST 1

/* Max SPI Data */
#define SPI_MAX_DATA 4096

/***********************************************************************
 * FUNCTIONS
 **/
int spi_open(char *spi_device);
int spi_close(int fd);
int spi_transfer(int fd, void *tx_buf, void *rx_buf, uint32_t num_words);
int spi_set_config(int fd, spi_config_t *p_spi_config);

#endif
