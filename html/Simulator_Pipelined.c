#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 // Maximum number of words in memory
#define NUMREGS 8		// Number of machine registers
#define MAXLINELENGTH 1000

#define ADD 0
#define NOR 1
#define LW 2 
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

/* File pointer for last result */
FILE *fp;

/* Structure for each stage*/
typedef struct IFIDStruct {
    int instr;
    int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
    int pcPlus1;
    int readRegA;
    int readRegB;
    int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
    int branchTarget;
    int aluResult;
    int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
    int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
    int writeData;
} WBENDType;

typedef struct stateStruct {
	int pc;					// Program counter
	int instrMem[NUMMEMORY];// Contents
    int dataMem[NUMMEMORY]; // Data memory
	int reg[NUMREGS];		// Register	
	int numMemory;			// Mem address
    int cycles;             // Total cycles
    int dataHazardBit;      // Comparison bit for data hazard
    IFIDType IFID;
    IDEXType IDEX;
    EXMEMType EXMEM;
    MEMWBType MEMWB;
    WBENDType WBEND;
} stateType;

void printFile(stateType*);     // Print present state at file
void printState(stateType *);	// Print present state
int field0(int);                // Get field0 value
int field1(int);                // Get field1 value
int field2(int);                // Get field2 value
int opcode(int);                // Get OPcode
void printInstrFile(int);       // Print instr's info at file
void printInstruction(int);     // Print instr's info
int convertNum(int);			// Convert 2's complement 16bit number to 32bit number
void mvQueue(int*, int);        // Move queue contents
int isDataHazard(int*, int*);   // Detect data hazard
void forwarding(stateType*, stateType*, int*, int*, int*, int*);    // Forwarding data to resolve data hazard
void flushPipelineReg(stateType*);          // Flush pipeline register before MEM/WB
void doADD(stateType*, stateType*, int);    // Do ADD in EX stage
void doNOR(stateType*, stateType*, int);    // Do NOR in EX stage
void doLW(stateType*, stateType*, int);     // Do LW in EX stage
void doSW(stateType*, stateType*, int);     // Do SW in EX stage
void doBEQ(stateType*, stateType*, int);    // Do BEQ in EX stage
void doOTHER(stateType*, stateType*);       // Do other instruction in EX Stage
void run(stateType *, stateType *);			// Run simulator

int
main(int argc, char *argv[])
{
	char line[MAXLINELENGTH];
	stateType state, newState;
	FILE *filePtr, *outFilePtr;
	
    fp = fopen("result.txt", "w");
    
    if(argc != 2) 
	{
		printf("error: usage: %s <machine-code file>\n", argv[0]);
		exit(1);
	}

	filePtr = fopen(argv[1], "r");
	if(filePtr == NULL) 
	{
		printf("error: can't open file %s", argv[1]);
		perror("fopen");
		exit(1);
	}
	
    // Read in the entire machine-code file into memory
	for(state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
		state.numMemory++) 
	{
		if(sscanf(line, "%d", state.instrMem + state.numMemory) != 1) 
		{
			printf("error in reading address %d\n", state.numMemory);
			exit(1);
		}
		
        if(sscanf(line, "%d", state.dataMem + state.numMemory) != 1) 
		{
			printf("error in reading address %d\n", state.numMemory);
			exit(1);
		}
		printf("memory[%d]=%d\n", state.numMemory, state.instrMem[state.numMemory]);
//		fprintf(fp, "%d %d\n", state.numMemory, state.instrMem[state.numMemory]);
    }

    // Print out initial state
    printf("%d memory words\n",state.numMemory);
    printf("\tinstruction memory:\n");
    for(int i = 0; i < state.numMemory; i++)
    {
        printf("\t\tinstrMem[ %d ] ",i);
        printInstruction(state.instrMem[i]);
    }

    // Print out file
//    fprintf(fp, "%d\n",state.numMemory);
//    for(int i = 0; i < state.numMemory; i++)
//    {
//        fprintf(fp, "%d ",i);
//        printInstrFile(state.instrMem[i]);
//    }


	// Initialize registers & PC
	for(int i = 0; i < 8; i++)
		state.reg[i] = 0;
	state.pc = 0;
    state.cycles = 0;

	// Run simulator
	run(&state, &newState);
    fclose(fp);
	return(0);
}

