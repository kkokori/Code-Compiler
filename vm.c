/*======================================================================================
Written by...: Esin Sari & Rebekah Salsburg
Date........ : 7 / 25 / 2020
Assignment.. : HW4
Purpose..... : Virtual Machine
Work done... : Windows Machine
======================================================================================== */

#include "headers.h"

// define constant values
#define MAX_DATA_STACK_HEIGHT 1000
#define EMPTY_STACK INT_MIN
#define FULL_STACK INT_MAX

typedef struct instruction
{
    int line; // line number
    int op;   //opcode
    int l;    //L
    int m;    //M
} instruction;

char vmOutput[200000] = "";
char vmTemp[MAX_CODE_LENGTH] = "";
FILE *output;
FILE *inputfile;
char opnames[11][4] = {"lit", "opr", "lod", "sto", "cal",
                       "inc", "jmp", "jpc", "sio", "sio", "sio"};

void push(int stack[], int *SP, int x);
int pop(int stack[], int *SP);
int peek(int stack[], int SP);
int full(int SP);
int empty(int SP);
int base(int stack[], int l, int base);
void printStack(int stack[], int SP, int flag, FILE *outputfile, int AR[], int calls);

//int main()
char *vm(char *filename)
{
    // initial PM/0 CPU register vals
    int SP = MAX_DATA_STACK_HEIGHT;
    int BP = SP - 1;
    int PC = 0;
    int IR = 0;
    int halt = 1;

    // other variable
    int line = 0, temp = 0, i = 0;
    int nextPC = 0, calls = 0;
    int AR[MAX_DATA_STACK_HEIGHT];
    int stack[MAX_DATA_STACK_HEIGHT] = {0};
    instruction inst[MAX_CODE_LENGTH];

    for (i = 0; i < MAX_DATA_STACK_HEIGHT; i++)
    {
        AR[i] = -1;
    }

    //Open files in.txt&out.txt for reading and writing purpose
    inputfile = fopen("ICGOutput.txt", "r");
    output = fopen("vmOutput.txt", "w");
    rewind(inputfile);

    if (inputfile == NULL || output == NULL) // check for file failure
    {
        printf("ERROR: File could not be opened.");
        sprintf(vmTemp, "ERROR: File could not be opened.");
        strcat(vmOutput, vmTemp);

        if (output != NULL)
        {
            fprintf(output, "ERROR: File could not be opened.");
            fclose(output);
        }

        fclose(inputfile);

        exit(0);
    }

    fprintf(output, "Line \tOP \tL \tM\n");
    sprintf(vmTemp, "Line \tOP \tL \tM\n");
    strcat(vmOutput, vmTemp);

    // put all instructions in the inst array
    while (fscanf(inputfile, "%d %d %d", &inst[line].op, &inst[line].l, &inst[line].m) != EOF)
    {
        inst[line].line = line;

        // prints out program in interpreted assembly with line numbers
        fprintf(output, "%1d \t%s \t%1d \t%1d\n", line, opnames[inst[line].op - 1], inst[line].l, inst[line].m);
        sprintf(vmTemp, "%1d \t%s \t%1d \t%1d\n", line, opnames[inst[line].op - 1], inst[line].l, inst[line].m);
        strcat(vmOutput, vmTemp);

        line++;
    }

    fprintf(output, "\n\n\t\t\t\t\tPC \tBP \tSP \tSTACK");
    fprintf(output, "\n\nInitial values: \t\t\t%1d \t%1d \t%1d\n", PC, BP, SP);

    sprintf(vmTemp, "\n\n\t\t\t\t\tPC \tBP \tSP \tSTACK");
    strcat(vmOutput, vmTemp);

    sprintf(vmTemp, "\n\nInitial values: \t\t\t%1d \t%1d \t%1d\n", PC, BP, SP);
    strcat(vmOutput, vmTemp);

    // line is the # lines in the whole file
    do
    {
        nextPC = PC + 1;
        if (inst[PC].op == 9)
        {
            sprintf(vmTemp, "%d\n", peek(stack, SP));
            strcat(vmOutput, vmTemp);

            fprintf(output, "%d\n", peek(stack, SP));
        }

        switch (inst[PC].op)
        {
        case 1: // LIT
            push(stack, &SP, inst[PC].m);
            break;

        case 2: // OPR
            switch (inst[PC].m)
            {
            case 0: // RET
                SP = BP + 1;
                nextPC = stack[SP - 4];
                BP = stack[SP - 3];
                calls--;
                break;
            case 1: // NEG
                stack[SP] = -stack[SP];
                break;
            case 2: // ADD
                SP++;
                stack[SP] += stack[SP - 1];
                break;
            case 3: // SUB
                SP++;
                stack[SP] -= stack[SP - 1];
                break;
            case 4: // MUL
                SP++;
                stack[SP] *= stack[SP - 1];
                break;
            case 5: // DIV
                SP++;
                stack[SP] /= stack[SP - 1];
                break;
            case 6: // ODD
                stack[SP] = stack[SP] % 2;
                break;
            case 7: // MOD
                SP++;
                stack[SP] %= stack[SP - 1];
                break;
            case 8: // EQL
                SP++;
                stack[SP] = (stack[SP] == stack[SP - 1]);
                break;
            case 9: // NEQ
                SP++;
                stack[SP] = (stack[SP] != stack[SP - 1]);
                break;
            case 10: // LSS
                SP++;
                stack[SP] = (stack[SP] < stack[SP - 1]);
                break;
            case 11: // LEQ
                SP++;
                stack[SP] = (stack[SP] <= stack[SP - 1]);
                break;
            case 12: // GTR
                SP++;
                stack[SP] = (stack[SP] > stack[SP - 1]);
                break;
            case 13: // GEQ
                SP++;
                stack[SP] = (stack[SP] >= stack[SP - 1]);
                break;
            default:
                printf("\nERROR: OPR operation not found");
                sprintf(vmTemp, "\nERROR: OPR operation not found");
                strcat(vmOutput, vmTemp);

                fprintf(output, "\nERROR: OPR operation not found");
                exit(0);
                break;
            }
            break;

        case 3: // LOD
            // load value to stack at M from L
            push(stack, &SP, stack[base(stack, inst[PC].l, BP) - inst[PC].m]);
            break;

        case 4: // STO
            // store value to stack at M from L
            stack[base(stack, inst[PC].l, BP) - inst[PC].m] = pop(stack, &SP);
            break;

        case 5:                                          // CAL
                                                         // call "function" at the line M
            stack[SP - 1] = 0;                           //space to return value
            stack[SP - 2] = base(stack, inst[PC].l, BP); //static link SL
            stack[SP - 3] = BP;                          //dynamic link DL
            stack[SP - 4] = PC + 1;                      //return address RA
            AR[calls++] = SP;
            BP = SP - 1;
            nextPC = inst[PC].m;
            break;

        case 6: // INC
            SP -= inst[PC].m;
            break;

        case 7: // JMP
            // jump to instruction M
            nextPC = inst[PC].m;
            break;

        case 8: // JPC
            // jump to M IF top stack == 0
            if (stack[SP] == 0)
            {
                nextPC = inst[PC].m;
            }
            SP++;
            break;

        case 9: // SIO, 1
            // write stack[SP] element to screen
            printf("%d\n", peek(stack, SP));
            /*sprintf(vmTemp, "\t\t%1d \t%1d \t%1d \t", nextPC, BP, SP);
			strcat(vmOutput, vmTemp);*/

            fprintf(output, "\t\t%1d \t%1d \t%1d \t", nextPC, BP, SP);
            //printStack(stack, SP, SP, outputfile, AR, calls - 1);
            SP++;
            break;

        case 10: // SIO, 2
            // read inputfile from user and store on top of stack
            printf("Enter a number: ");
            scanf("%d", &temp);
            push(stack, &SP, temp);
            break;

        case 11: // SIO, 3
            // end of program
            halt = 0;
            break;

        default:
            printf("\nERROR: Operation not found");
            sprintf(vmTemp, "\nERROR: Operation not found");
            strcat(vmOutput, vmTemp);

            fprintf(output, "\nERROR: Operation not found");
            exit(0);
            break;
        }

        sprintf(vmTemp, "%1d\t%s\t%1d\t%1d", PC, opnames[inst[PC].op - 1], inst[PC].l, inst[PC].m);
        strcat(vmOutput, vmTemp);
        fprintf(output, "%1d\t%s\t%1d\t%1d", PC, opnames[inst[PC].op - 1], inst[PC].l, inst[PC].m);
        /*
		if (inst[PC].op == 10) // read
		{
			sprintf(vmTemp, "\t\t%1d \t%1d \t%1d \t", nextPC, BP, SP - 1);
			strcat(vmOutput, vmTemp);

			fprintf(output, "\t\t%1d \t%1d \t%1d \t", nextPC, BP, SP - 1);
		}*/

        sprintf(vmTemp, "\t\t%1d \t%1d \t%1d \t", nextPC, BP, SP);
        strcat(vmOutput, vmTemp);

        fprintf(output, "\t\t%1d \t%1d \t%1d \t", nextPC, BP, SP);

        if (inst[PC].op == 10)
        {
        }

        if (inst[PC].op == 5)
            printStack(stack, SP, SP, output, AR, calls - 2);
        else
            printStack(stack, SP, SP, output, AR, calls - 1);

        sprintf(vmTemp, "\n");
        strcat(vmOutput, vmTemp);

        fprintf(output, "\n");

        PC = nextPC;
    } while (halt != 0 && PC < line);

    fclose(inputfile);
    fclose(output);

    return vmOutput;
}

