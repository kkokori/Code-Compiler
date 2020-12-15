/*======================================================================================
Written by...: Esin Sari & Rebekah Salsburg
Date........ : 7 / 25 / 2020
Assignment.. : HW4
Purpose..... : Compiler
Work done... : Windows Machine
======================================================================================== */

#include "headers.h"

void flagInfo();

int main(int argc, char *argv[])
{
    int l = 0, a = 0, v = 0;
    char temp;
    int i = 0;
    char filename[MAX_CODE_LENGTH];
    FILE *out = fopen("allOutput.txt", "w");
    FILE *screenOut = fopen("screenOutput.txt", "w");

    // argc == number of command line arguments
    // argc will always equal 2 if user typed ./compile <filename>.txt
    if (argc < 2) //
        flagInfo();

    strcpy(filename, argv[1]);

    // ensure the second argument is a text file
    int len = strlen(filename);
    if (filename[len - 1] != 't' && filename[len - 2] != 'x' && filename[len - 3] != 't')
        flagInfo();

    argc = argc - 2; // decrement to leave number of flags

    // set the flags
    while (argc >= 1)
    {
        temp = argv[argc + 1][1]; // sub 0 is the '-'
        switch (temp)
        {
        case 'l':
            l = 1;
            break;
        case 'a':
            a = 1;
            break;
        case 'v':
            v = 1;
            break;
        default:
            flagInfo();
            break;
        }
        argc--;
    }

    // call the functions

    char *lexOutput = lexAnalyzer(filename);
    char *parserOutput = parser();
    char *ICGOutput = codegen();
    char *vmOutput = vm();

    if (l == 1) // print lexeme list to screen
    {
        printf("\nTokens:\n%s", lexOutput);
        fprintf(screenOut, "\nTokens:\n%s", lexOutput);
    }

    if (a) // print assembly code to screen
    {
        printf("\nAssembly:\n%s", parserOutput);
        printf("\n%s", ICGOutput);
        fprintf(screenOut, "\nAssembly:\n%s", parserOutput);
        fprintf(screenOut, "\n%s", ICGOutput);
    }

    if (v) // print virtual machine execute trace to screen
    {
        printf("\nVM Execution Trace:\n%s", vmOutput);
        fprintf(screenOut, "\nVM Execution Trace:\n%s", vmOutput);
    }

    if (out != NULL)
    {
        fprintf(out, "%s", lexOutput);
        fprintf(out, "%s", parserOutput);
        fprintf(out, "%s", ICGOutput);
        fprintf(out, "%s", vmOutput);
    }
}

void flagInfo()
{
    printf("ERROR: Invalid command line argument. Please use the following format: ./compile <filename>.txt <-l> <-a> <-v>\n");
    printf("\n\t\"-l\" - : Print list of lexemes/tokens to the screen\n");
    printf("\n\t\"-a\" - : Print generated assembly code to the screen\n");
    printf("\n\t\"-v\" - : Print virtual machine execution trace to the screen\n");
    printf("\nNOTE: A text file name is REQUIRED\n");
    printf("NOTE: Flags are CASE SENSITIVE\n");
    exit(0);
}