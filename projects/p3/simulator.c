/*
 * EECS 370, University of Michigan, Fall 2023
 * Project 3: LC-2K Pipeline Simulator
 * Instructions are found in the project spec: https://eecs370.github.io/project_3_spec/
 * Make sure NOT to modify printState or any of the associated functions
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Machine Definitions
#define NUMMEMORY 65536 // maximum number of data words in memory
#define NUMREGS 8       // number of machine registers

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 // will not implemented for Project 3
#define HALT 6
#define NOOP 7

const char *opcode_to_str_map[] = {
    "add",
    "nor",
    "lw",
    "sw",
    "beq",
    "jalr",
    "halt",
    "noop"};

#define NOOPINSTR (NOOP << 22)

typedef struct IFIDStruct
{
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct
{
    int instr;
    int pcPlus1;
    int valA;
    int valB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct
{
    int instr;
    int branchTarget;
    int eq;
    int aluResult;
    int valB;
} EXMEMType;

typedef struct MEMWBStruct
{
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct
{
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct
{
    unsigned int numMemory;
    unsigned int cycles; // number of cycles run so far
    int pc;
    int instrMem[NUMMEMORY];
    int dataMem[NUMMEMORY];
    int reg[NUMREGS];
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
} stateType;

static inline int opcode(int instruction)
{
    return instruction >> 22;
}

static inline int field0(int instruction)
{
    return (instruction >> 19) & 0x7;
}

static inline int field1(int instruction)
{
    return (instruction >> 16) & 0x7;
}

static inline int field2(int instruction)
{
    return instruction & 0xFFFF;
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num)
{
    return num - ((num & (1 << 15)) ? 1 << 16 : 0);
}

void printState(stateType *);
void printInstruction(int);
void readMachineCode(stateType *, char *);

// === Helper: function to get destination register in EX stage===
int getDestReg(int instruction)
{
    //only cares about LW, ADD or NOR
    int operation = opcode(instruction);
    if (operation == LW)
        return field1(instruction);   // LW writes to regB (field1)
    else
        return field2(instruction);   // R-type writes to dest (field2)
}

int main(int argc, char *argv[])
{

    /* Declare state and newState.
       Note these have static lifetime so that instrMem and
       dataMem are not allocated on the stack. */

    static stateType state, newState;

    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    readMachineCode(&state, argv[1]);

    /* ------------ Initialize State ------------ */
    // At the start of the program, initialize the pc and all registers to zero.

    state.pc = 0;
    for (int i = 0; i < NUMREGS; i++)
    {
        state.reg[i] = 0;
    }
    // Initialize the instruction field in all pipeline registers to the noop instruction (0x1c00000).
    // A noop must travel through the pipeline, even though it has no effect on the state of the pipeline.
    state.IFID.instr = NOOPINSTR;
    state.IDEX.instr = NOOPINSTR;
    state.EXMEM.instr = NOOPINSTR;
    state.MEMWB.instr = NOOPINSTR;
    state.WBEND.instr = NOOPINSTR;

    /* ------------------- END ------------------ */

    newState = state;

    while (opcode(state.MEMWB.instr) != HALT)
    {
        printState(&state);

        newState.cycles += 1;

        /* ---------------------- IF stage --------------------- */
        /*
        gets the instruction from instruction memory at the address in pc
        gets pc+1
        mux pc + 1 or target and stores into pc
        stores both into register
        */
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.IFID.instr = state.instrMem[state.pc];
        newState.pc = state.pc + 1;

        //branching mux handled in MEM section

        /* ---------------------- ID stage --------------------- */
        /*
        gets pc + 1
        gets valA + valB from instruction field and register memory
        gets offset
        gets instruction
        */

        //setting pipeline values default if no errors
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        newState.IDEX.instr = state.IFID.instr;
        newState.IDEX.valA = state.reg[field0(state.IFID.instr)];
        newState.IDEX.valB = state.reg[field1(state.IFID.instr)];
        newState.IDEX.offset = convertNum(field2(state.IFID.instr));

        //consider hazards (lw followed by instr that uses loaded reg)
        bool id_hazard = (opcode(state.IDEX.instr) == LW) && ((field1(newState.IDEX.instr) == field1(state.IDEX.instr)) || (field0(newState.IDEX.instr) == field1(state.IDEX.instr)));
        //true if previous instruction is LW and either regA or regB of next instr uses the loaded register from lw

        if(id_hazard)
        {
            newState.IDEX.instr = NOOPINSTR;
            newState.pc = state.pc; //dont increment pc
            newState.IFID = state.IFID; //reverts default changes from above
        }
        
        /* ---------------------- EX stage --------------------- */

        /*
        gets target from pc + 1 + offset
        adds valA + offset/valB based on instruction offset for lw/sw, valB for add/nor
        checks if valA == offset/valB
        gets valB
        gets instruction
        */

        //DATA HAZARD WITH NO STALL -> DATA FORWARDING
        newState.EXMEM.instr = state.IDEX.instr; //pass instruction along
        newState.EXMEM.branchTarget = state.IDEX.offset + state.IDEX.pcPlus1; //find branch target

        int EX_op = opcode(state.IDEX.instr);
        int regA = field0(state.IDEX.instr);
        int regB = field1(state.IDEX.instr);

        //check for hazard in EX/MEM lecture 14
        int WB_dest = getDestReg(opcode(state.WBEND.instr));
        int MEM_dest = getDestReg(opcode(state.MEMWB.instr));

        //defualt valA/B values
        int valA = state.IDEX.valA, valB = state.IDEX.valB;

        if(MEM_dest == regA) //MEM TAKES PRIORITY
        {
            valA = state.MEMWB.writeData;
        }
        else if(WB_dest == regA)
        {
            valA = state.WBEND.writeData;
        }

        if(MEM_dest == regB) //MEM TAKES PRIORITY
        {
            valB = state.MEMWB.writeData;
        }
        else if(WB_dest == regB)
        {
            valB = state.WBEND.writeData;
        }
        //alu functionality

        if (EX_op == ADD)
            newState.EXMEM.aluResult = valA + valB;
        else if (EX_op == NOR)
            newState.EXMEM.aluResult = ~(valA | valB);
        else if (EX_op == LW || EX_op == SW)
            newState.EXMEM.aluResult = valA + state.IDEX.offset;
        else
            newState.EXMEM.aluResult = 0;

        if(valA == valB)newState.EXMEM.eq = 1;
        newState.EXMEM.valB = valB; //pass valB along
        
        /* --------------------- MEM stage --------------------- */
        /*
        gets dataMem or aluresult based on lw/sw
        store valb into datamem[aluresult]
        gets instruction
        if 
        */
        newState.MEMWB.instr = state.EXMEM.instr;

        int MEMop = opcode(state.EXMEM.instr);
        newState.MEMWB.writeData = state.EXMEM.aluResult; // directly load aluresult as default, would be discarded if its a sw/beq/noop/halt

        if (MEMop == LW)
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult]; // load from data memory
        else if (MEMop == SW)
            newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.valB; //store reg value to memory
        else if (MEMop == BEQ && state.EXMEM.eq == 1) //take the branch
        {
            newState.pc = state.EXMEM.branchTarget; //change the pc to the target
            newState.IFID.instr = NOOPINSTR; //add 3 noops to flush pipeline
            newState.IDEX.instr = NOOPINSTR;
            newState.EXMEM.instr = NOOPINSTR;
        }

        /* ---------------------- WB stage --------------------- */
        /*
        gets destination register based on instruction type
        gets write data from memory stage
        writes data to register memory
        gets instruction
        */
        newState.WBEND.writeData = state.MEMWB.writeData;
        newState.WBEND.instr = state.MEMWB.instr;
        int WBop = opcode(state.MEMWB.instr), WBdest;

        if (WBop == ADD || WBop == NOR)
        {
            WBdest = field2(state.MEMWB.instr); // destination register for add/nor
            newState.reg[WBdest] = state.MEMWB.writeData;
        }
        else if (WBop == LW)
        {
            WBdest = field1(state.MEMWB.instr); // destination register for lw
            newState.reg[WBdest] = state.MEMWB.writeData; //writes back to the register
        }
        /* ------------------------ END ------------------------ */
        state = newState; /* this is the last statement before end of the loop. It marks the end
        of the cycle and updates the current state with the values calculated in this cycle */
    }
    printf("Machine halted\n");
    printf("Total of %d cycles executed\n", state.cycles);
    printf("Final state of machine:\n");
    printState(&state);
}

