#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Memory and registers
char M[300][4], IR[4], GR[4];
int C;
int PTR;
int job = 0;

// PCB structure
struct PCB
{
    char job[4], TTL[4], TLL[4];
} pcb;

// Global variables
int VA, RA, TTC, LLC, TTL, TLL, EM, SI, TI, PI, ttl, tll, l = -1, IC, pte, InValid = 0;
FILE *fin, *fout;
char line[100];
int random_filled[30];
int done = 0; // Flag to track if all jobs are processed

// Function declarations
void initialization();
void load();
void Pagetable();
void allocate();
void startExecution();
void executeProgram();
void AddMap();
void Examine();
void MOS();
void Terminate();
void read();
void write();

// Initialize variables and memory
void initialization()
{
    printf("Starting job No.: %d\n\n", ++job);
    SI = TI = PI = TTC = LLC = TTL = TLL = EM = VA = RA = IC = PTR = InValid = 0;
    for (int i = 0; i < 30; i++)
    {
        random_filled[i] = 0;
    }
    for (int i = 0; i < 4; i++)
    {
        IR[i] = '&';
        GR[i] = '_';
    }
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            M[i][j] = '_';
        }
    }
}

void load()
{
    while (fgets(line, sizeof(line), fin) != NULL)
    {
        int i, j;
        int len = strlen(line);
        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        // Handle $AMJ
        if (line[0] == '$' && line[1] == 'A' && line[2] == 'M' && line[3] == 'J')
        {
            initialization();
            // Validate $AMJ length
            if (strlen(line) != 16)
            {
                printf("Warning: Malformed $AMJ, using default values\n");
                sprintf(pcb.job, "%04d", job); // Unique Job ID
                strcpy(pcb.TTL, "0010");
                strcpy(pcb.TLL, "0010");
            }
            else
            {
                for (i = 4, j = 0; i < 8; i++, j++)
                {
                    pcb.job[j] = line[i];
                }
                for (i = 8, j = 0; i < 12; i++, j++)
                {
                    pcb.TTL[j] = line[i];
                }
                for (i = 12, j = 0; i < 16; i++, j++)
                {
                    pcb.TLL[j] = line[i];
                }
            }
            // Convert TTL and TLL
            ttl = ((pcb.TTL[0] - 48) * 1000 + (pcb.TTL[1] - 48) * 100 + (pcb.TTL[2] - 48) * 10 + (pcb.TTL[3] - 48));
            tll = ((pcb.TLL[0] - 48) * 1000 + (pcb.TLL[1] - 48) * 100 + (pcb.TLL[2] - 48) * 10 + (pcb.TLL[3] - 48));
            // Ensure positive ttl and tll
            if (ttl <= 0)
            {
                printf("Warning: Invalid TTL, setting to 10\n");
                ttl = 10;
                strcpy(pcb.TTL, "0010");
            }
            if (tll <= 0)
            {
                printf("Warning: Invalid TLL, setting to 10\n");
                tll = 10;
                strcpy(pcb.TLL, "0010");
            }
            printf("Parsed $AMJ: Job=%c%c%c%c, TTL=%d, TLL=%d\n",
                   pcb.job[0], pcb.job[1], pcb.job[2], pcb.job[3], ttl, tll);
            Pagetable();
            allocate();
        }
        // Handle $DTA
        if (line[0] == '$' && line[1] == 'D' && line[2] == 'T' && line[3] == 'A')
        {
            startExecution();
        }
        // Handle $END
        if (line[0] == '$' && line[1] == 'E' && line[2] == 'N' && line[3] == 'D')
        {
            continue; // Skip $END to prevent reprocessing
        }
    }
    done = 1; // Mark all jobs processed
}

// Create page table
void Pagetable()
{
    int i, j;
    PTR = (rand() % 29) * 10;
    random_filled[PTR / 10] = 1;
    printf("PTR: %d\n", PTR);
    for (i = PTR; i < PTR + 10; i++)
    {
        for (j = 0; j < 4; j++)
        {
            M[i][j] = '*';
        }
    }
}

