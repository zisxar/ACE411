/*
 * main.c
 *
 * Created: 21/4/2019 14:07:26
 * Author : zisxa
 */ 

#include "main.h"

/// /// /// /// /// /// /// /
/// Timer functions start ///
/// /// /// /// /// /// /// /

unsigned short overflowCount = 0, 
               time_s = 40,
               time_s1 = 80;

ISR(TIMER0_OVF_vect) {

    TCNT0 = 0x00; // Set the timer to 0
    TIFR = 0x01;  // Clear timer overflow flag
    overflowCount++;

    // By trial and error the 1 second is every 40 overflows

    if (overflowCount >= time_s - 1 && overflowCount <= time_s) {
        if (my_turn == 1) {      // My turn
            send_String("LS", 3);
            sent_opcode = 205;
            check_for_response(sent_opcode);
            // End the game
            // Stop the counter
            TIMER_DISABLE;
            // Turn the LEDs off
            LEDS_OFF()
            // Initialize the player colours
            my_colour = 111;
            op_colour = 111;
        }
        else {                   // Opponents turn
            send_String("IT", 3);
            sent_opcode = 208;
            check_for_response(sent_opcode);
            my_turn = 1;
        }
    }

    if (overflowCount <= time_s) {
        LED_1_ON;
        LED_2_OFF;
    }
    else if (overflowCount <= time_s1) {
        LED_1_OFF;
        LED_2_ON;
    }
    else {
        overflowCount = 0;
        LED_2_OFF;
    }
}

void timer_Init() {
    TCNT0 = 0x00;                    // Counter initialization
    TCCR0 = (1 << CS00) | (1<<CS02); // Configure 1024 prescaler
    sei();                           // Global interrupt enable
}

void timer_Enable() {
    TCNT0 = 0x00;         // Counter initialization
    TIMSK = (1 << TOIE0); // Timer interrupt enable
}

void timer_Restart() {
    TCNT0 = 0x00;      // Set the timer to 0
    TIFR = 0x01;       // Clear timer overflow flag
    overflowCount = 0; // Initialize overflowCount
}

void timer_Update(uint8_t t_sec) {
    time_s = t_sec * 40;
    time_s1 = t_sec * 80;
}

/// /// /// /// /// /// ///
/// Timer functions end ///
/// /// /// /// /// /// ///

/// /// /// /// /// /// /// /// /// /
/// Communication functions start ///
/// /// /// /// /// /// /// /// /// /
void USART_Init(unsigned int ubrr) {

    UBRRH = (unsigned char)(ubrr>>8); // Set baud rate
    UBRRL = (unsigned char)ubrr;

    UCSRB = (1<<RXEN)|(1<<TXEN);      // Enable receiver and transmitter

    UCSRC = (1<<URSEL)|(3<<UCSZ0);    // Set frame format: 8 data, 1 stop bit
}

unsigned char USART_Receive(void) {

    while ( !(UCSRA & (1<<RXC)) ); // Wait for data to be received
    return (unsigned char)UDR;     // Get and return received data from buffer
}

void USART_Transmit(unsigned char data) {

    while ( !(UCSRA & (1<<UDRE)) ); // Wait for empty transmit buffer
    UDR = data;                     // Put data into buffer, sends the data
    while ( !(UCSRA & (1<<TXC)) );
}

unsigned char* read_String(unsigned char* received_message, uint8_t size) {

    uint8_t letter = 0;
    received_message[letter] = USART_Receive();
    while ((received_message[letter] != carriage_return) && (letter <= size-2)) {
        letter++;
        received_message[letter] = USART_Receive();
    }

    return received_message;
}

// Function that displays a sting to the terminal
void send_String(char* sending_mes, uint8_t size) {

    unsigned char* sending_message = (unsigned char*) sending_mes;
    uint8_t letter = 0;
    
    if (sending_message[letter] == carriage_return) {
        USART_Transmit(carriage_return);
    }
    while ((sending_message[letter] != carriage_return) && (letter <= size-1)) {
        USART_Transmit(sending_message[letter]);
        letter++;
    }
}
/// /// /// /// /// /// /// /// ///
/// Communication functions end ///
/// /// /// /// /// /// /// /// ///

