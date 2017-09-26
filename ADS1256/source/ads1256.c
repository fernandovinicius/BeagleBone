/***********************************************************************
 * INCLUDES
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "conf.h"
#include "ads1256.h"
#include "gpio_interface.h"
#include "spi_interface.h"

/***********************************************************************
 * DEFINES
 **/

/***********************************************************************
 * MACROS
 **/
#define US_DELAY(x)   (usleep(x))
#define MS_DELAY(x)   (usleep(1000*x))

/***********************************************************************
 * GLOBALS
 **/

/***********************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES
 **/
void ads1256_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len);
void ads1256_set_cs(uint8_t value);
int ads1256_wait_drdy(void);
uint8_t ads1256_drdy_state(void);
void ads1256_hard_reset(void);
void ads1256_soft_reset(void);
void ads1256_us_delay(uint32_t us);
void ads1256_ms_delay(uint32_t ms);

/***********************************************************************
 * FUNCTIONS
 **/
/***********************************************************************
 * @fn      ads1256_read_channel
 *
 * @brief
 *
 * @param   ch - 0:7
 *
 * @return  none
 */
int32_t ads1256_read_channel(uint8_t ch)
{
  uint8_t  read_data_cmd = ADS1256_CMD_RDATA;
  uint32_t result = 0;
  uint8_t  rx_buf[3] = {0,0,0};

  /* Select channel */
  ads1256_set_channel(ch);

  /* Send Sync Command */
  ads1256_send_cmd(ADS1256_CMD_SYNC);

  /* Send Wake Up Command */
  ads1256_send_cmd(ADS1256_CMD_WAKEUP);
  ads1256_us_delay(50);

  /* Select ADS1256 for SPI Communication */
  ads1256_set_cs(LOW);

  /* Send Read Data command */
  ads1256_spi_transfer(&read_data_cmd, NULL, 1);

  /* Read 3 Bytes */
  ads1256_spi_transfer(NULL, rx_buf, 3);

  /* Finish SPI Communication */
  ads1256_set_cs(HIGH);

  /* Parse result */
  result = ((uint32_t)rx_buf[0] << 16) & 0x00FF0000;
  result |= ((uint32_t)rx_buf[1] << 8);
  result |= rx_buf[2];

  /* Extend a signed number*/
  if (result & 0x800000)
  {
    result |= 0xFF000000;
  }

  return (int32_t)result;
}

/***********************************************************************
 * @fn      ads1256_send_cmd
 *
 * @brief
 *
 * @param   cmd
 *
 * @return  none
 */
void ads1256_send_cmd(uint8_t cmd)
{
  /* Select ADS1256 for SPI Communication */
  ads1256_set_cs(LOW);

  /* Send Command */
  ads1256_spi_transfer(&cmd, NULL, 1);

  /* Finish SPI Communication */
  ads1256_set_cs(HIGH);
}

/***********************************************************************
 * @fn      ads1256_config
 *
 * @brief
 *
 * @param   none
 *
 * @return  none
 */
void ads1256_config(void)
{
  const uint8_t status = ADS1256_MSB_FIRST | ADS1256_ACAL_DIS | ADS1256_BUF_DIS;
  const uint8_t mux    = ADS1256_POS_AIN0 | ADS1256_NEG_AINC;
  const uint8_t adcon  = ADS1256_CLKOUT_OFF | ADS1256_PGA_GAIN_1;
  const uint8_t drate  = ADS1256_SMPS_15000;

  uint8_t tx_buf[7];

  tx_buf[0] = ADS1256_CMD_WREG;
  tx_buf[1] = 3;
  tx_buf[2] = status;
  tx_buf[3] = mux;
  tx_buf[4] = adcon;
  tx_buf[5] = drate;

  /* Wait DRDY Signal */
  ads1256_wait_drdy();

  /* Select ADS1256 for SPI Communication */
  ads1256_set_cs(LOW);

  /* Send data */
  ads1256_spi_transfer(tx_buf, NULL, 6);

  /* Finish SPI Communication */
  ads1256_set_cs(HIGH);
}

/***********************************************************************
 * @fn      ads1256_set_channel
 *
 * @brief
 *
 * @param   ch
 *
 * @return  none
 */
void ads1256_set_channel(uint8_t ch)
{
  if ( ch > 7 )
  {
    return;
  }
  ads1256_write_register(ADS1256_REG_MUX, (ch << 4) | (1 << 3));
}

/***********************************************************************
 * @fn      ads1256_read_register
 *
 * @brief
 *
 * @param   reg
 *
 * @return  Register data
 */
