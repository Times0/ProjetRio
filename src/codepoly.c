#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../include/codepoly.h"

// print nb bits of val
void print_word(int nb,uint16_t val)
{
    printf("0b");
    for(int i = nb-1;i>=0;i--)
        printf("%d",val >> i & 1);
    printf("\n");
}

// set nth bit to value (0-1)
uint16_t set_nth_bit(int nthbit, uint16_t message, int value)
{
    uint16_t maske = value;
    return (maske <<= nthbit) | message;
}

// get nth bit of m
uint16_t get_nth_bit(int n, uint16_t m)
{
    return m >> n & 1;
}

// change nth bit of m
uint16_t chg_nth_bit(int nth, uint16_t m)
{
    uint16_t maske = 1;
    return (maske <<= nth) ^ m;
}

// give the number of 1 in message
int cardbit(uint16_t mess)
{
    int count = 0;
    for(int i = 0;i < 16;i++)
        count += get_nth_bit(i,mess);
    return count;
}

// divide message by poly and return rest
uint8_t divisionpoly(uint8_t poly, uint16_t message)
{
    uint8_t registre = 0;   // register to 0
    
    for(int i = 0;i<16;i++)
    {
        int bitleft = get_nth_bit(7,registre); // bit left after shifting
        registre <<= 1; // shift the registrer
        
        //add next bit of message in registre
        if(get_nth_bit(15-i,message))
            registre = set_nth_bit(0,registre,1);
        else
            registre = set_nth_bit(0,registre,0);

        // XOR register-polynom
        if(bitleft)
            registre = registre ^ poly;
    }
    return registre;
}

uint16_t encode(uint8_t wrd, uint8_t poly)
{
    uint16_t fmess = wrd;
    fmess <<= 8; // shift original message

    return fmess | divisionpoly(poly,fmess); // add rest
}

// give the Hamming distance
int distanceHamming(uint8_t poly)
{
    int min = 16;
    for(int i = 1;i<256;i++)
    {
        int nbbit = cardbit(encode(i,poly));
        if(nbbit < min)
            min = nbbit;
    }
    return min;
}

int trycorrect(uint16_t *wrd, uint8_t rest, uint8_t table[16][2])
{
    for(int i = 0;i<16;i++)
    {
        if(table[i][1] == rest)
        {
            *wrd = chg_nth_bit(i,*wrd);
            return 0;
        }
    }
    return -1;
}

//return -1 if message can't be decoded, uint_8 else
int decode(uint16_t wrd, uint8_t poly, uint8_t table[16][2])
{
    uint8_t rest = divisionpoly(poly,wrd);
    if(rest)
    {
        if(trycorrect(&wrd,rest,table) == -1)
            return -1;
    }
    return wrd >> 8;
}

void initerrortable(uint8_t poly, uint8_t table[16][2])
{
    uint16_t message = 0b000000000;
    for(int i = 0;i<16;i++)
    {
        uint16_t encoded = encode(message,poly);
        encoded = chg_nth_bit(i,encoded);
        table[i][0] = i;
        table[i][1] = divisionpoly(poly,encoded);
    }
}

int get_index(uint16_t wrd, uint16_t arr[], int size)
{
    for(int i = 0;i<size;i++)
    {
        if(wrd == arr[i])
            return i;
    }
    return -1;
}

#ifdef MAIN
int main(void)
{
    uint16_t wrd = 'A';
    
    uint8_t table[16][2];
    initerrortable(polynom,table);

    wrd = encode(wrd,polynom);
    wrd = chg_nth_bit(13,wrd);
    wrd = decode(wrd,polynom,table);
    
    
    printf("Hamming distance's of code is equal to %d\n",distanceHamming(polynom));
    
    return 0;
}
#endif
