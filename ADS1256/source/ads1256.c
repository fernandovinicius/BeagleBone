/***********************************************************************
 * INCLUDES
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "ads1256.h"
#include "spi_interface.h"

/***********************************************************************
 * DEFINES
 **/

/***********************************************************************
 * MACROS
 **/
#define ADS1256_DRDY_IS_LOW()   (1)
#define SET_ADS1256_CS_LOW()   
#define SET_ADS1256_CS_HIGH()   
#define US_DELAY(x)             (usleep(x))
#define MS_DELAY(x)             (usleep(1000*x))


/***********************************************************************
 * GLOBALS
 **/
 
 
/***********************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES
 **/
static void ads1256_send_data(uint8_t _data);
static uint8_t ads1256_read_data(void);

/***********************************************************************
 * FUNCTIONS
 **/
/***********************************************************************
 * @fn      ads1256_read_channel
 * 
 * @brief  
 * 
 * @param   ch
 * 
 * @return  none
 */
int32_t ads1256_read_channel(uint8_t ch)
{
  uint32_t result = 0;
  uint8_t  buf[3] = {0,0,0};
  
  
  ads1256_set_channel(0);
  US_DELAY(5);

  ads1256_send_cmd(ADS1256_CMD_SYNC);
  US_DELAY(5);

  ads1256_send_cmd(ADS1256_CMD_WAKEUP);
  US_DELAY(25);


	SET_ADS1256_CS_LOW();

    ads1256_send_cmd(ADS1256_CMD_RDATA);
    US_DELAY(10);

    /* Read the sample results 24bit */
    buf[0] = ads1256_read_data();
    buf[1] = ads1256_read_data();
    buf[2] = ads1256_read_data();
  
  SET_ADS1256_CS_HIGH();

  result = ((uint32_t)buf[0] << 16) & 0x00FF0000;
  result |= ((uint32_t)buf[1] << 8);
  result |= buf[2];

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
 * @param   _cmd
 * 
 * @return  none
 */
void ads1256_send_cmd(uint8_t _cmd)
{
  SET_ADS1256_CS_LOW();
  ads1256_send_data(_cmd);
  SET_ADS1256_CS_HIGH();
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
  const uint8_t status = MSB_FIRST | ACAL_DIS | BUF_DIS;
  const uint8_t mux    = POS_AIN0 | NEG_AINC;
  const uint8_t adcon  = CLKOUT_ON_DIV1 | PGA_GAIN_1;
  const uint8_t drate  = SMPS_30000;

  ads1256_wait_drdy();
  
  SET_ADS1256_CS_LOW();
    
    /* Write Register command, starting from 0h Address (Status Reg) */
    ads1256_send_data(ADS1256_CMD_WREG | 0x00);
    
    /* Number of registers to write (N-1) */
		ads1256_send_data(3);

    ads1256_send_data(status);
    ads1256_send_data(mux);
    ads1256_send_data(adcon);
    ads1256_send_data(drate);

  SET_ADS1256_CS_HIGH();
  
  usleep(50);
}

/***********************************************************************
 * @fn      ads1256_set_channel
 * 
 * @brief  
 * 
 * @param   _ch
 * 
 * @return  none
 */
void ads1256_set_channel(uint8_t _ch)
{
	if (_ch > 7)
	{
		return;
	}
	ads1256_write_register(ADS1256_REG_MUX, (_ch << 4) | (1 << 3));
}
 
/***********************************************************************
 * @fn      ads1256_read_register
 * 
 * @brief  
 * 
 * @param   none
 * 
 * @return  none
 */
uint8_t ads1256_read_register(uint8_t _reg)
{
  uint8_t read = 0;

	SET_ADS1256_CS_LOW();
  
    /* Write command register */
    ads1256_send_data(ADS1256_CMD_RREG | _reg);
    
    /* Write the register number */
    ads1256_send_data(0x00);	
    
    /* Delay time */
    US_DELAY(10);

    /* Read the register values */
    read = ads1256_read_data();	
  
	SET_ADS1256_CS_HIGH();

	return read;
}

/***********************************************************************
 * @fn      ads1256_write_register
 * 
 * @brief  
 * 
 * @param   none
 * 
 * @return  none
 */
void ads1256_write_register(uint8_t _reg, uint8_t _value)
{
	SET_ADS1256_CS_LOW();
  
    /* Write command register */
    ads1256_send_data(ADS1256_CMD_WREG | _reg);
    
    /* Write the register number */
    ads1256_send_data(0x00);	
    
    /* Write register value */
    ads1256_send_data(_value);
  
	SET_ADS1256_CS_HIGH();
}


/***********************************************************************
 * @fn      ads1256_wait_drdy
 * 
 * @brief   Wait untill DRDY Pin goes LOW
 * 
 * @param   none
 * 
 * @return   0 - SUCCESS
 *          -1 - ERROR
 */
int ads1256_wait_drdy(void)
{
	uint32_t i;

	for (i = 0; i < 400000; i++)
	{
		if ( ADS1256_DRDY_IS_LOW() )
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
 * @fn      ads1256_read_chip_id
 * 
 * @brief   Read chip ID
 * 
 * @param   none
 * 
 * @return  -1 - ERROR
 *          >0 - ID
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
 * @fn      ads1256_send_data
 * 
 * @brief   Send data through SPI interface
 * 
 * @param   _data
 * 
 * @return  none
 */
static void ads1256_send_data(uint8_t _data)
{
	//bcm2835_spi_transfer(_data);
}

/***********************************************************************
 * @fn      ads1256_read_data
 * 
 * @brief   Receive data from SPI
 * 
 * @param   none
 * 
 * @return  _data
 */
static uint8_t ads1256_read_data(void)
{
	//return bcm2835_spi_transfer(0xFF);
  return 0;
}