void
printFile(stateType *statePtr)
{
    int i;
    fprintf(fp, "%d\n", statePtr->cycles);
    fprintf(fp, "%d\n", statePtr->pc);

    //printf("\tdata memory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        fprintf(fp, "%s%d: %d\n", "DataMem", i, statePtr->dataMem[i]);
    }

    //printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        fprintf(fp, "%s%d: %d\n", "Reg", i, statePtr->reg[i]);
    }

    //printf("\tIFID:\n");
    //printf("\t\tinstruction ");
    printInstrFile(statePtr->IFID.instr);
    fprintf(fp, "%d\n", statePtr->IFID.pcPlus1);

    printInstrFile(statePtr->IDEX.instr);
    fprintf(fp, "%d\n", statePtr->IDEX.pcPlus1);
    fprintf(fp, "%d\n", statePtr->IDEX.readRegA);
    fprintf(fp, "%d\n", statePtr->IDEX.readRegB);
    fprintf(fp, "%d\n", statePtr->IDEX.offset);

    printInstrFile(statePtr->EXMEM.instr);
    fprintf(fp, "%d\n", statePtr->EXMEM.branchTarget);
    fprintf(fp, "%d\n", statePtr->EXMEM.aluResult);
    fprintf(fp, "%d\n", statePtr->EXMEM.readRegB);

    printInstrFile(statePtr->MEMWB.instr);
    fprintf(fp, "%d\n", statePtr->MEMWB.writeData);

    printInstrFile(statePtr->WBEND.instr);
    fprintf(fp, "%d\n", statePtr->WBEND.writeData);

    return ;
}


// Print present state
void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
    }

    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }

    printf("\tIFID:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IFID.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);

    printf("\tIDEX:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->IDEX.instr);
    printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
    printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
    printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
    printf("\t\toffset %d\n", statePtr->IDEX.offset);

    printf("\tEXMEM:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->EXMEM.instr);
    printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
    printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
    printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);

    printf("\tMEMWB:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->MEMWB.instr);
    printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);

    printf("\tWBEND:\n");
    printf("\t\tinstruction ");
    printInstruction(statePtr->WBEND.instr);
    printf("\t\twriteData %d\n", statePtr->WBEND.writeData);

    return ;
}

int
field0(int instruction)
{
    return ((instruction>>19) & 0x7);
}

int
field1(int instruction)
{
    return ((instruction>>16) & 0x7);
}

int
field2(int instruction)
{
    return(instruction & 0xffff);
}

int
opcode(int instruction)
{
    return(instruction>>22);
}

void
printInstrFile(int instr)
{
    char opcodeString[10];

    if(opcode(instr) == ADD) { strcpy(opcodeString, "add"); }
    else if(opcode(instr) == NOR) { strcpy(opcodeString, "nor"); }
    else if(opcode(instr) == LW) { strcpy(opcodeString, "lw"); }
    else if(opcode(instr) == SW) { strcpy(opcodeString, "sw"); }
    else if(opcode(instr) == BEQ) { strcpy(opcodeString, "beq"); }
    else if(opcode(instr) == JALR) { strcpy(opcodeString, "jalr"); }
    else if(opcode(instr) == HALT) { strcpy(opcodeString, "halt"); }
    else if(opcode(instr) == NOOP) { strcpy(opcodeString, "noop"); }
    else { strcpy(opcodeString, "data"); }

    fprintf(fp, "%s %d %d %d\n", opcodeString, field0(instr), field1(instr), field2(instr));
}

void
printInstruction(int instr)
{
    char opcodeString[10];

    if(opcode(instr) == ADD) { strcpy(opcodeString, "add"); }
    else if(opcode(instr) == NOR) { strcpy(opcodeString, "nor"); }
    else if(opcode(instr) == LW) { strcpy(opcodeString, "lw"); }
    else if(opcode(instr) == SW) { strcpy(opcodeString, "sw"); }
    else if(opcode(instr) == BEQ) { strcpy(opcodeString, "beq"); }
    else if(opcode(instr) == JALR) { strcpy(opcodeString, "jalr"); }
    else if(opcode(instr) == HALT) { strcpy(opcodeString, "halt"); }
    else if(opcode(instr) == NOOP) { strcpy(opcodeString, "noop"); }
    else { strcpy(opcodeString, "data"); }

    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr), field2(instr));
}