/*
 * DO NOT MODIFY ANY OF THE CODE BELOW.
 */

void printInstruction(int instr)
{
    const char *instr_opcode_str;
    int instr_opcode = opcode(instr);
    if (ADD <= instr_opcode && instr_opcode <= NOOP)
    {
        instr_opcode_str = opcode_to_str_map[instr_opcode];
    }

    switch (instr_opcode)
    {
    case ADD:
    case NOR:
    case LW:
    case SW:
    case BEQ:
        printf("%s %d %d %d", instr_opcode_str, field0(instr), field1(instr), convertNum(field2(instr)));
        break;
    case JALR:
        printf("%s %d %d", instr_opcode_str, field0(instr), field1(instr));
        break;
    case HALT:
    case NOOP:
        printf("%s", instr_opcode_str);
        break;
    default:
        printf(".fill %d", instr);
        return;
    }
}

void printState(stateType *statePtr)
{
    printf("\n@@@\n");
    printf("state before cycle %d starts:\n", statePtr->cycles);
    printf("\tpc = %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (int i = 0; i < statePtr->numMemory; ++i)
    {
        printf("\t\tdataMem[ %d ] = 0x%08X\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i = 0; i < NUMREGS; ++i)
    {
        printf("\t\treg[ %d ] = %d\n", i, statePtr->reg[i]);
    }

    // IF/ID
    printf("\tIF/ID pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->IFID.instr);
    printInstruction(statePtr->IFID.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IFID.pcPlus1);
    if (opcode(statePtr->IFID.instr) == NOOP)
    {
        printf(" (Don't Care)");
    }
    printf("\n");

    // ID/EX
    int idexOp = opcode(statePtr->IDEX.instr);
    printf("\tID/EX pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->IDEX.instr);
    printInstruction(statePtr->IDEX.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IDEX.pcPlus1);
    if (idexOp == NOOP)
    {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\tvalA = %d", statePtr->IDEX.valA);
    if (idexOp >= HALT || idexOp < 0)
    {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\tvalB = %d", statePtr->IDEX.valB);
    if (idexOp == LW || idexOp > BEQ || idexOp < 0)
    {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\toffset = %d", statePtr->IDEX.offset);
    if (idexOp != LW && idexOp != SW && idexOp != BEQ)
    {
        printf(" (Don't Care)");
    }
    printf("\n");

    // EX/MEM
    int EX_op = opcode(statePtr->EXMEM.instr);
    printf("\tEX/MEM pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->EXMEM.instr);
    printInstruction(statePtr->EXMEM.instr);
    printf(" )\n");
    printf("\t\tbranchTarget %d", statePtr->EXMEM.branchTarget);
    if (EX_op != BEQ)
    {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\teq ? %s", (statePtr->EXMEM.eq ? "True" : "False"));
    if (EX_op != BEQ)
    {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\taluResult = %d", statePtr->EXMEM.aluResult);
    if (EX_op > SW || EX_op < 0)
    {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\tvalB = %d", statePtr->EXMEM.valB);
    if (EX_op != SW)
    {
        printf(" (Don't Care)");
    }
    printf("\n");

    // MEM/WB
    int MEM_op = opcode(statePtr->MEMWB.instr);
    printf("\tMEM/WB pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->MEMWB.instr);
    printInstruction(statePtr->MEMWB.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->MEMWB.writeData);
    if (MEM_op >= SW || MEM_op < 0)
    {
        printf(" (Don't Care)");
    }
    printf("\n");

    // WB/END
    int WB_op = opcode(statePtr->WBEND.instr);
    printf("\tWB/END pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->WBEND.instr);
    printInstruction(statePtr->WBEND.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->WBEND.writeData);
    if (WB_op >= SW || WB_op < 0)
    {
        printf(" (Don't Care)");
    }
    printf("\n");

    printf("end state\n");
    fflush(stdout);
}

// File
#define MAXLINELENGTH 1000 // MAXLINELENGTH is the max number of characters we read

void readMachineCode(stateType *state, char *filename)
{
    char line[MAXLINELENGTH];
    FILE *filePtr = fopen(filename, "r");
    if (filePtr == NULL)
    {
        printf("error: can't open file %s", filename);
        exit(1);
    }

    printf("instruction memory:\n");
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state->numMemory)
    {
        if (sscanf(line, "%x", state->instrMem + state->numMemory) != 1)
        {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("\tinstrMem[ %d ] = 0x%08X ( ", state->numMemory,
               state->instrMem[state->numMemory]);
        printInstruction(state->dataMem[state->numMemory] = state->instrMem[state->numMemory]);
        printf(" )\n");
    }
}
