#ifndef _ADS1256_H
#define _ADS1256_H
/***********************************************************************
 * DEFINES
 **/
/* ADS1256 Registers Address (followed by reset the default values) */
#define ADS1256_REG_STATUS      0x00  // x1H
#define ADS1256_REG_MUX         0x01  // 01H
#define ADS1256_REG_ADCON       0x02  // 20H
#define ADS1256_REG_DRATE       0x03  // F0H
#define ADS1256_REG_IO          0x04  // E0H
#define ADS1256_REG_OFC0        0x05  // xxH
#define ADS1256_REG_OFC1        0x06  // xxH
#define ADS1256_REG_OFC2        0x07  // xxH
#define ADS1256_REG_FSC0        0x08  // xxH
#define ADS1256_REG_FSC1        0x09  // xxH
#define ADS1256_REG_FSC2        0x0A  // xxH

/* ADS1256 Commands */
#define ADS1256_CMD_WAKEUP      0x00  // Completes SYNC and Exits Standby Mode
#define ADS1256_CMD_RDATA       0x01  // Read Data
#define ADS1256_CMD_RDATAC      0x03  // Read Data Continuously
#define ADS1256_CMD_SDATAC      0x0F  // Stop Read Data Continuously
#define ADS1256_CMD_RREG        0x10  // Read from REG
#define ADS1256_CMD_WREG        0x50  // Write to REG
#define ADS1256_CMD_SELFCAL     0xF0  // Offset and Gain Self-Calibration
#define ADS1256_CMD_SELFOCAL    0xF1  // Offset Self-Calibration
#define ADS1256_CMD_SELFGCAL    0xF2  // Gain Self-Calibration
#define ADS1256_CMD_SYSOCAL     0xF3  // System Offset Calibration
#define ADS1256_CMD_SYSGCAL     0xF4  // System Gain Calibration
#define ADS1256_CMD_SYNC        0xFC  // Synchronize the A/D Conversion
#define ADS1256_CMD_STANDBY     0xFD  // Begin Standby Mode
#define ADS1256_CMD_RESET       0xFE  // Reset to Power-Up Values

/* ADS1256 Status Register */
#define ADS1256_MSB_FIRST       0x00
#define ADS1256_LSB_FIRST       0x08
#define ADS1256_ACAL_EN         0x04
#define ADS1256_ACAL_DIS        0x00
#define ADS1256_BUF_EN          0x02
#define ADS1256_BUF_DIS         0x00

/* ADS1256 Mux Register */
#define ADS1256_POS_AIN0        0x00
#define ADS1256_POS_AIN1        0x10
#define ADS1256_POS_AIN2        0x20
#define ADS1256_POS_AIN3        0x30
#define ADS1256_POS_AIN4        0x40
#define ADS1256_POS_AIN5        0x50
#define ADS1256_POS_AIN6        0x60
#define ADS1256_POS_AIN7        0x70
#define ADS1256_POS_AINC        0xF0
#define ADS1256_NEG_AIN0        0x00
#define ADS1256_NEG_AIN1        0x01
#define ADS1256_NEG_AIN2        0x02
#define ADS1256_NEG_AIN3        0x03
#define ADS1256_NEG_AIN4        0x04
#define ADS1256_NEG_AIN5        0x05
#define ADS1256_NEG_AIN6        0x06
#define ADS1256_NEG_AIN7        0x07
#define ADS1256_NEG_AINC        0x0F

/* ADS1256 ADCon Register */
#define ADS1256_CLKOUT_OFF      0x00
#define ADS1256_CLKOUT_ON_DIV1  0x20
#define ADS1256_CLKOUT_ON_DIV2  0x40
#define ADS1256_CLKOUT_ON_DIV4  0x60
#define ADS1256_PGA_GAIN_1      0x00
#define ADS1256_PGA_GAIN_2      0x01
#define ADS1256_PGA_GAIN_4      0x02
#define ADS1256_PGA_GAIN_8      0x03
#define ADS1256_PGA_GAIN_16     0x04
#define ADS1256_PGA_GAIN_32     0x05
#define ADS1256_PGA_GAIN_64     0x06

/* ADS1256 AD Rate Register */
#define ADS1256_SMPS_30000      0xF0
#define ADS1256_SMPS_15000      0xE0
#define ADS1256_SMPS_7500       0xD0
#define ADS1256_SMPS_3750       0xC0
#define ADS1256_SMPS_2000       0xB0
#define ADS1256_SMPS_1000       0xA1
#define ADS1256_SMPS_500        0x92
#define ADS1256_SMPS_100        0x82
#define ADS1256_SMPS_60         0x72
#define ADS1256_SMPS_50         0x63
#define ADS1256_SMPS_30         0x53
#define ADS1256_SMPS_25         0x43
#define ADS1256_SMPS_15         0x33
#define ADS1256_SMPS_10         0x23
#define ADS1256_SMPS_5          0x13
#define ADS1256_SMPS_2          0x03

/* ADS1256 AD GPIO Register */
#define ADS1256_D3_OUTPUT       0x00
#define ADS1256_D2_OUTPUT       0x00
#define ADS1256_D1_OUTPUT       0x00
#define ADS1256_D0_OUTPUT       0x00
#define ADS1256_D3_INPUT        0x80
#define ADS1256_D2_INPUT        0x40
#define ADS1256_D1_INPUT        0x20
#define ADS1256_D0_INPUT        0x10
#define ADS1256_D3_HIGH         0x08
#define ADS1256_D2_HIGH         0x04
#define ADS1256_D1_HIGH         0x02
#define ADS1256_D0_HIGH         0x01

/***********************************************************************
 * PROTOTYPES
 **/
int32_t ads1256_read_channel(uint8_t ch);
void ads1256_config(void);
void ads1256_send_cmd(uint8_t cmd);
void ads1256_set_channel(uint8_t ch);
uint8_t ads1256_read_register(uint8_t reg);
void ads1256_write_register(uint8_t reg, uint8_t val);
int ads1256_read_chip_id(void);

#endif
