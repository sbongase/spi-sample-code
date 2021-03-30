/*
 * spi_example.c:
 *	Code to communicate with MCP3008 ADC
 ***********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define	SPI_CHAN		0

static const uint8_t     spiBPW   = 8 ;
static const uint16_t    spiDelay = 0 ;
static int spiSpeed = 1000000;

static int fd = -1;

/*
 * spiDataRW:
 *	Write and Read a block of data over the SPI bus.
 *	Note the data ia being read into the transmit buffer, so will
 *	overwrite it!
 *	This is also a full-duplex operation.
 *********************************************************************************
 */

int spiDataRW (int fd, unsigned char *data, int len)
{
  struct spi_ioc_transfer spi ;

  memset (&spi, 0, sizeof (spi)) ;

  spi.tx_buf        = (unsigned long)data ;
  spi.rx_buf        = (unsigned long)data ;
  spi.len           = len ;
  spi.delay_usecs   = spiDelay ;
  spi.speed_hz      = spiSpeed;
  spi.bits_per_word = spiBPW ;

  return ioctl (fd, SPI_IOC_MESSAGE(1), &spi) ;
}

/*
 * spiSetupMode:
 *	Open the SPI device, and set it up, with the mode, etc.
 *********************************************************************************
 */

int spiSetupMode (int spi_chan, int slave_no, int mode)
{

  char spiDev [32] ;

  mode    &= 3 ;	// Mode is 0, 1, 2 or 3

  snprintf (spiDev, 31, "/dev/spidev%d.%d", spi_chan, slave_no) ;

  if ((fd = open (spiDev, O_RDWR)) < 0) {
    printf("Unable to open SPI device: %s\n", strerror (errno)) ;
	return -1;
  }

// Set SPI parameters.

  if (ioctl (fd, SPI_IOC_WR_MODE, &mode) < 0) {
    printf("SPI Mode Change failure: %s\n", strerror (errno));
	return -1;
  }
  
  if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0) {
    printf("SPI BPW Change failure: %s\n", strerror (errno)) ;
	return -1;
  }

  if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeed)   < 0) {
    printf("SPI Speed Change failure: %s\n", strerror (errno));
	return -1;
  }

  return fd ;
}



static int mcp3008read (int pin)
{
  unsigned char spiData [3] ;
  unsigned char chanBits ;
  int chan = pin ;

  chanBits = 0b10000000 | (chan << 4) ;

  spiData [0] = 1 ;		// Start bit
  spiData [1] = chanBits ;
  spiData [2] = 0 ;

  spiDataRW (fd, spiData, 3) ;

  return ((spiData [1] << 8) | spiData [2]) & 0x3FF ;
}

/*
 * mcp3008Setup:
 *	setup mcp3008 on the Pi's SPI interface.
 *********************************************************************************
 */

int mcp3008Setup (int spiChannel)
{
  if ((spiSetupMode (spiChannel, 0, 0)) < 0)
    return -1 ;
  else
    printf("fd: %d\n",fd);
  return 0 ;
}

int main()
{
	int mpc_chan = 0;;
	int light_intensity;
	//if you use a mcp3008 write canche it in the setup
	printf("mcp3008Setup %d\n",mcp3008Setup(SPI_CHAN));
/************************************************/
	while (1)
    {
		// with chan can you set what analog channels you want to read

		light_intensity = mcp3008read(mpc_chan);
		printf("%d ", light_intensity);

		printf("\n");
		sleep(1);
    }
	
	close(fd);
	// return 0 breaks the loop
	return 0; 
}