/// /// /// /// /// /// /// /
/// Other functions start ///
/// /// /// /// /// /// /// /

// Check if the string str1 equals with str2, both strings must be of the same size,
// if they are equal, returns 1, else 0.
uint8_t check_Sting(char* str1, char* str2, uint8_t size) {

    uint8_t equal = 1;
    for (uint8_t i = 0; i < size; i++) {
        if (str1[i] != str2[i]) {
            equal = 0;
            break;
        }
    }
    return equal;
}

/// /// /// /// /// /// ///
/// Other functions end ///
/// /// /// /// /// /// ///

/// /// /// /// /// /// /// //
/// Memory functions start ///
/// /// /// /// /// /// /// //

uint8_t index_calc(uint8_t i_ind, uint8_t j_ind) {

    if (i_ind == 0) {
        return (j_ind);
    }
    else if (i_ind == 1) {
        return (8 + j_ind);
    }
    else if (i_ind == 2) {
        return (16 + j_ind);
    }
    else if (i_ind == 3) {
        return (24 + j_ind);
    }
    else if (i_ind == 4) {
        return (32 + j_ind);
    }
    else if (i_ind == 5) {
        return (40 + j_ind);
    }
    else if (i_ind == 6) {
        return (48 + j_ind);
    }
    else if (i_ind == 7) {
        return (56 + j_ind);
    }
    else {
        return (0);
    }
}

// Read access the array address[] at the position address[i_ind, j_ind]
unsigned char mem_read(uint8_t i_ind, uint8_t j_ind, uint8_t* address) {

    uint8_t index = index_calc(i_ind, j_ind);

    unsigned char retval = *(address + index);
    return retval;
}

// Write access the array address[] at the position address[i_ind, j_ind] and
// write the value player_colour
void make_move(uint8_t i_ind, uint8_t j_ind, uint8_t player_colour, uint8_t* alloc_address) {

    uint8_t index = index_calc(i_ind, j_ind);
    
    *(alloc_address + index) = player_colour;
}

// Allocates memory for one board and initializes it to all empty
// 250 empty
// 0 black player
// 1 white player
void memory_Allocation_Init(uint8_t* alloc_address) {

    uint8_t init_pos0 = index_calc(3, 3); // D4 White
    uint8_t init_pos1 = index_calc(4, 4); // E5 White
    uint8_t init_pos2 = index_calc(4, 3); // D5 Black
    uint8_t init_pos3 = index_calc(3, 4); // E4 Black

    /**alloc_address = 249;
    alloc_address++;*/
    for (uint8_t i = 0; i <= 63; i++) {
        
        // Initialize the pawns at the centre of the board
        if (i == init_pos0 || i == init_pos1) {
            *alloc_address = 1;
        }
        // Initialize the pawns at the centre of the board
        else if (i == init_pos2 || i == init_pos3) {
            *alloc_address = 0;
        }
        // Initialize all the other pawns at the board
        else {
            *alloc_address = 250;
        }

        alloc_address++;
    }
}

// Allocates space for num_Of_Boards boards
void board_Creation(uint8_t num_Of_Boards, uint8_t* alloc_address) {

    uint8_t* all_address = alloc_address;
    for (uint8_t i = 0; i < num_Of_Boards; i++) {
        memory_Allocation_Init(all_address);
        all_address += 64;
    }
}

/// /// /// /// /// /// ////
/// Memory functions end ///
/// /// /// /// /// /// ////

/// /// /// /// /// /// /// /// /// /// //
/// Game functionality functions start ///
/// /// /// /// /// /// /// /// /// /// //

