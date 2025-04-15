#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h" // not included
#include "hardware/spi.h"


int main()
{
    stdio_init_all();
   
    // Initializing the SPI
    spi_init(spi_default, 1000 * 1000); // the baud, or bits per second
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

}