// Allocate memory for instructions
void allocate()
{
    int check = 0;
    int i, j, pos = 0;
    int k = 0;
    char str[3];
    while (check != 1)
    {
        pos = (rand() % 29) * 10;
        while (random_filled[pos / 10] != 0)
        {
            pos = (rand() % 29) * 10;
        }
        random_filled[pos / 10] = 1;
        sprintf(str, "%d", pos);
        if (pos < 100)
        {
            M[PTR][2] = '0';
            M[PTR][3] = str[0];
        }
        else
        {
            M[PTR][2] = str[0];
            M[PTR][3] = str[1];
        }
        if (fgets(line, sizeof(line), fin) == NULL)
            break;
        int len = strlen(line);
        if (line[len - 1] == '\n')
            line[len - 1] = '\0';
        k = 0;
        i = 0;
        while (k < strlen(line))
        {
            // Skip spaces
            while (k < strlen(line) && line[k] == ' ')
                k++;
            if (k >= strlen(line))
                break;
            for (j = 0; j < 4; j++)
            {
                if (k < strlen(line) && line[k] != ' ')
                {
                    M[pos + i][j] = line[k];
                    k++;
                }
                else
                {
                    M[pos + i][j] = j == 0 ? '_' : '0'; // Pad with '0' for instructions
                }
            }
            if (M[pos + i][0] == 'H')
            {
                M[pos + i][0] = 'H';
                M[pos + i][1] = '0';
                M[pos + i][2] = '0';
                M[pos + i][3] = '0';
                check = 1;
                break;
            }
            i++;
        }
    }
}

// Start execution
void startExecution()
{
    IC = 0;
    executeProgram();
}

void executeProgram()
{
    int no;
    char v[3];
    v[0] = M[PTR][2];
    v[1] = M[PTR][3];
    v[2] = '\0';
    no = ((v[0] - 48) * 10) + (v[1] - 48);
    while (IR[0] != 'H')
    {
        for (int k = 0; k < 4; k++)
        {
            IR[k] = M[(no * 10) + IC][k];
        }
        if (IR[0] != 'H' && (!isdigit(IR[2]) || !isdigit(IR[3]) || isalpha(IR[2]) || isalpha(IR[3])))
        {
            PI = 2;
            if (TTC >= ttl)
                TI = 2;
            else
                TI = 0;
            MOS();
        }
        VA = ((IR[2] - 48) * 10) + (IR[3] - 48);
        AddMap();
        Examine();
    }
}

void AddMap()
{
    int pos;
    pte = PTR + (VA / 10);
    if (M[pte][3] == '*')
    {
        printf("Valid Page Fault Handled for VA=%d\n", VA);
        pos = (rand() % 29) * 10;
        while (random_filled[pos / 10] != 0)
            pos = (rand() % 29) * 10;
        random_filled[pos / 10] = 1;
        char str[10];
        sprintf(str, "%d", pos);
        if (pos / 100 == 0)
        {
            M[pte][2] = '0';
            M[pte][3] = str[0];
        }
        else
        {
            M[pte][2] = str[0];
            M[pte][3] = str[1];
        }
        PI = 0; // Reset PI after handling page fault
    }
    else
    {
        PI = 0;
    }
    int p = (M[pte][2] - 48) * 10 + (M[pte][3] - 48);
    RA = (p * 10) + (VA % 10);
    if (RA >= 300)
    {
        PI = 2;
        TI = 0;
        MOS();
    }
}

