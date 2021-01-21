//
// Created by Prasoon Dwivedi on 20.12.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

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
        strncpy(pTmp->pLine , strLine, iSize); // last parameter is number of bytes to be copied
        pTmp->pLine[iSize - 1] = 0; //add tailing on last position
    }

    return pTmp;
}

void LI_free(tListItem * pItem) // free every memory we ask for
{
    if(pItem != NULL)
    {
        if(pItem->pLine != NULL)
        {
            free(pItem->pLine);
            pItem->pLine = NULL;
        }
        free(pItem);
    }
}

void L_free(tListItem * pItem)
{
    while (pItem != NULL)
    {
        tListItem * pN = pItem->pN;
        LI_free(pItem);
        pItem = pN;
    }
}

void L_PrintForward(tListItem * pItem)
{
    while (pItem != NULL)
    {
        printf("node: %s", pItem->pLine);

        pItem = pItem->pN;
    }
}

void L_PrintBackward(tListItem * pItem)
{
    while (pItem != NULL)
    {
        printf("node: %s",pItem->pLine);

        pItem = pItem->pP;
    }
}

/*
 * @param: node in doubly linked list
 * @brief: find end of linked list
 * @retval: last node
*/
tListItem * L_findEnd(tListItem * pItem)
{
    if(pItem != NULL)
    {
        while (pItem->pN != NULL)
        {
            pItem = pItem->pN;
        }
        return pItem;
    }
    return NULL;
}

/*
 * @param: node in doubly linked list
 * @brief: find beginning of linked list
 * @retval: first node
*/
tListItem * L_findBeg(tListItem * pItem)
{
    if(pItem != NULL)
    {
        while (pItem->pP != NULL)
        {
            pItem = pItem->pP;
        }
        return pItem;
    }
    return NULL;
}


/*
 * @brief: open and read from file
 * @retval: pointer to first item in list
*/
tListItem * L_loadFromFile(char * strFileName)
{
    tListItem * pFirstLI;
    tListItem * pTmp;
    tListItem * pNew;

    FILE * pFile = fopen(strFileName, "r");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int iCnt = 0;

    if(pFile == NULL)
    {
        fprintf(stderr, "File thread: can't open file \"%s\".", strFileName);
        exit(1);
    }

    while ((read = getline(&line, &len, pFile)) != -1)
    {
        if(len < 2) continue;       // remove newline chars but not empty lines

//        line[len-2] = 0;            // get rid of \r\n

        pNew = LI_create(line);     // create variable item in memory

        if (iCnt == 0)              // only first line
        {
            pFirstLI = pNew;        // save first variable for the first line
            pTmp = pNew;            // save temporary value (pNew)
        } else
        {
            pTmp->pN = pNew;
            pNew->pP = pTmp;
            pTmp = pNew;
        }
        iCnt++;
    }
    free(line);
    fclose( pFile);

    return pFirstLI;
}
/*
 * @brief: find colons and their indexes, used by processIncludes function
 * @retval: index at which colon found, else -1
*/
int locFindColon(char * str)
{
    int iCnt = 0;
    int iRetVal = -1;

    while (str[iCnt] != 0)
    {
        if (str[iCnt] == ':')
        {
            return iCnt;
        }
        iCnt++;
    }

    return iRetVal;
}

/*
 * @brief: process include statements in file
 * @retval: number of includes found
*/
int L_processIncludes(tListItem * pItem)
{
    int iFilesFound = 0;

    if (pItem == NULL) return 0;
    while (pItem != NULL) //
    {
        char * pIncl = strstr(pItem->pLine, "#include:");
        if (pIncl)
        {
            int iFirstColon = locFindColon(pItem->pLine);
            if (iFirstColon > 0)
            {
                int iSecondColon = locFindColon(&pItem->pLine[iFirstColon+1]);
                if (iSecondColon > 0)
                {
                    char strFileName[255];
                    memset(strFileName, 0, 255);
                    strncpy(strFileName, &pItem->pLine[iFirstColon+1],iSecondColon);
//                    printf("File found %s \n", strFileName);
                    {
                        tListItem * pLocFirstLI;
                        tListItem * pLocLastLI;
                        pLocFirstLI = L_loadFromFile(strFileName);
                        if (pLocFirstLI)
                        {
                            pLocLastLI = L_findEnd(pLocFirstLI);

                            pItem->pP->pN = pLocFirstLI;
                            pLocFirstLI->pP = pItem->pP;

                            pLocLastLI->pN = pItem->pN;
                            pItem->pN->pP = pLocLastLI;

                            LI_free(pItem);
                        }   //END if (pLocFirstLI)

                    }       //END LOCAL BLOCK to handle new list into the existing

                }           // END if (iSecondColon > 0)

            }               //END if (iFirstColon > 0)

            iFilesFound ++;
        }
        pItem = pItem->pN; // move to next pointer
    } // END if (pItem != NULL)

    return iFilesFound ;
}

/*
 * @param: first node and label to find
 * @brief: find labels in entire list
 * @retval: node with searched label
*/
tListItem *L_findLabel(tListItem * pItem, char * strLabel )
{
    if (pItem == NULL) return 0;
    while (pItem != NULL)
    {   // scan if line contains label
        char * pIncl = strstr(pItem->pLine, "#label:");

        if (pIncl)
        {   // found something like #label:LabelLeft:

            int iFirstColon = locFindColon(pItem->pLine);
            if (iFirstColon > 0)
            {
                int iSecondColon = locFindColon(&pItem->pLine[iFirstColon+1]);
                if (iSecondColon > 0)
                {
                    char strLocLabel[255];
                    memset(strLocLabel, 0, 255);
                    strncpy(strLocLabel, &pItem->pLine[iFirstColon+1],iSecondColon);    // string between colons
//                    printf("Label found %s \n", strLocLabel);

                    if (strcmp(strLocLabel, strLabel) == 0)
                    {
                        return (pItem);
                    }
                } // END if (iSecondColon > 0)

            } //END if (iFirstColon > 0)
        }
        pItem = pItem->pN;
    } // END if (pItem != NULL)

    return NULL;
}