// In this function the memory for the boards is allocated
// and the timer is set to start over.
void new_game() {

    if (my_colour == 111) {
        send_String("ER", 3);
    }
    else {
        // From the address 0x0060 to 0x00D8 there are other data
        // so our base address is 0x00D9
        main_board = (uint8_t*) 0x00E8;
        possible_boards = main_board + 32;

        // Allocate to the memory and initialize the given number of boards
        board_Creation(1, main_board);
        // There is room for twelve more boards only
        // board_Creation(5, possible_boards);
        board_Creation(1, possible_boards);

        my_turn = (my_colour == 0) ? 1 : 0;
        // Set and start the timer for the game
        timer_Enable();
        
        if (my_turn == 1) {
            make_my_move();
        }
    }
}

uint8_t is_valid(uint8_t i_pos, uint8_t j_pos, uint8_t* board) {

    if (i_pos > 7 || i_pos < 0 || j_pos > 7 || j_pos < 0) { // Out of boundaries
        return (uint8_t) 0;
    }
    uint8_t i, j, ret_val;
    unsigned char read_val;

    read_val = mem_read(i_pos, j_pos, board);
    // This position is already occupied
    if (read_val != 250) {
        return (uint8_t) 0;
    }

    // Check for the rest of the pawns

    // Down swipe
    i = i_pos;
    j = j_pos;
        
    read_val = mem_read(i, j, board);
    if (read_val == 250) {
        read_val = mem_read(i + 1, j, board);
        i++;
        while (read_val == my_colour && i <= 6) {
            read_val = mem_read(i + 1, j, board);
            if (read_val == op_colour) {
                return (uint8_t) 1;
            }
            i++;
        }
    }
    else {
        ret_val = 31;
    }

    // Upper swipe
    i = i_pos;
    j = j_pos;
        
    read_val = mem_read(i, j, board);
    if (read_val == 250) {
        read_val = mem_read(i - 1, j, board);
        i--;
        while (read_val == my_colour && i >= 1) {
            read_val = mem_read(i - 1, j, board);
            if (read_val == op_colour) {
                return (uint8_t) 1;
            }
            i--;
        }
    }
    else {
        ret_val = 32;
    }

    // Right swipe
    i = i_pos;
    j = j_pos;
        
    read_val = mem_read(i, j, board);
    if (read_val == 250) {
        read_val = mem_read(i, j + 1, board);
        j++;
        while (read_val == my_colour && j <= 6) {
            read_val = mem_read(i, j + 1, board);
            if (read_val == op_colour) {
                return (uint8_t) 1;
            }
            j++;
        }
    }
    else {
        ret_val = 33;
    }

    // Left swipe
    i = i_pos;
    j = j_pos;
        
    read_val = mem_read(i, j, board);
    if (read_val == 250) {
        read_val = mem_read(i, j - 1, board);
        j--;
        while (read_val == my_colour && i >= 1) {
            read_val = mem_read(i, j - 1, board);
            if (read_val == op_colour) {
                return (uint8_t) 1;
            }
            j--;
        }
    }
    else {
        ret_val = 0;
    }

    return ret_val;
}

