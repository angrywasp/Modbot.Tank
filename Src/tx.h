#include "simpletools.h"

#include "./lib/modules/MCU.h"
#include "./lib/modules/PSU.h"

#include "./lib/drivers/SSD1306.h"
#include "./lib/drivers/NRF24L01.h"
#include "./lib/IO.h"
#include "./lib/refs.h"

static byte *rf_addr = (byte *)"pbot";
static byte channel = 113;

#define OLED_SCL 9
#define OLED_SDA 8

#define BATTERY_PORT ad7812_A8

#define NRF_MOSI 27
#define NRF_MISO 25
#define NRF_CLK 24
#define NRF_CS 26
#define NRF_CE 23
#define NRF_PAYLOAD_SIZE 6

ssd1306_context_t *oled;
nrf_context_t *nrf;
psu_context_t * psu;

char print_out[32] = {0};

static void _display_text(ssd1306_context_t* oled, ushort y, char* str)
{
    int len = ssd1306_string_width(str, strlen(str));
    ssd1306_txt(oled, (oled->width - len) / 2, y, str); 
}

static nrf_context_t *_init_transmitter()
{
    nrf_context_t *tx = nrf_init(NRF_MOSI, NRF_MISO, NRF_CLK, NRF_CS, NRF_CE);

    char output[21] = {0};

    byte a = nrf_set_primary_mode(tx, 0);
    byte b = nrf_enable_rx_pipes(tx, 0x00001);
    byte c = nrf_set_shockburst(tx, 1);
    byte d = nrf_set_payload_width(tx, 0, NRF_PAYLOAD_SIZE);
    byte e = nrf_set_transmitter_power(tx, 0);
    byte f = nrf_set_data_rate(tx, 0);
    byte g = nrf_set_rf_channel(tx, channel);
    byte h = nrf_write_rx_pipe_address(tx, 0, rf_addr);
    byte i = nrf_write_tx_pipe_address(tx, rf_addr);

    printf("TX Initialized: %02x%02x%02x%02x%02x%02x%02x%02x%02x\n", a, b, c, d, e, f, g, h, i);

    return tx;
}

void cog_io();
