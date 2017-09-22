/***********************************************************************
 * INCLUDES
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "gpio_interface.h"
#include "conf.h"

/***********************************************************************
 * DEFINES
 **/
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_NAME  64

/***********************************************************************
 * MACROS
 **/

/***********************************************************************
 * GLOBALS
 **/

/***********************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES
 **/

/***********************************************************************
 * FUNCTIONS
 **/
/***********************************************************************
 * @fn      gpio_write
 *
 * @brief
 *
 * @param   gpio - GPIO Number
 *          pin_level - HIGH or LOW
 *
 * @return
 */
int gpio_write(uint32_t gpio_num, uint8_t pin_level)
{
  char gpio_file_name[MAX_NAME] = "";
  int fd = 0;
  int len = 0;

  len = snprintf(gpio_file_name, MAX_NAME, SYSFS_GPIO_DIR "/gpio%d/value", gpio_num);
  if ( len < 0 )
  {
    perror("snprintf()");
    return -1;
  }

  /* Open Pin File */
  fd = open(gpio_file_name, O_WRONLY);
  if ( fd < 0 )
  {
    perror("gpio/set-value");
    return -1;
  }

  /* Write pin level */
  if ( pin_level )
  {
    write(fd, "1", 2);
  }
  else
  {
    write(fd, "0", 2);
  }

  /* Close Pin file */
  close(fd);

  return 0;
}

/***********************************************************************
 * @fn      gpio_read
 *
 * @brief
 *
 * @param   gpio
 *          pin_level
 *
 * @return
 */
int gpio_read(uint32_t gpio_num, uint8_t *pin_level)
{
  char gpio_file_name[MAX_NAME] = "";
  char level = 0;
  int fd = 0;
  int len = 0;

  len = snprintf(gpio_file_name, MAX_NAME, SYSFS_GPIO_DIR "/gpio%d/value", gpio_num);
  if ( len < 0 )
  {
    perror("snprintf()");
    return -1;
  }

  /* Open Pin File */
  fd = open(gpio_file_name, O_RDONLY);
  if (fd < 0)
  {
    perror("gpio/get-value");
    return -1;
  }

  /* Read Pin File */
  read(fd, &level, 1);
  if ( level == '0' )
  {
    *pin_level = LOW;
  }
  else if ( level == '1' )
  {
    *pin_level = HIGH;
  }
  else
  {
    printf("Undefined Pin Level at GPIO%d: %c\n", gpio_num, level);
    return -1;
  }

  /* Close Pin file */
  close(fd);

  return 0;
}

