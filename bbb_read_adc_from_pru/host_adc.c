/***********************************************************************
 * INCLUDES
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

/***********************************************************************
 * DEFINES
 **/
#define DEF_SMP_RATE   1000
#define SAMPLE_SIZE    2
#define ADC_FIFO0_LEN  50
#define PRU_NUM        0

/***********************************************************************
 * LOCAL FUNCTIONS PROTOTYPES
 **/
/* Signals */
void signal_handler(int signal);
int  install_signal(void *signal_handler);

/* Misc */
int check_sample_rate(uint32_t smps);

/* PRU */
int get_pru_shared_mem_info(uint32_t *p_addr, uint32_t *p_size);

/* Output data file */
int parse_rcv_data_to_file(char *file_name, uint32_t shr_mem_addr, uint32_t num_samples, uint32_t sample_size);

/***********************************************************************
 * MAIN
 **/
int main(int argc, char *argv[])
{
  /* Test user */
  if ( getuid() != 0 )
  {
    printf("You must run this program as root. Exiting.\n");
    exit(EXIT_FAILURE);
  }

  /* Test input parameters */
  if ( argc != 4 )
  {
    printf("Wrong parameters.\n");
    printf("Usage: %s <CHANNEL> <SAMPLE_RATE_HZ> <DURATION_SEC>\n\n", argv[0]);
    printf("\tChannels: 0-6 (Just one channel allowed!)\n\n");
    printf("\tSample rates (Hz): 1600000,  800000, 400000,\n");
    printf("\t                    200000,  100000,  50000,\n");
    printf("\t                     20000,   10000,   5000,\n"); 
    printf("\t                      2000,    1000,    500,\n");
    printf("\t                       200,     100\n\n");
    exit(EXIT_FAILURE);
  }

  /* Get shared memory info */
  uint32_t shr_mem_addr = 0;
  uint32_t shr_mem_size = 0;
  if ( get_pru_shared_mem_info(&shr_mem_addr, &shr_mem_size) < 0 )
  {
    return -1;
  }

  /* Parse channel */
  uint32_t channel = atoi(argv[1]);
  if ( channel > 6 )
  {
    printf("Channel doesn't exist. Sampling CH0 (Default)\n");
    channel = 0;
  }
  uint32_t ch_cfg_code = (channel << 19) | (channel << 15) | 0x00000001;
  
  /* Parse sample rate */
  uint32_t sample_rate = atoi(argv[2]);
  if ( check_sample_rate(sample_rate) < 0 )
  {
    printf("Sample rate not supported. Sampling at %d Hz (Default)\n\n", DEF_SMP_RATE);
    sample_rate = DEF_SMP_RATE;
  }
  uint32_t clk_div = (1600000/sample_rate) - 1;
    
  /* Parse acquiring duration */
  float acquisition_time = atof(argv[3]);
  float max_time = (float)(shr_mem_size / SAMPLE_SIZE) / sample_rate;
  if ( acquisition_time > max_time )
  {
    printf("Shared memory not enough for specified acquiring duration.");
    printf("Acquiring for %.2f seconds.\n\n", max_time);
    acquisition_time = max_time;
  }

  /* Number of samples */
  uint32_t num_samples = acquisition_time * sample_rate;
  uint32_t num_loops   = num_samples / ADC_FIFO0_LEN;

  /* Install signal */
  install_signal(&signal_handler);

  /* Initialize struct used by prussdrv_pruintc_intc */
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

  /* Allocate and initialize memory */
  prussdrv_init();
  prussdrv_open(PRU_EVTOUT_0);

  /* Data to pass to PRU0 */
  uint32_t pru0_data[10];
  pru0_data[0] = shr_mem_addr;
  pru0_data[1] = clk_div;
  pru0_data[2] = num_loops;
  pru0_data[3] = ch_cfg_code;
  pru0_data[4] = ADC_FIFO0_LEN;
  prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, pru0_data, 5 * sizeof(uint32_t));

  /* Map PRU's interrupts */
  prussdrv_pruintc_init(&pruss_intc_initdata);

  /* Print settings */
  printf("Sampling settings:\n");
  printf("\tSample rate:   %d Hz\n", sample_rate);
  printf("\tTime:          %f seg\n", acquisition_time);
  printf("\tSample size:   %d bytes\n", SAMPLE_SIZE);
  printf("\tTotal samples: %d\n", num_samples);
  printf("Collecting...\n");
  
  /* Load and execute the PRU program on the PRU */
  prussdrv_exec_program (PRU_NUM, "./pru_adc.bin");

  /* Wait for event completion from PRU, returns the PRU_EVTOUT_0 number */
  prussdrv_pru_wait_event(PRU_EVTOUT_0);
  printf("Done!\n");

  /* Save received data into a file */
  printf("Saving file...\n");
  parse_rcv_data_to_file("data_samples.txt", shr_mem_addr, num_samples, SAMPLE_SIZE);
  printf("ok!\n\n");

  /* Disable PRU and close memory mappings */
  prussdrv_pru_disable(PRU_NUM);
  prussdrv_exit();

  return 0;
}

