#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

//////////////////////////////////
// bits numeroted in order 15-0 //
//////////////////////////////////

void print_word(int nb,uint16_t val);

// set nth bit to value (0-1)
uint16_t set_nth_bit(int nthbit, uint16_t message, int value);

// get nth bit
uint16_t get_nth_bit(int n, uint16_t m);

// change nth bit
uint16_t chg_nth_bit(int nth, uint16_t m);

// give the number of 1 in message
int cardbit(uint16_t mess);

// divide message by poly and return rest
uint8_t divisionpoly(uint8_t poly, uint16_t message);

uint16_t encode(uint8_t wrd, uint8_t poly);

// give the Hamming distance
int distanceHamming(uint8_t poly);

// try to correct message return -1 if cannot
int trycorrect(uint16_t *wrd, uint8_t rest, uint8_t table[16][2]);

// return decoded message, 6 if can't be corrected
int decode(uint16_t wrd, uint8_t poly, uint8_t table[16][2]);

// initialise the error table
void initerrortable(uint8_t poly, uint8_t table[16][2]);

int get_index(uint16_t wrd, uint16_t arr[], int size);

// polynom (x^8 isn't precised always equals to 1), most significant bit at left 
static uint8_t polynom = 0b10010011;