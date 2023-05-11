#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

void print_word(char val)
{
    printf("0b");
    for(int i = 8;i>=0;i--)
        printf("%d",val >> i & 1);
    printf("\n");
}

// set nth bit to 1
char set_nth_bit(int n, char m)//15-0
{
    char maske = 0b00000001;
    return (maske <<= n) | m;
}

// get nth bit
char get_nth_bit(int n, char m)//15-0
{
    return m >> n & 1;
}

// change nth bit
char chg_nth_bit(int n, char m)//15-0
{
    char maske = 0b00000001;
    return (maske <<= n) ^ m;
}

int main(int argc, char const *argv[])
{
    
    return 0;
}