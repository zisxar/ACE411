# Course Project: Microcontroller Basics

## Description

This project fulfills the requirements of the university course "Embedded Systems ACE411" for the 2018-2019 academic year.  
The description files have not been uploaded.  
The goal of the project is to develop an [Othello](https://hewgill.com/othello/) game on a microcontroller, which can be played against either a human player or another STK500.  
The interface operates through a serial port, allowing the platform to accept instructions (in ASCII) and send moves, error codes, or messages.  
The Othello game is a simplified version of the traditional Chinese game, GO.

## Technologies Used

- Languages: C
- Tools: Linux, ATMEL Studio 7.0
- Hardware: STK500 with ATmega16

## Implementation

Files:

```text
ACE411/
├── main.c
├── main.h
└── README.md
```

For this project an STK500 with an ATmega16 has been utilized, and ATMEL Studio 7.0 was used for programming the microcontroller.

Initially, to familiarize with the STK500, an LED blinking functionality using interrupts was implemented. Later, this LED functionality was adapted to meet the needs of the game, as detailed in the table.  
Additionally, a timer was implemented using interrupts.

### Serial Communication

The serial port interface was utilized, to effectively achieve communication. This interface is necessary for transferring instructions to and from the AVR.  
A set of predefined instruction was created as shown below.  
It is important to note that the PC is the user, or another AVR, and as AVR is mentioned the current microcontroller.

#### PC to AVR  

| Instruction | Response | Functionality | Additional Functionality |
|---|:---:|:---:|:---:|
| AT\<CR> | OK\<CR> | Simple OK answer |  |
| RST\<CR> | OK\<CR> | RESET (Warm Start) |  |
| SP\<SP>{B, W}\<CR> | OK\<CR> | Set Player color (Black or White) |  |
| NG\<CR> | OK\<CR> | New Game |  |
| EG\<CR> | OK\<CR> | End Game |  |
| ST\<SP>[1-9]\<CR> | OK\<CR> | Set Time in seconds (1-9) | Default = 2 sec |
| MV\<SP>{[A-H], [1-8]}\<CR> | OK\<CR> | Move of the PC, e.g. MV_G3 |  |
| PS\<CR> | OK\<CR> | Pass. PC doesn't make a move and AVR plays again |  |
| WN\<CR> | OK\<CR> | PC Wins. AVR quitted, made an illegal move, or time's up |  |

#### AVR to PC  

| Instruction | Response | Functionality | Additional Functionality |
|---|:---:|:---:|:---:|
| MM\<SP>{[A-H], [1-8]}\<CR> | OK\<CR> | Move of the AVR, e.g. MM_G3 |  |
| MP\<CR> | OK\<CR> | Pass. AVR doesn't make a move and PC plays again |  |
| WN\<CR> | OK\<CR> | AVR Wins. PC quitted, made an illegal move, or time's up | Turn on LED 1 |
| LS\<CR> | OK\<CR> | AVR Loses | Turn on LED 2 |
| TE\<CR> | OK\<CR> | Tie | Turn on LED 3 |
| IL\<CR> | OK\<CR> accept or PL\<CR> reject request | Illegal Move. The opponent made an illegal move |  |
| IT\<CR> | OK\<CR> accept or PL\<CR> reject request | Illegal Time. The opponent exceeded the time limit |  |
| QT\<CR> | OK\<CR> | AVR Quits the game |  |

### Memory

The chessboard is stored in the internal memory, and it looks like in the image bellow. Also, in this image four possible moves are shown.  

![Boards in memory!](/ACE411/memory.jpg "Boards in memory")  

| Value | Meaning |
|:---:|:---:|
| 0 | Black color |
| 1 | White color |
| 250 | Empty position |

### Logic

The chosen strategy for the game is a greedy algorithm, and it has been chosen for its simplicity and low resource consumption. This approach selects the move that maximizes the player's points.

### Validation Checks

The AVR is responsible for performing various validation checks, which include:  

- Verifying whether the next move is legal.
- Detecting if the opponent made an illegal move.
- Identifying if a timeout occurs, either for the opponent or for the AVR.

## Contributors

- [Zisis Charokopos](https://github.com/zisxar)
