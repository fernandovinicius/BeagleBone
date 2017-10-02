/***********************************************************************
 * INCLUDES
 **/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

/***********************************************************************
 * DEFINES
 **/
#define PRU_NUM     0

#define MMAP1_ADDR_FILE_DIR   "/sys/class/uio/uio0/maps/map1/addr"
#define MMAP1_SIZE_FILE_DIR   "/sys/class/uio/uio0/maps/map1/size"

#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)
#define MMAP_LOC   "/sys/class/uio/uio0/maps/map1/"

/***********************************************************************
 * PROTOTYPES
 **/
int parse_rcv_data_to_file(char *file_name, uint32_t shr_mem_addr, uint32_t num_samples);
int get_pru_shared_mem_info(uint32_t *p_addr, uint32_t *p_size);

/***********************************************************************
 * MAIN
 **/
int main (int argc, char *argv[])
{
  uint32_t shr_mem_addr = 0;
  uint32_t shr_mem_size = 0;
  uint32_t num_samples = 0;
  
  /* Test user */
  if ( getuid() != 0 )
  {
    printf("You must run this program as root. Exiting.\n");
    exit(EXIT_FAILURE);
  }
  
  /* Get shared memory info */
  if ( get_pru_shared_mem_info(&shr_mem_addr, &shr_mem_size) < 0 )
  {
    return -1;
  }
  printf("The DDR External Memory Pool\n");
  printf("Address: 0x%x\n", shr_mem_addr);
  printf("Size:    %u bytes (0x%x)\n\n", shr_mem_size);
  
  /* Parse sample number */
  if ( argc < 2 )
  {
    printf("User didn't entered the number of samples to collect.\nCollecting 10 samples (default)\n");
    num_samples = 10;
  }
  else
  {
    num_samples = atoi(argv[1]);
    if ( num_samples * sizeof(uint32_t) >= shr_mem_size )
    {
      num_samples = shr_mem_size / sizeof(uint32_t);
      printf("Number of samples too large.\nCollecting %u samples (max)\n", num_samples);
    }
  }

  /* Initialize structure used by prussdrv_pruintc_intc */
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

  /* Data to pass to PRU0 */
  uint32_t pru0_data[3] = {0, 0, 0};
  pru0_data[0] = num_samples;
  pru0_data[1] = shr_mem_addr;
  pru0_data[2] = shr_mem_size;

  /* Allocate and initialize memory */
  prussdrv_init();
  prussdrv_open(PRU_EVTOUT_0);

  /* Write the samples number, address and size into PRU0 Data RAM0 */
  prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, pru0_data, 3 * sizeof(uint32_t));

  /* Map PRU's interrupts */
  prussdrv_pruintc_init(&pruss_intc_initdata);

  /* Load and execute the PRU program on the PRU */
  prussdrv_exec_program (PRU_NUM, "./pru_ads1256.bin");

  /* Wait for event completion from PRU, returns the PRU_EVTOUT_0 number */
  int n = prussdrv_pru_wait_event(PRU_EVTOUT_0);
  printf("EBB PRU program completed, event number %d.\n", n);

  /* Save received data into a file */
  parse_rcv_data_to_file("data_out", shr_mem_addr, num_samples);
  
  /* Disable PRU and close memory mappings */
  prussdrv_pru_disable(PRU_NUM);
  prussdrv_exit();

  return 0;
}

/***********************************************************************
 * FUNCTIONS
 **/
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
int parse_rcv_data_to_file(char *file_name, uint32_t shr_mem_addr, uint32_t num_samples)
{
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
      sample = *((uint32_t *)p_addr_idx);
      fprintf(fp, "%u\t%u\n", i, sample);
      offset += sizeof(uint32_t);
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
  unsigned int val = 0;
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
