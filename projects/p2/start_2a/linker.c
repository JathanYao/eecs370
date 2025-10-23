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

int readHeader(FILE *inFilePtr, int *text, int* data, int* symbols, int* relocations);
int readRelocation(FILE *inFilePtr, int* address, char* opcode, char* label);
int readSymbol(FILE *inFilePtr, char* label, char* type, int* address);
int readTextorData(FILE *inFilePtr, int *instr);

static inline void printHexToFile(FILE *, int);

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
    struct 
    {
        char label[7];
        char type;
        int address;
    }symbolTable[MAXLINELENGTH];
    struct 
    {
        int fromFile;
        int address;
        char opcode[7];
        char label[7];
    }relocTable[MAXLINELENGTH];
}File;

int getGlobal(File *files, int numFiles, char* label);

int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;

    if (argc < 2) // must have at least 1 object file and 1 mc file
    {
        printf("error: usage: %s <file_0.obj> <file_1.obj> ... <file_N.obj> <machine_code.mc>\n", argv[0]);
        exit(1);
    }
    int numFiles = argc - 2;
    outFileString = argv[argc - 1]; //mc file location

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    File files[6]; //max 6 files 
    for(int i = 0; i < numFiles; i++)//start at argv[1]
    {
        inFileString = argv[i + 1]; // read in file
        inFilePtr = fopen(inFileString, "r");
        if (inFilePtr == NULL)
        {
            printf("error in opening %s\n", inFileString);
            exit(1);
        }

        //read ins
        
        //----------------------------------READ IN HEADER----------------------------------//
        readHeader(inFilePtr, &files[i].textSize, &files[i].dataSize, &files[i].symbolSize, &files[i].relocSize);
        //----------------------------------READ IN HEADER----------------------------------//

        //----------------------------------READ IN TEXT----------------------------------//
        for(int j = 0; j < files[i].textSize; j++)
        {
            readTextorData(inFilePtr, &files[i].text[j]);
        }
        //----------------------------------READ IN TEXT----------------------------------//
    
        //----------------------------------READ IN DATA----------------------------------//
        for(int j = 0; j < files[i].dataSize; j++)
        {
            readTextorData(inFilePtr, &files[i].data[j]);
        }
        //----------------------------------READ IN DATA----------------------------------//

        //----------------------------------READ IN SYMBOLS----------------------------------//
        for(int j = 0; j < files[i].symbolSize; j++)
        {
            readSymbol(inFilePtr, files[i].symbolTable[j].label, &files[i].symbolTable[j].type, &files[i].symbolTable[j].address);
            if (!strcmp(files[i].symbolTable[j].label, "Stack") && files[i].symbolTable[j].type != 'U'){
                printf("Error: Stack label defined\n");
                exit(1);
            }
        }
        //----------------------------------READ IN SYMBOLS----------------------------------//

        //----------------------------------READ IN RELOCATIONS----------------------------------//
        for(int j = 0; j < files[i].relocSize; j++)
        {
            files[i].relocTable[j].fromFile = i;
            readRelocation(inFilePtr, &files[i].relocTable[j].address, files[i].relocTable[j].opcode, files[i].relocTable[j].label);
        }
        //----------------------------------READ IN RELOCATIONS----------------------------------//
    }
    fclose(inFilePtr);
    //start changing
    int startLine = 0;
    //find starting points of text and data sections for each file
    for(int i = 0; i < numFiles; i++)
    {
        files[i].textStart = startLine;
        startLine += files[i].textSize;
    }
    for(int i = 0; i < numFiles; i++)
    {
        files[i].dataStart = startLine;
        startLine += files[i].dataSize;
    }

    //check global labels defined in > 1 files
    for(int i = 0; i < numFiles - 1; i++)
    {
        for(int j = 0; j < files[i].symbolSize; j++)
        {
            for(int k = i + 1; k < numFiles; k++)
            {
                for(int l = 0; l < files[k].symbolSize; l++)
                {
                    bool sameLabel = (!strcmp(files[i].symbolTable[k].label, files[k].symbolTable[l].label));
                    bool bothDefined = files[i].symbolTable[k].type != 'U' && files[k].symbolTable[l].type != 'U';
                    if(sameLabel && bothDefined)
                    {
                        printf("Error: Duplicate Label\n");
                        exit(1);
                    }
                }
            }
        }
    }
    int ogOffset = 0, newOffset = 0;
    //update text and data with relocations
    for(int i = 0; i < numFiles; i++)
    {
        for(int j = 0; j < files[i].relocSize; j++)
        {
            char* label = files[i].relocTable[j].label;
            char* opcode = files[i].relocTable[j].opcode;
            bool isLocal = islower(label[0]);
            bool isStack = (!strcmp(label, "Stack"));
            int address = files[i].relocTable[j].address;
            //check text or data relocation
            if(!strcmp(opcode, "lw") || !strcmp(opcode, "sw"))
            {
                //in text section (lw, sw)
                int instruction = files[i].text[address];
                //find offset from instruction
                ogOffset = instruction & 0xFFFF; //gets offset bits
                if(isLocal)
                {
                    if(ogOffset < files[i].textSize)
                        files[i].text[address] = files[i].textStart + instruction; // compute newoffset by adding offset to file start location in text block
                    else
                    {
                        int newInstr = files[i].dataStart - files[i].textSize + instruction;
                        files[i].text[address] = newInstr; //label is located in data section (not a .fill)
                    }
                }
                else
                {
                    //global label
                    if(isStack) //check if its the Stack label
                    {
                        //use startLine since it accumulated all text + data sizes
                        newOffset = startLine; 
                    }
                    else
                    {
                        //find the global label value
                        newOffset = getGlobal(files, numFiles, label);
                    }
                    // compute newInstr by adding new offset
                    files[i].text[address] = instruction - ogOffset + newOffset; //set the new computed offset
                }
            }
            else //in data section (.fill)
            {
                ogOffset = files[i].data[address];

                if(isLocal)
                {
                    if(ogOffset < files[i].textSize) //in text section
                        files[i].data[address] = files[i].textStart + ogOffset; //ogOffset would start at 0
                    else
                        files[i].data[address] = files[i].dataStart - files[i].textSize + ogOffset;
                }
                else //100% CORRECT
                {
                    //ogOffset is always 0
                    if(isStack) //check if its the Stack label
                    {
                        //use startLine since it accumulated all text + data sizes
                        newOffset = startLine; 
                    }
                    else
                    {
                        //find the global label value
                        newOffset = getGlobal(files, numFiles, label);
                    }
                    files[i].data[address] = newOffset; //set the new computed offset
                }
            }
        }
    }

    //print all the text and data
    for(int i = 0; i < numFiles; i++)
    {
        for(int j = 0; j < files[i].textSize; j++)
        {
            printHexToFile(outFilePtr, files[i].text[j]);
        }
    }
    for(int i = 0; i < numFiles; i++)
    {
        for(int j = 0; j < files[i].dataSize; j++)
        {
            printHexToFile(outFilePtr, files[i].data[j]);
        }
    }
    fclose(outFilePtr);
    
    return (0);
}

//returns value of the global label
int getGlobal(File *files, int numFiles, char* label)
{
    int value = -1; //use to error check
    for(int i = 0; i < numFiles; i++)
    {
        for(int j = 0; j < files[i].symbolSize; j++)
        {
            if(!strcmp(files[i].symbolTable[j].label, label))
            {
                char type = files[i].symbolTable[j].type;
                switch(type)
                {
                    case 'T':
                        value = files[i].symbolTable[j].address + files[i].textStart;
                        break;
                    case 'D':
                        value = files[i].symbolTable[j].address + files[i].dataStart;
                        break;
                }
            }
        }
    }
    if(value == -1)
    {
        printf("Error: Undefined Global Label\n");
        exit(1);
    }
    return value;
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
    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(line, "%d %d %d %d", text, data, symbols, relocations);
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

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(line, "%x", instr);
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
    /* delete prior values */
    label[0] = type[0] = '\0';
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

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(line, "%6s %6s %d", label, type, address);
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
    /* delete prior values */
    opcode[0] = label[0] = '\0';
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
    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(line, "%d %6s %6s", address, opcode, label);
    return (1);
}

static inline void
printHexToFile(FILE *outFilePtr, int word)
{
    fprintf(outFilePtr, "0x%08X\n", word);
}