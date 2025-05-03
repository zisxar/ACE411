#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 10000000UL // Set clock frequency (10 MHz)

#include <avr/io.h>
#include <util/delay.h> // Include a lib for the delay function
#include <avr/interrupt.h> // Include a lib for the interrupts

// LEDs and Buttons
#define LED_1_OFF PORTB |= (1<<PORTB1)
#define LED_1_ON PORTB &= ~(1<<PORTB1)
#define LED_1_TOGGLE PORTB ^= (1<<PORTB1)

#define LED_2_OFF PORTB |= (1<<PORTB2)
#define LED_2_ON PORTB &= ~(1<<PORTB2)
#define LED_2_TOGGLE PORTB ^= (1<<PORTB2)

#define LED_3_OFF PORTB |= (1<<PORTB3)
#define LED_3_ON PORTB &= ~(1<<PORTB3)
#define LED_3_TOGGLE PORTB ^= (1<<PORTB3)

#define LED_4_OFF PORTB |= (1<<PORTB4)
#define LED_4_ON PORTB &= ~(1<<PORTB4)
#define LED_4_TOGGLE PORTB ^= (1<<PORTB4)

#define LED_7_OFF PORTB |= (1<<PORTB7)
#define LED_7_ON PORTB &= ~(1<<PORTB7)
#define LED_7_TOGGLE PORTB ^= (1<<PORTB7)

#define SW_7_PRESSED PIND & (1<<PIND3) // SW7 is pressed, SW7 is connected with PD3
#define SW_6_PRESSED PIND & (1<<PIND2) // SW6 is pressed, SW6 is connected with PD2

#define LED_Init()\
        DDRB |= (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4) | (1<<DDB7);\
        LED_1_OFF;\
        LED_2_OFF;\
        LED_3_OFF;\
        LED_4_OFF;\
        LED_7_OFF;

#define LEDS_OFF()\
        LED_1_OFF;\
        LED_2_OFF;\
        LED_3_OFF;\
        LED_4_OFF;\
        LED_7_OFF;

#define TIMER_DISABLE TIMSK &= ~(1 << TOIE0); // Timer interrupt disable

#define DEBOUNCE_TIME 500 // ms

// Communication
#define FOSC 10000000 // Device_programming/Board_Settings/Clock_Generator
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define POINT_I(index) ((index - 1) * 4) + 1

const unsigned char enter = 10, carriage_return = 13, space = 32,
                    num_0 = 48,
                    num_1 = 49,
                    num_2 = 50,
                    num_3 = 51,
                    num_4 = 52,
                    num_5 = 53,
                    num_6 = 54,
                    num_7 = 55,
                    num_8 = 56,
                    num_9 = 57,
                    lt_A = 65,
                    lt_B = 66,
                    lt_C = 67,
                    lt_D = 68,
                    lt_E = 69,
                    lt_F = 70,
                    lt_G = 71,
                    lt_H = 72,
                    lt_I = 73,
                    lt_J = 74,
                    lt_K = 75,
                    lt_L = 76,
                    lt_M = 77,
                    lt_N = 78,
                    lt_O = 79,
                    lt_P = 80,
                    lt_Q = 81,
                    lt_R = 82,
                    lt_S = 83,
                    lt_T = 84,
                    lt_U = 85,
                    lt_V = 86,
                    lt_W = 87,
                    lt_X = 88,
                    lt_Y = 89,
                    lt_Z = 90;

uint8_t opcode = 200,      // opcode taken from the received instruction
                           // OK<CR>                      opcode = 0
                           // AT<CR>                      opcode = 1
                           // RST<CR>                     opcode = 2
                           // SP<SP>{B, W}<CR>            opcode = 3
                           // NG<CR>                      opcode = 4
                           // EG<CR>                      opcode = 5
                           // ST<SP>[1-9] <CR>            opcode = 6
                           // MV<SP>{[A-H], [1-8]}<CR>    opcode = 7
                           // PS<CR>                      opcode = 8
                           // WN<CR>                      opcode = 9
                           // PL<CR>                      opcode = 10

        sent_opcode = 200, // opcode from the instruction AVR previously sent
                           // ER<CR>                      sent_opcode = 200
                           // OK<CR>                      sent_opcode = 201
                           // MM<SP>{[A-H],[1-8]}<CR>     sent_opcode = 202
                           // MP<CR>                      sent_opcode = 203
                           // WN<CR>                      sent_opcode = 204
                           // LS<CR>                      sent_opcode = 205
                           // ΤΕ<CR>                      sent_opcode = 206
                           // IL<CR>                      sent_opcode = 207
                           // IT<CR>                      sent_opcode = 208
                           // QT<CR>                      sent_opcode = 209

        a1 = 0, a2 = 0,    // arguments extracted from the instruction
        my_colour = 111, op_colour = 111,
        my_points = 0, op_points = 0,
        my_turn = 0;

char at_instr[3] = {'A', 'T', '\r'},       // AT<CR>
     rst_instr[4] = {'R', 'S', 'T', '\r'}, // RST<CR>
     sp__instr[3] = {'S', 'P', ' '},       // SP<SP>
     st__instr[3] = {'S', 'T', ' '},       // ST<SP>
     ng_instr[3] = {'N', 'G', '\r'},       // NG<CR>
     eg_instr[3] = {'E', 'G', '\r'},       // EG<CR>
     mv__instr[3] = {'M', 'V', ' '},       // MV<SP>
     ps_instr[3] = {'P', 'S', '\r'},       // PS<CR>
     wn_instr[3] = {'W', 'N', '\r'},       // WN<CR>
     ok_instr[3] = {'O', 'K', '\r'},       // OK<CR>
     pl_instr[3] = {'P', 'L', '\r'};       // PL<CR>

unsigned short* main_board = 0x00E8,               // Address of the main board in memory
                possible_boards = 0x011A,          // Address of the first possible board 
                                                   // in memory
                count_and_positions = 0x03D8;      // Address of the count of moves, 
                                                   // the position (i, j), the direction
                                                   // of each move
                                                   // and points added from each move

void send_String(char* sending_mes, uint8_t size);
void find_move(uint8_t* cnt_and_pos);
void choose_move(uint8_t* cnt_and_pos);
uint8_t find_direction(uint8_t i_ind, uint8_t j_ind, uint8_t player_colour);
void check_for_response(uint8_t s_opcode);
void eat_neighbors(uint8_t i_ind, uint8_t j_ind, uint8_t direction, uint8_t player_colour);
void make_my_move();

#endif /* MAIN_H_ */