void instruction_Decode(char* instruction, uint8_t size) {

    // OK<CR>
    if (check_Sting(instruction, ok_instr, 3) == 1) {
        opcode = 0;
    }

    // AT<CR>
    else if (check_Sting(instruction, at_instr, 3) == 1) {
        opcode = 1;
    }

    // RST<CR>
    else if (check_Sting(instruction, rst_instr, 4) == 1) {
        opcode = 2;
    }

    // SP<SP>{B, W}<CR>
    else if (check_Sting(instruction, sp__instr, 3) == 1) {
        if (instruction[3] == 'B' || instruction[3] == 'W') {
            my_colour = (instruction[3] == 'W') ? 1 : 0; // 1 for White, 0 for Black
            op_colour = (instruction[3] == 'W') ? 0 : 1;
            if (instruction[4] == '\r') {
                opcode = 3;
            }
            else {opcode = 200;}
        }
        else {opcode = 200;}
    }

    // NG<CR>
    else if (check_Sting(instruction, ng_instr, 3) == 1) {
        opcode = 4;
    }

    // EG<CR>
    else if (check_Sting(instruction, eg_instr, 3) == 1) {
        opcode = 5;
    }

    // ST<SP>[1-9] <CR>
    else if (check_Sting(instruction, st__instr, 3) == 1) {
        if (instruction[3] >= '1' && instruction[3] <= '9') {
            a1 = (uint8_t) (instruction[3] - 48);
            if (instruction[4] == '\r') {
                opcode = 6;
            }
            else {opcode = 200;}
        }
        else {opcode = 200;}
    }

    // MV<SP>{[A-H], [1-8]}<CR>
    else if (check_Sting(instruction, mv__instr, 3) == 1) {
        if (instruction[3] >= 'A' && instruction[3] <= 'H') {
            a1 = (uint8_t) (instruction[3] - 65);
            if (instruction[4] >= '1' && instruction[4] <= '8') {
                a2 = (uint8_t) (instruction[4] - 49);
                if (instruction[5] == '\r') {
                    opcode = 7;
                }
                else {opcode = 200;}
            }
            else {opcode = 200;}
        }
        else {opcode = 200;}
    }
    // PS<CR>
    else if (check_Sting(instruction, ps_instr, 3) == 1) {
        opcode = 8;
    }

    // WN<CR>
    else if (check_Sting(instruction, wn_instr, 3) == 1) {
        opcode = 9;
    }

    // PL<CR>
    else if (check_Sting(instruction, pl_instr, 3) == 1) {
        opcode = 10;
    }

    else {opcode = 200;}
}

void instruction_Execution () {

    switch (opcode) {
        case 200:
            send_String("ER", 3);
            sent_opcode = 200;
            break;
        case 0:
            // check if the ok came from a previous sent instruction
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 1: // AT<CR>
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 2: // RST<CR>
            // Stop the counter
            TIMER_DISABLE;
            // Turn the LEDs off
            LEDS_OFF()
            // Initialize the player colours
            my_colour = 111;
            op_colour = 111;
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 3: // SP<SP>{B, W}<CR>
            if (my_colour == 111) {
                send_String("ER", 3);
                sent_opcode = 200;
            }
            else {
                send_String("OK", 3);
                sent_opcode = 201;
            }
            break;
        case 4: // NG<CR>
            new_game();
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 5: // EG<CR>
            // Stop the counter
            TIMER_DISABLE;
            // Turn the LEDs off
            LEDS_OFF()
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 6: // ST<SP>[1-9] <CR>
            timer_Update(a1);
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 7: // MV<SP>{[A-H], [1-8]}<CR>
            if (is_valid(a1, a2, main_board) == (uint8_t)1) {
                make_move(a1, a2, op_colour, main_board);
                op_points++;
                eat_neighbors(a1, a2, find_direction(a1, a2, op_colour), op_colour);
                my_turn = 1;
                timer_Restart();

                send_String("OK", 3);
                sent_opcode = 201;
            }
            else {
                send_String("IL", 3);
                sent_opcode = 207;
                check_for_response(sent_opcode);
            }
            break;
        case 8: // PS<CR>
            send_String("OK", 3);
            sent_opcode = 201;
            break;
        case 9: // WN<CR>
            send_String("OK", 3);
            sent_opcode = 201;
            // End the game
            // Stop the counter
            TIMER_DISABLE;
            // Turn the LEDs off
            LEDS_OFF()
            // Initialize the player colours
            my_colour = 111;
            op_colour = 111;
            break;
        case 10: // PL<CR>
        // Do nothing
            break;

        default:
            send_String("ER", 3);
            sent_opcode = 200;
    }
}

