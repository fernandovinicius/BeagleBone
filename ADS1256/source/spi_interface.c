/***********************************************************************
 * INCLUDES
 **/
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "spi_interface.h"

/***********************************************************************
 * DEFINES
 **/
#ifndef HIGH
  #define HIGH 1
#endif
#ifndef LOW
  #define LOW 0
#endif

#ifndef TRUE
  #define TRUE 1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

/***********************************************************************
 * MACROS
 **/

/***********************************************************************
 * GLOBALS
 **/

/***********************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES
 **/
int spi_set_clock_freq(int fd, uint32_t clk_freq);
int spi_get_clock_freq(int fd, uint32_t *p_clk_freq);
int spi_set_clock_mode(int fd, uint32_t clk_mode);
int spi_get_clock_mode(int fd, uint8_t *p_clk_mode);
int spi_enable_cs(int fd);
int spi_disable_cs(int fd);
int spi_set_cs_act_low(int fd);
int spi_set_cs_act_high(int fd);
int spi_set_bits_per_word(int fd, uint8_t bits_per_word);
int spi_get_bits_per_word(int fd, uint8_t *p_bits);
int spi_set_endianness(int fd, uint8_t endianness);
int spi_set_mode(int fd, uint8_t mode);
int spi_get_mode(int fd, uint8_t *p_mode);
uint8_t spi_parse_mode(int fd, uint8_t cs_active_mode, uint8_t clk_mode);

/***********************************************************************
 * FUNCTIONS
 **/
/***********************************************************************
 * @fn      spi_open
 *
 * @brief
 *
 * @param   spi_device - SPI device path (string)
 *
 * @return  fd - SPI file descriptor
 */
int spi_open(char *spi_device)
{
  int fd = 0;
  fd = open(spi_device, O_RDWR);
  if ( fd < 0 )
  {
    fprintf(stderr, "open(%s):", spi_device);
    perror("");

    return -1;
  }

  return fd;
}

/***********************************************************************
 * @fn      spi_close
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *
 * @return
 */
int spi_close(int fd)
{
  return close(fd);
}

/***********************************************************************
 * @fn      spi_transfer
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          tx_buf - Pointer to transmit buffer
 *          rx_buf - Pointer to transmit buffer
 *          num_words - Number of buffer words to transmit
 *          word_len  - Word size (in bits)
 *
 * @return
 */
int spi_transfer_delay(int fd, void *tx_buf, void *rx_buf, uint32_t num_words, uint32_t delay_us)
{
  uint32_t buf_size = 0;
  uint8_t  bits_per_word = 0;

  /* Get Word Size (in bits) */
  if ( spi_get_bits_per_word(fd, &bits_per_word) < 0 )
  {
    return -1;
  }

  /* Check bytes number */
  buf_size = (uint32_t)((bits_per_word * num_words) >> 3);
  if ( buf_size > SPI_MAX_DATA )
  {
    buf_size = SPI_MAX_DATA;
  }

  /* Fill transfer struct */
  struct spi_ioc_transfer transfer;
  memset((void *) &transfer, 0, sizeof(struct spi_ioc_transfer));
  transfer.tx_buf         = (uintptr_t)tx_buf;
  transfer.rx_buf         = (uintptr_t)rx_buf;
  transfer.len            = buf_size;
  transfer.speed_hz       = 0;
  transfer.delay_usecs    = delay_us;
  transfer.bits_per_word  = bits_per_word;
  transfer.cs_change      = 0;
  transfer.pad            = 0;

  /* Send data */
  if ( ioctl(fd, SPI_IOC_MESSAGE(1), &transfer) < 0 )
  {
    perror("ioctl(SPI_IOC_MESSAGE(1))");

    return -1;
  }

  return buf_size;
}

/***********************************************************************
 * @fn      spi_transfer
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          tx_buf - Pointer to transmit buffer
 *          rx_buf - Pointer to transmit buffer
 *          num_words - Number of buffer words to transmit
 *          word_len  - Word size (in bits)
 *
 * @return
 */
int spi_transfer(int fd, void *tx_buf, void *rx_buf, uint32_t num_words)
{
  return spi_transfer_delay(fd, tx_buf, rx_buf, num_words, 0);
}

/***********************************************************************
 * @fn      spi_set_config
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          p_spi_config -
 *
 * @return
 */
