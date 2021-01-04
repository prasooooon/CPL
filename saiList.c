//
// Created by Prasoon Dwivedi on 20.12.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"


tListItem * LI_create( char * strLine )
{
    tListItem * pTmp;  // temporary pointer on tListItem
    pTmp = (tListItem * )malloc(sizeof(tListItem));
    if (pTmp == NULL)
    {
        return NULL;
    }
    // assign NULL to all structure members
    (void) memset( (void *)pTmp, 0, sizeof(tListItem)); // void cast before function implies we don't need the return value of memset
    {
        int iLen = strlen(strLine);
        int iSize = iLen + 1;    // tailing 0 at the end of every string

        pTmp->pLine = (char *)malloc(iSize);
        (void)memset( (void *)pTmp->pLine , 0, iSize);

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

void L_free(tListItem * pItem){
    while (pItem != NULL)
    {
        tListItem * pN = pItem->pN;

        LI_free(pItem);

        pItem = pN;
    }
}

void L_PrintForward(tListItem * pItem) {
    while (pItem != NULL) {
        printf("LI: %s", pItem->pLine);

        pItem = pItem->pN;
    }
}

void L_PrintBackward(tListItem * pItem) {
    while (pItem != NULL) {
        printf("LI: %s", pItem->pLine);

        pItem = pItem->pP;
    }
}

tListItem * L_findEnd(tListItem * pItem) {
    if (pItem != NULL)
    {
        while (pItem->pN != NULL)
        {
            pItem = pItem->pN;
        }
        return pItem;
    }
    return NULL;
}

tListItem * L_loadFromFile(char * strFileName) {
    tListItem *pFirstLI;
    tListItem *pTmp;
    tListItem *pNew;

    FILE *pFile = fopen(strFileName, "r");
    if (pFile == NULL)
        return NULL;


    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int iCnt = 0;

    while ((read = getline(&line, &len, pFile)) != -1)  // read file line by line
    {

        if (len < 2) continue;

//        line [len] == 0
//        line [len - 1] == '\n'
//        line [len - 2] == '\r'

        line[len - 2] = 0;     // get rid of \r\n unless line has 0 len

        pNew = LI_create(line);   // create variable item in memory

        if (iCnt == 0) {
            pFirstLI = pNew;        // save first variable for the first line
            pTmp = pNew;            // save temporary value (pNew)
        } else {
            pTmp->pN = pNew;
            pNew->pP = pTmp;
            pTmp = pNew;
        }
        iCnt++;
    }
    free(line);
    fclose(pFile);

    return pFirstLI;    //return pointer to first list item
}

int localFindColon(char * str) {

    int iCnt = 0;
    int iRetVal = -1;

    while(str[iCnt] != 0)
    {
        if(str[iCnt] == ':')
        {
            return iCnt;
        }
        iCnt++ ;
    }
    return iRetVal;
}

int L_processIncludes(tListItem * pItem) {

    int iFilesFound = 0;

    if (pItem == NULL) return 0;
    while (pItem != NULL)
    {
        char * pInclude = strstr(pItem->pLine, "#include:");
        if (pInclude)
        {
            int iFirstSemicolon = localFindColon(pItem->pLine);
            if (iFirstSemicolon > 0 )
            {
                int iSecondSemicolon = localFindColon(&pItem->pLine[iFirstSemicolon + 1]);
                if (iSecondSemicolon > 0)
                {
                    char strFileName[255];
                    memset (strFileName, 0, 255);
                    strncpy(strFileName, &pItem->pLine[iFirstSemicolon + 1], iSecondSemicolon );

                    // local block to handle linking of new list into existing list
                    {

                        tListItem * pLocalFirstLI;
                        tListItem * pLocalLastLI;

                        pLocalFirstLI = L_loadFromFile(strFileName);

                        if (pLocalFirstLI)
                        {
                            pLocalLastLI = L_findEnd(pLocalFirstLI);

                            pItem->pP->pN = pLocalFirstLI;
                            pLocalFirstLI->pP = pItem->pP;

                            pLocalLastLI->pN = pItem->pN;
                            pItem->pN->pN = pLocalLastLI;

                            LI_free(pItem);
                        }   // END if

                    }   // END local block

                }   // END if (iSecondSemicolon > 0)

            }  // END  if (iFirstSemicolon > 0 )

            iFilesFound++ ;
        }
        pItem = pItem->pN;
    }   // END while pItem != NULL
    return iFilesFound;
}

tListItem * L_findLabel(tListItem * pItem, char * strLabel ) {

    if (pItem == NULL) return 0;

    while (pItem != NULL) {

        char * pInclude = strstr(pItem->pLine, "#label:");

        if (pInclude)
        {
            // #label:LabelIntro:

            int iFirstSemicolon = localFindColon(pItem->pLine);
            if (iFirstSemicolon > 0 )
            {
                int iSecondSemicolon = localFindColon(&pItem->pLine[iFirstSemicolon + 1]);
                if (iSecondSemicolon > 0)
                {
                    char strLocalLabel[255];
                    memset (strLocalLabel, 0, 255);
                    strncpy(strLocalLabel, &pItem->pLine[iFirstSemicolon + 1], iSecondSemicolon );

                    printf("Label found %s \n", strLocalLabel);


                    if (strcmp( strLocalLabel, strLabel) == 0 )
                    {
                        return pItem;
                    }
                }   // END if (iSecondSemicolon > 0)

            }  // END  if (iFirstSemicolon > 0 )
        }
        pItem = pItem->pN;
    }
    return NULL;
}
