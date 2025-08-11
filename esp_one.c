#include <math.h>

#include "driver/spi_master.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/uart.h"

#define TAG "MAX6675K"

// Wiring to thermocouple sensor
#define PIN_NUM_MISO 37
#define PIN_NUM_MOSI -1  // Not used by MAX6675K
#define PIN_NUM_CLK  36 
#define PIN_NUM_CS   34 

/* Parameters for UART communication */
#define UART_PORT_NUM      UART_NUM_1  
#define TX_PIN             17           // GPIO for TX
#define RX_PIN             18           // GPIO for RX 
#define BUF_SIZE           1024

static spi_device_handle_t g_spi_handler;

static void ConfigUART(void);
static void ConfigTempSensor(void);
static float GetTemp(void);


void app_main(void)
{
    float temp;
    ConfigTempSensor();
    ConfigUART();

    while (true) 
    {
        temp= GetTemp(); 

        if (!isnan(temp)) 
        {
            ESP_LOGI(TAG, "Temperature: %.2f °C", temp);

            
            /* sending message via UART*/
            uart_write_bytes(UART_PORT_NUM, &temp, 4);
        }

        else
        {
            ESP_LOGI(TAG, "Thermocouple not connected");
        }

        /*delay 3 seconds before recording current temperature */
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}


static void ConfigUART(void)
{
    uart_config_t uart_config = {
        .baud_rate = 11520,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, TX_PIN, RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0));                                 

}


static float GetTemp(void) 
{
    
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_RXDATA,
        .length = 16,   // 16 bits
        .rxlength = 16,
    };

    ESP_ERROR_CHECK(spi_device_transmit(g_spi_handler, &t));

    uint16_t data = (t.rx_data[0] << 8) | t.rx_data[1];

    if (data & 0x4) /* Bit 2 fault bit set */
    {  
        return NAN;
    }

    data >>= 3; /* remove lower 3 bits */
    float temp = data * 0.25;
    /*returning temperature in celsius */
    return temp;
}

static void ConfigTempSensor(void) 
{
    spi_bus_config_t bus_config = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI, // not used, set -1
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 2        // 2 bytes
    };

    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 4000000,  // MAX6675 max 4.3 MHz
        .mode = 0,                  // SPI mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = PIN_NUM_CS, // CS pin
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config , &g_spi_handler));
}