int spi_set_config(int fd, spi_config_t *p_spi_config)
{
  if ( spi_set_clock_freq(fd, p_spi_config->clk_freq) < 0 )
  {
    return -1;
  }

  if ( spi_set_bits_per_word(fd, p_spi_config->bits_per_word) < 0 )
  {
    return -1;
  }

  if ( spi_set_endianness(fd, p_spi_config->endianess) < 0 )
  {
    return -1;
  }

  uint8_t mode = spi_parse_mode(fd, p_spi_config->cs_active_mode, p_spi_config->clk_mode);
  if ( spi_set_mode(fd, mode) < 0 )
  {
    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_set_clock_freq
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          clk_freq -
 *
 * @return
 */
int spi_set_clock_freq(int fd, uint32_t clk_freq)
{
  if ( ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &clk_freq) < 0 )
  {
    perror("ioctl(SPI_IOC_WR_MAX_SPEED_HZ)");

    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_get_clock_freq
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          p_clk_freq - pointer to clock_frequency variable
 *
 * @return
 */
int spi_get_clock_freq(int fd, uint32_t *p_clk_freq)
{
  if ( ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, p_clk_freq) < 0 )
  {
    perror("ioctl(SPI_IOC_RD_MAX_SPEED_HZ)");

    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_set_clock_mode
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          clk_mode - Clock Mode
 *
 * @return
 */
int spi_set_clock_mode(int fd, uint32_t clk_mode)
{
  uint8_t spi_mode = 0;

  /* Get current SPI mode */
  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    return -1;
  }

  /* Clear clock configuration bits */
  spi_mode &= ~0x03;
  spi_mode |= (clk_mode & 0x03);

  return spi_set_mode(fd, spi_mode);
}

/***********************************************************************
 * @fn      spi_get_clock_mode
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          p_clk_mode - Pointer to Clock Mode variable
 *
 * @return
 */
int spi_get_clock_mode(int fd, uint8_t *p_clk_mode)
{
  uint8_t spi_mode = 0;

  /* Get current SPI mode */
  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    return -1;
  }

  *p_clk_mode = (spi_mode & 0x03);

  return 0;
}

/***********************************************************************
 * @fn      spi_enable_cs
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *
 * @return
 */
int spi_enable_cs(int fd)
{
  uint8_t spi_mode = 0;
  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    return -1;
  }

  spi_mode &= ~SPI_NO_CS;

  return spi_set_mode(fd, spi_mode);
}

/***********************************************************************
 * @fn      spi_disable_cs
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *
 * @return
 */
int spi_disable_cs(int fd)
{
  uint8_t spi_mode = 0;
  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    return -1;
  }

  spi_mode |= SPI_NO_CS;

  return spi_set_mode(fd, spi_mode);
}

/***********************************************************************
 * @fn      spi_set_cs_act_low
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *
 * @return
 */
int spi_set_cs_act_low(int fd)
{
  uint8_t spi_mode = 0;
  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    return -1;
  }

  spi_mode &= ~SPI_CS_HIGH;

  return spi_set_mode(fd, spi_mode);
}

/***********************************************************************
 * @fn      spi_set_cs_act_high
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *
 * @return
 */
int spi_set_cs_act_high(int fd)
{
  uint8_t spi_mode = 0;
  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    return -1;
  }

  spi_mode |= SPI_CS_HIGH;

  return spi_set_mode(fd, spi_mode);
}

/***********************************************************************
 * @fn      spi_set_bits_per_word
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          bits_per_word
 *
 * @return
 */
int spi_set_bits_per_word(int fd, uint8_t bits_per_word)
{
  if ( ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0 ) 
  {
    perror("ioctl(SPI_IOC_WR_BITS_PER_WORD)");

    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_get_bits_per_word
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          p_bits -
 *
 * @return
 */
int spi_get_bits_per_word(int fd, uint8_t *p_bits)
{
  if ( ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, p_bits ) < 0)
  {
    perror("ioctl(SPI_IOC_RD_BITS_PER_WORD)");

    return -1;
  }

  if ( *p_bits == 0 )
  {
    *p_bits = 8;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_set_endianness
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          endianness - MSB_FIRST or LSB_FIRST
 *
 * @return
 */
int spi_set_endianness(int fd, uint8_t endianness)
{
  if ( ioctl(fd, SPI_IOC_WR_LSB_FIRST, &endianness) < 0 )
  {
    perror("ioctl(SPI_IOC_WR_LSB_FIRST)");

    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_set_mode
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          mode - SPI mode value
 *
 * @return
 */
int spi_set_mode(int fd, uint8_t mode)
{
  if ( ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 )
  {
    perror("ioctl(SPI_IOC_WR_MODE)");

    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_get_mode
 *
 * @brief
 *
 * @param   fd - SPI file descriptor
 *          p_mode - Pointer to SPI mode variable
 *
 * @return
 */
int spi_get_mode(int fd, uint8_t *p_mode)
{
  if ( ioctl(fd, SPI_IOC_RD_MODE, p_mode) < 0 )
  {
    perror("ioctl(SPI_IOC_RD_MODE)");

    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_parse_mode
 *
 * @brief
 *
 * @param
 *
 * @return
 */
uint8_t spi_parse_mode(int fd, uint8_t cs_active_mode, uint8_t clk_mode)
{
  uint8_t spi_mode = 0;

  if ( spi_get_mode(fd, &spi_mode) < 0 )
  {
    spi_mode = 0;
  }

  /* Enable CS */
  spi_mode &= ~SPI_NO_CS;

  /* CS Polarity */
  if ( cs_active_mode == HIGH )
  {
    spi_mode |= SPI_CS_HIGH;
  }
  else
  {
    spi_mode &= ~SPI_CS_HIGH;
  }

  /* Clock Mode */
  spi_mode &= ~0x03;
  spi_mode |= (clk_mode & 0x03);

  return spi_mode;
}
