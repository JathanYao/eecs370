/**
 * Project 2
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
typedef enum
{
    txt = 0,
    data = 1
} Section;
typedef struct
{
    char Label[MAXLINELENGTH];
    char opcode[MAXLINELENGTH];
    char arg[3][MAXLINELENGTH];
    int address;
    Section sec;
    int section_idx;
} Line;

typedef struct
{
    Section sec;
    int offset;
    char opcode[MAXLINELENGTH];
    char label[MAXLINELENGTH];
} Reloc;

typedef struct
{
    char name[MAXLINELENGTH];
    Section sec;
    int offset;
    int global;
    int local_define;
} Symbol;

static void new_unresolved(char unresolved[][MAXLINELENGTH], int *unresolved_num, const char *name);
static void new_reloc(Reloc relocs[], int *reloc_num, const Line *L, const char *op, const char *symbol);

// strcmp helpers
static int isFill(const char *op) { return strcmp(op, ".fill") == 0; }
static int isLw(const char *op) { return strcmp(op, "lw") == 0; }
static int isSw(const char *op) { return strcmp(op, "sw") == 0; }
static int isBeq(const char *op) { return strcmp(op, "beq") == 0; }
static int OpcodeOrDir(const char *tok)
{
    return strcmp(tok, ".fill") == 0 || strcmp(tok, "add") == 0 || strcmp(tok, "nor") == 0 || strcmp(tok, "lw") == 0 || strcmp(tok, "sw") == 0 || strcmp(tok, "beq") == 0 || strcmp(tok, "jalr") == 0 || strcmp(tok, "halt") == 0 || strcmp(tok, "noop") == 0;
}

// isuppercase
static int uppercase(const char *s)
{
    return s[0] >= 'A' && s[0] <= 'Z';
}
// islowercase
static int lowercase(const char *s)
{
    return s[0] >= 'a' && s[0] <= 'z';
}
// bigger than 16bits
static void biggerThan16(int v)
{
    if (v < -32768 || v > 32767)
    {
        exit(1);
    }
}
// helper to find symbols
static int find_symbol(const Symbol *tab, int n, const char *name)
{
    for (int i = 0; i < n; i++)
    {
        if (strcmp(tab[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static void new_unresolved(char unresolved[][MAXLINELENGTH], int *unresolved_num, const char *name)
{
    for (int i = 0; i < *unresolved_num; i++)
    {
        if (strcmp(unresolved[i], name) == 0)
            return;
    }
    strncpy(unresolved[*unresolved_num], name, MAXLINELENGTH - 1);
    unresolved[*unresolved_num][MAXLINELENGTH - 1] = '\0';
    (*unresolved_num)++;
}

static void new_reloc(Reloc relocs[], int *reloc_num, const Line *L, const char *op, const char *symbol)
{
    if (*reloc_num >= MAXLINELENGTH)
    {
        exit(1);
    }
    Reloc *r = &relocs[(*reloc_num)++];
    r->sec = L->sec;
    r->offset = L->section_idx;
    strncpy(r->opcode, op, MAXLINELENGTH - 1);
    r->opcode[MAXLINELENGTH - 1] = '\0';
    strncpy(r->label, symbol, MAXLINELENGTH - 1);
    r->label[MAXLINELENGTH - 1] = '\0';
}

/**
 * Requires: readAndParse is non-static and unmodified from project 1a.
 *   inFilePtr and outFilePtr must be opened.
 *   inFilePtr must be rewound before calling this function.
 * Modifies: outFilePtr
 * Effects: Prints the correct machine code for the input file. After
 *   reading and parsing through inFilePtr, the pointer is rewound.
 *   Most project 1a error checks are done. No undefined labels of any
 *   type are checked, and these are instead resolved to 0.
 */
/**
 * This function will be provided in an instructor object file once the
 * project 1a deadline + late days has passed.
 */
extern void print_inst_machine_code(FILE *inFilePtr, FILE *outFilePtr);

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);