// Convert a 16-bit number into a 32-bit Linux integer
int
convertNum(int num)
{
	if (num & (1 << 15)) 
		num -= (1 << 16);
	
    return(num);
}

// Move queue contents
void
mvQueue(int queue[], int new)
{
    for(int i = 2; i >= 0; i--)
        queue[i+1] = queue[i];
    
    queue[0] = new;
}

// Make data hazard comparison 4-bit
int
isDataHazard(int isDataHazardA[], int isDataHazardB[])
{
    char detectDataHazard[6] = {'0','0','0','0','0','0'};
    int num = 0;

    for(int i = 0; i < 3; i++)
        if(isDataHazardA[i] == 1)
            detectDataHazard[i] ='1';

    for(int i = 0; i < 3; i++)
        if(isDataHazardB[i] == 1)
            detectDataHazard[i+3] ='1';

    num = atoi(detectDataHazard);

    return num;
}

// Forwarding for data hazard
void
forwarding(stateType *state, stateType *newState, int *isLW, int queue[], int isDataHazardA[], int isDataHazardB[])
{
    // Initialize data hazard array
    for(int i = 0; i < 3; i++)
        isDataHazardA[i] = isDataHazardB[i] = 0;

    // NOOP and HALT just go down
    if(opcode(state->IFID.instr) == NOOP | opcode(state->IFID.instr) == HALT)
    {
        newState->IDEX.readRegA = state->reg[field0(state->IFID.instr)];
        newState->IDEX.readRegB = state->reg[field1(state->IFID.instr)];
        mvQueue(queue, -1);
    }
    // Else instruction go here
    else
    {
        int isDataHazard = 0;

        // For ADD, NOR and BEQ
        if(opcode(state->IFID.instr) == ADD || opcode(state->IFID.instr) == NOR || opcode(state->IFID.instr) == BEQ)
        {
            // Search for queue
            for(int i = 0; i < 4; i++)
            {
                // Compare regA and destination register
                if(queue[i] == field0(state->IFID.instr))
                {
                    if(i == 3)
                        break;
                    isDataHazardA[i] = 1;
                    newState->IDEX.readRegB = state->reg[field1(state->IFID.instr)];
                    isDataHazard = 1;
                    break;
                }
            }
            for(int i = 0; i< 4; i++)
            {
                // Compare regB and destination register
                if(queue[i] == field1(state->IFID.instr))
                {
                    if(i == 3)
                        break;
                    isDataHazardB[i] = 1;
                    newState->IDEX.readRegA = state->reg[field0(state->IFID.instr)];
                    isDataHazard = 1;
                    break;
                }
            }
        }

        // For LW and SW
        else if(opcode(state->IFID.instr) == LW || opcode(state->IFID.instr) == SW)
        {
            for(int i = 0; i < 4; i++)
            {
                // Compare source register and destination register
                if(queue[i] == field0(state->IFID.instr))
                {
                    if(i == 3)
                        break;
                    isDataHazardA[i] = 1;
                    newState->IDEX.readRegB = state->reg[field1(state->IFID.instr)];
                    isDataHazard = 1;
                    break;
                }
            }
            for(int i = 0; i< 4; i++)
            {
                // Compare offset register and destination register
                if(queue[i] == field2(state->IFID.instr))
                {
                    if(i == 3)
                        break;
                    isDataHazardB[i] = 1;
                    newState->IDEX.readRegA = state->reg[field0(state->IFID.instr)];
                    isDataHazard = 1;
                    break;
                }
            }
        }

        // If there is data hazard, go here
        if(isDataHazard == 1)
        {
            if(*isLW)
            {
                // If LW intruction has data hazard, stall 1 cycle
                if(isDataHazardA[0] == 1 || isDataHazardB[0] == 1)
                {
                    newState->pc --;
                    newState->IFID.instr = state->IFID.instr;
                    newState->IDEX.instr = 0x1c00000;

                    // Change hazard array
                    if(isDataHazardA[0] == 1)
                    {
                        isDataHazardA[0] = 0;
                        isDataHazardA[1] = 1;
                    }
                    else
                    {
                        isDataHazardB[0] = 0;
                        isDataHazardB[1] = 1;
                    }
                    mvQueue(queue, -1);
                    *isLW = 0;  
                }
                else
                    mvQueue(queue, field1(state->IFID.instr));
            }
            else if(opcode(state->IDEX.instr) == LW)
            {
                if(isDataHazardA[0] == 1 || isDataHazardB[0] == 1)
                {
                    newState->pc--;
                    newState->IFID.instr = state->IFID.instr;
                    newState->IDEX.instr = 0x1c00000;

                    if(isDataHazardA[0] == 1)
                    {
                        isDataHazardA[0] = 0;
                        isDataHazardB[1] = 1;
                    }
                    else
                    {
                        isDataHazardA[0] = 0;
                        isDataHazardB[1] = 1;
                    }
                    mvQueue(queue,-1);
                }
                else
                    mvQueue(queue,field1(state->IFID.instr));
            }
            else if(opcode(state->IFID.instr) == LW || opcode(state->IFID.instr) == SW)
                mvQueue(queue, field1(state->IFID.instr));
            else
                mvQueue(queue, field2(state->IFID.instr));
        }

        // If there isn't any data hazard, go here
        else if(isDataHazard == 0)
        {
            newState->IDEX.readRegA = state->reg[field0(state->IFID.instr)];
            newState->IDEX.readRegB = state->reg[field1(state->IFID.instr)];
            if(opcode(state->IFID.instr) == LW || opcode(state->IFID.instr) == SW)
                mvQueue(queue, field1(state->IFID.instr));
            else
                mvQueue(queue, field2(state->IFID.instr));
        }
    }
    // Make comparison bit
    newState->dataHazardBit = isDataHazard(isDataHazardA, isDataHazardB);
}

