/*======================================================================================
Written by...: Esin Sari & Rebekah Salsburg
Date........ : 7 / 25 / 2020
Assignment.. : HW4
Purpose..... : Parser
Work done... : Windows Machine
======================================================================================== */

// ICG: https://codeshare.io/5XxlVn
// PARSER: https://codeshare.io/G617pp
// VM: https://codeshare.io/29W7E1
// LEX: https://codeshare.io/G77LRb
// HEADERS: https://codeshare.io/5Xg4Dj
// COMPILER DRIVER: https://codeshare.io/G8ldQE

#include "headers.h"

typedef struct
{
    char value[CMAX]; // Value of identifier
    char type[DMAX];  // Token type
    int numType;      // converted to int
} TOKEN;

FILE *inputfile;
FILE *outputfile;
SYMBOL_TABLE symbol_table[MAX_SYMBOL_TABLE_SIZE];
TOKEN token;
char parserOutput[10000] = "";
char parserTemp[MAX_CODE_LENGTH] = "";
int tableAddress;
int table_size;
void syntaxError(int errorCode);
void symboltable_insert(int k, char *name, char *val, int level, int addr, int mark);
int rel_op();
void getToken();

//Recursive Descent Parser Methods
void program();
void block(int level);
void const_dec(int level, int *identNum);
void var_dec(int level, int *identNum);
void proc_dec(int level, int *identNum);
void statement(int level);
void condition(int level);
void expression(int level);
void term(int level);
void factor(int level);
void printSymbols();

char *parser()
{
    tableAddress = 4;

    inputfile = fopen("tokens.txt", "r");
    outputfile = fopen("parserOutput.txt", "w");
    rewind(inputfile);

    program();

    sprintf(parserTemp, "No errors. Program is syntactically correct!\n\n");
    strcat(parserOutput, parserTemp);
    fprintf(outputfile, "No errors. Program is syntactically correct!\n\n");

    fclose(inputfile);
    fclose(outputfile);

    return parserOutput;
}

void printSymbols()
{
    int kind;        // const = 1, var = 2, proc = 3
    char name[CMAX]; // name up to 11 chars
    int val;         // number <ASCII value>
    int level;       // L level
    int addr;        // M address
    int mark;        // to indicate unavailable or delete d

    printf("\t\tKIND\t\tVAL\t\tLVL\t\tADDR\t\tMARK\n");
    for (int i = 0; i < table_size; i++)
    {

        printf("%s\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d", symbol_table[i].name, symbol_table[i].kind, symbol_table[i].val, symbol_table[i].level, symbol_table[i].addr, symbol_table[i].mark);
        printf("\n-------------------------------------------------------------------------------------------\n\n\n");
    }
}

int searchTable(char *name, int level, int mark)
{
    if (mark == -1) // search by name/level only
    {
        for (int i = table_size - 1; i >= 0; i--)
        {
            if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].level == level)
                return i;
        }
    }
    else if (mark == 0) // search for unmarked
    {
        for (int i = table_size - 1; i >= 0; i--)
        {
            if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].level == level && symbol_table[i].mark == mark)
                return i;
        }
    }
    return -1;
}

int findClosest(char *name, int level, int kind)
{
    if (kind != 0)
    {
        level = level + 1;
        for (int i = table_size - 1; i >= 0; i--)
        {

            if (symbol_table[i].level > level || symbol_table[i].mark != 0 || symbol_table[i].kind != procedure)
                continue;

            if (strcmp(symbol_table[i].name, name) == 0)
                return i;
        }
    }
    else
    {
        for (int i = table_size - 1; i >= 0; i--)
        {
            if (symbol_table[i].level > level || symbol_table[i].mark != 0 || symbol_table[i].kind == procedure)
                continue;

            if (strcmp(symbol_table[i].name, name) == 0)
                return i;
        }
    }

    return -1;
}

void symboltable_insert(int k, char *name, char *val, int level, int addr, int mark)
{
    if (table_size >= MAX_SYMBOL_TABLE_SIZE)
        syntaxError(6);

    if (searchTable(name, level, 0) != -1)
        syntaxError(1);

    int intVal = atoi(val);
    symbol_table[table_size].kind = k;
    strcpy(symbol_table[table_size].name, name);
    symbol_table[table_size].level = level;
    symbol_table[table_size].val = intVal;
    symbol_table[table_size].addr = addr;
    symbol_table[table_size].mark = mark;
    table_size++;
}

