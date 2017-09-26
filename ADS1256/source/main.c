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
#include "conf.h"
#include "gpio_interface.h"
#include "spi_interface.h"
#include "ads1256.h"

/***********************************************************************
 * DEFINES
 **/

/***********************************************************************
 * GLOBALS
 **/
volatile int  SPI_FD = 0;
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
 * @fn      main
 *
 * @brief
 *
 * @param
 *
 * @return
 */
int main(int argc, char *argv[])
{
  /* Install Signals */
  if ( install_signal(&signal_handler) < 0 )
  {
    exit(-1);
  }

  /* Init SPI Bus */
  SPI_FD = init_spi();
  if ( SPI_FD < 0 )
  {
    exit(-1);
  }

  /* Configure ADC */
  ads1256_config();
  ads1256_send_cmd(ADS1256_CMD_SDATAC);

  while ( FINISH != TRUE )
  {
    double volt[3] = {0,0,0};
    int i = 0;
    for ( i = 0; i < 3; i++ )
    {
      volt[i] = ads1256_read_channel(i) * 5.0 / 8388608.0;
    }
    printf("Ch0: %f V   Ch1: %f V   Ch2: %f\n", volt[0], volt[1], volt[2]);
    usleep(500000);
  }

  /* Close SPI */
  spi_close(SPI_FD);

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
