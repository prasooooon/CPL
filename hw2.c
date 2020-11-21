#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char menu[] = "== program menu ==\nItem 1: Control LED\nItem 2: Read button state\nItem 3: Read joystick\nItem 4: Control display\nItem c: Enter a custom command\nItem e: Exit\nSelection:";
void printMenu (char *selection);

int main(int argc, char *argv[])
{
    if (argc <= 1) // check if any argument passed
    {   fprintf(stderr, "main: No argument passed. \n");
        exit(1);    }

    char selection[256] = "1";

    printf("Welcome! The input parameter is %s\n", argv[1]);

    printMenu(&*selection);

    fflush(stdin);
    return 1; // assuming 1 signifies True and 0 signifies False.
}

void printMenu (char *selection)
{
    int count = sizeof(menu)/sizeof(char*);
    char command[256];

    for (;;)
    {
        if (*selection == 'e') {
            exit(1);
        }

        else if (*selection == 'c') {
            printf("%s", "Enter command: ");
            scanf("%s", &*command);

            printf("\nThe command is %s \n\n", command);
            fflush(stdin);

            printf("%s", menu);
            scanf("%s", &*selection);
        }

        else if (*selection == '1' || *selection == '2' ||*selection == '3' ||*selection == '4'){            printf("%s", menu);
            scanf("%s", &*selection);
        }

        else {
            fprintf(stderr, "printMenu: selection out of bounds of menu.\n");
            exit(1);
        }
    }
    return;
}