int rel_op()
{
    /*
	rel-op ::= "="|“<>"|"<"|"<="|">"|">=“.
	*/

    if (token.numType == eqlsym) //eqslym
        return opr_eql;
    else if (token.numType == neqsym) //neqsym
        return opr_neq;
    else if (token.numType == lessym) //lessym
        return opr_lss;
    else if (token.numType == leqsym) //leqsym
        return opr_leq;
    else if (token.numType == gtrsym) //gtrsym
        return opr_gtr;
    else if (token.numType == geqsym) //geqsym
        return opr_geq;
    else
        return -1; //not relational operation
}

// Function that takes the next token from inputfile
void getToken()
{
    // reset the token
    token.numType = -1;
    strcpy(token.value, "");
    strcpy(token.type, "");
    char t;

    if (fscanf(inputfile, "%s", token.type) != EOF)
    {
        token.numType = atoi(token.type);

        if (token.numType == identsym || token.numType == numbersym)
        {
            fscanf(inputfile, "%s", token.value);
        }
        else
        {
            //token is not identsym or numbersym
            token.value[0] = '\0';
        }
    }
}

/*******RECURSIVE DESCENT PARSER METHODS*******/

void program()
{
    /*
	program ::= block "." .
	*/
    int level = 0;
    getToken();
    block(level);

    if (token.numType != periodsym) //periodsym
        syntaxError(8);

    getToken();
    if (token.numType != -1)
        syntaxError(20);
}

void block(int level)
{
    int identNum = 0;
    char token_name[CMAX];

    const_dec(level, &identNum);
    var_dec(level, &identNum);
    proc_dec(level + 1, &identNum);
    int startMarks = table_size;
    statement(level);

    int marked = 0;
    for (int i = startMarks - 1; marked < identNum; i--)
    {
        if (!symbol_table[i].mark)
        {
            symbol_table[i].mark = 1;
            marked++;
        }
    }
}

void const_dec(int level, int *identNum)
{
    char token_name[CMAX];
    if (token.numType == constsym) //constsym
    {
        do
        {
            getToken();
            if (token.numType != identsym) //identsym
                syntaxError(4);

            strcpy(token_name, token.value);

            getToken();
            if (token.numType != eqlsym) //eqsym
                syntaxError(3);

            getToken();
            if (token.numType != numbersym) //numbersym
                syntaxError(2);

            // inserting const into symbol table
            symboltable_insert(constant, token_name, token.value, level, 0, 0);
            (*identNum)++;

            getToken();

            if (token.numType == identsym)
                syntaxError(5);

        } while (token.numType == commasym); //commasym

        if (token.numType != semicolonsym) //semicolomsym
            syntaxError(5);

        getToken();
    }
}

void var_dec(int level, int *identNum)
{
    if (token.numType == varsym) //varsym
    {
        int numVars = 0;
        do
        {
            numVars++;
            getToken();

            if (token.numType != identsym) //identsym
                syntaxError(4);

            symboltable_insert(variable, token.value, "0", level, numVars + 3, 0);
            tableAddress++;
            (*identNum)++;

            getToken();

            if (token.numType == identsym)
                syntaxError(5);

        } while (token.numType == commasym); //commasym

        if (token.numType != semicolonsym) //semicolomsym
            syntaxError(5);

        getToken();
    }
}

void proc_dec(int level, int *identNum)
{
    if (token.numType == procsym)
    {
        do
        {
            getToken();

            if (token.numType != identsym)
                syntaxError(4);

            // Procedure insertion to the symbol table
            symboltable_insert(procedure, token.value, "0", level, 0, 0);
            (*identNum)++;

            getToken();

            if (token.numType != semicolonsym)
                syntaxError(5);

            getToken();
            block(level);

            if (token.numType != semicolonsym)
                syntaxError(5);

            getToken();
        } while (token.numType == procsym);
    }
}

