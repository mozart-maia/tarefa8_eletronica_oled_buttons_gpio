#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "inc/ssd1306.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

// void measure_freqs(void) {
//     uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
//     uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
//     uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
//     uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
//     uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
//     uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
//     uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
// #ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
//     uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
// #endif

//     printf("pll_sys  = %dkHz\n", f_pll_sys);
//     printf("pll_usb  = %dkHz\n", f_pll_usb);
//     printf("rosc     = %dkHz\n", f_rosc);
//     printf("clk_sys  = %dkHz\n", f_clk_sys);
//     printf("clk_peri = %dkHz\n", f_clk_peri);
//     printf("clk_usb  = %dkHz\n", f_clk_usb);
//     printf("clk_adc  = %dkHz\n", f_clk_adc);
// #ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
//     printf("clk_rtc  = %dkHz\n", f_clk_rtc);
// #endif

//     // Can't measure clk_ref / xosc as it is the ref
// }

#define I2C_SDA_PIN 14 
#define I2C_SCL_PIN 15
#define OLED_WIDTH 128 
#define OLED_HEIGHT 32

#define BUTTON_B 6

#define LOAD_PIN 8
#define CLK_PIN 17
#define SERIAL_IN_PIN 16


void show_message_oled(char* message[], int lines) {
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);
    // limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    int y = 0;

    for (uint i = 0; i < lines; i++) {
        printf("imprimindo na tela: %s\n", message[i]);
        ssd1306_draw_string(ssd,5,y,message[i]);
        // movendo para a proxima "linha" no display
        y += 8;
    }
    render_on_display(ssd, &frame_area);
}

void display_bitmap(int mode){
    ssd1306_t ssd_bm;
    ssd1306_init_bm(&ssd_bm, 128, 64, false, 0x3C, i2c1);
    ssd1306_config(&ssd_bm);
  
    switch (mode) {
      case 1:
        // ssd1306_draw_bitmap(&ssd_bm, usuario1_select);
        break;      
    }  
}

void oled_setup() { 
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    ssd1306_init();
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);     
}

void gpio_setup() {
  gpio_init(BUTTON_B);
  gpio_set_dir(BUTTON_B, GPIO_IN);
  gpio_pull_up(BUTTON_B);

  /* De acordo com o datasheet do SN74HC166 :
        The SNx4HC166 is a parallel-load 8-bit shift register with asynchronous clear ( CLR). This parallel-in or serial-in,
    serial-out shift register features gated clock (CLK, CLK INH) inputs and an overriding clear (CLR) input. The
    parallel-in or serial-in modes are established by the mode select (SH/LD) input. When high, SH/LD enables the
    serial (SER) data input and couples the eight flip-flops for serial shifting with each clock (CLK) pulse. When low,
    the parallel (A through H) data inputs are enabled, and synchronous loading occurs on the next clock pulse.
    During parallel loading, serial data flow is inhibited. Clocking is accomplished on the rising edge of CLK or CLK
    INH, permitting one input to be used as a clock-enable or clock-inhibit function. Holding either CLK or CLK INH
    high inhibits clocking; holding either low enables the other clock input. CLK INH should be changed to the high
    level only when CLK is high.
    CLR overrides all other inputs, including CLK, and resets all flip-flops to zero. 
  */ 
  // pino para alternar entre função de shift output (1) e load input (0)
  gpio_init(LOAD_PIN);
  gpio_set_dir(LOAD_PIN, GPIO_OUT);
//   gpio_pull_down(LOAD_PIN);
  gpio_put(LOAD_PIN, false);

  // pino do clock para alternar no ci SN74HC166
  gpio_init(CLK_PIN);
  gpio_set_dir(CLK_PIN, GPIO_OUT);
  gpio_put(CLK_PIN, false);
//   gpio_pull_down(CLK_PIN);

  // pino que vai receber o sinal serial a cada pulso de clock no SN74HC166
  gpio_init(SERIAL_IN_PIN);
  gpio_set_dir(SERIAL_IN_PIN, GPIO_IN);
  gpio_put(SERIAL_IN_PIN, false);
//   gpio_pull_down(SERIAL_IN_PIN);
}

int main()
{
    stdio_init_all();
    oled_setup();
    gpio_setup();

    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    2 * MHZ,
                    2 * MHZ);


    int n_read;    
    int button_b_state;

    char saida_oled[20];
    sprintf(saida_oled, "%f", 42.00);
    char *text[] = {
        "VAL",
        saida_oled
    };
    show_message_oled(text,2);    
    
    while (true) {
        button_b_state = gpio_get(BUTTON_B);
        if (button_b_state == 0) {
            
            double decimal_number = 0.0;
            printf("botao pressionado!\n");
            gpio_put(LOAD_PIN, false);
            gpio_put(CLK_PIN, true);
            gpio_put(CLK_PIN, false);
            
            // modo de leitura serial ativado
            gpio_put(LOAD_PIN, true);
            // gpio_put(CLK_PIN, true);
            printf("load pin : %d\n", gpio_get(LOAD_PIN));
            for (int i=0; i < 8; i++){
                // clock
                sleep_ms(200);
                gpio_put(CLK_PIN, true);
                // gpio_put(CLK_PIN, true);
                // int clock_gpio = gpio_get(CLK_PIN);
                // valor lido da saida serial
                n_read = gpio_get(SERIAL_IN_PIN);
                decimal_number += n_read * (pow(2, (double)i));
                printf("i - %d | leitura do serial -  %d | clock_gpio - %d |  load_gpio - %d | soma: %f \n", i, n_read, gpio_get(LOAD_PIN), gpio_get(CLK_PIN), decimal_number);
                // printf(" \n", n_read);
                // printf("numero ate agora - %f \n", decimal_number);
                gpio_put(CLK_PIN, false);
            }
            // modo de leitura serial desativado
            gpio_put(LOAD_PIN, false);
            printf("numero lido: %d" , (int)decimal_number);
            char number_str[20];
            sprintf(number_str, "%d", (int)decimal_number);
            char *saida_oled[] = {
                "VAL",
                number_str
            };
            show_message_oled(saida_oled,2);           
                        
        }       
        sleep_ms(100);
    }
}
