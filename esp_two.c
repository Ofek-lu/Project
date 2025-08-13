

/* NOTE : CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM needs to be enabled
for gptimer_set_raw_count and gptimer_set_alarm_action to be called
safely from an ISR  */

#include "driver/gpio.h" /* GPIO_NUM_X, GPIO_MODE_OUTPUT */
#include "esp_log.h"
#include "driver/gptimer.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "esp_attr.h"

/* Define the GPIO number for the LED*/
#define LED_GPIO GPIO_NUM_2

/* Parameters for UART communication */
#define UART_PORT_NUM      UART_NUM_1   
#define TX_PIN             17           // GPIO for TX
#define RX_PIN             18           // GPIO for RX 
#define BUF_SIZE           1024


/* LED control */
uint8_t g_led_state = 1;
DRAM_ATTR const uint32_t g_on_time = 220000;        /* 220 ms  */
volatile uint32_t g_off_time = 1780000;             /* 1780 ms */

static const char *TAG = "Logging:";

static void ConfigUART(void);
static void ConfigLED(void);
/* define interrupt handler to toggle the LED */
static bool IRAM_ATTR TimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);
static void ConfigTimer(void);
static void ModifyOffTime(void);

void app_main(void)
{
    ConfigLED();
    ConfigUART();
    /* Flush the buffer */
    uart_flush(UART_PORT_NUM);
    ConfigTimer();

	while (true)
	{
        ESP_LOGI(TAG, "Turning the LED %s!", g_led_state == true ? "ON" : "OFF");
        ModifyOffTime();
	}
	
}

static void ModifyOffTime(void)
{
    float  temp;

    ESP_ERROR_CHECK(!(uart_read_bytes(UART_PORT_NUM, &temp, sizeof(float), portMAX_DELAY)
                                                                         == sizeof(float)));
    
    if (temp < 10.0f)
    {
        g_off_time = 1780000;
    }

    else if (temp > 30.0f)
    {
        g_off_time = 40000;
    }

    else
    {
        g_off_time = 2620000 - temp * 84000;
    }

    ESP_LOGI(TAG, "temp = %f g_off_time = %u ", temp ,g_off_time);
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

static void ConfigLED(void)
{
    ESP_ERROR_CHECK(gpio_reset_pin(LED_GPIO));
    /* Set the GPIO as a push/pull output */
    ESP_ERROR_CHECK(gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT));
}



static void ConfigTimer(void)
{
    gptimer_handle_t ret_timer;

    static gptimer_event_callbacks_t callback_ISR = {
        .on_alarm = TimerISR,
        
    };

    /* start with the LED on */
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = g_on_time,
        
    };
    
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        /* each count is 1 microsecond */
        .resolution_hz = 1000000,
        
    };

    /* create a new timer */
    ESP_ERROR_CHECK(gptimer_new_timer(&config, &ret_timer));

    /* set the intial value of the counter */
    ESP_ERROR_CHECK(gptimer_set_raw_count(ret_timer, 0));
    
    /* enable interrupt for this timer and determine the ISR */
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(ret_timer,&callback_ISR  , NULL));

    /* intial state the LED is on */
    ESP_ERROR_CHECK(gptimer_set_alarm_action(ret_timer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_enable(ret_timer));
    ESP_ERROR_CHECK(gptimer_start(ret_timer));
}


static bool TimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    /* the time until next interrupt */
    uint64_t next_interrupt;
    static gptimer_alarm_config_t alarm_config =
    {
        .flags.auto_reload_on_alarm = false,
    };

    /* Toggle the LED */
    g_led_state ^= 1;
    gpio_set_level(LED_GPIO, g_led_state);

    /* Setting the counter to the next interrupt and then initialize the counter to zero */
    next_interrupt = g_led_state ? g_on_time : g_off_time;
    alarm_config.alarm_count = next_interrupt;
    gptimer_set_raw_count(timer, 0);
    gptimer_set_alarm_action(timer, &alarm_config);

    /* return false since there is no switch to other task after the interrupt*/
    return false;

}