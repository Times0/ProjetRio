#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_word(int nb,uint16_t val)
{
    printf("0b");
    for(int i = nb-1;i>=0;i--)
        printf("%d",val >> i & 1);
    printf("\n");
}

// set nth bit to value (0-1)
uint16_t set_nth_bit(int nthbit, uint16_t message, int value)//15-0
{
    uint16_t maske = value;
    return (maske <<= nthbit) | message;
}

// get nth bit
char get_nth_bit(int n, uint16_t m)//15-0
{
    return m >> n & 1;
}

// change nth bit
uint16_t chg_nth_bit(int nth, uint16_t m)//15-0
{
    uint16_t maske = 1;
    return (maske <<= nth) ^ m;
}

// give the number of 1 in message
int cardbit(uint16_t mess)
{
    int count = 0;
    for(int i = 0;i < 16;i++)
    {
        if(get_nth_bit(i,mess))
            count++;
    }
    return count;
}

// divide message by poly and return rest
uint8_t divisionpoly(uint8_t poly, uint16_t message)
{
    uint8_t registre = 0;   // registre a 0
    
    for(int i = 0;i<16;i++)
    {
        int sortant = get_nth_bit(7,registre); // bit sortant du registre apres decalage
        registre <<= 1; // decalage du registre
        
        //insertion du bit de message
        if(get_nth_bit(15-i,message))
            registre = set_nth_bit(0,registre,1);
        else
            registre = set_nth_bit(0,registre,0);

        // XOR registre-polynome
        if(sortant)
            registre = registre ^ poly;
    }
    return registre;
}

uint16_t encode(uint8_t message, uint8_t polynome)
{
    //correspond au polynome poids fort a gauche (x^n)
    uint16_t fmess = message;
    fmess <<= 8; //decale le message original

    return fmess | divisionpoly(polynome,fmess);
}

// give the Hamming distance
int distanceHamming(uint8_t polynome)
{
    int min = 16;
    for(int i = 1;i<256;i++)
    {
        int nbbit = cardbit(encode(i,polynome));
        if(nbbit < min)
            min = nbbit;
    }
    return min;
}


int trycorrect(uint16_t *message, uint8_t rest, uint8_t table[16][2])
{
    for(int i = 0;i<16;i++)
    {
        if(table[i][1] == rest)
        {
            *message = chg_nth_bit(i,*message);
            return 0;
        }
    }
    return -1;
}

// renvoie le mot originel et place dans reste si non NULL le reste de la division
uint8_t decode(uint16_t message, uint8_t polynome, uint8_t table[16][2])
{
    //correspond au polynome poids fort gauche (x^n)
    uint8_t rest = divisionpoly(polynome,message);
    if(rest)
    {
        if(trycorrect(&message,rest,table) == -1)
        {
            printf("Error CRC\n");
        }
    }
    return message >> 8;
}

void tablerreur(uint8_t polynome, uint8_t table[16][2])
{
    uint16_t message = 0b000000000;
    uint8_t reste;
    for(int i = 0;i<16;i++)
    {
        uint16_t encoded = encode(message,polynome);
        encoded = chg_nth_bit(i,encoded);
        table[i][0] = i;
        table[i][1] = divisionpoly(polynome,encoded);
    }
}


/*int main(int argc, char const *argv[])
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
    
    
    //printf("La distance de Hamming du code est %d\n",distanceHamming(polynome));
    
    return 0;
}*/