void statement(int level)
{
    if (token.numType == identsym) //identsym
    {
        int index = findClosest(token.value, level, 0);
        if (index == -1)
            syntaxError(9); // identifier not declared

        if (symbol_table[index].kind != variable)
            syntaxError(11);

        getToken();

        if (token.numType != becomessym) //becommessym
            syntaxError(7);

        getToken();

        expression(level);
    }
    else if (token.numType == callsym)
    {
        getToken();

        if (token.numType != identsym)
            syntaxError(4);

        int index = findClosest(token.value, level, procedure);

        if (index == -1)
            syntaxError(19);

        getToken();
    }
    else if (token.numType == beginsym) //beginsym
    {

        getToken();
        statement(level);

        if (token.numType != semicolonsym)
            syntaxError(10);
        while (token.numType == semicolonsym) //semicolonsym
        {
            getToken();
            statement(level);
        }
        if (token.numType != endsym) //endsym
            syntaxError(17);

        getToken();
    }
    else if (token.numType == ifsym) //ifsym
    {
        getToken();
        if (token.numType == lparentsym)
            syntaxError(18);
        condition(level);

        if (token.numType != thensym) //thensym
            syntaxError(16);

        getToken();

        statement(level);
        if (token.numType == elsesym)
        {
            getToken();
            statement(level);
        }
    }
    else if (token.numType == whilesym) //whilesym
    {
        getToken();
        if (token.numType == lparentsym)
            syntaxError(18);
        condition(level);

        if (token.numType != dosym) //dosym
            syntaxError(15);

        getToken();
        statement(level);
    }
    else if (token.numType == readsym)
    {
        getToken();

        if (token.numType != identsym)
            syntaxError(4);

        int index = findClosest(token.value, level, 0);

        if (index == -1)
            syntaxError(9);

        if (symbol_table[index].kind != variable)
            syntaxError(21);

        getToken();
    }
    else if (token.numType == writesym)
    {
        getToken();
        expression(level);
    }
}

void condition(int level)
{
    if (token.numType == oddsym) //oddsym
    {
        getToken();
        expression(level);
    }
    else
    {
        expression(level);
        int operator= rel_op();

        if (operator== - 1)
            syntaxError(14);

        getToken();
        expression(level);
    }
}

void expression(int level)
{
    if (token.numType == plussym || token.numType == minussym) // plussym or minnusym
        getToken();

    term(level);

    while (token.numType == plussym || token.numType == minussym) // plussym or minnusym
    {
        getToken();
        term(level);
    }
}

void term(int level)
{
    factor(level);

    while (token.numType == multsym || token.numType == slashsym) // multsym or slashsym
    {
        getToken();
        factor(level);
    }
}

void factor(int level)
{
    if (token.numType == identsym)
    {
        int index = findClosest(token.value, level, 0);
        if (index == -1)
            syntaxError(9);

        getToken();
    }
    else if (token.numType == numbersym)
    {
        getToken();
    }
    else if (token.numType == lparentsym)
    {
        getToken();
        expression(level);

        if (token.numType != rparentsym)
            syntaxError(13);

        getToken();
    }
    else
    {
        syntaxError(12);
    }
}

