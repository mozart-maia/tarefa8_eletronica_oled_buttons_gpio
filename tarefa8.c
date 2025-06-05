#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "inc/ssd1306.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#define I2C_SDA_PIN 14 
#define I2C_SCL_PIN 15
#define OLED_WIDTH 128 
#define OLED_HEIGHT 32

#define BUTTON_B 6


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
}

int main()
{
    stdio_init_all();
    oled_setup();
    gpio_setup();
    
    int button_b_state = 0;
    // button_b_state = gpio_get(BUTTON_B);
    gpio_put(BUTTON_B, 1);

    char *text[] = {
        "teste",
        };
    // printf("texto:  \n");
    show_message_oled(text,1);
    
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
