#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tListItem
{
    char * pLine;               // pointer on line, holds information about line

    struct tListItem *pP;       // pointer on previous structure
    struct tListItem *pN;

} tListItem;

tListItem * LI_create( char * strLine )
{
    tListItem  * pTmp;  // temporary pointer on tListItem
    pTmp = (tListItem * )malloc(sizeof(tListItem));
    if (pTmp == NULL)
    {
        return NULL;
    }
            // option 1 for assigning members of structure NULL
    (void) memset( (void *)pTmp, 0, sizeof(tListItem)); // void cast before function implies we don't need the return value of memset
            // option 2
    pTmp->pLine = NULL;
    pTmp->pP = NULL;
    pTmp->pN = NULL;

    {   // get length of string
        int iLen = strlen(strLine);
        int iSize = iLen + 1;    // because of tailing 0 at the end of every string

        pTmp->pLine = (char *)malloc(iSize);
        (void) memset( (void *)pTmp->pLine , 0, iSize);

        if (pTmp->pLine == NULL)    // in case memory wasn't assigned properly by malloc
        {
            free(pTmp);
            return NULL;
        }
        strncpy(pTmp->pLine, strLine, iLen);    // last parameter iLen is number of bytes to be copied
        pTmp->pLine[iSize - 1] = 0;             // putting tailing = on last position
    }

    return pTmp;
}

void LI_free(tListItem * pItem)         // free every memory that we ask for
{
    if(pItem != NULL)
    {
        if (pItem->pLine != NULL)
        {
            free(pItem->pLine);
            pItem->pLine = NULL;
        }

        free(pItem);
    }

}


int main (int argc, char * argv[])
{
    tListItem * pFirstLI;
    tListItem * pTmp;
    tListItem * pNew;

    char strFirstLine[] = "First Line";

    pNew = LI_create("First Line");
    pFirstLI = pNew;
    pNew->pN = NULL;
    pNew->pP = NULL;
    pTmp = pNew;

    pNew = LI_create("Second Line");
    pTmp->pN = pNew;
    pNew->pP = pTmp;
    pNew->pN = NULL;
    pTmp = pNew;

    pNew = LI_create("Third Line");
    pTmp->pN = pNew;
    pNew->pP = pTmp;
    pNew->pN = NULL;
    pTmp = pNew;

    pNew = LI_create("Fourth Line");
    pTmp->pN = pNew;
    pNew->pP = pTmp;
    pNew->pN = NULL;
    pTmp = pNew;


    pTmp = pFirstLI;
    while (pTmp != NULL)
    {
        printf("Item: %s\n", pTmp->pLine);

        pTmp = pTmp->pN;
    }

    pTmp = pFirstLI;
    while (pTmp != NULL)
    {
        tListItem * pN = pTmp->pN;

        LI_free(pTmp);

        pTmp = pN;
    }

    return EXIT_SUCCESS;
}


