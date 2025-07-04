#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_memory_block();
void print_instruction_register();
void print_general_purpose_register();
void Load();
void INIT();
void START_EXECUTION();
void EXECUTE_USER_PROGRAM();
void MOS();
void GD();
void PD();
void H();
void BT();
void SR();
void LR();
void CR();

int flag = 0;
int currentLine;

char M[100][4];  //For Memory of OS
char IR[4];     //Instruction register to hold next instruction
char R[4];      //Register to store temporary data
char buffer[40];       //Buffer to hold data
int IC;         //Instruction Counter to track address
int C;      //Toggle to check conditions and set System Interrupt
int SI;


// print the memory block
void print_memory_block()
{
    printf("Memory block is:\n\n");
    printf("\n\n");
    int i, j;
    for (i = 0; i < 100; i++)
    {
        if (i >= 10)
            printf("%d  ", i);
        else
            printf("%d   ", i);
        for (j = 0; j < 4; j++)
        {
            printf("|%c", M[i][j]);
        }

        printf("|\n");
    }
}
// print the instruction register
void print_instruction_register()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        printf("%c", IR[i]);
    }
    printf("\n");
}

// print the general purpose register
void print_general_purpose_register()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        printf("%c", R[i]);
    }
    printf("\n");
}

void Load()
{
    int i = 0;
    FILE *fp = NULL;
    char ch;
    fp = fopen("input.txt", "r");
    char str[100];
    while (fgets(str, 41, fp) != NULL)
    {
        if (str[0] == '$' && str[1] == 'A' && str[2] == 'M' && str[3] == 'J')
        {
            printf("AMJ instruction found\n");
            getchar();
            int row = 0;
            int col = 0;

            while (1)
            {

                fgets(str, 100, fp);

                if ((str[0] == '$' && str[1] == 'D' && str[2] == 'T' && str[3] == 'A'))
                    break;

                for (int i = 0; i < strlen(str) - 1; i++)
                {
                    if (str[i] == ' ')
                        break;
                    M[row][col] = str[i];
                    col++;
                    if (col == 4)
                    {
                        row++;
                        col = 0;
                    }
                }
            }

            currentLine = ftell(fp);

            printf("loading is completed\n");
        }
        if ((str[0] == '$' && str[1] == 'D' && str[2] == 'T' && str[3] == 'A'))
        {
            START_EXECUTION();
        }
        if ((str[0] == '$' && str[1] == 'E' && str[2] == 'N' && str[3] == 'D'))
        {
            i++;
            print_memory_block();
            printf("\n\n\nEND OF JOB no. %d\n\n\n", i);
            printf("enter to load another job:\n");
            getchar();

            INIT();
        }
    }
    fclose(fp);

}

void INIT()
{
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            M[i][j] = '*';
        }
    }
    for (int i = 0; i < 4; i++)
    {
        IR[i] = '*';
        R[i] = '*';
    }
    IC = 0;
    SI = 0;
    C = 0;
    currentLine = 0;
}

void START_EXECUTION()
{
    IC = 0;
    EXECUTE_USER_PROGRAM();
}

void EXECUTE_USER_PROGRAM()
{
    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            IR[i] = M[IC][i];
        }
        IC++;
        printf("\n\nInstruction register has:\n\n");
        print_instruction_register();
        if (IR[0] == 'G' && IR[1] == 'D')
        {
            SI = 1;
            MOS();
        }
        else if (IR[0] == 'P' && IR[1] == 'D')
        {
            SI = 2;
            MOS();
        }
        else if (IR[0] == 'H' && IR[1] == '*')
        {
            SI = 3;

            MOS();
            // printf("execution is completed\n");
            break;
        }
        else if (IR[0] == 'L' && IR[1] == 'R')
        {
            LR();
        }
        else if (IR[0] == 'S' && IR[1] == 'R')
        {
            SR();
        }
        else if (IR[0] == 'B' && IR[1] == 'T')
        {
            BT();
        }
        else if (IR[0] == 'C' && IR[1] == 'R')
        {
            CR();
        }
    }
}

void MOS()
{
    if (SI == 1)
    {
        GD();
    }
    else if (SI == 2)
    {
        PD();
    }
    else if (SI == 3)
    {
        H();
    }
}

void GD()
{

    int row = (IR[2] - '0') * 10 + (IR[3] - '0');
    int col = 0;
    char str[100];
    FILE *fp = NULL;
    fp = fopen("input.txt", "r");

    fseek(fp, currentLine, SEEK_SET);
    fgets(str, 100, fp);
    for (int i = 0; i < strlen(str) - 1; i++)
    {
        M[row][col] = str[i];
        col++;
        if (col == 4)
        {
            row++;
            col = 0;
        }
    }
    currentLine = ftell(fp);
    fclose(fp);

}

void PD()
{
    FILE *fp = NULL;
    fp = fopen("output.txt", "a");
    if (flag == 0)
        flag++;
    else
        fprintf(fp, "\n");

    int row = (IR[2] - '0') * 10 + (IR[3] - '0');
    int col = 0;
    for (int i = row; i < row + 10; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (M[i][j] == '*' || M[i][j] == '\0') break;

            fputc(M[i][j], fp);
        }
    }
    fclose(fp);

}
void H()
{
    FILE *fp = NULL;
    fp = fopen("output.txt", "a");
    fputc('\n', fp);
    fputc('\n', fp);
}

void LR()
{
    int row = (IR[2] - '0') * 10 + (IR[3] - '0');
    for (int i = 0; i < 4; i++)
    {
        R[i] = M[row][i];
    }
}

void SR()
{
    int row = (IR[2] - '0') * 10 + (IR[3] - '0');
    for (int i = 0; i < 4; i++)
    {
        M[row][i] = R[i];
    }
}
void CR()
{
    int row = (IR[2] - '0') * 10 + (IR[3] - '0');

    for (int i = 0; i < 4; i++)
    {
        if (R[i] != M[row][i])
        {
            C = 0;
            break;
        }
        else
        {
            C = 1;
        }
    }
}

void BT()
{
    if (C == 1)
    {
        IC = (IR[2] - '0') * 10 + (IR[3] - '0');
        C = 0;
    }
}

// write main function
int main()
{
    // declare variables

    printf("This program will show implementation of os phase 1\n");
    printf("The code written in input.txt is\n\n");
    // code to read from a file and print the contents on the screen
    
    FILE *f = NULL;   
    char ch;
    f = fopen("input.txt", "r");
    if (f == NULL)
    {
        printf("File not found");
        exit(1); 
    }
    while ((ch = fgetc(f)) != EOF)
    {
        printf("%c", ch);
    }
    fclose(f);

    printf("\n\n\nEnter any key to continue\n\n");

    INIT();
    Load();

    return 0;
}