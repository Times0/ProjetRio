#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_word(int nb,uint16_t val);

// set nth bit to value (0-1)
// bit numeroted in 15-0
uint16_t set_nth_bit(int nthbit, uint16_t message, int value);

// get nth bit
// bit numeroted in 15-0
char get_nth_bit(int n, uint16_t m);

// change nth bit
// bit numeroted in 15-0
uint16_t chg_nth_bit(int nth, uint16_t m);

// give the number of 1 in message
int cardbit(uint16_t mess);

// divide message by poly and return rest
uint8_t divisionpoly(uint8_t poly, uint16_t message);

uint16_t encode(uint8_t message, uint8_t polynome);

// give the Hamming distance
int distanceHamming(uint8_t polynome);

// try to correct message return -1 if cannot
int trycorrect(uint16_t *message, uint8_t rest, uint8_t table[16][2]);

// renvoie le mot originel et place dans reste si non NULL le reste de la division
uint8_t decode(uint16_t message, uint8_t polynome, uint8_t table[16][2]);

void tablerreur(uint8_t polynome, uint8_t table[16][2]);


int main(int argc, char const *argv[])
{
    uint8_t polynome =  0b10010011; // 0b10010011
    uint16_t message = 'A';
    
    uint8_t table[16][2];
    tablerreur(polynome,table);

    message = encode(message,polynome);
    // print_word(16,message);
    message = chg_nth_bit(13,message);
    
    message = decode(message,polynome,table);
    printf("%c\n",message);

    // print_word(8,message);
    
    
    //printf("La distance de Hamming du code est %d\n",distanceHamming(polynome));*/
    
    return 0;
}