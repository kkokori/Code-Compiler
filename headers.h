#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#define CMAX 12 // maximum number of chars for idents is 11, plus null sentinel
#define DMAX 5  // max digits in a num
#define MAX_CODE_LENGTH 500
#define MAX_SYMBOL_TABLE_SIZE 100

typedef enum
{
    // reserved words
    nulsym = 1,
    oddsym = 8,
    beginsym = 21,
    endsym = 22,
    ifsym = 23,
    thensym = 24,
    whilesym = 25,
    dosym = 26,
    callsym = 27,
    constsym = 28,
    varsym = 29,
    procsym = 30,
    writesym = 31,
    readsym = 32,
    elsesym = 33,

    // other words and numbers
    identsym = 2,
    numbersym = 3,

    // special symbols
    plussym = 4,
    minussym = 5,
    multsym = 6,
    slashsym = 7,
    eqlsym = 9,
    lessym = 11,
    gtrsym = 13,
    lparentsym = 15,
    rparentsym = 16,
    commasym = 17,
    semicolonsym = 18,
    periodsym = 19,

    // combo symbols
    neqsym = 10,
    leqsym = 12,
    geqsym = 14,
    becomessym = 20,

} token_type;

typedef enum
{
    constant = 1,
    variable = 2,
    procedure = 3,
} symbol_kind;

typedef enum
{
    lit = 1,
    opr = 2,
    lod = 3,
    sto = 4,
    cal = 5,
    inc = 6,
    jmp = 7,
    jpc = 8,
    sio1 = 9,
    sio2 = 10,
    sio3 = 11,
} OP_codes;

typedef enum
{
    opr_ret = 0,
    opr_neg = 1,
    opr_add = 2,
    opr_sub = 3,
    opr_mul = 4,
    opr_div = 5,
    opr_odd = 6,
    opr_mod = 7,
    opr_eql = 8,
    opr_neq = 9,
    opr_lss = 10,
    opr_leq = 11,
    opr_gtr = 12,
    opr_geq = 13
} OPR_M_codes;

typedef struct
{
    int kind;        // const = 1, var = 2, proc = 3
    char name[CMAX]; // name up to 11 chars
    int val;         // number <ASCII value>
    int level;       // L level
    int addr;        // M address
    int mark;        // to indicate unavailable or delete d
} SYMBOL_TABLE;

char *vm();
char *lexAnalyzer(char *filename);
char *parser();
char *codegen();
int findClosest(char *name, int level, int mark);
int searchTable(char *name, int level, int mark);