void convert_from_uchar(unsigned char* data, uint8_t size, char* instr) {

    for (uint8_t i = 0; i < size; i++) {
        switch (data[i]) {
            case (unsigned char) num_0:
                instr[i] = '0';
                break;
            case (unsigned char) num_1:
                instr[i] = '1';
                break;
            case (unsigned char) num_2:
                instr[i] = '2';
                break;
            case (unsigned char) num_3:
                instr[i] = '3';
                break;
            case (unsigned char) num_4:
                instr[i] = '4';
                break;
            case (unsigned char) num_5:
                instr[i] = '5';
                break;
            case (unsigned char) num_6:
                instr[i] = '6';
                break;
            case (unsigned char) num_7:
                instr[i] = '7';
                break;
            case (unsigned char) num_8:
                instr[i] = '8';
                break;
            case (unsigned char) num_9:
                instr[i] = '9';
                break;
            case (unsigned char) lt_A:
                instr[i] = 'A';
                break;
            case (unsigned char) lt_B:
                instr[i] = 'B';
                break;
            case (unsigned char) lt_C:
                instr[i] = 'C';
                break;
            case (unsigned char) lt_D:
                instr[i] = 'D';
                break;
            case (unsigned char) lt_E:
                instr[i] = 'E';
                break;
            case (unsigned char) lt_F:
                instr[i] = 'F';
                break;
            case (unsigned char) lt_G:
                instr[i] = 'G';
                break;
            case (unsigned char) lt_H:
                instr[i] = 'H';
                break;
            case (unsigned char) lt_I:
                instr[i] = 'I';
                break;
            case (unsigned char) lt_J:
                instr[i] = 'J';
                break;
            case (unsigned char) lt_K:
                instr[i] = 'K';
                break;
            case (unsigned char) lt_L:
                instr[i] = 'L';
                break;
            case (unsigned char) lt_M:
                instr[i] = 'M';
                break;
            case (unsigned char) lt_N:
                instr[i] = 'N';
                break;
            case (unsigned char) lt_O:
                instr[i] = 'O';
                break;
            case (unsigned char) lt_P:
                instr[i] = 'P';
                break;
            case (unsigned char) lt_Q:
                instr[i] = 'Q';
                break;
            case (unsigned char) lt_R:
                instr[i] = 'R';
                break;
            case (unsigned char) lt_S:
                instr[i] = 'S';
                break;
            case (unsigned char) lt_T:
                instr[i] = 'T';
                break;
            case (unsigned char) lt_U:
                instr[i] = 'U';
                break;
            case (unsigned char) lt_V:
                instr[i] = 'V';
                break;
            case (unsigned char) lt_W:
                instr[i] = 'W';
                break;
            case (unsigned char) lt_X:
                instr[i] = 'X';
                break;
            case (unsigned char) lt_Y:
                instr[i] = 'Y';
                break;
            case (unsigned char) lt_Z:
                instr[i] = 'Z';
                break;
            case (unsigned char) carriage_return:
                instr[i] = '\r';
                break;
            case (unsigned char) space:
                instr[i] = ' ';
                break;
            
            default:
                instr[i] = '@';
                break;
        }
    }
}

