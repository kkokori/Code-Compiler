/*======================================================================================
Written by...: Esin Sari & Rebekah Salsburg
Date........ : 7 / 25 / 2020
Assignment.. : HW4
Purpose..... : Lexical Analyzer
Work done... : Windows Machine
======================================================================================== */

#include "headers.h"

// define constant values
#define NORW 14   // number of reserved words
#define NUMSYM 12 // number of valid symbols

typedef struct specialSymbols
{
    char symbol;
    int token;
} specialSymbols;

typedef struct reservedWords
{
    char word[20];
    int token;
} reservedWords;

typedef struct lexemeTable
{
    char lexeme[CMAX]; // stores lexeme as string
    int numLex;        // stores constants in integer form
    int tokenType;
} lexemeTable;

char lexTemp[MAX_CODE_LENGTH] = "";
char lexOutput[10000] = "";

// Declaration of Functions
void displayLexeme(lexemeTable lexemes[], int codeLength, FILE *outputfile);
void displaySourceCode(char sourceCode[], int codeLength, FILE *outputfile);
void errorMessage(FILE *inputfile, FILE *outputfile, char *message);
int isComment(char sourceCode[], int i, int codeLength);

char *lexAnalyzer(char *filename)
{
    specialSymbols ssym[NUMSYM] = {
        {'+', plussym},
        {'-', minussym},
        {'*', multsym},
        {'/', slashsym},
        {'(', lparentsym},
        {')', rparentsym},
        {'=', eqlsym},
        {',', commasym},
        {'.', periodsym},
        {'<', lessym},
        {'>', gtrsym},
        {';', semicolonsym},
    };

    reservedWords rwords[NORW] = {
        {"const", constsym},
        {"var", varsym},
        {"procedure", procsym},
        {"call", callsym},
        {"if", ifsym},
        {"then", thensym},
        {"else", elsesym},
        {"while", whilesym},
        {"do", dosym},
        {"read", readsym},
        {"write", writesym},
        {"odd", oddsym},
        {"begin", beginsym},
        {"end", endsym}};

    lexemeTable lexemes[MAX_CODE_LENGTH]; // tracks lexemes

    char ch;                                //temp char var
    char sourceCode[MAX_CODE_LENGTH];       // source code as chars
    int codeLength = 0;                     // total chars in code
    int lexemeCount = 0;                    //total number of lexemes
    int commentIndex = 0;                   // holds end of comment index
    int index = 0;                          // index of loop through code
    char digitTemp[MAX_CODE_LENGTH] = "";   // current inspected digit during loop
    char currentWord[MAX_CODE_LENGTH] = ""; // holds current inspected word during loop

    // FILE *inputfile = fopen(filename, "r");
    FILE *inputfile = fopen(filename, "r");
    FILE *outputfile = fopen("lexAnalyzerOutput.txt", "w");

    if (inputfile == NULL || outputfile == NULL) // check for file failure
    {
        sprintf(lexTemp, "ERROR: File could not be opened.");
        strcat(lexOutput, lexTemp);
        if (outputfile != NULL)
        {
            fprintf(outputfile, "ERROR: File could not be opened.");
            fclose(outputfile);
        }
        else
            fclose(inputfile);
        exit(0);
    }

    rewind(inputfile);

    // put source code into sourceCode var
    while (fscanf(inputfile, "%c", &ch) != EOF)
    {
        sourceCode[codeLength++] = ch;
    }

    displaySourceCode(sourceCode, codeLength, outputfile);
    sprintf(lexTemp, "\nLexeme Table:\n");
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "\nLexeme Table:\n");

    sprintf(lexTemp, "Lexeme\t\tToken Type\n");
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "Lexeme\t\tToken Type\n");

    index = 0; // starting index for traversing the code

    do
    {
        // ignore whitespace
        if (isspace(sourceCode[index]) || iscntrl(sourceCode[index]))
        {
            index++;
            continue;
        }
        else if (ispunct(sourceCode[index])) // char is a symbol
        {
            commentIndex = isComment(sourceCode, index, codeLength); // check denoting a comment

            if (commentIndex != -1) // if sourceCode[index] starts a comment, returns the end index
            {
                index = commentIndex + 1;
                continue;
            }

            int symbolFound = 0;             // flags valid symbols
            for (int i = 0; i < NUMSYM; i++) // check if the symbol is in the symbol table!
            {
                // handles check for special symbol usage
                if (sourceCode[index] == ssym[i].symbol || sourceCode[index] == ':' || sourceCode[index] == '!') // symbol found in list
                {
                    symbolFound = 1;

                    // store the current symbol
                    lexemes[lexemeCount].lexeme[0] = sourceCode[index]; // since they're both single chars, no need for strcpy

                    // determine if a combo symbol <>, !=, <=, >=, :=
                    if ((index + 1) < codeLength && ispunct(sourceCode[index + 1]))
                    {
                        switch (sourceCode[index])
                        {
                        case '<':                             // neq <> or leq <=
                            if (sourceCode[index + 1] == '>') // neq
                            {
                                lexemes[lexemeCount].tokenType = neqsym;
                                lexemes[lexemeCount].lexeme[1] = sourceCode[index + 1];
                                index++;
                            }
                            else if (sourceCode[index + 1] == '=') // leq
                            {
                                lexemes[lexemeCount].tokenType = leqsym;
                                lexemes[lexemeCount].lexeme[1] = sourceCode[index + 1];
                                index++;
                            }
                            else
                                lexemes[lexemeCount].tokenType = ssym[i].token;
                            break;
                        case '!': // neq != or leq <=
                            if (sourceCode[index + 1] == '=')
                            {
                                lexemes[lexemeCount].tokenType = neqsym;
                                lexemes[lexemeCount].lexeme[1] = sourceCode[index + 1];
                                index++;
                            }
                            else
                                errorMessage(inputfile, outputfile, "Symbol does not exist");
                            break;
                        case '>': // geq >=
                            if (sourceCode[index + 1] == '=')
                            {
                                lexemes[lexemeCount].tokenType = geqsym;
                                lexemes[lexemeCount].lexeme[1] = sourceCode[index + 1];
                                index++;
                            }
                            else
                                lexemes[lexemeCount].tokenType = ssym[i].token;
                            break;
                        case ':': // becomes :=
                            if (sourceCode[index + 1] == '=')
                            {
                                lexemes[lexemeCount].tokenType = becomessym;
                                lexemes[lexemeCount].lexeme[1] = sourceCode[index + 1];
                                index++;
                            }
                            else
                                errorMessage(inputfile, outputfile, "Symbol does not exist");
                            break;
                        default: // give it the symbol's token type if it isn't a possible combo symbol
                            lexemes[lexemeCount].tokenType = ssym[i].token;
                            break;
                        } // end switch case
                    }
                    else // this is the last char of the code or the next char is not a symbol
                    {
                        if (sourceCode[index] == ':' || sourceCode[index] == '!')
                            errorMessage(inputfile, outputfile, "Symbol does not exist");

                        lexemes[lexemeCount].tokenType = ssym[i].token;
                    }
                    index++;
                    lexemeCount++;
                    break;
                }
            }

            if (!symbolFound) // not a valid symbol
                errorMessage(inputfile, outputfile, "Symbol does not exist");
        }
        else if (isalpha(sourceCode[index])) // first char is a LETTER
        {
            int letterCount = 0;               // count number of letters being input
            while (isalnum(sourceCode[index])) // identifiers can be alphanumeric
            {
                currentWord[letterCount] = sourceCode[index];
                index++;
                letterCount++;
            }

            currentWord[letterCount++] = '\0'; // assign null sentinel to denote end of word

            if (letterCount > CMAX) // invalid word
                errorMessage(inputfile, outputfile, "Identifier is over 11 characters long");
            else // valid word
            {
                strcpy(lexemes[lexemeCount].lexeme, currentWord);

                for (int i = 0; i < NORW; i++) // check if keyword
                {
                    if (strcmp(rwords[i].word, currentWord) == 0)
                    {
                        lexemes[lexemeCount].tokenType = rwords[i].token;
                        break;
                    }
                    else
                        lexemes[lexemeCount].tokenType = identsym;
                }
                lexemeCount++;
            }
        }
        else if (isdigit(sourceCode[index])) // char is a number
        {
            int digitCount = 0;

            while (isdigit(sourceCode[index]))
            {
                digitTemp[digitCount] = sourceCode[index];
                index++;
                digitCount++;
            }

            digitTemp[digitCount++] = '\0';

            if (isalpha(sourceCode[index])) // if next char is a letter (indicating an identifier)
                errorMessage(inputfile, outputfile, "Identifiers cannot start with a number");
            else if (digitCount <= DMAX)
            {
                strcpy(lexemes[lexemeCount].lexeme, digitTemp);
                lexemes[lexemeCount].numLex = atoi(digitTemp);
                lexemes[lexemeCount].tokenType = numbersym;
                lexemeCount++;
            }
            else
                errorMessage(inputfile, outputfile, "Number is over 5 digits long");
        }

        if (lexemes[lexemeCount - 1].tokenType == numbersym)
        {
            sprintf(lexTemp, "%d", lexemes[lexemeCount - 1].numLex);
            strcat(lexOutput, lexTemp);
            fprintf(outputfile, "%d", lexemes[lexemeCount - 1].numLex);
        }
        else
        {
            sprintf(lexTemp, "%s", lexemes[lexemeCount - 1].lexeme);
            strcat(lexOutput, lexTemp);
            fprintf(outputfile, "%s", lexemes[lexemeCount - 1].lexeme);
        }

        sprintf(lexTemp, "\t\t%d\n", lexemes[lexemeCount - 1].tokenType);
        strcat(lexOutput, lexTemp);
        fprintf(outputfile, "\t\t%d\n", lexemes[lexemeCount - 1].tokenType);

    } while (index < codeLength);

    displayLexeme(lexemes, lexemeCount, outputfile);
    fclose(inputfile);
    fclose(outputfile);

    return lexOutput;
}

