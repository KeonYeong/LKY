#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

// Struct for save label info
typedef struct lableStruct
{
    char label[MAXLINELENGTH];          // Label
    int labelAddress;                   // Label's PC
    char directiveAddr[MAXLINELENGTH];  // Directive address
}labelType;

int numLabel(FILE *, char*);                // Check for number of labels
int isLabel(FILE *, char *, labelType *);   // Save label's info
/* Read from file and parse */
int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);                       // Check string is number

/* Make binarycode */
int makeBinary(char *, char *, char *, char *, char *, labelType *);
void makeOpcode(char *, char*, int *);      // Make OPcode
int PC = -1;        // PC
int cnt = 0;        // Count number of labels
int labelNum = 0;   // Number of labels

    int
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
    arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    int output = 0;         // Variable for binarycode
    // Make infoLabel structure by malloc
    labelType *infoLabel;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // Find Label
    while(!feof(inFilePtr))
    {
        if(!numLabel(inFilePtr,label)){
            /* Reached end of file */
        }
    }

    rewind(inFilePtr);

    infoLabel = (labelType*)malloc(cnt*sizeof(labelType));

    // Save label
    while (!feof(inFilePtr))
    {
        if (!isLabel(inFilePtr, label, infoLabel)) {
            /* Reached end of file */
        }
    }

    // Move file pointer to the top
    rewind(inFilePtr);
    // Initialize PC
    PC = -1;

    // Parsing the instruction
    while (!feof(inFilePtr))
    {
        if (!readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
            /* Reached end of file */
        }
        else
        {
            // Make binarycode
            output = makeBinary(label, opcode, arg0, arg1, arg2, infoLabel);
            // Printout to result file
            fprintf(outFilePtr, "%d\n",output);
        }
    }

    free(infoLabel);
    //printf("Succesfully make Result.txt!\n");
    return(0);
}


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
    int
numLabel(FILE *inFilePtr, char *label)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* Reached end of file */
        return(0);
    }

    if (strchr(line, '\n') == NULL) {
        /* Line too long */
        printf("error: line too long in Label\n");
    //    exit(1);
    }

    /* Is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* Successfully read label; advance pointer over the label */
        cnt++;
    }
    return 1;
}


// Check for label and save label
    int
isLabel(FILE *inFilePtr, char *label, labelType *infoLabel)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    char opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], temp[MAXLINELENGTH];
    int size = 0;

    // Initialize previous label
    label[0] = '\0';
    PC++;

    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* Reached end of file */
        return(0);
    }

    if (strchr(line, '\n') == NULL) {
        /* Line too long */
        printf("error: line too long in Label\n");
   //     exit(1);
    }

    /* Is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* Successfully read label; advance pointer over the label */
        // Save label at structure
        strcpy(infoLabel[labelNum].label, label);
        size = strlen(label);
        infoLabel[labelNum].label[size] = '\0';
        infoLabel[labelNum].labelAddress = PC;

        ptr += strlen(label);
        // Read OPcode and arg0
        sscanf(ptr,"%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0);

        // If directive
        if(!strcmp(opcode, ".fill"))
        {
            // Directive has number info
            if(isNumber(arg0))
            {
                // Save arg0(number) to structure
                strcpy(infoLabel[labelNum].directiveAddr, arg0);
                size = strlen(arg0);
                infoLabel[labelNum].directiveAddr[size] = '\0';
            }
            // Directive has String info
            else
            {
                // Find the label and save that label's PC
                for(int i =0; i < cnt ; i++)
                {
                    if(!strcmp(arg0, infoLabel[i].label))
                    {
                        sprintf(temp, "%d", infoLabel[i].labelAddress);
                        strcpy(infoLabel[labelNum].directiveAddr, temp);
                        size = strlen(arg0);
                        infoLabel[labelNum].directiveAddr[size] = '\0';
                        break;
                    }
                }
            }
        }
        // Find for next label
        labelNum++;
        return 1;
    }
}


// Read from file and parse the instruction
    int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
        char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    PC ++;

    /* Delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* Read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* Reached end of file */
        return(0);
    }

    /* Check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* Line too long */
        printf("error: line too long in parsing\n");
//        exit(1);
    }

    /* Is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* Successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
            opcode, arg0, arg1, arg2);

    return(1);

}

// Check for string contains number
    int
isNumber(char *string)
{
    /* Return 1 if string is a number */
    int i;
    return((sscanf(string, "%d", &i)) == 1);
}

// Make binary code
    int
makeBinary(char *label, char *opcode, char *arg0, char *arg1, char *arg2, labelType *infoLabel)
{
    int tempOpcode = 0;
    int tempArg0 = atoi(arg0);  // Change string to integer
    int tempArg1 = atoi(arg1);
    int tempArg2 = atoi(arg2);
    int output = 0;

    makeOpcode(label, opcode, &tempOpcode);
    // Shift left as much as need
    tempArg0 = tempArg0 << 19;
    tempArg1 = tempArg1 << 16;
    // If arg2 is not a number but string should go here
    if(!isNumber(arg2))
    {
        // If beq instruction
        if(!strcmp(opcode, "beq"))
        {
            // Find the offset label
            for(int i = 0; i < cnt; i++)
            {
                if(!strcmp(arg2, infoLabel[i].label))
                {
                    // Convert 32bit to 16bit 2's complement
                    tempArg2 = (infoLabel[i].labelAddress - (PC+1)) & 65535;
                    break;
                }
            }

        }
        // For lw and sw instruction
        else
        {
            // Find thr offset address
            for(int i = 0; i < cnt; i++)
            {
                if(!strcmp(arg2, infoLabel[i].label))
                {
                    tempArg2 = infoLabel[i].labelAddress;
                    break;
                }
            }
        }
    }

    // Directive should go here
    if(!strcmp(opcode, ".fill"))
    {
        for(int i = 0; i < cnt; i++)
        {
            if(!strcmp(label, infoLabel[i].label))
            {
                output = atoi(infoLabel[i].directiveAddr);
                break;
            }
        }
    }
    // Other instructions should go here
    else
        output = (tempOpcode | tempArg0 | tempArg1 | tempArg2);
    printf("%d\n", output);
    return output;
}

//Make OPcode
    void
makeOpcode(char *label, char *opcode, int *output)
{
    // Set OPcode and shift left as much as need
    if (!strcmp(opcode, "add"))
    {
        *output = 0x0;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "nor"))
    {
        *output = 0x1;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "lw"))
    {
        *output = 0x2;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "sw"))
    {
        *output = 0x3;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "beq"))
    {
        *output = 0x4;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "jalr"))
    {
        *output = 0x5;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "halt"))
    {
        *output = 0x6;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, "noop"))
    {
        *output = 0x7;
        *output = *output << 22;
    }

    else if (!strcmp(opcode, ".fill")) {}

    else
    {
        printf("Wrong Opcode !! : %s\n",opcode);
    }
}