// Find the possible moves and write them at the possible_boards address
// Return via pointer the number of the possible moves and the position of them.
void find_move(uint8_t* cnt_and_pos) {

    uint8_t op_cnt_hl, op_cnt_hr, op_cnt_vd, op_cnt_vu, pos_moves_cnt = 0;
    uint8_t i_st, j_st = 0;
    unsigned char read_val;
    unsigned short move = 0;

    for (uint8_t i = 0; i <= 7; i++) {
        for (uint8_t j = 0; j <= 7; j++) {
            
            read_val = mem_read(i, j, main_board);

            // search horizontally from the left to the right (pawn)
            if (read_val == op_colour && op_cnt_hr == 0) {
                op_cnt_hl++;
                if (op_cnt_hl == 1) {
                    j_st = j;
                }
            }
            else if (op_cnt_hl >= 1 && read_val == my_colour) {
                if (j_st - 1 >= 0 && mem_read(i, j_st - 1, main_board) == 250) {
                    pos_moves_cnt++;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt)) = i;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 1) = j_st - 1;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 2) = 2; // right direction
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 3) = op_cnt_hl; // points earned
                    *cnt_and_pos = pos_moves_cnt;
                    // make_move(i, j_st - 1, my_colour, possible_boards + move);
                    move = move + 32;
                }
                op_cnt_hl = 0;
                j_st = 0;
            }
            else {
                op_cnt_hl = 0;
                j_st = 0;
            }

            // search horizontally from the left (pawn) to the right
            if (read_val == my_colour && op_cnt_hl == 0) {
                if (j < 7 && mem_read(i, j + 1, main_board) == op_colour) { // found the first op_colour after
                    op_cnt_hr++;                                            // my_colour, start the counting
                }
            }
            else if (op_cnt_hr >= 1) {
                if (read_val == 250) {
                    pos_moves_cnt++;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt)) = i;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 1) = j;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 2) = 1; // left direction
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 3) = op_cnt_hr; // points earned
                    *cnt_and_pos = pos_moves_cnt;
                    // make_move(i, j, my_colour, possible_boards + move);
                    move = move + 32;
                    op_cnt_hr = 0;
                }
                else if (read_val == my_colour) {
                    op_cnt_hr = 0;
                }
                else {
                    op_cnt_hr++;
                }
            }
            else {
                op_cnt_hr = 0;
            }
        }
        j_st = 0;
        op_cnt_hl = 0;
        op_cnt_hr = 0;
    }

    for (uint8_t j = 0; j <= 7; j++) {
        for (uint8_t i = 0; i <= 7; i++) {
            
            read_val = mem_read(i, j, main_board);

            // search vertically from up to down (pawn)
            if (read_val == op_colour && op_cnt_vd == 0) {
                op_cnt_vu++;
                if (op_cnt_vu == 1) {
                    i_st = i;
                }
            }
            else if (op_cnt_vu >= 1 && read_val == my_colour) {
                if (i_st - 1 >= 0 && mem_read(i_st - 1, j, main_board) == 250) {
                    pos_moves_cnt++;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt)) = i_st - 1;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 1) = j;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 2) = 4; // down direction
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 3) = op_cnt_vu; // points earned
                    *cnt_and_pos = pos_moves_cnt;
                    // make_move(i_st - 1, j, my_colour, possible_boards + move);
                    move = move + 32;
                }
                op_cnt_vu = 0;
                i_st = 0;
            }
            else {
                op_cnt_vu = 0;
                j_st = 0;
            }

            // search vertically from up (pawn) to down
            if (read_val == my_colour && op_cnt_vu == 0) {
                if (i < 7 && mem_read(i + 1, j, main_board) == op_colour) { // found the first op_colour after
                    op_cnt_vd++;                                            // my_colour, start the counting
                }
            }
            else if (op_cnt_vd >= 1) {
                if (read_val == 250) {
                    pos_moves_cnt++;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt)) = i;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 1) = j;
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 2) = 3; // up direction
                    *(cnt_and_pos + POINT_I(pos_moves_cnt) + 3) = op_cnt_vd; // points earned
                    *cnt_and_pos = pos_moves_cnt;
                    // make_move(i, j, my_colour, possible_boards + move);
                    move = move + 32;
                    op_cnt_vd = 0;
                }
                else if (read_val == my_colour) {
                    op_cnt_vd = 0;
                }
                else {
                    op_cnt_vd++;
                }
            }
            else {
                op_cnt_vd = 0;
            }
        }
        i_st = 0;
        op_cnt_vu = 0;
        op_cnt_vd = 0;
    }
}

