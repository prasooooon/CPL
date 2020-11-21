/*
Compile using `clang -triangle.c -std=c99 -o triangle` in a unix based terminal.
* `-std=c99` specifies the language standard to compile for. (my IDE compiles and tests for C99).
* `-o` writes output to file 'triangle'.
Type `./triangle; echo $?` in terminal.
* `./triangle` runs the 'triangle' script compiled from the previous command.
* `echo $?` returns the exit code of the last run process (here, the 'triangle' script).
*/

// Checks if first parameter is positive integer. If it is, print inverted half pyramid; if not, error.
// Return integers greater than 255 are not printed on console properly. Linux limits exit codes from 0 to 255.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void printTriangle(int nrows);  // prototype function because compiler reads from top to bottom

int main (int argc, char *argv[])
{

    long literalVal, rows;
    int digits;

    if (argc <= 1) // check if any argument passed
    {
        fprintf(stderr, "main: No argument passed. \n");
        exit(1);
    }

    literalVal = atoi(argv[1]);

    digits = floor(log10(literalVal)) + 1;
    if (strlen(argv[1]) != digits)  // verify first parameter is entirely number and not ex: 10a
    {
        fprintf(stderr, "main: Is first parameter partly string? \n");
        exit(1);
    }

    if (literalVal > 0)
    {
        rows = literalVal;
    }
    else
        {
            fprintf(stderr, "main: Positive integer required as first parameter. \n");
            exit(1);
        }

    printTriangle (rows);

    long b;
    b = rows + (rows * (rows - 1)/2);
    return b;
}

void printTriangle(int nrows)
{
    for(long int i=nrows ; i>0 ; i--) // draw triangle
    {
        for(long int j=i ; j >= 1 ;j--)
        {
            printf("*");
        }
        printf("\n");
    }
    return;
}