// Flush pipe line registers need to be changed
void
flushPipelineReg(stateType *newState)
{
    newState->IFID.instr = 0x1c00000;           
    newState->IDEX.instr = 0x1c00000;
    newState->IDEX.pcPlus1 = 0;
    newState->IDEX.readRegA = 0;
    newState->IDEX.readRegB = 0;
    newState->IDEX.offset = 0;
    newState->EXMEM.instr = 0x1c00000;
    newState->EXMEM.branchTarget = 0;
    newState->EXMEM.aluResult = 0;
    newState->EXMEM.readRegB = 0;
}

// Do ADD in EX stage
void
doADD(stateType *state, stateType *newState, int dataHazardBit)
{
    switch(dataHazardBit)
    {
        // Data hazard for regA before 1 cycle
        case 100000:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->IDEX.readRegB;
            break;
     
        // Data hazard for regB before 1 cycle
        case 100:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->EXMEM.aluResult;
            break;
     
        // Data hazard for regA before 2 cycles
        case 10000:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->IDEX.readRegB;
            break;
     
        // Data hazard for regB before 2 cycles
        case 10:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->MEMWB.writeData;
            break;
        
        // Data hazard for regA before 3 cycles
        case 1000:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->IDEX.readRegB;
            break;

        // Data hazard for regB before 3 cycles
        case 1:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->WBEND.writeData;
            break;

        // Data hazard for regA before 1 cycle and for regB before 2 cycles
        case 100010:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->MEMWB.writeData;
            break;
        
        // Data hazard for regA before 1 cycle and for regB before 3 cycles
        case 100001:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->WBEND.writeData;
            break;

        // Data hazard for regA before 2 cycles and for regB before 1 cycle
        case 10100:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->EXMEM.aluResult;
            break;

        // Data hazard for regA before 2 cycles and for regB before 3 cycles
        case 10001:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->WBEND.writeData;
            break;

        // Data hazard for regA before 3 cycles and for regB before 1 cycle
        case 1100:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->EXMEM.aluResult;
            break;

        // Data hazard for regA before 3 cycles and for regB before 2 cycles
        case 1010:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->MEMWB.writeData;
            break;

        default :
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->IDEX.readRegB;
            break;
    }
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
    newState->EXMEM.readRegB = state->IDEX.readRegB;
}