// Function that searches for the direction the pawns should be turned
// return left : 1, right : 2, up : 3, down : 4, if not found : 0
uint8_t find_direction(uint8_t i_ind, uint8_t j_ind, uint8_t player_colour) {

    unsigned char read_val;
    uint8_t player_op_colour = (player_colour == 0) ? 1 : 0;

    read_val = mem_read(i_ind, j_ind - 1, main_board); // Check left
    if (read_val == player_op_colour) {
        return 1;
    }
    
    read_val = mem_read(i_ind, j_ind + 1, main_board); // Check right
    if (read_val == player_op_colour) {
        return 2;
    }
    
    read_val = mem_read(i_ind - 1, j_ind, main_board); // Check up
    if (read_val == player_op_colour) {
        return 3;
    }
    
    read_val = mem_read(i_ind + 1, j_ind, main_board); // Check down
    if (read_val == player_op_colour) {
        return 4;
    }

    return 0; // If the direction can't be found
}

void eat_neighbors(uint8_t i_ind, uint8_t j_ind, uint8_t direction, uint8_t player_colour) {

    // left : 1, right : 2, up : 3, down : 4
    uint8_t i = i_ind,
            j = j_ind,
            player_points = 0,// (player_colour == my_colour) ? my_points : op_points,
            player_op_colour = (player_colour == 0) ? 1 : 0;
    unsigned char read_val;

    read_val = mem_read(i, j, main_board);

    if (direction == 1) { // left
        j--;
        read_val = mem_read(i, j, main_board);
        while (j >= 1 && read_val == player_op_colour) {
            make_move(i, j, player_colour, main_board);
            player_points++;
            j--;
            read_val = mem_read(i, j, main_board);
        }
    }
    else if (direction == 2) { // right
        j++;
        read_val = mem_read(i, j, main_board);
        while (j <= 6 && read_val == player_op_colour) {
            make_move(i, j, player_colour, main_board);
            player_points++;
            j++;
            read_val = mem_read(i, j, main_board);
        }
    }
    else if (direction == 3) { // up
        i--;
        read_val = mem_read(i, j, main_board);
        while (i >= 1 && read_val == player_op_colour) {
            make_move(i, j, player_colour, main_board);
            player_points++;
            i--;
            read_val = mem_read(i, j, main_board);
        }
    }
    else if (direction == 4) { // down
        i++;
        read_val = mem_read(i, j, main_board);
        while (i <= 6 && read_val == player_op_colour) {
            make_move(i, j, player_colour, main_board);
            player_points++;
            i++;
            read_val = mem_read(i, j, main_board);
        }
    }

    if (player_colour == my_colour) {
        my_points = my_points + player_points;
        if (player_points > op_points) {
            op_points = 0;
        }
        else {
            op_points = op_points - player_points;
        }
    }
    else {
        op_points = op_points + player_points;
        if (player_points > my_points) {
            my_points = 0;
        }
        else {
            my_points = my_points - player_points;
        }
    }
}

void choose_move(uint8_t* cnt_and_pos) {

    uint8_t num_of_pos_moves = *cnt_and_pos,
            points,
            max_points = 0,
            max_ind = 1;

    if (num_of_pos_moves >= 0) {

        uint8_t move = 1,
                i = *(cnt_and_pos + POINT_I(move)),
                j = *(cnt_and_pos + POINT_I(move) + 1),
                dir = *(cnt_and_pos + POINT_I(move) + 2);
                points = *(cnt_and_pos + POINT_I(move) + 3);

        for (move = 1; move <= num_of_pos_moves; move++) {
            if (points > max_points) {
                max_points = points;
                max_ind = move;
            }
            move++;
            points = *(cnt_and_pos + POINT_I(move) + 3);
        }

        i = *(cnt_and_pos + POINT_I(max_ind)),
        j = *(cnt_and_pos + POINT_I(max_ind) + 1),
        dir = *(cnt_and_pos + POINT_I(max_ind) + 2);

        make_move(i, j, my_colour, main_board);
        my_points++;
        eat_neighbors(i, j, dir, my_colour);
        USART_Transmit(lt_M);
        USART_Transmit(lt_M);
        USART_Transmit(space);
        USART_Transmit((unsigned char) (i + 65));
        USART_Transmit((unsigned char) (j + 49));
        USART_Transmit(carriage_return);
        sent_opcode = 202;
    }
    else {
        send_String("MP", 3);
        sent_opcode = 203;
        check_for_response(sent_opcode);
    }
}

