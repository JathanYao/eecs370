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


typedef struct 
{
    char label[7];
    char type;
    int address;
}symbol;

typedef struct 
{
    int fromFile;
    int address;
    char opcode[7];
    char label[7];
}relocation;

typedef struct 
{
    int textSize;
    int textStart;
    int dataSize;
    int dataStart;
    int symbolSize;
    int relocSize;
    int text[MAXLINELENGTH];
    int data[MAXLINELENGTH];
    symbol symbolTable[MAXLINELENGTH];
    relocation relocTable[MAXLINELENGTH];
}File;


int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;

    if (argc < 2) // must have at least 1 object file and 1 mc file
    {
        printf("error: usage: %s <file_0.obj> <file_1.obj> ... <file_N.obj> <machine_code.mc>\n", argv[0]);
        exit(1);
    }

    outFileString = argv[argc - 1]; //mc file location

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    File files[6]; //max 6 files 
    for(int i = 1; i < argc - 1; i++)//start at argv[1]
    {
        inFileString = argv[i]; // read in file
        inFilePtr = fopen(inFileString, "r");
        if (inFilePtr == NULL)
        {
            printf("error in opening %s\n", inFileString);
            exit(1);
        }

        char lines[MAXLINELENGTH];
        //read ins
        {
        //----------------------------------READ IN HEADER----------------------------------//
        readHeader(inFilePtr, files[i].textSize, files[i].dataSize, files[i].symbolSize, files[i].relocSize);
        //----------------------------------READ IN HEADER----------------------------------//

        //----------------------------------READ IN TEXT----------------------------------//
        for(int j = 0; j < files[i].textSize; j++)
        {
            readTextorData(inFilePtr, files[i].text[j]);
        }
        //----------------------------------READ IN TEXT----------------------------------//
    
        //----------------------------------READ IN DATA----------------------------------//
        for(int j = 0; j < files[i].dataSize; j++)
        {
            readTextorData(inFilePtr, files[i].data[j]);
        }
        //----------------------------------READ IN DATA----------------------------------//

        //----------------------------------READ IN SYMBOLS----------------------------------//
        for(int j = 0; j < files[i].symbolSize; j++)
        {
            readSymbol(inFilePtr, files[i].symbolTable[j].label, files[i].symbolTable[j].type, files[i].symbolTable[j].address);
        }
        //----------------------------------READ IN SYMBOLS----------------------------------//

        //----------------------------------READ IN RELOCATIONS----------------------------------//
        for(int j = 0; j < files[i].relocSize; j++)
        {
            files[i].relocTable[j].fromFile = i;
            readRelocation(inFilePtr, files[i].relocTable[j].address, files[i].relocTable[j].opcode, files[i].relocTable[j].label);
        }
        //----------------------------------READ IN RELOCATIONS----------------------------------//
        fclose(inFilePtr);
        }
        //start changing

        int startLine = 0;
        //find starting points of text and data sections for each file
        for(int i = 0; i < sizeof(files); i++)
        {
            files[i].textStart = startLine;
            startLine += files[i].textSize;
        }
        for(int i = 0; i < sizeof(files); i++)
        {
            files[i].dataStart = startLine;
            startLine += files[i].dataSize;
        }

        //check global labels defined in > 1 files
        for(int i = 0; i < sizeof(files) - 1; i++)
        {
            for(int j = 0; j < files[i].symbolSize; j++)
            {
                for(int k = i + 1; k < sizeof(files); k++)
                {
                    for(int l = 0; l < files[k].symbolSize; l++)
                    {
                        bool sameLabel = (!strcmp(files[i].symbolTable[k].label, files[k].symbolTable[l].label));
                        bool bothDefined = files[i].symbolTable[k].label != "U" && files[k].symbolTable[l].label != "U";
                        if(sameLabel && bothDefined)
                        {
                            printf("Error: Duplicate Label");
                            exit(1);
                        }
                    }
                }
            }
        }

        //fix all local labels
        
    }

    fclose(outFilePtr);
    
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
 * MODIFIED read header
 *
 * Return values:
 *     0 if reached eof
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readHeader(FILE *inFilePtr, int *text, int* data, int* symbols, int* relocations)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    text[0] = data[0] = symbols[0] = relocations[0] = 0;

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

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", text, data, symbols, relocations);
    return (1);
}

/*
 * MODIFIED read line (text/data)
 *
 * Return values:
 *     0 if reached eof
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readTextorData(FILE *inFilePtr, int *instr)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    instr[0] = 0;

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

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]", instr);
    return (1);
}

/*
 * MODIFIED read symbol
 *
 * Return values:
 *     0 if reached eof
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readSymbol(FILE *inFilePtr, char* label, char* type, int* address)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = type[0] = '/0';
    address[0] = 0;

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

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", label, type, address);
    return (1);
}


/*
 * MODIFIED read relocation
 *
 * Return values:
 *     0 if reached eof
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readRelocation(FILE *inFilePtr, int* address, char* opcode, char* label)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    opcode[0] = label[0] = '/0';
    address[0] = 0;

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

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", address, opcode, label);
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