// Do NOR in EX stage
void
doNOR(stateType *state, stateType *newState, int dataHazardBit)
{
    switch(dataHazardBit)
    {
        case 100000:
            newState->EXMEM.aluResult = ~(state->EXMEM.aluResult | state->IDEX.readRegB);
            break;
     
        case 100:
            newState->EXMEM.aluResult = ~(state->IDEX.readRegA | state->EXMEM.aluResult);
            break;
     
        case 10000:
            newState->EXMEM.aluResult = ~(state->MEMWB.writeData | state->IDEX.readRegB);
            break;
     
        case 10:
            newState->EXMEM.aluResult = ~(state->IDEX.readRegA | state->MEMWB.writeData);
            break;
        
        case 1000:
            newState->EXMEM.aluResult = ~(state->WBEND.writeData | state->IDEX.readRegB);
            break;

        case 1:
            newState->EXMEM.aluResult = ~(state->IDEX.readRegA | state->WBEND.writeData);
            break;

        case 100010:
            newState->EXMEM.aluResult = ~(state->EXMEM.aluResult | state->MEMWB.writeData);
            break;
        
        case 100001:
            newState->EXMEM.aluResult = ~(state->EXMEM.aluResult | state->WBEND.writeData);
            break;

        case 10100:
            newState->EXMEM.aluResult = ~(state->MEMWB.writeData | state->EXMEM.aluResult);
            break;

        case 10001:
            newState->EXMEM.aluResult = ~(state->MEMWB.writeData | state->WBEND.writeData);
            break;

        case 1100:
            newState->EXMEM.aluResult = ~(state->WBEND.writeData | state->EXMEM.aluResult);
            break;

        case 1010:
            newState->EXMEM.aluResult = ~(state->WBEND.writeData | state->MEMWB.writeData);
            break;

        default :
            newState->EXMEM.aluResult = ~(state->IDEX.readRegA | state->IDEX.readRegB);
            break;
    }
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
    newState->EXMEM.readRegB = state->IDEX.readRegB;
}

// Do LW in EX stage
void
doLW(stateType *state, stateType *newState, int dataHazardBit)
{
    switch(dataHazardBit)
    {
        case 100000:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->IDEX.offset;
            break;

        case 100:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->EXMEM.aluResult;
            break;

        case 10000:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->IDEX.offset;
            break;

        case 10:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->MEMWB.writeData;
            break;

        case 1000:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->IDEX.offset;
            break;

        case 1:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->WBEND.writeData;
            break;

        case 100010:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->MEMWB.writeData;
            break;

        case 100001:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->WBEND.writeData;
            break;

        case 10100:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->EXMEM.aluResult;
            break;

        case 10001:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->WBEND.writeData;
            break;

        case 1100:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->EXMEM.aluResult;
            break;

        case 1010:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->MEMWB.writeData;
            break;

        default :
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->IDEX.offset;
            break;
    }
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
    newState->EXMEM.readRegB = state->IDEX.readRegB;
}

