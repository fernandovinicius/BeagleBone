/***********************************************************************
 * INCLUDES
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "spi_interface.h"
#include "ads1256.h"

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
#define SPI_CLOCK_MODE      0
#define SPI_ENDIANNESS      MSB_FIRST
#define SPI_CS_ACT_MODE     LOW
#define SPI_BITS_PER_WORD   8

 
/***********************************************************************
 * GLOBALS
 **/
volatile bool FINISH = FALSE;

/***********************************************************************
 * PROTOTYPES
 **/
/* Signals */
void signal_handler(int signal);
int install_signal(void *signal_handler);

/* SPI */
int init_spi(void);

/***********************************************************************
 * MAIN
 **/
/***********************************************************************
 * @fn
 * 
 * @brief
 * 
 * @param
 * 
 * @return
 */
int main(int argc, char *argv[])
{
  int spi_fd = 0;
  
  
  /* Install Signals */
  if ( install_signal(&signal_handler) < 0 )
  {
    exit(-1);
  }
  
  /* Init SPI Bus */
  spi_fd = init_spi();
  if ( spi_fd < 0 )
  {
    exit(-1);
  }

  while ( FINISH != TRUE )
  {

    sleep(1);
  }
  
  end:
  /* Close SPI */
  spi_close(spi_fd);
  
  return 0;
}

/***********************************************************************
 * FUNCTIONS
 **/
/***********************************************************************
 * @fn      signal_handler
 * 
 * @brief   
 * 
 * @param   signal
 * 
 * @return  void
 **/
void signal_handler(int signal)
{
  if ( signal == SIGINT )
  {
    FINISH = TRUE;
  }
}

/***********************************************************************
 * @fn      install_signal
 * 
 * @brief   
 * 
 * @param   void
 * 
 * @return  void
 **/
int install_signal(void *signal_handler)
{
  struct sigaction sig_cb;

  sig_cb.sa_handler = signal_handler;
  sig_cb.sa_flags   = SA_NOMASK;
  if ( sigaction(SIGINT, &sig_cb, NULL) )
  {
    perror("sigaction(SIGINT)");
    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      spi_init
 * 
 * @brief   
 * 
 * @param   void
 * 
 * @return  void
 **/
int init_spi(void)
{
  /* Open SPI */
  int fd = spi_open("/dev/spidev1.0");
  if ( fd < 0 )
  {
    return -1;
  }
  
  /* SPI Settings */
  spi_config_t spi_config;
  memset(&spi_config, 0, sizeof(spi_config_t));
  spi_config.clk_freq       = SPI_CLOCK_FREQ_HZ;
  spi_config.clk_mode       = SPI_CLOCK_MODE;
  spi_config.endianess      = SPI_ENDIANNESS;
  spi_config.bits_per_word  = SPI_BITS_PER_WORD;
  spi_config.cs_active_mode = SPI_CS_ACT_MODE;
  
  if ( spi_set_config(fd, &spi_config) < 0 )
  {
    spi_close(fd);
    
    return -1;
  }
  
  return fd;
}
