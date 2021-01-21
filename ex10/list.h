//
// Created by Prasoon Dwivedi on 20.12.2020.
//

#ifndef CPL_LIST_H
#define CPL_LIST_H

typedef struct tListItem
{
    char * pLine;               // pointer on line, holds information about line

    struct tListItem* pP;
    struct tListItem* pN;


} tListItem;

//struct tListItem* XOR (struct tListItem *a, struct tListItem *b);

//void insert(struct tListItem **head_ref, char *data);

//void printList (struct tListItem * head);



tListItem * LI_create( char * strLine );

void LI_free(tListItem * pItem);
void L_free(tListItem * pItem);

void L_PrintForward(tListItem * pItem);
void L_PrintBackward(tListItem * pItem);

tListItem * L_findEnd(tListItem * pItem);
tListItem * L_loadFromFile(char * strFileName);

int L_processIncludes(tListItem * pItem);

tListItem * L_findLabel(tListItem * pItem, char * strLabel );

#endif //CPL_LIST_H