// Do SW in EX stage
void
doSW(stateType *state, stateType *newState, int dataHazardBit)
{
    switch(dataHazardBit)
    {
        case 100000:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->IDEX.offset;
            break;

        case 100:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->EXMEM.aluResult;
            break;

        case 10000:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->IDEX.offset;
            break;

        case 10:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->MEMWB.writeData;
            break;

        case 1000:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->IDEX.offset;
            break;

        case 1:
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->WBEND.writeData;
            break;

        case 100010:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->MEMWB.writeData;
            break;

        case 100001:
            newState->EXMEM.aluResult = state->EXMEM.aluResult + state->WBEND.writeData;
            break;

        case 10100:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->EXMEM.aluResult;
            break;

        case 10001:
            newState->EXMEM.aluResult = state->MEMWB.writeData + state->WBEND.writeData;
            break;

        case 1100:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->EXMEM.aluResult;
            break;

        case 1010:
            newState->EXMEM.aluResult = state->WBEND.writeData + state->MEMWB.writeData;
            break;

        default :
            newState->EXMEM.aluResult = state->IDEX.readRegA + state->IDEX.offset;
            break;
    }
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
    newState->EXMEM.readRegB = state->IDEX.readRegB;
}

// Do BEQ in EX stage
void
doBEQ(stateType *state, stateType *newState, int dataHazardBit)
{
    switch(dataHazardBit)
    {
        case 100000:
            newState->EXMEM.aluResult = (state->EXMEM.aluResult == state->IDEX.readRegB)? 1 : 0;
            break;

        case 100:
            newState->EXMEM.aluResult = (state->IDEX.readRegA == state->EXMEM.aluResult)? 1 : 0;
            break;

        case 10000:
            newState->EXMEM.aluResult = (state->MEMWB.writeData == state->IDEX.readRegB)? 1 : 0;
            break;

        case 10:
            newState->EXMEM.aluResult = (state->IDEX.readRegA == state->MEMWB.writeData)? 1 : 0;
            break;

        case 1000:
            newState->EXMEM.aluResult = (state->WBEND.writeData == state->IDEX.readRegB)? 1 : 0;
            break;

        case 1:
            newState->EXMEM.aluResult = (state->IDEX.readRegA == state->WBEND.writeData)? 1 : 0;
            break;

        case 100010:
            newState->EXMEM.aluResult = (state->EXMEM.aluResult == state->MEMWB.writeData)? 1 : 0;
            break;

        case 100001:
            newState->EXMEM.aluResult = (state->EXMEM.aluResult == state->WBEND.writeData)? 1 : 0;
            break;

        case 10100:
            newState->EXMEM.aluResult = (state->MEMWB.writeData == state->EXMEM.aluResult)? 1 : 0;
            break;

        case 10001:
            newState->EXMEM.aluResult = (state->MEMWB.writeData == state->WBEND.writeData)? 1 : 0;
            break;

        case 1100:
            newState->EXMEM.aluResult = (state->WBEND.writeData == state->EXMEM.aluResult)? 1 : 0;
            break;

        case 1010:
            newState->EXMEM.aluResult = (state->WBEND.writeData == state->MEMWB.writeData)? 1 : 0;
            break;

        default :
            newState->EXMEM.aluResult = (state->IDEX.readRegA == state->IDEX.readRegB)? 1 : 0;
            break;
    }
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
    newState->EXMEM.readRegB = state->IDEX.readRegB;
}

// Do JALR, HALT, NOOP in EX stage
void
doOTHER(stateType *state, stateType *newState)
{
    newState->EXMEM.branchTarget = state->IDEX.pcPlus1 + state->IDEX.offset;
    newState->EXMEM.aluResult = state->IDEX.pcPlus1;
    newState->EXMEM.readRegB = state->IDEX.readRegB;
}