void check_for_response(uint8_t s_opcode) {

    uint8_t size = 8;
    unsigned char received_message[size];
    char inst_str[size];

    for (uint8_t i = 0; i<= size-2; i++) {
        received_message[i] = num_0;
        inst_str[i] = 0;
    }
    received_message[size-1] = carriage_return;

    // MM<SP>{[A-H],[1-8]}<CR> MP<CR>           WN<CR>             LS<CR>             ΤΕ<CR>             IL<CR>             IT<CR>
    if (s_opcode == 202 || s_opcode == 203 || s_opcode == 204 || s_opcode == 205 || s_opcode == 206 || s_opcode == 207 || s_opcode == 208) {

        // Convert the string from unsigned char to char and int
        convert_from_uchar(read_String(received_message, size), size, inst_str);
        // Decode the instruction
        instruction_Decode(inst_str, size);

        if (opcode == 2) { // RST came
            instruction_Execution();
        }
        else if (opcode == 0) { // The answer to the sent instruction is OK
            if (s_opcode == 207 || s_opcode == 208) {
                send_String("WN", 3);
                sent_opcode = 204;
                s_opcode = 204;
                // End the game
                // Stop the counter
                TIMER_DISABLE;
                // Turn the LEDs off
                LEDS_OFF()
                // Initialize the player colours
                my_colour = 111;
                op_colour = 111;
            }
        }
        else {
            send_String("QT", 3);
            sent_opcode = 209;
            s_opcode = 209;
            // End the game
            // Stop the counter
            TIMER_DISABLE;
            // Turn the LEDs off
            LEDS_OFF()
            // Initialize the player colours
            my_colour = 111;
            op_colour = 111;
        }
    }
}

void make_my_move() {

    find_move(count_and_positions);
    choose_move(count_and_positions);
    my_turn = 0;
    timer_Restart();
}

/// /// /// /// /// /// /// /// /// /// 
/// Game functionality functions end //
/// /// /// /// /// /// /// /// /// /// 

int main(void) {

    uint8_t size = 8;
    unsigned char received_message[size];
    char inst_str[size];

    for (uint8_t i = 0; i<= size-2; i++) {
        received_message[i] = num_0;
        inst_str[i] = 0;
    }
    received_message[size-1] = carriage_return;
    
    timer_Init();

    USART_Init(MYUBRR);

    LED_Init();

    //// if new game, this has to be deleted
    /*my_colour = 1; // White
    op_colour = 0; // Black
    main_board = 0x00E8;
    possible_boards = main_board + 32; // it works with +32 instead of +64,
    make_move(0, 0, 155, main_board);
    make_move(0, 1, 155, main_board);
    make_move(0, 2, 155, main_board);
    // Allocate to the memory and initialize the given number of boards
    board_Creation(1, main_board);
    make_move(0, 1, 156, main_board + 32);
    new_game();
    make_move(0, 0, 159, main_board + 32);
    
    // There is room for eleven more boards only
    // board_Creation(7, possible_boards);
    make_move(0, 2, 157, main_board + 32);

    find_move(count_and_positions);

    make_move(0, 3, 158, main_board + 32);
    choose_move(count_and_positions);*/

    while (1) {

        // Convert the string from unsigned char to char and int
        convert_from_uchar(read_String(received_message, size), size, inst_str);

        // Decode the instruction
        instruction_Decode(inst_str, size);

        // Check the opcode and execute the instruction
        instruction_Execution();

        // Received instruction is MV or PS and AVRs answer is OK, so AVR has to make a move
        if ((opcode == 7 || opcode == 8) && sent_opcode == 201) {
            make_my_move();
        }
    }
}