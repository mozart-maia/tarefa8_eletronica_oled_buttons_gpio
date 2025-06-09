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

#define I2C_SDA_PIN 14 
#define I2C_SCL_PIN 15
#define OLED_WIDTH 128 
#define OLED_HEIGHT 32

#define BUTTON_B 6

#define LOAD_PIN 28
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
  gpio_pull_down(BUTTON_B);

  /* De acordo com o datasheet do SN74HC166 :
        The SNx4HC166 is a parallel-load 8-bit shift register with asynchronous clear ( CLR). This parallel-in or serial-in,
    serial-out shift register features gated clock (CLK, CLK INH) inputs and an overriding clear (CLR) input. The
    parallel-in or serial-in modes are established by the mode select (SH/LD) input. When high, SH/LD enables the
    serial (SER) data input and couples the eight flip-flops for serial shifting with each clock (CLK) pulse. When low,
    the parallel (A through H) data inputs are enabled, and synchronous loading occurs on the next clock pulse.
    During parallel loading, serial data flow is inhibited. Clocking is accomplished on the rising edge of CLK or CLK
    INH, permitting one input to be used as a clock-enable or clock-inhibit function. Holding either CLK or CLK INH
    high inhibits clocking; holding either low enables the other clock input. CLK INH should be changed to the high 
  */ 
  // pino para alternar entre função de shift output (1) e load input (0)
  gpio_init(LOAD_PIN);
  gpio_set_dir(LOAD_PIN, GPIO_OUT);
  gpio_pull_down(LOAD_PIN);

  // pino do clock para alternar no ci SN74HC166
  gpio_init(CLK_PIN);
  gpio_set_dir(CLK_PIN, GPIO_OUT);
  gpio_pull_down(CLK_PIN);

  // pino que vai receber o sinal serial a cada pulso de clock no SN74HC166
  gpio_init(SERIAL_IN_PIN);
  gpio_set_dir(SERIAL_IN_PIN, GPIO_IN);
  gpio_pull_down(SERIAL_IN_PIN);
}

int main()
{
    stdio_init_all();
    oled_setup();
    gpio_setup();

    int a,b,c,d,e,f,g,h;
    
    int button_b_state = 0;
    
    // gpio_put(BUTTON_B, 1);

    char *text[] = {
        "VAL",
        "10"
        };
    // printf("texto:  \n");
    show_message_oled(text,1);
    
    while (true) {
        button_b_state = gpio_get(BUTTON_B);
        if (button_b_state) {
            double decimal_number = 0;
            printf("botao pressionado!\n");
            gpio_put(LOAD_PIN, true);
            for (int i=0; i < 9; i++){
                gpio_put(CLK_PIN, 1);
                a = gpio_get(SERIAL_IN_PIN);
                decimal_number += a * (pow(2, (double)i));
                gpio_put(CLK_PIN, 0);
            }
            gpio_put(LOAD_PIN, true);
            printf("numero lido: %d" , decimal_number);            
        }       
        sleep_ms(100);
    }
}
