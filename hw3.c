#include <stdio.h>
#include <stdlib.h>

typedef  struct tItem
{
    int iCount;
    int iData;
} tItem;

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        fprintf(stderr, "main: No argument passed, need output file name. \n");
        exit(2);
    }

    int tempInt;        // temporarily save integers

    FILE *pointFile = fopen(argv[1], "w");
    if (pointFile == NULL)
    {
        fprintf(stderr, "main: could not open file at %s\n" , argv[1]);
        exit(2);
    }

    struct tItem *arrayPointer = (tItem*)malloc(20*sizeof(int));

    if (arrayPointer == NULL) { return 0; }

    for (int iCounter = 0; iCounter < 20; iCounter++)
    {
        if (EOF == scanf("%d", &tempInt)) { break; }

        else
        {
            arrayPointer[iCounter].iCount = iCounter;
            arrayPointer[iCounter].iData = tempInt;
            fwrite(&arrayPointer[iCounter], 1, sizeof(tItem), pointFile);
        }
    }

    fclose(pointFile);
    free(arrayPointer); // deallocate memory previously assigned to malloc

    return 1;
}