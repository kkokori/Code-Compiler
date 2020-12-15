/*======================================================================================
Written by...: Esin Sari & Rebekah Salsburg
Date........ : 7 / 25 / 2020
Assignment.. : HW4
Purpose..... : Intermediate Code Generator
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
    int op; // opcode
    int l;  // lexicographical level
    int m;  // modifier
} ASSEMBLY_CODE;

typedef struct
{
    char value[CMAX]; // Value of identifier
    char type[DMAX];  // Token type
    int numType;      // converted to int
} TOKEN;

int cx; // code index
int mainStart = 0;
int unmarkIndex = 0;

FILE *inputfile;
FILE *outputfile;
ASSEMBLY_CODE code[MAX_CODE_LENGTH];
TOKEN token;
extern int table_size;
extern SYMBOL_TABLE symbol_table[MAX_SYMBOL_TABLE_SIZE];
char ICGOutput[10000] = "";
char ICGTemp[MAX_CODE_LENGTH] = "";

void syntaxError(int errorCode);
void init_assembly();
void emit(int op, int l, int m);
int ICG_rel_op();
void ICGgetToken();
void printassembly();

//Recursive Descent Parser Methods
void ICGprogram();
void ICGblock(int level, int index);
void ICGstatement(int level);
void ICGcondition(int level);
void ICGexpression(int level);
void ICGterm(int level);
void ICGfactor(int level);
void printSymbols();

char *codegen()
{
    cx = 0;

    inputfile = fopen("tokens.txt", "r");
    outputfile = fopen("ICGOutput.txt", "w");
    rewind(inputfile);

    init_assembly();
    ICGprogram();

    printassembly();

    fclose(inputfile);
    fclose(outputfile);
    return ICGOutput;
}

void printassembly()
{
    FILE *out = fopen("assembly.txt", "w");
    int i = 0;

    fprintf(out, "Assembly Code\n");

    while (!(code[i].op == 0 && code[i].l == 0 && code[i].m == 0))
    {
        sprintf(ICGTemp, "%d %d %d \n", code[i].op, code[i].l, code[i].m);
        strcat(ICGOutput, ICGTemp);
        fprintf(outputfile, "%d %d %d \n", code[i].op, code[i].l, code[i].m);
        fprintf(out, "%d %d %d \n", code[i].op, code[i].l, code[i].m);
        i++;
    }
    fclose(out);
}

void init_assembly()
{
    for (int i = 0; i < MAX_CODE_LENGTH; i++)
    {
        code[i].op = 0;
        code[i].l = 0;
        code[i].m = 0;
    }
}

void emit(int op, int l, int m)
{
    code[cx].op = op; // opcode
    code[cx].l = l;   // lexicographical level
    code[cx].m = m;   // modifier
    cx++;
}

int ICG_rel_op()
{
    /*
	rel-op ::= "="|â€œ<>"|"<"|"<="|">"|">=â€œ.
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
void ICGgetToken()
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

void ICGprogram()
{
    /*
	program ::= block "." .
	*/

    int level = 0;
    int numProc = 0;
    int numVar = 0;
    int varIndex = 0;
    int constIndex = 0;

    emit(jmp, 0, 0); // for main

    // get total number of all identifiers
    for (int i = table_size - 1; i >= 0; i--)
    {
        if (symbol_table[i].kind == procedure)
            numProc++;
    }

    for (int i = 0; i < numProc; i++)
        emit(jmp, 0, 0);

    ICGgetToken();
    ICGblock(level, -1);

    if (token.numType == periodsym && cx == 0) // empty program
    {
        emit(inc, 0, 4);  // creates a fake AR for program's sake
        emit(sio3, 0, 3); // end program
        return;
    }

    code[0].m = mainStart;

    int procJmps = 1;
    // set jmp values

    for (int i = table_size - 1; procJmps <= numProc; i--)
    {
        if (symbol_table[i].kind == procedure)
        {
            code[procJmps].m = symbol_table[i].addr;
            procJmps++;
        }
    }

    emit(sio3, 0, 3); //end program
}

void ICGblock(int level, int index)
{
    char token_name[CMAX];
    int numSymbols = 0;
    int numVars = 0;

    if (token.numType == constsym) //constsym
    {
        do
        {
            numSymbols++;
            ICGgetToken(); // ident
            // int index = searchTable(token.value, level, -1);
            symbol_table[unmarkIndex].mark = 0;
            unmarkIndex++;

            ICGgetToken(); // eq
            ICGgetToken(); // num
            ICGgetToken(); // possible comma

        } while (token.numType == commasym); //commasym

        ICGgetToken(); // possible var
    }

    if (token.numType == varsym) //varsym
    {
        do
        {
            numSymbols++;
            numVars++;

            ICGgetToken(); // ident
            // int index = searchTable(token.value, level, -1);
            symbol_table[unmarkIndex].mark = 0;
            unmarkIndex++;

            ICGgetToken(); // possible comma

        } while (token.numType == commasym); //commasym

        ICGgetToken(); // possible proc
    }

    if (token.numType == procsym)
    {
        int index;
        level++;
        do
        {
            numSymbols++;
            ICGgetToken(); // ident sym

            index = searchTable(token.value, level, -1);
            symbol_table[unmarkIndex].mark = 0;
            unmarkIndex++;
            int ctemp = cx;

            ICGgetToken(); // semicolon

            ICGgetToken(); // ?

            ICGblock(level, index);

            emit(opr, 0, 0); // return
            mainStart = cx;

            ICGgetToken(); // ?
        } while (token.numType == procsym);
        level--;
    }

    symbol_table[index].addr = cx;
    emit(inc, 0, numVars + 4);

    ICGstatement(level);

    int marked = 0;

    for (int i = table_size; marked < numSymbols; i--)
    {
        if (!symbol_table[i].mark)
        {
            symbol_table[i].mark = 1;
            marked++;
        }
    }
}

