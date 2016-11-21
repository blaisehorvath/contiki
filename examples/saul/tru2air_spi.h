#include "contiki.h"
#include "ti-lib.h"
#include <stdio.h>

/*----- Defining SPI stuff -----*/
#define SPI_SCK_PIN IOID_12
#define MOSI_PIN IOID_11
#define MISO_PIN IOID_9
#define CS_PIN IOID_8

/**
 * @brief This function initializes the SPI on the cc1310
 *
 * @param spi_int_cb A callback function that will be called when data arrives to the
 *  SPI FIFO. T
 *  @section Usage
 *  The callback function must use the getByteFromSPI() function from this header to stop the
 *  interrupt to trigger endlessly. Optionally the callback can call sendByteviaSPI
 *  function from this header. If sendByteviaSPI is not called the default response to
 *  the spi master is 0.
 */
void initSPISlave(void (spi_int_cb)());

/**
 * @brief This function gets a byte from the SPI FIFO
 *
 * @return The received byte.
 */
uint8_t getByteFromSPI();

/**
 * @brief This function sends a byte in response of a master request.
 *
 * @param answer The byte to send.
 *
 * @section Usage
 */
void sendByteviaSPI(uint8_t answer);
