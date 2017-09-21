#ifndef _ADS1256_H
#define _ADS1256_H
/***********************************************************************
 * INCLUDES
 **/


/***********************************************************************
 * DEFINES
 **/
/* ADS1256 Pins */
#define ADS1256_DRDY  (0) 
#define ADS1256_RST   (0)
#define	ADS1256_CS    (0) 

/* ADS1256 Registers Address (followed by reset the default values) */
typedef enum ads1256_reg_addr_t
{
	ADS1256_REG_STATUS = 0, // x1H
	ADS1256_REG_MUX,        // 01H
	ADS1256_REG_ADCON,      // 20H
	ADS1256_REG_DRATE,      // F0H
	ADS1256_REG_IO,         // E0H
	ADS1256_REG_OFC0,       // xxH
	ADS1256_REG_OFC1,       // xxH
	ADS1256_REG_OFC2,       // xxH
	ADS1256_REG_FSC0,       // xxH
	ADS1256_REG_FSC1,       // xxH
	ADS1256_REG_FSC2        // xxH
} ads1256_reg_addr_t;

/* ADS1256 Commands */
typedef enum ads1256_commands_t
{
	ADS1256_CMD_WAKEUP  = 0x00,	// Completes SYNC and Exits Standby Mode
	ADS1256_CMD_RDATA   = 0x01, // Read Data
	ADS1256_CMD_RDATAC  = 0x03, // Read Data Continuously
	ADS1256_CMD_SDATAC  = 0x0F, // Stop Read Data Continuously
	ADS1256_CMD_RREG    = 0x10, // Read from REG
	ADS1256_CMD_WREG    = 0x50, // Write to REG
	ADS1256_CMD_SELFCAL = 0xF0, // Offset and Gain Self-Calibration
	ADS1256_CMD_SELFOCAL= 0xF1, // Offset Self-Calibration
	ADS1256_CMD_SELFGCAL= 0xF2, // Gain Self-Calibration
	ADS1256_CMD_SYSOCAL = 0xF3, // System Offset Calibration
	ADS1256_CMD_SYSGCAL = 0xF4, // System Gain Calibration
	ADS1256_CMD_SYNC    = 0xFC, // Synchronize the A/D Conversion
	ADS1256_CMD_STANDBY = 0xFD, // Begin Standby Mode
	ADS1256_CMD_RESET   = 0xFE  // Reset to Power-Up Values
} ads1256_commands_t;

/* Status */
#define MSB_FIRST 0x00
#define LSB_FIRST 0x08
#define ACAL_EN   0x04
#define ACAL_DIS  0x00
#define BUF_EN    0x02
#define BUF_DIS   0x00

/* Mux */
#define POS_AIN0  0x00
#define POS_AIN1  0x10
#define POS_AIN2  0x20
#define POS_AIN3  0x30
#define POS_AIN4  0x40
#define POS_AIN5  0x50
#define POS_AIN6  0x60
#define POS_AIN7  0x70
#define POS_AINC  0xF0
#define NEG_AIN0  0x00
#define NEG_AIN1  0x01
#define NEG_AIN2  0x02
#define NEG_AIN3  0x03
#define NEG_AIN4  0x04
#define NEG_AIN5  0x05
#define NEG_AIN6  0x06
#define NEG_AIN7  0x07
#define NEG_AINC  0x0F

/* ADCon */
#define CLKOUT_OFF      0x00
#define CLKOUT_ON_DIV1  0x20
#define CLKOUT_ON_DIV2  0x40
#define CLKOUT_ON_DIV4  0x60
#define PGA_GAIN_1      0x00
#define PGA_GAIN_2      0x01
#define PGA_GAIN_4      0x02
#define PGA_GAIN_8      0x03
#define PGA_GAIN_16     0x04
#define PGA_GAIN_32     0x05
#define PGA_GAIN_64     0x06

/* AD Rate */
#define SMPS_30000      0xF0
#define SMPS_15000      0xE0
#define SMPS_7500       0xD0
#define SMPS_3750       0xC0
#define SMPS_2000       0xB0
#define SMPS_1000       0xA1
#define SMPS_500        0x92
#define SMPS_100        0x82
#define SMPS_60         0x72
#define SMPS_50         0x63
#define SMPS_30         0x53
#define SMPS_25         0x43
#define SMPS_15         0x33
#define SMPS_10         0x23
#define SMPS_5          0x13
#define SMPS_2          0x03

/* AD GPIO */
#define D3_OUTPUT       0x00
#define D2_OUTPUT       0x00
#define D1_OUTPUT       0x00
#define D0_OUTPUT       0x00
#define D3_INPUT        0x80
#define D2_INPUT        0x40
#define D1_INPUT        0x20
#define D0_INPUT        0x10
#define D3_HIGH         0x08
#define D2_HIGH         0x04
#define D1_HIGH         0x02
#define D0_HIGH         0x01

/***********************************************************************
 * PROTOTYPES
 **/
int32_t ads1256_read_channel(uint8_t ch);
void ads1256_config(void);
void ads1256_send_cmd(uint8_t _cmd);
void ads1256_set_channel(uint8_t _ch);
uint8_t ads1256_read_register(uint8_t _reg);
void ads1256_write_register(uint8_t _reg, uint8_t _value);
int ads1256_wait_drdy(void);
int ads1256_read_chip_id(void);

#endif