uint8_t ads1256_read_register(uint8_t reg)
{
  uint8_t tx_buf[2] = {0,0};
  uint8_t reg_data = 0;

  /* Select ADS1256 for SPI Communication */
  ads1256_set_cs(LOW);

  /* Send Read Register Command */
  tx_buf[0] = ADS1256_CMD_RREG | reg;
  tx_buf[1] = 0;
  ads1256_spi_transfer(tx_buf, NULL, 2);

  /* Read data */
  ads1256_spi_transfer(NULL, &reg_data, 1);

  /* Finish SPI Communication */
  ads1256_set_cs(HIGH);

  return reg_data;
}

/***********************************************************************
 * @fn      ads1256_write_register
 *
 * @brief
 *
 * @param   reg
 *          val
 *
 * @return  none
 */
void ads1256_write_register(uint8_t reg, uint8_t val)
{
  uint8_t tx_buf[3];

  /* Fill buf */
  tx_buf[0] = ADS1256_CMD_WREG | reg; // Write command register
  tx_buf[1] = 0;                      // Number of registers to write (N-1)
  tx_buf[2] = val;                    // Write register value

  /* Wait DRDY Signal */
  ads1256_wait_drdy();

  /* Select ADS1256 for SPI Communication */
  ads1256_set_cs(LOW);

  /* Send data */
  ads1256_spi_transfer(tx_buf, NULL, 3);

  /* Finish SPI Communication */
  ads1256_set_cs(HIGH);
}

/***********************************************************************
 * @fn      ads1256_read_chip_id
 *
 * @brief   Read chip ID
 *
 * @param   none
 *
 * @return
 */
int ads1256_read_chip_id(void)
{
  if ( ads1256_wait_drdy() < 0 )
  {
    return -1;
  }

  uint8_t id = ads1256_read_register(ADS1256_REG_STATUS);

  return (id >> 4);
}

/***********************************************************************
 * @fn      ads1256_spi_transfer
 *
 * @brief   Send data through SPI interface
 *
 * @param   data
 *
 * @return  none
 */
void ads1256_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len)
{
  spi_transfer(SPI_FD, tx_buf, rx_buf, len);
}

/***********************************************************************
 * @fn      ads1256_us_delay
 *
 * @brief
 *
 * @param   value - HIGH or LOW
 *
 * @return  none
 */
void ads1256_set_cs(uint8_t value)
{
  gpio_write(ADS1256_CS_GPIO, value);
}

/***********************************************************************
 * @fn      ads1256_wait_drdy
 *
 * @brief   Wait untill DRDY Pin goes LOW
 *
 * @param   none
 *
 * @return
 */
int ads1256_wait_drdy(void)
{
  uint32_t i;

  for (i = 0; i < 400000; i++)
  {
    if ( ads1256_drdy_state() == LOW )
    {
      break;
    }
  }
  if (i >= 400000)
  {
    printf("ads1256_wait_drdy() Time Out ...\r\n");
    return -1;
  }

  return 0;
}

/***********************************************************************
 * @fn      ads1256_drdy_state
 *
 * @brief
 *
 * @param   none
 *
 * @return  HIGH or LOW
 */
uint8_t ads1256_drdy_state(void)
{
  uint8_t val = 0;

  if ( gpio_read(ADS1256_DRDY_GPIO, &val) < 0 )
  {
    return HIGH; /* BUG!! */
  }

  return val;
}

/***********************************************************************
 * @fn      ads1256_hard_reset
 *
 * @brief
 *
 * @param   none
 *
 * @return  none
 */
void ads1256_hard_reset(void)
{
  gpio_write(ADS1256_RESET_GPIO, LOW);
  ads1256_us_delay(500);
  gpio_write(ADS1256_RESET_GPIO, HIGH);
}

/***********************************************************************
 * @fn      ads1256_soft_reset
 *
 * @brief
 *
 * @param   none
 *
 * @return  none
 */
void ads1256_soft_reset(void)
{
  ads1256_send_cmd(ADS1256_CMD_RESET);
}

/***********************************************************************
 * @fn      ads1256_us_delay
 *
 * @brief
 *
 * @param   us
 *
 * @return  none
 */
void ads1256_us_delay(uint32_t us)
{
  US_DELAY(us);
}

/***********************************************************************
 * @fn      ads1256_ms_delay
 *
 * @brief
 *
 * @param   ms
 *
 * @return  none
 */
void ads1256_ms_delay(uint32_t ms)
{
  US_DELAY(ms);
}
