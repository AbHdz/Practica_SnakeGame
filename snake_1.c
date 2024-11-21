#include "ripes_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MATRIX_WIDTH LED_MATRIX_0_WIDTH
#define MATRIX_HEIGHT LED_MATRIX_0_WIDTH 

#define SNAKE_COLOR 0xFF0000    // Rojo
#define APPLE_COLOR 0x39FF14    // Verde
#define BORDER_COLOR 0x40E0D0   // Azul
#define GAME_OVER_COLOR 0x800080 // Morado

// Global variables
volatile unsigned int * led_base = (int*) LED_MATRIX_0_BASE;
volatile unsigned int * d_pad_up = (int*) D_PAD_0_UP;
volatile unsigned int * d_pad_do = (int*) D_PAD_0_DOWN;
volatile unsigned int * d_pad_le = (int*) D_PAD_0_LEFT;
volatile unsigned int * d_pad_ri = (int*) D_PAD_0_RIGHT;
volatile unsigned int * switch_base = (int*) SWITCHES_0_BASE;

// Snake array
volatile unsigned int * snake[100];
int snake_length = 0;
bool game_over = false;
int score = 0;

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
    // Superior
    for(int i = 0; i < MATRIX_WIDTH; i++) {
        current[i] = BORDER_COLOR;
    }
    // Laterales
    for(int i = 0; i < MATRIX_HEIGHT; i++) {
        current[i * MATRIX_WIDTH] = BORDER_COLOR;  // Left
        current[i * MATRIX_WIDTH + (MATRIX_WIDTH-1)] = BORDER_COLOR;  // Right
    }
    // Inferior
    for(int j = 0; j < MATRIX_WIDTH; j++) {
        current[(MATRIX_HEIGHT-1) * MATRIX_WIDTH + j] = BORDER_COLOR;
    }
}

// Game over, pantallazo morado
void show_game_over() {
    volatile unsigned int screen = (volatile unsigned int)LED_MATRIX_0_BASE;
    for(int i = 0; i < LED_MATRIX_0_SIZE; i++) {
        screen[i] = GAME_OVER_COLOR;
    }
    // En terminal se refleja el puntaje
    printf("Game Over! Final Score: %d\n", score);
    game_over = true;
}

// Dibuja la serpiente de 2x2
void draw_snake_segment(volatile unsigned int* position, unsigned int color) {
    *position = color;
    *(position + 1) = color;
    *(position + MATRIX_WIDTH) = color;
    *(position + MATRIX_WIDTH + 1) = color;
}

// Dibuja la manzana
void draw_apple(volatile unsigned int * position, unsigned int color) {
    draw_snake_segment(position, color);
}

// Todo negro
void clear_screen() {
    for(int i = 0; i < LED_MATRIX_0_SIZE; i++) {
        ((volatile unsigned int)LED_MATRIX_0_BASE + i) = 0x000000;
    }
}

void main() {
    unsigned int mask = 0;
    int current_direction = 3;
    int move_counter = 0;
    int apple_counter = 42;
    
    clear_screen();
    draw_border();
    
    // Posición inicial de la serpiente
    snake[0] = (int*)LED_MATRIX_0_BASE + MATRIX_WIDTH + 2;
    snake_length = 1;
    draw_snake_segment(snake[0], SNAKE_COLOR);

    volatile unsigned int * apple_base = generate_valid_apple_position(apple_counter);
    draw_apple(apple_base, APPLE_COLOR);
    
    while(1) {
        if(game_over) {
            mask = 0x01 & *switch_base;
            if(mask) {
                game_over = false;
                clear_screen();
                draw_border();
                
                // Reset
                snake[0] = (int*)LED_MATRIX_0_BASE + MATRIX_WIDTH + 2;
                snake_length = 1;
                current_direction = 3;
                move_counter = 0;
                score = 0;
                apple_counter = 42;
                draw_snake_segment(snake[0], SNAKE_COLOR);
                apple_base = generate_valid_apple_position(apple_counter);
                draw_apple(apple_base, APPLE_COLOR);
            }
            continue;
        }

        // D-pad
        if(*d_pad_up) current_direction = 0;
        else if(*d_pad_do) current_direction = 1;
        else if(*d_pad_le) current_direction = 2;
        else if(*d_pad_ri) current_direction = 3;
        
        move_counter++;
        
        if(move_counter >=  300) {
            move_counter = 0;
            
            // Clear
            for(int i = 0; i < snake_length; i++) {
                draw_snake_segment(snake[i], 0x000000);
            }

            // Mover serpiente
            for(int i = snake_length - 1; i > 0; i--) {
                snake[i] = snake[i-1];
            }

            // Mover a cabeza
            switch(current_direction) {
                case 0: snake[0] -= MATRIX_WIDTH; break;  // Up
                case 1: snake[0] += MATRIX_WIDTH; break;  // Down
                case 2: snake[0] -= 1; break;  // Left
                case 3: snake[0] += 1; break;  // Right
            }
            
            // Revisa si choca
            if(is_border_position(snake[0])) {
                show_game_over();
                continue;
            }

            // Redibujar serpiente
            for(int i = 0; i < snake_length; i++) {
                draw_snake_segment(snake[i], SNAKE_COLOR);
            }
            
            // Revisa si come manzana
            if(snake[0] == apple_base) {
                // La serpiente crece
                snake_length += 2;
                
                // Genera otra manzana
                apple_counter += 13; // Número primo
                apple_base = generate_valid_apple_position(apple_counter);
                draw_apple(apple_base, APPLE_COLOR);
                
                // Marcador
                score++;
            }
        }
        
        // Encender switch 0 para reiniciar
        mask = 0x01 & *switch_base;
        if(mask) {
            game_over = false;
            clear_screen();
            draw_border();
            
            snake[0] = (int*)LED_MATRIX_0_BASE + MATRIX_WIDTH + 2;
            snake_length = 1;
            current_direction = 3;
            move_counter = 0;
            score = 0;
            apple_counter = 42;
            draw_snake_segment(snake[0], SNAKE_COLOR);
            apple_base = generate_valid_apple_position(apple_counter);
            draw_apple(apple_base, APPLE_COLOR);
        }
    }
}

/*
Cambié la estructura de la generación de la serpiente para que creciera y eso terminó en varios cambios, pero ya crece al comer manzanas
*/