void syntaxError(int errorCode)
{
    switch (errorCode)
    {
    case 1:
        printf("\nERROR 1: Duplicate identifiers are not allowed");
        fprintf(outputfile, "\nERROR 1: Duplicate identifiers are not allowed");
        sprintf(parserTemp, "\nERROR 1: Duplicate identifiers are not allowed");
        strcat(parserOutput, parserTemp);

        break;
    case 2:
        printf("\nERROR 2: \"=\" must be followed by a positive number");
        fprintf(outputfile, "\nERROR 2: \"=\" must be followed by a positive number");
        sprintf(parserTemp, "\nERROR 2: \"=\" must be followed by a positive number");
        strcat(parserOutput, parserTemp);

        break;
    case 3:
        printf("\nERROR 3: Constant identifiers must be followed by a \"=\"");
        fprintf(outputfile, "\nERROR 3: Constant identifiers must be followed by a \"=\"");
        sprintf(parserTemp, "\nERROR 3: Constant identifiers must be followed by a \"=\"");
        strcat(parserOutput, parserTemp);

        break;
    case 4:
        printf("\nERROR 4: Expected an identifier");
        fprintf(outputfile, "\nERROR 4: Expected an identifier");
        sprintf(parserTemp, "\nERROR 4: Expected an identifier");
        strcat(parserOutput, parserTemp);

        break;
    case 5:
        printf("\nERROR 5: Semicolon or comma missing");
        fprintf(outputfile, "\nERROR 5: Semicolon or comma missing");
        sprintf(parserTemp, "\nERROR 5: Semicolon or comma missing");
        strcat(parserOutput, parserTemp);

        break;
    case 6:
        printf("\nERROR 6: Symbol table full");
        fprintf(outputfile, "\nERROR 6: Symbol table full");
        sprintf(parserTemp, "\nERROR 6: Symbol table full");
        strcat(parserOutput, parserTemp);

        break;
    case 7:
        printf("\nERROR 7: Variable identifiers must be followed by a \":=\"");
        fprintf(outputfile, "\nERROR 7: Variable identifiers must be followed by a \":=\"");
        sprintf(parserTemp, "\nERROR 7: Variable identifiers must be followed by a \":=\"");
        strcat(parserOutput, parserTemp);
        break;
    case 8:
        printf("\nERROR 8: Period expected");
        fprintf(outputfile, "\nERROR 8: Period expected");
        sprintf(parserTemp, "\nERROR 8: Period expected");
        strcat(parserOutput, parserTemp);

        break;
    case 9:
        printf("\nERROR 9: Undeclared or marked identifier");
        fprintf(outputfile, "\nERROR 9: Undeclared identifier");
        sprintf(parserTemp, "\nERROR 9: Undeclared identifier");
        strcat(parserOutput, parserTemp);

        break;
    case 10:
        printf("\nERROR 10: Semicolon between statements missing");
        fprintf(outputfile, "\nERROR 10: Semicolon between statements missing");
        sprintf(parserTemp, "\nERROR 10: Semicolon between statements missing");
        strcat(parserOutput, parserTemp);

        break;
    case 11:
        printf("\nERROR 11: Assignment to constant or procedure is not allowed");
        fprintf(outputfile, "\nERROR 11: Assignment to constant or procedure is not allowed");
        sprintf(parserTemp, "\nERROR 11: Assignment to constant or procedure is not allowed");
        strcat(parserOutput, parserTemp);

        break;
    case 12:
        printf("\nERROR 12: Factor expected");
        fprintf(outputfile, "\nERROR 12: Factor expected");
        sprintf(parserTemp, "\nERROR 12: Factor expected");
        strcat(parserOutput, parserTemp);

        break;
    case 13:
        printf("\nERROR 13: Right parenthesis missing");
        fprintf(outputfile, "\nERROR 13: Right parenthesis missing");
        sprintf(parserTemp, "\nERROR 13: Right parenthesis missing");
        strcat(parserOutput, parserTemp);
        break;
    case 14:
        printf("\nERROR 14: Relational operator expected");
        fprintf(outputfile, "\nERROR 14: Relational operator expected");
        sprintf(parserTemp, "\nERROR 14: Relational operator expected");
        strcat(parserOutput, parserTemp);

        break;
    case 15:
        printf("\nERROR 15: \"do\" expected");
        fprintf(outputfile, "\nERROR 15: \"do\" expected");
        sprintf(parserTemp, "\nERROR 15: \"do\" expected");
        strcat(parserOutput, parserTemp);

        break;
    case 16:
        printf("\nERROR 16: \"then\" expected");
        fprintf(outputfile, "\nERROR 16: \"then\" expected");
        sprintf(parserTemp, "\nERROR 16: \"then\" expected");
        strcat(parserOutput, parserTemp);

        break;
    case 17:
        printf("\nERROR 17: \"end\" expected");
        fprintf(outputfile, "\nERROR 17: \"end\" expected");
        sprintf(parserTemp, "\nERROR 17: \"end\" expected");
        strcat(parserOutput, parserTemp);

        break;
    case 18:
        printf("\nERROR 18: Unexpected parenthesis after \"while\" or \"if\"");
        fprintf(outputfile, "\nERROR 18: Unexpected parenthesis after \"while\" or \"if\"");
        sprintf(parserTemp, "\nERROR 18: Unexpected parenthesis after \"while\" or \"if\"");
        strcat(parserOutput, parserTemp);
        break;
    case 19:
        printf("\nERROR 19: Call must be followed by a procedure identifier");
        fprintf(outputfile, "\nERROR 19: Call must be followed by a procedure identifier");
        sprintf(parserTemp, "\nERROR 19: Call must be followed by a procudure identifier");
        strcat(parserOutput, parserTemp);
        break;
    case 20:
        printf("\nERROR 20: Expected end of program");
        fprintf(outputfile, "\nERROR 20: Expected end of program");
        sprintf(parserTemp, "\nERROR 20: Expected end of program");
        strcat(parserOutput, parserTemp);
        break;
    case 21:
        printf("\nERROR 21: Cannot read a non-variable");
        fprintf(outputfile, "\nERROR 21: Cannot read a non-variable");
        sprintf(parserTemp, "\nERROR 21: Cannot read a non-variable");
        strcat(parserOutput, parserTemp);
        break;
    default:
        printf("\nERROR UNKNOWN: Generated code contains unidentified error");
        fprintf(outputfile, "\nERROR UNKNOWN: Generated code contains unidentified error");
        sprintf(parserTemp, "\nERROR UNKNOWN: Generated code contains unidentified error");
        strcat(parserOutput, parserTemp);
    }

    exit(0);
}
