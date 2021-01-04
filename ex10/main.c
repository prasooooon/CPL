#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

int main (int argc, char * argv[])
{
    tListItem * pFirstLI;

    pFirstLI = L_loadFromFile("main.txt");

    if (pFirstLI != NULL)
    {
        printf("Basic List\n");
        printf("-------------------------------------\n");
        L_PrintForward(pFirstLI);
        printf("-------------------------------------\n");

        printf("Extended List\n");
        printf("-------------------------------------\n");

        int iRetVal = L_processIncludes(pFirstLI);
        printf("main: Files found: %d \n", iRetVal);

        printf("-------------------------------------\n");
        L_PrintForward( pFirstLI );
        printf("-------------------------------------\n");
//        L_PrintBackward( L_findEnd(pFirstLI) );
//        printf("-------------------------------------\n");

        tListItem * pLabelLabelLeft = L_findLabel(pFirstLI, "LabelLeft" );
        printf("Label found: %s\n", pLabelLabelLeft->pLine);
        printf("Label found next: %s\n", pLabelLabelLeft->pN->pLine);

        printf("-------------------------------------\n");

        L_free(pFirstLI);
    }

    return EXIT_SUCCESS;
}