void printStack(int stack[], int SP, int flag, FILE *outputfile, int AR[], int calls)
{

    if (!empty(SP))
    {
        if (SP == AR[calls] && SP != flag)
        {
            printStack(stack, SP + 1, flag, outputfile, AR, calls - 1);
        }
        else
        {
            printStack(stack, SP + 1, flag, outputfile, AR, calls);
        }
        sprintf(vmTemp, "%1d ", peek(stack, SP));
        strcat(vmOutput, vmTemp);
        fprintf(outputfile, "%1d ", peek(stack, SP));

        if (SP == AR[calls] && SP != flag)
        {
            sprintf(vmTemp, "| ");
            strcat(vmOutput, vmTemp);

            fprintf(outputfile, "| ");
        }
    }
}

/*==========IMPLEMENTATIONS FOR STACK==========*/
void push(int stack[], int *SP, int x)
{
    if (full(*SP))
    {
        printf("\nERROR: The stack is full\n");
        sprintf(vmTemp, "\nERROR: The stack is full\n");
        strcat(vmOutput, vmTemp);
        fprintf(output, "\nERROR: The stack is full\n");
        exit(0);
    }

    stack[*SP - 1] = x;
    (*SP)--;
}

int pop(int stack[], int *SP)
{
    int popvalue;

    if (empty(*SP))
    {
        return EMPTY_STACK;
    }

    popvalue = stack[*SP];
    (*SP)++;

    return popvalue;
}

int peek(int stack[], int SP)
{
    int peekvalue;

    if (empty(SP))
    {
        return EMPTY_STACK;
    }

    peekvalue = stack[SP];

    return peekvalue;
}

int full(int SP)
{
    return (SP == 0);
}

int empty(int SP)
{
    return (SP == MAX_DATA_STACK_HEIGHT);
}

int base(int stack[], int l, int base)
{
    int b1; //find base L levels down
    b1 = base;
    while (l > 0)
    {
        b1 = stack[b1 - 1];
        l--;
    }

    return b1;
}