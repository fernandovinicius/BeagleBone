#ifndef _CONF_H
#define _CONF_H
/***********************************************************************
 * INCLUDES
 **/
 
/***********************************************************************
 * MACROS
 **/

/***********************************************************************
 * DEFINES
 **/
/* Boolean */
#ifndef TRUE
  #define TRUE  1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

/* GPIO */
#ifndef HIGH
  #define HIGH  1
#endif
#ifndef LOW
  #define LOW 0
#endif

/* SPI */
#define SPI_CLOCK_FREQ_HZ   2000000
#define SPI_CLOCK_MODE      1
#define SPI_ENDIANNESS      MSB_FIRST
#define SPI_CS_ACT_MODE     LOW
#define SPI_BITS_PER_WORD   8

/* ADS1256 Pins */
#define ADS1256_DRDY_GPIO     60 /* P9_12 -- Refer to Cape Header */
#define ADS1256_RESET_GPIO    0  /* P9_xx -- Refer to Cape Header */
#define ADS1256_CS_GPIO       48 /* P9_15 -- Refer to Cape Header */
 
/***********************************************************************
 * GLOBALS
 **/
extern volatile int  SPI_FD;

#endif
