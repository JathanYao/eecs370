/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000
#define MAXADDRESSES 65535

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);

typedef struct
{
    char label[MAXLINELENGTH];
    int line;
} label_line;

int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL)
    {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // FIRST PASS, IDENTIFY ALL LABELS
    label_line lab[MAXLINELENGTH];
    int pc = 0, count = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (strcmp(label, ""))
        {
            lab[count].line = pc;
            strcpy(lab[count].label, label); // store line of label
            count++;
        }
        pc++;
    }

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);
    pc = 0;
    // SECOND PASS, GENERATE MACHINE LANGUAGE INSTRUCTIONS
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        int a0 = atoi(arg0), a1 = atoi(arg1), a2 = -1, line = 0;
        
        if (strcmp(opcode, "halt") && strcmp(opcode, "noop") && strcmp(opcode, ".fill"))
        {
            //only perform for RIJ types
            if (!isNumber(arg2)) // arg2 is a label, find line for the label from label_line
            {
                for (int i = 0; i < count; i++)
                {
                    if (!strcmp(lab[i].label, arg2))
                    {
                        // found corresponding label
                        a2 = lab[i].line;
                    }
                }
                if (a2 < 0)
                {
                    printf("Label undefined");
                    exit(1);
                }
            }
            else
                a2 = atoi(arg2);

            line += (a0 << 19); //reg A
            line += (a1 << 16); //reg B
        }
        
        // ALL 8 FUNCTIONS
        if (!strcmp(opcode, "add") || !strcmp(opcode, "nor"))
        {
            if(!strcmp(opcode, "nor"))line += (1 << 22);
            line += a2; // destReg
        }
        else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq"))
        {
            line += (2 << 22);
            if(!strcmp(opcode, "sw"))line += (1 << 22);
            if(!strcmp(opcode, "beq"))
            {
                line += (2 << 22);
                if (!isNumber(arg2)) // arg2 is a label, find line for the label from label_line
                {
                    for (int i = 0; i < count; i++)
                    {
                        if (!strcmp(lab[i].label, arg2))
                        {
                            // found corresponding label
                            a2 = lab[i].line;
                            break;
                        }
                    }
                    if (a2 < 0)
                    {
                        printf("Label undefined");
                        exit(1);
                    }
                    int16_t offset = (a2 - (pc + 1));
                    line += offset & 0xFFFF; //must jump to label line, make it 16-bits
                }
                else
                {
                    //regular addition jump
                    a2 = atoi(arg2);
                    line += (a2);
                }
            }else line += (a2);
        }
        else if (!strcmp(opcode, "jalr"))
        {
            line += (5 << 22);
        }
        else if(!strcmp(opcode, "halt") || !strcmp(opcode, "noop"))
        {
            line += (6 << 22);
            if(!strcmp(opcode, "noop"))line += (1 << 22);
        }
        else if(!strcmp(opcode, ".fill"))
        {
            if (!isNumber(arg0)) // arg0 is a label, find line for the label from label_line
            {
                for (int i = 0; i < count; i++)
                {
                    if (!strcmp(lab[i].label, arg0))
                    {
                        // found corresponding label
                        a0 = lab[i].line;
                        break;
                    }
                }
                if (a0 < 0)
                {
                    printf("Label undefined");
                    exit(1);
                }
                line += a0; //must jump to label line, make it 16-bits
            }
            else
            {
                //regular addition jump
                a0 = atoi(arg0);
                line += (a0);
            }
        }
        else // error
        {
            printf("error");
            exit(1);
        }
         printHexToFile(outFilePtr, line);
         pc++;
    }

    /* here is an example of using printHexToFile. This will print a
       machine code word / number in the proper hex format to the output file */
    //printHexToFile(outFilePtr, 123);
    return (0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line)
{
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for (int line_idx = 0; line_idx < strlen(line); ++line_idx)
    {
        int line_char_is_whitespace = 0;
        for (int whitespace_idx = 0; whitespace_idx < 4; ++whitespace_idx)
        {
            if (line[line_idx] == whitespace[whitespace_idx])
            {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if (!line_char_is_whitespace)
        {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr)
{
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for (int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address)
    {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH - 1)
        {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if (lineIsBlank(line))
        {
            if (!blank_line_encountered)
            {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        }
        else
        {
            if (blank_line_encountered)
            {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}

/*
 * NOTE: The code defined below is not to be modifed as it is implemented correctly.
 */

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {
        /* reached end of file */
        return (0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH - 1)
    {
        printf("error: line too long\n");
        exit(1);
    }

    // Ignore blank lines at the end of the file.
    if (lineIsBlank(line))
    {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label))
    {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);

    return (1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return ((sscanf(string, "%d%c", &num, &c)) == 1);
}

// Prints a machine code word in the proper hex format to the file
static inline void
printHexToFile(FILE *outFilePtr, int word)
{
    fprintf(outFilePtr, "0x%08X\n", word);
}
