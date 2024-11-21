#include "ripes_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SW0 (0x01)
#define SW1 (0x02)
#define SW2 (0x04)
#define SW3 (0x08)

#define MATRIX_WIDTH LED_MATRIX_0_WIDTH
#define MATRIX_HEIGHT LED_MATRIX_0_WIDTH 

#define SNAKE_COLOR 0xFF0000    // Rojo
#define APPLE_COLOR 0x39FF14    // Verde
#define BORDER_COLOR 0x40E0D0   // Turquesa
#define GAME_OVER_COLOR 0x800080 // Morado

// Variables globales
volatile unsigned int * led_base = (int*) LED_MATRIX_0_BASE;
volatile unsigned int * d_pad_up = (int*) D_PAD_0_UP;
volatile unsigned int * d_pad_do = (int*) D_PAD_0_DOWN;
volatile unsigned int * d_pad_le = (int*) D_PAD_0_LEFT;
volatile unsigned int * d_pad_ri = (int*) D_PAD_0_RIGHT;
volatile unsigned int * switch_base = (int*) SWITCHES_0_BASE;

bool game_over = false;
int score = 0;

// Posición random de la manzana
volatile unsigned int* generate_valid_apple_position(int counter) {    
    int x = rand() % (MATRIX_WIDTH - 4) + 2;
    int y = rand() % (MATRIX_HEIGHT - 4) + 2;
    return (volatile unsigned int*)LED_MATRIX_0_BASE + (y * MATRIX_WIDTH + x);
}

bool is_border_position(volatile unsigned int* position) {
    int pos_index = position - (volatile unsigned int*)LED_MATRIX_0_BASE;
    int x = pos_index % MATRIX_WIDTH;
    int y = pos_index / MATRIX_WIDTH;
    
    return x == 0 || x == MATRIX_WIDTH-1 || y == 0 || y == MATRIX_HEIGHT-1;
}

// Bordes azules
void draw_border() {
    volatile unsigned int current = (volatile unsigned int)LED_MATRIX_0_BASE;
    // Borde superior
    for(int i = 0; i < MATRIX_WIDTH; i++) {
        current[i] = BORDER_COLOR;
    }
    // Bordes laterales
    for(int i = 0; i < MATRIX_HEIGHT; i++) {
        current[i * MATRIX_WIDTH] = BORDER_COLOR;  // Izquierdo
        current[i * MATRIX_WIDTH + (MATRIX_WIDTH-1)] = BORDER_COLOR;  // Derecho
    }
    // Borde inferior
    for(int j = 0; j < MATRIX_WIDTH; j++) {
        current[(MATRIX_HEIGHT-1) * MATRIX_WIDTH + j] = BORDER_COLOR;
    }
}

// Pantalla morada y score final
void show_game_over() {
    volatile unsigned int screen = (volatile unsigned int)LED_MATRIX_0_BASE;
    for(int i = 0; i < LED_MATRIX_0_SIZE; i++) {
        screen[i] = GAME_OVER_COLOR;
    }
    printf("Game Over! Final Score: %d\n", score);
    game_over = true;
}

// Dibujar la serpiente
void snake(volatile unsigned int * position, unsigned int color, int length) {
    if (is_border_position(position)) {
        show_game_over();
        return;
    }
    
    volatile unsigned int * head = position;
    volatile unsigned int * tail = position;

    *position = color;
    *(position + 1) = color;
    *(position + MATRIX_WIDTH) = color;
    *(position + MATRIX_WIDTH + 1) = color;

    for (int i = 0; i < length - 1; i++) {
        *(tail - i * MATRIX_WIDTH - i) = color;
        *(tail - (i + 1) * MATRIX_WIDTH - (i + 1)) = 0x000000;
        tail -= MATRIX_WIDTH + 1;
    }
}

void apple(volatile unsigned int * position, unsigned int color) {
    *position = color;
    *(position + 1) = color;
    *(position + MATRIX_WIDTH) = color;
    *(position + MATRIX_WIDTH + 1) = color;
}

void clear_screen() {
    for(int i = 0; i < LED_MATRIX_0_SIZE; i++) {
        ((volatile unsigned int)LED_MATRIX_0_BASE + i) = 0x000000;
    }
}

void main() {
    unsigned int mask = 0;
    volatile unsigned int * tmp;
    int current_direction = 3;
    int move_counter = 0;
    int length = 0;
    int apple_counter = 42;
    
    clear_screen();
    draw_border();
    
    led_base = (int*)LED_MATRIX_0_BASE + MATRIX_WIDTH + 2;
    snake(led_base, SNAKE_COLOR, length);

    volatile unsigned int * apple_base = generate_valid_apple_position(apple_counter);
    apple(apple_base, APPLE_COLOR);
    
    while(1) {
        if(game_over) {
            mask = 0x01 & *switch_base;
            if(mask) {
                game_over = false;
                clear_screen();
                draw_border();
                led_base = (int*)LED_MATRIX_0_BASE + MATRIX_WIDTH + 2;
                apple_counter = 42;
                apple_base = generate_valid_apple_position(apple_counter);
                length = 0;
                current_direction = 3;
                move_counter = 0;
                snake(led_base, SNAKE_COLOR, length);
                apple(apple_base, APPLE_COLOR);
            }
            continue;
        }

        if(*d_pad_up) current_direction = 0;
        else if(*d_pad_do) current_direction = 1;
        else if(*d_pad_le) current_direction = 2;
        else if(*d_pad_ri) current_direction = 3;
        
        move_counter++;
        
        if(move_counter >= 800) {
            move_counter = 0;
            
            snake(led_base, 0x000000, length);

            switch(current_direction) {
                case 0: led_base -= MATRIX_WIDTH; break;
                case 1: led_base += MATRIX_WIDTH; break;
                case 2: led_base -= 1; break;
                case 3: led_base += 1; break;
            }
            
            snake(led_base, SNAKE_COLOR, length);
            
            if(led_base == apple_base) {
                length += 2;
                apple_counter += 13;
                apple_base = generate_valid_apple_position(apple_counter);
                apple(apple_base, APPLE_COLOR);
                score ++;
            }
        }
        
        mask = 0x01 & *switch_base;
        if(mask) {
            game_over = false;
            clear_screen();
            draw_border();
            led_base = (int*)LED_MATRIX_0_BASE + MATRIX_WIDTH + 2;
            apple_counter = 42;//solicitar explicación de que hace el 42
            apple_base = generate_valid_apple_position(apple_counter);
            length = 0;
            current_direction = 3;
            move_counter = 0;
            snake(led_base, SNAKE_COLOR, length);
            apple(apple_base, APPLE_COLOR);
        }
    }
}

/*
No tiene muchos cambios pero la manzana ya se genera de manera random y se imprime el score al perder
*/