void errorMessage(FILE *inputfile, FILE *outputfile, char *message)
{
    sprintf(lexTemp, "ERROR: %s\n", message);
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "ERROR: %s\n", message);
    fclose(inputfile);
    fclose(outputfile);
    exit(0);
}

void displaySourceCode(char sourceCode[], int codeLength, FILE *outputfile)
{
    printf("Source Program: \n");
    sprintf(lexTemp, "Source Program: \n");
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "Source Program: \n");

    for (int i = 0; i < codeLength; i++)
    {
        printf("%c", sourceCode[i]);
        sprintf(lexTemp, "%c", sourceCode[i]);
        strcat(lexOutput, lexTemp);
        fprintf(outputfile, "%c", sourceCode[i]);
    }

    printf("\n");
    sprintf(lexTemp, "\n");
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "\n");
}

void displayLexeme(lexemeTable lexemes[], int lexemeCount, FILE *outputfile)
{
    FILE *out = fopen("tokens.txt", "w");
    if (out == NULL)
    {
        sprintf(lexTemp, "Unable to write tokens to file.");
        strcat(lexOutput, lexTemp);
    }
    sprintf(lexTemp, "Lexeme List:\n");
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "Lexeme List:\n");

    for (int i = 0; i < lexemeCount; i++)
    {
        sprintf(lexTemp, "%d ", lexemes[i].tokenType);
        strcat(lexOutput, lexTemp);
        fprintf(outputfile, "%d ", lexemes[i].tokenType);
        fprintf(out, "%d ", lexemes[i].tokenType);

        if (lexemes[i].tokenType == identsym) // print the identifier
        {
            sprintf(lexTemp, "%s ", lexemes[i].lexeme);
            strcat(lexOutput, lexTemp);
            fprintf(outputfile, "%s ", lexemes[i].lexeme);
            fprintf(out, "%s ", lexemes[i].lexeme);
        }
        if (lexemes[i].tokenType == numbersym) // print the constant
        {
            sprintf(lexTemp, "%d ", lexemes[i].numLex);
            strcat(lexOutput, lexTemp);
            fprintf(outputfile, "%d ", lexemes[i].numLex);
            fprintf(out, "%d ", lexemes[i].numLex);
        }
    }

    sprintf(lexTemp, "\n");
    strcat(lexOutput, lexTemp);
    fprintf(outputfile, "\n");
    fprintf(out, "\n");
    fclose(out);
}

int isComment(char sourceCode[], int i, int codeLength)
{
    if (sourceCode[i] != '/')
        return -1;
    else                                                       // detect start of a comment
        if ((i + 1) >= codeLength || sourceCode[i + 1] != '*') // we have a sequence of  /something not /*, or this is the last char in the code
        return -1;
    else // a comment started
        for (i = i + 2; i < codeLength; i++)
            if (sourceCode[i] == '*') // detect end of a comment
                if ((i + 1) < codeLength && sourceCode[i + 1] == '/')
                    return (i + 1); // return index of end of comment
}