void Examine()
{
    printf("Executing IR: %c%c%c%c, TTC=%d, LLC=%d\n", IR[0], IR[1], IR[2], IR[3], TTC, LLC);
    if (IR[0] == 'G')
    {
        IC = IC + 1;
        if (IR[1] == 'D')
        {
            SI = 1;
            TTC = TTC + 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("GD: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
        else
        {
            PI = 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("GD Invalid: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
    }
    else if (IR[0] == 'P')
    {
        IC = IC + 1;
        if (IR[1] == 'D')
        {
            LLC = LLC + 1;
            TTC = TTC + 1;
            SI = 2;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("PD: TTC=%d, TI=%d, LLC=%d\n", TTC, TI, LLC);
            MOS();
        }
        else
        {
            PI = 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("PD Invalid: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
    }
    else if (IR[0] == 'L')
    {
        IC = IC + 1;
        if (IR[1] == 'R')
        {
            if (PI == 3)
            {
                InValid = 1;
                TI = 0;
                printf("LR Page Fault: TTC=%d, TI=%d\n", TTC, TI);
                MOS();
            }
            else
            {
                for (int j = 0; j < 4; j++)
                    GR[j] = M[RA][j];
                TTC++;
                if (TTC <= ttl)
                    TI = 0;
                else
                    TI = 2;
                printf("LR: TTC=%d, TI=%d\n", TTC, TI);
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("LR Invalid: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
    }
    else if (IR[0] == 'S')
    {
        IC = IC + 1;
        if (IR[1] == 'R')
        {
            for (int j = 0; j < 4; j++)
                M[RA][j] = GR[j];
            TTC = TTC + 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("SR: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
        else
        {
            PI = 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("SR Invalid: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
    }
    else if (IR[0] == 'C')
    {
        IC = IC + 1;
        if (IR[1] == 'R')
        {
            if (PI == 3)
            {
                InValid = 1;
                TI = 0;
                printf("CR Page Fault: TTC=%d, TI=%d\n", TTC, TI);
                MOS();
            }
            else
            {
                if (M[RA][0] == GR[0] && M[RA][1] == GR[1] && M[RA][2] == GR[2] && M[RA][3] == GR[3])
                    C = 1;
                else
                    C = 0;
                TTC++;
                if (TTC <= ttl)
                    TI = 0;
                else
                    TI = 2;
                printf("CR: TTC=%d, TI=%d\n", TTC, TI);
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("CR Invalid: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
    }
    else if (IR[0] == 'B')
    {
        IC = IC + 1;
        if (IR[1] == 'T')
        {
            if (PI == 3)
            {
                InValid = 1;
                TI = 0;
                printf("BT Page Fault: TTC=%d, TI=%d\n", TTC, TI);
                MOS();
            }
            else
            {
                if (C == 1)
                    IC = VA;
                TTC++;
                if (TTC <= ttl)
                    TI = 0;
                else
                    TI = 2;
                printf("BT: TTC=%d, TI=%d\n", TTC, TI);
                MOS();
            }
        }
        else
        {
            PI = 1;
            if (TTC <= ttl)
                TI = 0;
            else
                TI = 2;
            printf("BT Invalid: TTC=%d, TI=%d\n", TTC, TI);
            MOS();
        }
    }
    else if (IR[0] == 'H')
    {
        IC = IC + 1;
        TTC++;
        if (TTC <= ttl)
        {
            TI = 0;
            SI = 3;
        }
        else
        {
            TI = 2;
        }
        printf("H: TTC=%d, TI=%d\n", TTC, TI);
        MOS();
    }
    else
    {
        PI = 1;
        if (TTC <= ttl)
            TI = 0;
        else
            TI = 2;
        printf("Invalid Instruction: TTC=%d, TI=%d\n", TTC, TI);
        MOS();
    }
}

void MOS()
{
    if (PI == 1)
    {
        if (TI == 0)
        {
            EM = 4;
            printf("Opcode Error\n");
            Terminate();
        }
        else if (TI == 2)
        {
            EM = 3;
            printf("Time Limit Exceeded\n");
            EM = 4;
            printf("Opcode Error\n");
            Terminate();
        }
    }
    else if (PI == 2)
    {
        if (TI == 0)
        {
            EM = 5;
            printf("Operand Error\n");
            Terminate();
        }
        else if (TI == 2)
        {
            EM = 3;
            printf("Time Limit Exceeded\n");
            EM = 5;
            printf("Operand Error\n");
            Terminate();
        }
    }
    else if (PI == 3)
    {
        if (TI == 0)
        {
            if (InValid == 1)
            {
                EM = 6;
                printf("Invalid Page Fault\n");
                Terminate();
            }
        }
        else if (TI == 2)
        {
            EM = 3;
            printf("Time Limit Exceeded\n");
            Terminate();
        }
    }
    if (SI == 1)
    {
        if (TI == 0)
            read();
        else if (TI == 2)
        {
            EM = 3;
            printf("Time Limit Exceeded\n");
            Terminate();
        }
    }
    if (SI == 2)
    {
        if (TI == 0)
            write();
        else if (TI == 2)
        {
            write();
            EM = 3;
            printf("Time Limit Exceeded\n");
            Terminate();
        }
    }
    if (SI == 3)
    {
        EM = 0;
        printf("No Error\n");
        Terminate();
    }
}

void Terminate()
{
    printf("\n\nMEMORY (After Execution):\n");
    for (int i = 0; i < 300; i++)
    {
        if (i < 100)
            printf("M [%d] \t\t\t |", i);
        else
            printf("M [%d] \t\t |", i);
        for (int j = 0; j < 4; j++)
        {
            printf("%c|", M[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("Job ID : %c%c%c%c \tTTL =%d\t\tTLL =%d\t\tTTC = %d\tLLC =%d\n",
           pcb.job[0], pcb.job[1], pcb.job[2], pcb.job[3], ttl, tll, TTC, LLC);
    printf("PTR = %d\tIC = %d\t\tEM = %d\t\tIR = ", PTR, IC, EM);
    for (int i = 0; i < 4; i++)
        printf("%c", IR[i]);
    fprintf(fout, "Job ID : %c%c%c%c \tTTL =%d\t\tTLL =%d\t\tTTC = %d\tLLC =%d\n",
            pcb.job[0], pcb.job[1], pcb.job[2], pcb.job[3], ttl, tll, TTC, LLC);
    fprintf(fout, "PTR = %d\tIC = %d\t\tEM = %d\t\tIR = ", PTR, IC, EM);
    for (int i = 0; i < 4; i++)
        fprintf(fout, "%c", IR[i]);
    printf("\n");
    if (EM == 0)
    {
        fprintf(fout, "\nNo Errors\n");
    }
    else if (EM == 1)
    {
        fprintf(fout, "\nOut of Data\n");
    }
    else if (EM == 2)
    {
        fprintf(fout, "\nLine Limit Exceeded\n");
    }
    else if (EM == 3)
    {
        fprintf(fout, "\nTime Limit Exceeded\n");
    }
    else if (EM == 4)
    {
        fprintf(fout, "\nOpcode Error\n");
    }
    else if (EM == 5)
    {
        fprintf(fout, "\nOperand Error\n");
    }
    else if (EM == 6)
    {
        fprintf(fout, "\nInvalid Page Fault\n");
    }
    printf("\nEnding of job No.: %d\n", job);
    printf("\n\n\n");
    fprintf(fout, "-----------------------------------------------------------------------------------\n");
    fprintf(fout, "-----------------------------------------------------------------------------------\n");
    if (done)
    {
        fclose(fin);
        fclose(fout);
        exit(0); // Exit after all jobs are done
    }
}

void read()
{
    if (fgets(line, sizeof(line), fin) == NULL)
    {
        EM = 1;
        printf("Out of Data\n");
        Terminate();
        return;
    }
    int len = strlen(line);
    if (line[len - 1] == '\n')
        line[len - 1] = '\0';
    printf("%s\n", line);
    if (line[0] == '$' && line[1] == 'E' && line[2] == 'N' && line[3] == 'D')
    {
        EM = 1;
        printf("Out of Data\n");
        Terminate();
        return;
    }
    int i, j, k;
    k = 0;
    for (i = 0; k < strlen(line); i++)
    {
        for (j = 0; j < 4 && k < strlen(line); j++)
        {
            M[RA + i][j] = line[k];
            k++;
        }
    }
}

void write()
{
    char buff[40] = {0}; // Initialize buffer
    int ra = RA, i, k = 0;
    if (LLC > tll)
    {
        EM = 2;
        printf("Line Limit Exceeded\n");
        Terminate();
        return;
    }
    while (ra < 300)
    {
        for (i = 0; i < 4; i++)
        {
            if (M[ra][i] == '_' || M[ra][i] == '\0')
                break;
            buff[k++] = M[ra][i];
        }
        if (M[ra][i] == '_' || M[ra][i] == '\0')
            break;
        ra++;
    }
    buff[k] = '\0';
    fprintf(fout, "%s\n", buff);
    printf("%s\n", buff);
}

int main()
{
    srand(time(NULL));
    printf("Reading input file:\n");
    fin = fopen("input.txt", "r");
    fout = fopen("output.txt", "w");
    if (fin == NULL || fout == NULL)
    {
        printf("Error opening files!\n");
        return 1;
    }
    char ch;
    while ((ch = fgetc(fin)) != EOF)
    {
        printf("%c", ch);
    }
    printf("\n\n");
    rewind(fin);
    load();
    fclose(fin);
    fclose(fout);
    return 0;
}