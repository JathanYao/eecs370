/**
 * Project 2
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

    // DECLARING VARIABLES
    label_line lab[MAXLINELENGTH];
    int pc = 0, count = 0;
    int textLength = 0;
    int symbolLength = 0;
    int dataLength = 0;
    int relocLength = 0;

    // FIRST PASS, IDENTIFY ALL LABELS, SECTION LENGTHS FOR HEADER
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (strcmp(label, "")) // not empty
        {
            if (isupper(label[0]))
                symbolLength++; // count symbols, labels that start with uppercase letter
            lab[count].line = pc;
            strcpy(lab[count].label, label); // store line of label
            count++;
        }
        if (strcmp(opcode, ".fill"))
            textLength++; // count as long as its not a .fill
        if (!strcmp(opcode, ".fill"))
            dataLength++; // count data words
        if ((!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) && !isNumber(arg2))
            relocLength++; // count relocations, lw/sw with label in arg2
        if (!strcmp(opcode, ".fill") && !isNumber(arg0))
            relocLength++; // count relocations, .fill with label in arg0
        pc++;
    }

    // error checking, check for duplicates
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < count; j++)
        {
            if (j == i)
                continue;
            if (!strcmp(lab[i].label, lab[j].label))
            {
                printf("duplicate labels");
                exit(1);
            }
        }
    }

    rewind(inFilePtr);

    // check all labels in lw sw if they are undefined or not
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if ((!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) && isupper(arg2[0])) // if is lw or sw
        {
            bool found = false;
            for (int i = 0; i < count; i++)
            {
                if (!strcmp(lab[i].label, arg2))
                {
                    // found corresponding label
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                // if not found, check if its just an undefined local
                if (!isupper(arg2[0]))
                {
                    // just a normal undefined label
                    printf("Label undefined lw/sw");
                    exit(1);
                }
                symbolLength++; // count undefined global as a symbol
            }
        }
    }

    // print header line to output file
    fprintf(outFilePtr, "%d %d %d %d\n", textLength, dataLength, symbolLength, relocLength);

    // STEP 1 HEADER LINE:
    /*
    - count all instructions
    - count all symbols]
    - count all data words
    - count all relocations
    */

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    pc = 0;
    // OUTPUT TEXT + DATA TABLE
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        int line = 0, a0 = atoi(arg0), a1 = atoi(arg1), a2 = atoi(arg2);
        // ALL 8 FUNCTIONS
        if (!strcmp(opcode, "add") || !strcmp(opcode, "nor"))
        {
            if (a0 > 7 || a0 < 0 || a1 > 7 || a1 < 0 || a2 > 7 || a2 < 0 || !isNumber(arg2))
            {
                printf("invalid registers");
                exit(1);
            }
            a2 = atoi(arg2);
            line += (a0 << 19) + (a1 << 16) + a2; // opcode is 000, just add the three arguments
            if (!strcmp(opcode, "nor"))
                line += (1 << 22);
        }
        else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq"))
        {
            if (!isNumber(arg0) || !isNumber(arg1) || a0 > 7 || a0 < 0 || a1 > 7 || a1 < 0)
            {
                printf("invalid registers");
                exit(1);
            }
            if (!strcmp(opcode, "lw"))
                line += (0b010 << 22); // lw
            else if (!strcmp(opcode, "sw"))
                line += (0b011 << 22); // sw
            else
            {
                line += (0b100 << 22); // beq
            }

            line += (a0 << 19) + (a1 << 16);

            // check if arg2 is a label or a number
            if (isNumber(arg2))
            {
                a2 = atoi(arg2);
                if (a2 >= 32768 || a2 <= -32769)
                {
                    printf("arg2 exceeds limit");
                    exit(1);
                }
                if (!strcmp(opcode, "beq"))
                    line += (a2 & 0xFFFF);
            }
            else // is a label
            {
                // lookup label name address
                bool found = false;
                bool global = false;
                for (int i = 0; i < count; i++)
                {
                    if (!strcmp(lab[i].label, arg2))
                    {
                        // found corresponding label
                        a2 = lab[i].line;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    // if not found, check if its just an undefined global
                    if (isupper(arg2[0]))
                    {
                        a2 = 0; // undefined global, set to 0
                        global = true;
                    }
                    else
                    {
                        // just a normal undefined label
                        printf("Label undefined lw/sw/beq");
                        exit(1);
                    }
                }
                if (!strcmp(opcode, "beq"))
                {
                    if (global)
                    {
                        printf("beq cannot have an undefined global");
                        exit(1);
                    }
                    else
                        line += (0xFFFF & (a2 - pc - 1)); // no global
                }
            }
            if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw"))
                line += (a2 & 0xFFFF); // ensures that offset is a twos complement number
        }
        else if (!strcmp(opcode, "jalr"))
        {
            if (!isNumber(arg0) || !isNumber(arg1) || a0 > 7 || a0 < 0 || a1 > 7 || a1 < 0)
            {
                printf("invalid registers");
                exit(1);
            }
            line += (0b101 << 22) + (a0 << 19) + (a1 << 16); // opcode is 101
        }
        else if (!strcmp(opcode, "halt"))
            line += (0b110 << 22); // opcode is 110
        else if (!strcmp(opcode, "noop"))
            line += (0b111 << 22); // opcode is 111
        else if (!strcmp(opcode, ".fill"))
        {
            if (isNumber(arg0))
            {
                a0 = atoi(arg0);
                if (a0 >= 32768 || a0 <= -32769)
                {
                    printf("arg0 exceeds limit");
                    exit(1);
                }
            }
            else
            {
                // lookup label name address
                bool found = false;
                for (int i = 0; i < count; i++)
                {
                    if (!strcmp(lab[i].label, arg0))
                    {
                        // found corresponding label
                        a0 = lab[i].line;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    if (isupper(arg0[0]))
                        a0 = 0; // undefined global, set to 0
                    else
                    {
                        // just a normal undefined label
                        printf("Label undefined .fill");
                        exit(1);
                    }
                }
            }
            line += a0;
        }
        else // error unrecognized opcode
        {
            printf("error unrecognized opcode");
            exit(1);
        }
        printHexToFile(outFilePtr, line);
        pc++;
        textLength++;
    }

    rewind(inFilePtr);

    // output symbol table
    dataLength = 0; // reset dataLength to reuse as data counter
    textLength = 0; // reset textLength to reuse as text counter
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (strcmp(label, "")) // not empty
        {
            if (isupper(label[0]))
            {
                // check if its a fill
                if (!strcmp(opcode, ".fill"))
                    // data symbol
                    fprintf(outFilePtr, "%s D %d\n", label, dataLength);
                else
                {
                    // text symbol
                    fprintf(outFilePtr, "%s T %d\n", label, textLength);
                }
            }
        }
        if ((!strcmp(opcode, "lw") || !strcmp(opcode, "sw")))
        {
            if (isupper(arg2[0]))
            {
                bool found = false;
                for (int i = 0; i < count; i++)
                {
                    if (!strcmp(lab[i].label, arg2))
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    fprintf(outFilePtr, "%s U 0\n", arg2);
            }
        }
        if (!strcmp(opcode, ".fill"))
            dataLength++;
        textLength++;
    }

    rewind(inFilePtr);
    pc = 0;
    dataLength = 0; // reset dataLength to reuse as data counter
    textLength = 0; // reset textLength to reuse as text counter
    // OUTPUT RELOCATION TABLE
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if ((!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) && !isNumber(arg2))
            fprintf(outFilePtr, "%d %s %s\n", textLength, opcode, arg2);
        if (!strcmp(opcode, ".fill"))
        {
            if (!isNumber(arg0))
                fprintf(outFilePtr, "%d %s %s\n", dataLength, opcode, arg0);
            dataLength++;
        }
        textLength++;
    }

    /* here is an example of using printHexToFile. This will print a
       machine code word / number in the proper hex format to the output file */
    // printHexToFile(outFilePtr, 123);
    fclose(outFilePtr);
    fclose(inFilePtr);
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