void ICGstatement(int level)
{
    if (token.numType == identsym) //identsym
    {
        int index = findClosest(token.value, level, 0);
        ICGgetToken(); // becomes
        ICGgetToken(); // [expression]
        ICGexpression(level);

        // stores the result of expression
        emit(sto, level - symbol_table[index].level, symbol_table[index].addr); // opcode 4
    }
    else if (token.numType == callsym)
    {
        ICGgetToken(); // ident
        int index = findClosest(token.value, level, procedure);

        emit(cal, level - symbol_table[index].level + 1, symbol_table[index].addr);

        ICGgetToken();
    }
    else if (token.numType == beginsym) //beginsym
    {
        ICGgetToken();
        ICGstatement(level);

        while (token.numType == semicolonsym) //semicolomsym
        {
            ICGgetToken();
            ICGstatement(level);
        }

        ICGgetToken();
    }
    else if (token.numType == ifsym) //ifsym
    {

        ICGgetToken();
        ICGcondition(level);
        int ctemp = cx;
        emit(jpc, 0, 0); // opcode 8

        ICGgetToken();
        ICGstatement(level);

        if (token.numType == elsesym)
        {
            ICGgetToken();
            int ctemp2 = cx;
            emit(jmp, 0, 0);
            code[ctemp].m = cx;
            ICGstatement(level);
            code[ctemp2].m = cx;
        }
        else
        {
            code[ctemp].m = cx;
        }
    }
    else if (token.numType == whilesym) //whilesym
    {

        ICGgetToken();
        int cx1 = cx;
        ICGcondition(level);
        ICGgetToken();
        int cx2 = cx;

        emit(jpc, 0, 0); // opcode 8

        ICGstatement(level);
        emit(jmp, 0, cx1); // opcode 7
        code[cx2].m = cx;
    }
    else if (token.numType == readsym)
    {
        ICGgetToken();
        int index = findClosest(token.value, 0, 0);
        ICGgetToken();
        emit(sio2, 0, 2);                                                       // read
        emit(sto, level - symbol_table[index].level, symbol_table[index].addr); // store
    }
    else if (token.numType == writesym)
    {
        ICGgetToken();
        ICGexpression(level);
        emit(sio1, 0, 1);
    }
}
void ICGcondition(int level)
{
    /*
	condition ::= "odd" expression
	| expression  rel-op  expression.
	*/
    if (token.numType == oddsym) //oddsym
    {
        ICGgetToken();
        ICGexpression(level);
        emit(opr, 0, opr_odd);
    }
    else
    {
        ICGexpression(level);
        int operator= ICG_rel_op();
        ICGgetToken();
        ICGexpression(level);
        emit(opr, 0, operator);
    }
}

void ICGexpression(int level)
{
    /*
	expression ::= [ "+"|"-"] term { ("+"|"-") term}.
	*/

    if (token.numType == plussym || token.numType == minussym) // plussym or minnusym
    {
        ICGgetToken();
        ICGterm(level);

        if (token.numType == minussym) // minnussym
            emit(opr, 0, opr_neg);     // opcode 11
    }
    else
        ICGterm(level);

    while (token.numType == plussym || token.numType == minussym) // plussym or minnusym
    {
        if (token.numType == plussym) // plussym
        {
            ICGgetToken();
            ICGterm(level);
            emit(opr, 0, opr_add); // opcode 12
        }
        else if (token.numType == minussym)
        {
            ICGgetToken();
            ICGterm(level);
            emit(opr, 0, opr_sub); // opcode 13
        }
    }
}

void ICGterm(int level)
{
    /*
	term ::= factor {("*"|"/") factor}.
	*/
    ICGfactor(level);

    while (token.numType == multsym || token.numType == slashsym) // multsym or slashsym
    {
        if (token.numType == multsym)
        {
            ICGgetToken();
            ICGfactor(level);
            emit(opr, 0, opr_mul); // opcode 14
        }
        else if (token.numType == slashsym)
        {
            ICGgetToken();
            ICGfactor(level);
            emit(opr, 0, opr_div); // opcode 15
        }
    }
}

void ICGfactor(int level)
{
    /*
	factor ::= ident | number | "(" expression ")â€œ.
	*/

    if (token.numType == identsym)
    {
        int index = findClosest(token.value, level, 0);
        if (symbol_table[index].kind == constant)
            emit(lit, 0, symbol_table[index].val);
        else if (symbol_table[index].kind == variable)
            emit(lod, level - symbol_table[index].level, symbol_table[index].addr);

        ICGgetToken();
    }
    else if (token.numType == numbersym)
    {
        emit(lit, 0, atoi(token.value));
        ICGgetToken();
    }
    else
    {
        ICGgetToken();
        ICGexpression(level);
        ICGgetToken();
    }
}