// Run simulator
void
run(stateType *state, stateType *newState)
{
    int isLW = 0;
    int queue[4] = {-1, -1, -1,-1};
    int isDataHazardA[3] = {0, 0, 0};
    int isDataHazardB[3] = {0, 0, 0};

    // Initialize pipe line register to "noop"
    state->IFID.instr  = 0x1c00000;
    state->IDEX.instr  = 0x1c00000;
    state->EXMEM.instr = 0x1c00000;
    state->MEMWB.instr = 0x1c00000;
    state->WBEND.instr = 0x1c00000;
    fprintf(fp, "%d\n", state->numMemory);
	while (1)
	{
        printState(state);
    	for(int i = 0; i < 4 ; i++)
            printf("queue[%d]: %d\n",i,queue[i]);

        printFile(state);
        for(int i = 0; i < 4; i++)
            fprintf(fp, "%d %d\n", i, queue[i]);

        /* Check for halt*/
        if(opcode(state->MEMWB.instr) == HALT)
        {
            printf("machine halted\n");
            printf("total of %d cycles executed\n",state->cycles);
            fprintf(fp, "%d\n", state->cycles);
            exit(0);
        }
        *newState = *state;
        newState->cycles++;
        
        /*------------------- IF  stage --------------------*/
        newState->IFID.instr = state->instrMem[state->pc];
        newState->IFID.pcPlus1 = state->pc+1;

        // Is there LW?
        if(opcode(state->instrMem[state->pc]) == LW)
            isLW = 1;
        
        /*------------------- ID  stage --------------------*/
        newState->IDEX.instr = state->IFID.instr;
        newState->IDEX.pcPlus1 = state->IFID.pcPlus1;
        
        // Forwarding if need 
        forwarding(state, newState, &isLW, queue, isDataHazardA, isDataHazardB);

        newState->IDEX.offset = convertNum(field2(state->IFID.instr));

        /*------------------- EX  stage --------------------*/
        newState->EXMEM.instr = state->IDEX.instr;

        switch(opcode(state->IDEX.instr))
        {
            case ADD:
                doADD(state, newState, state->dataHazardBit);
                break;
 
            case NOR:
                doNOR(state, newState, state->dataHazardBit);
                break;
               
             case LW:
                doLW(state, newState, state->dataHazardBit);
                break;

             case SW:
                doSW(state, newState, state->dataHazardBit);
                break;

             case BEQ:
                doBEQ(state, newState, state->dataHazardBit);
                break;

             case JALR:
                doOTHER(state, newState);
                break;

             case HALT:
                doOTHER(state, newState);
                break;

             case NOOP:
                doOTHER(state, newState);
                break;

            default:
                break;
        }    

        
        /*------------------- MEM stage --------------------*/
        newState->MEMWB.instr = state->EXMEM.instr;

        switch(opcode(state->EXMEM.instr))
        {
            case ADD:
                newState->MEMWB.writeData = state->EXMEM.aluResult;
                break;

            case NOR:
                newState->MEMWB.writeData = state->EXMEM.aluResult;
                break;

            case LW:
                newState->MEMWB.writeData = state->dataMem[state->EXMEM.aluResult];
                break;

            case SW:
                newState->dataMem[state->EXMEM.aluResult] = state->EXMEM.readRegB;
                break;

            case BEQ:
                // If branch taken go here
                if(state->EXMEM.aluResult == 1)
                {
                    newState->pc = state->EXMEM.branchTarget;
                    newState->pc--;   
                    // After change PC then flush pipeline registers need to be changed
                    flushPipelineReg(newState);
                }
                break;

            default :
                break;
        }
        
        /*------------------- WB  stage --------------------*/
        newState->WBEND.instr = state->MEMWB.instr;
        
        switch(opcode(state->MEMWB.instr))
        {
            case ADD:
                newState->WBEND.writeData = state->MEMWB.writeData;
                newState->reg[field2(state->MEMWB.instr)] = state->MEMWB.writeData;
                break;

            case NOR:
                newState->WBEND.writeData = state->MEMWB.writeData;
                newState->reg[field2(state->MEMWB.instr)] = state->MEMWB.writeData;
                break;

            case LW:
                newState->WBEND.writeData = state->MEMWB.writeData;
                newState->reg[field1(state->MEMWB.instr)] = state->MEMWB.writeData;
                break;

            default :
                break;
        }

        newState->pc++;
        // Marks the end of the cycle and updates the current state
        // with the values calcylates in this cycle
        *state = *newState;
	}
}

