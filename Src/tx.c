#include "./tx.h"

#define JOYSTICK_RANGE 105

volatile io_adc_binding_t *rx;
volatile io_adc_binding_t *ry;
volatile io_adc_binding_t *lx;
volatile io_adc_binding_t *ly;

volatile io_adc_binding_t *batt;

void cog_io()
{
    ad7812_context_t* adc = adc_init();
    batt = io_add_adc_binding(adc, BATTERY_PORT, 0);
    batt->value->lastVal = 1000;
    rx = io_add_adc_binding(adc, ad7812_A4, 0);
    ry = io_add_adc_binding(adc, ad7812_A5, 0);
    lx = io_add_adc_binding(adc, ad7812_A6, 0);
    ly = io_add_adc_binding(adc, ad7812_A7, 0);

    pause(100);
    
    int rx_center = io_joystick_get_center(rx) * -1;
    int ry_center = io_joystick_get_center(ry) * -1;
    int lx_center = io_joystick_get_center(lx) * -1;
    int ly_center = io_joystick_get_center(ly) * -1;

    pause(100);

    while (true)
    {
        pause(10);
        psu_read_battery(psu, batt);
        io_joystick(rx, rx_center, false, -JOYSTICK_RANGE, JOYSTICK_RANGE);
        io_joystick(ry, ry_center, true, -JOYSTICK_RANGE, JOYSTICK_RANGE);
        io_joystick(lx, lx_center, true, -JOYSTICK_RANGE, JOYSTICK_RANGE);
        io_joystick(ly, ly_center, false, -JOYSTICK_RANGE, JOYSTICK_RANGE);
    }
}

int main()
{
    byte *tx_dat = (byte *)malloc(NRF_PAYLOAD_SIZE);
    memset(tx_dat, 0x00, NRF_PAYLOAD_SIZE);

    printf("Starting\n");
    printf("Initializing display\n");

    oled = ssd1306_init(SSD1306_GEOMETRY_128_32, OLED_SCL, OLED_SDA);
    memset(oled->buffer, 0x00, oled->buffer_size);
    printf(ssd1306_detect(oled) ? "OLED found\n" : "OLED not found\n");
    ssd1306_clear(oled);
    ssd1306_txt(oled, 0, 0, "Hello");
    ssd1306_txt(oled, 0, 16, "Starting up...");
    ssd1306_display(oled);

    printf("Initializing transmitter\n");

    nrf = _init_transmitter();

    printf("Initializing PSU\n");
    psu = psu_init(22000, 47000, 3.3f, 2.75f, 4.2f);

    printf("%d %d %f %f %f\n", psu->adc_min, psu->adc_max, psu->battery_min_voltage, psu->battery_max_voltage, psu->vref);

    printf("Stating IO cog\n");
    cog_run(cog_io, 128);

    printf("resting...\n");
    pause(1000);

    printf("Initializing main loop\n");
    while (true)
    {
        if (batt->value->newValueReady && batt->value->changed)
        {
            sprintf(print_out, "%d%%", batt->value->lastVal);
            ssd1306_clear(oled);
            ssd1306_txt(oled, 0, 0, "Battery:");
            ssd1306_txt(oled, 128 - (strlen(print_out) * 7), 0, print_out);
            ssd1306_display(oled);

            batt->value->newValueReady = false;
        }

        if (rx->value->newValueReady && rx->value->changed)
        {
            printf("rx: %d\n", rx->value->lastVal);
            rx->value->newValueReady = false;
        }

        if (ry->value->newValueReady && ry->value->changed)
        {
            printf("ry: %d\n", ry->value->lastVal);
            ry->value->newValueReady = false;
        }

        if (lx->value->newValueReady && lx->value->changed)
        {
            printf("lx: %d\n", lx->value->lastVal);
            lx->value->newValueReady = false;
        }

        if (ly->value->newValueReady && ly->value->changed)
        {
            printf("ly: %d\n", ly->value->lastVal);
            ly->value->newValueReady = false;
        }
    }
}