int main(int argc, char **argv)
{
    char *inFileStr, *outFileStr;
    FILE *inFilePtr, *outFilePtr;
    //char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
       // arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileStr = argv[1];
    outFileStr = argv[2];

    inFilePtr = fopen(inFileStr, "r");
    if (inFilePtr == NULL)
    {
        printf("error in opening %s\n", inFileStr);
        exit(1);
    }
    static Line lines[MAXLINELENGTH];
    int line_num = 0;
    static Symbol definition[MAXLINELENGTH];
    int def_num = 0;
    static Symbol global[MAXLINELENGTH];
    int global_num = 0;
    static Reloc relocs[MAXLINELENGTH] = {0};
    int reloc_num = 0;

    int text_num = 0;
    int fill_num = 0;
    int in_data = 0;
    static char unresolved[MAXLINELENGTH][MAXLINELENGTH];
    int unresolved_num = 0;

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileStr, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileStr);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);
    // first pass
    while (1)
    {
        char label[MAXLINELENGTH] = {0};
        char op[MAXLINELENGTH] = {0};
        char a0[MAXLINELENGTH] = {0};
        char a1[MAXLINELENGTH] = {0};
        char a2[MAXLINELENGTH] = {0};
        if (!readAndParse(inFilePtr, label, op, a0, a1, a2))
        {
            break;
        }
        int lable_is_op = OpcodeOrDir(label);
        // shift if needed
        if (label[0] != '\0' && lable_is_op)
        {
            strcpy(a2, a1);
            strcpy(a1, a0);
            strcpy(a0, op);
            strcpy(op, label);
            label[0] = '\0';
        }
        if (!OpcodeOrDir(op))
            exit(1);

        if (isFill(op))
        {
            in_data = 1;
        }
        else
        {
            if (in_data)
            {
                exit(1);
            }
        }
        if (line_num >= MAXLINELENGTH)
        {
            exit(1);
        }
        Line *l = &lines[line_num];
        strncpy(l->Label, label, sizeof(l->Label) - 1);
        strncpy(l->opcode, op, sizeof(l->Label) - 1);
        strncpy(l->arg[0], a0, sizeof(l->Label) - 1);
        strncpy(l->arg[1], a1, sizeof(l->Label) - 1);
        strncpy(l->arg[2], a2, sizeof(l->Label) - 1);
        l->address = line_num;
        l->sec = isFill(op) ? data : txt;
        if (l->sec == txt)
        {
            l->section_idx = text_num++;
        }
        else
        {
            l->section_idx = fill_num++;
        }
        line_num++;

        if (label[0] != '\0' && !lable_is_op)
        {
            if (find_symbol(definition, def_num, label) >= 0)
            {
                exit(1);
            }
            Symbol sym;
            strncpy(sym.name, label, sizeof(l->Label) - 1);
            sym.sec = l->sec;
            sym.offset = l->section_idx;
            sym.global = uppercase(label);
            sym.local_define = 1;
            definition[def_num++] = sym;
        }
    }

    // pass 2
    for (int i = 0; i < line_num; i++)
    {
        Line *l = &lines[i];

        if (isLw(l->opcode) || isSw(l->opcode))
        {
            const char *arg2 = l->arg[2];
            if (isNumber((char *)arg2))
            {
                biggerThan16(atoi(arg2));
            }
            else
            {
                const char *symbol = arg2;
                int index = find_symbol(definition, def_num, symbol);
                if (lowercase(symbol))
                {
                    if (index < 0)
                    {
                        exit(1);
                    }
                }
                else if (uppercase(symbol))
                {
                    if (index < 0)
                    {
                        new_unresolved(unresolved, &unresolved_num, symbol);
                    }
                }
                new_reloc(relocs, &reloc_num, l, l->opcode, symbol);
            }
        }
        else if (isBeq(l->opcode))
        {
            const char *arg2 = l->arg[2];
            if (isNumber((char *)arg2))
            {
                biggerThan16(atoi(arg2));
            }
            else
            {
                int index = find_symbol(definition, def_num, arg2);
                if (index < 0)
                {
                    exit(1);
                }
                Symbol d = definition[index];
                int target = (d.sec == txt) ? d.offset : (d.offset + text_num);
                int pc = l->address;
                int off = target - (pc + 1);
                biggerThan16(off);
            }
        }
        else if (isFill(l->opcode))
        {
            const char *a0 = l->arg[0];
            if (isNumber((char *)a0))
            {
            }
            else
            {
                int index = find_symbol(definition, def_num, a0);
                if (lowercase(a0))
                {
                    if (index < 0)
                        exit(1);
                }
                else if (uppercase(a0))
                {
                    if (index < 0)
                    {
                        new_unresolved(unresolved, &unresolved_num, a0);
                    }
                }
                new_reloc(relocs, &reloc_num, l, ".fill", a0);
            }
        }
    }
    for (int i = 0; i < def_num; i++)
    {
        if (definition[i].global)
        {
            global[global_num++] = definition[i];
        }
    }
    for (int i = 0; i < unresolved_num; i++)
    {
        const char *name = unresolved[i];
        if (find_symbol(global, global_num, name) < 0)
        {
            Symbol g;
            memset(&g, 0, sizeof(g));
            strncpy(g.name, name, MAXLINELENGTH - 1);
            g.global = 1;
            g.local_define = 0;
            global[global_num++] = g;
        }
    }
    FILE *output = tmpfile();
    if (!output)
    {
        exit(1);
    }
    rewind(inFilePtr);
    print_inst_machine_code(inFilePtr, output);
    fprintf(outFilePtr, "%d %d %d %d\n", text_num, fill_num, global_num, reloc_num);
    rewind(output);
    char buf[64];
    for (int i = 0; i < text_num + fill_num; i++)
    {
        if (!fgets(buf, sizeof(buf), output))
        {
            exit(1);
        }
        fputs(buf, outFilePtr);
    }
    for (int i = 0; i < global_num; i++)
    {
        Symbol *g = &global[i];
        char sectionLetter = 'U';
        int off = 0;
        if (g->local_define)
        {
            sectionLetter = (g->sec == txt) ? 'T' : 'D';
            off = g->offset;
        }
        fprintf(outFilePtr, "%s %c %d\n", g->name, sectionLetter, off);
    }
    for (int i = 0; i < reloc_num; i++)
    {
        Reloc *r = &relocs[i];
        fprintf(outFilePtr, "%d %s %s\n", r->offset, r->opcode, r->label);
    }

    fclose(output);
    fclose(outFilePtr);
    fclose(inFilePtr);
    return (0);
}

/*
 * NOTE: The code defined below is not to be modifed as it is implemented correctly.
 */

// Returns non-zero if the line contains only whitespace.
int lineIsBlank(char *line)
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
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
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