/***********************************************************************
 * LOCAL FUNCTIONS
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
    exit(-1);
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
 * @fn      check_sample_rate
 *
 * @brief
 *
 * @param   smps
 *
 * @return
 **/
int check_sample_rate(uint32_t smps)
{
  if ( smps == 100    || smps == 200    || smps == 500    || 
       smps == 1000   || smps == 2000   || smps == 5000   ||
       smps == 10000  || smps == 20000  || smps == 50000  ||
       smps == 100000 || smps == 200000 || smps == 400000 ||
       smps == 800000 || smps == 1600000
     )
  {
    return 0;
  }
  
  return -1;
}

/***********************************************************************
 * @fn      get_pru_shared_mem_info
 *
 * @brief   Read map1 files and loads size and address of shared mem.
 *
 * @param   p_addr
 *          p_size
 *
 * @return
 **/
int get_pru_shared_mem_info(uint32_t *p_addr, uint32_t *p_size)
{
  const char MMAP1_ADDR_FILE_DIR[] = "/sys/class/uio/uio0/maps/map1/addr";
  const char MMAP1_SIZE_FILE_DIR[] = "/sys/class/uio/uio0/maps/map1/size";
  uint32_t val = 0;
  FILE *fp = NULL;

  /* Read addr file */
  fp = fopen(MMAP1_ADDR_FILE_DIR, "rt");
  if ( fscanf(fp, "%x", &val) < 0 )
  {
    perror("fscanf(\"/sys/class/uio/uio0/maps/map1/addr)\"");
    fclose(fp);
    return -1;
  }
  fclose(fp);
  *p_addr = val;

  /* Read size file */
  fp = fopen(MMAP1_SIZE_FILE_DIR, "rt");
  if ( fscanf(fp, "%x", &val) < 0 )
  {
    perror("fscanf(/sys/class/uio/uio0/maps/map1/size)");
    fclose(fp);
    return -1;
  }
  fclose(fp);
  *p_size = val;

  return 0;
}

/***********************************************************************
 * @fn      parse_rcv_data_to_file
 *
 * @brief
 *
 * @param   file_name
 *          shr_mem_addr
 *          num_samples
 *
 * @return
 **/
int parse_rcv_data_to_file(char *file_name, uint32_t shr_mem_addr, uint32_t num_samples, uint32_t sample_size)
{
  const uint32_t MAP_SIZE = 0x0FFFFFFF;
  const uint32_t MAP_MASK = (MAP_SIZE - 1);

  uint32_t sample = 0;
  off_t offset = shr_mem_addr;
  void *p_addr_idx = NULL;
  void *p_map_addr = NULL;
  int fd = 0;

  /* Open mem device */
  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if ( fd < 0 )
  {
    perror("open(\"/dev/mem\")");
    return -1;
  }

  p_map_addr = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset & ~MAP_MASK);
  if( p_map_addr == (void *) - 1 )
  {
    perror("mmap()");
    close(fd);
    return -1;
  }

  /* Open data file */
  FILE *fp = NULL;
  fp = fopen(file_name, "wb");
  if ( fp != NULL )
  {
    /* Store data into file */
    int i = 0;
    for ( i = 0; i < num_samples; i++ )
    {
      p_addr_idx = p_map_addr + (offset & MAP_MASK);
      if ( sample_size == 1 )
      {
        sample = *((uint8_t *)p_addr_idx);
      }
      else if ( sample_size == 2 )
      {
        sample = *((uint16_t *)p_addr_idx);
      }
      else
      {
        sample = *((uint32_t *)p_addr_idx);
        sample_size = sizeof(uint32_t);
      }
      fprintf(fp, "%u\t%u\n", i, sample);
      offset += sample_size;
    }

    /* Close file */
    fclose(fp);
  }
  else
  {
    perror("fopen(data_file)");
  }

  /* Unmap memory */
  if ( munmap(p_map_addr, MAP_SIZE) == -1 )
  {
    perror("munmap()");
    close(fd);
    return -1;
  }

  /* Close descriptor */
  close(fd);

  return 0;
}
