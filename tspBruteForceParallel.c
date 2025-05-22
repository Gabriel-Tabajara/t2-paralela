#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
// sudo apt-get install libopenmpi-dev
// gcc tspBruteForce.c -o tspBruteForce -lm && ./tspBruteForce coordinates8.txt
#include "mpi.h"

// #define TAREFAS 7

int RESULT_TAG = 0;
int REQUEST_TAG = 1;
int KILL_TAG = 2;
int WORK_TAG = 3;

int coordinates[][2] = {
    {10, 20},
    {12, 24},
    {21, 46},
    {33, 18},
    {49, 31},
    // {50, 95},
    // {62, 22},
    // {73, 10}
};

int *minPath;
int minCost = INT_MAX;
int pathCount = 0;
// int qnt_restante = TAREFAS;

int **allocateDistanceMatrix(int n)
{
    int **matrix = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = (int *)malloc(n * sizeof(int));
    }
    return matrix;
}

void freeDistanceMatrix(int **matrix, int n)
{
    for (int i = 0; i < n; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

int calculateCost(int *route, int **distanceMatrix, int n, int *baseRoute)
{
    int cost = 0;
    int completeRoute[n + 2];
    completeRoute[0] = baseRoute[0];
    completeRoute[1] = baseRoute[1];
    for (int i = 2; i < n; i++)
    {
        completeRoute[i] = route[i-2];
    }
    for (int i = 0; i < n - 1; i++)
    {
        assert(completeRoute[i] >= 0 && completeRoute[i] < n);
        assert(completeRoute[i + 1] >= 0 && completeRoute[i + 1] < n);
        cost += distanceMatrix[completeRoute[i]][completeRoute[i + 1]];
    }
    assert(completeRoute[n - 1] >= 0 && completeRoute[n - 1] < n);
    assert(completeRoute[0] >= 0 && completeRoute[0] < n+2);
    cost += distanceMatrix[completeRoute[n - 1]][completeRoute[0]];
    return cost;
}

void printPath(int *route, int n, int cost, int* baseRoute)
{
    int completeRoute[n + 2];
    completeRoute[0] = baseRoute[0];
    completeRoute[1] = baseRoute[1];
    for (int i = 2; i < n; i++)
    {
        completeRoute[i] = route[i-2];
    }

    for (int i = 0; i < n; i++)
    {
        printf("%d -> ", completeRoute[i]);
    }
    printf("%d | Cost: %d\n", completeRoute[0], cost);
}

void tryAllRoutes(int *route, int **distanceMatrix, int start, int n, int *baseRoute)
{
    if (start == n - 1)
    {
        pathCount++;
        int cost = calculateCost(route, distanceMatrix, n+2, baseRoute);
        printPath(route, n+2, cost, baseRoute);
        if (cost < minCost)
        {
            minCost = cost;
            memcpy(minPath, route, sizeof(int) * n);
            minPath[n] = route[0];
        }
        return;
    }

    for (int i = start; i < n; i++)
    {
        int temp = route[start];
        route[start] = route[i];
        route[i] = temp;

        tryAllRoutes(route, distanceMatrix, start + 1, n, baseRoute);

        temp = route[start];
        route[start] = route[i];
        route[i] = temp;
    }
}

double calculateDistance(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void generateDistanceMatrix(int coordinates[][2], int n, int **distanceMatrix)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                distanceMatrix[i][j] = 0;
            }
            else
            {
                distanceMatrix[i][j] = (int)calculateDistance(coordinates[i][0], coordinates[i][1], coordinates[j][0], coordinates[j][1]);
            }
        }
    }
}

void saveResultToFile(const char *fileName, int coordinates[][2], int *bestPath, int n)
{
    FILE *file = fopen(fileName, "w");
    if (file == NULL)
    {
        printf("Error: Could not create result file\n");
        return;
    }

    for (int i = 0; i <= n; i++)
    {
        int id = bestPath[i];
        fprintf(file, "%d %d\n", coordinates[id][0], coordinates[id][1]);
    }

    fclose(file);
}

// mpirun -np 2 ./tspBruteForceParallel
int main(int argc, char *argv[])
{
    int n = sizeof(coordinates) / sizeof(coordinates[0]); // Number of coordinates

    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuário na linha de comando (np)
    int message;       // Buffer para as mensagens
    int saco[n]; // saco de trabalho

    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    int **distanceMatrix = allocateDistanceMatrix(n);

    minPath = (int *)malloc((n + 1) * sizeof(int));

    generateDistanceMatrix(coordinates, n, distanceMatrix);

    if (my_rank == 0)
    {
        printf("Number of processes: %d\n", proc_n);

        // if (argc < 2)
        // {
        //     printf("Usage: %s <file_name>\n", argv[0]);
        //     return 1;
        // }

        // const char *file_name = argv[1];
        // FILE *file = fopen(file_name, "r");
        // if (file == NULL)
        // {
        //     printf("Error: Could not open file %s\n", file_name);
        //     return 1;
        // }

        // fscanf(file, "%d", &n);
        // int coordinates[n][2];
        // for (int i = 0; i < n; i++)
        // {
            // fscanf(file, "%d %d", &coordinates[i][0], &coordinates[i][1]);
        // }
        // fclose(file);
        printf("Process %d started\n", my_rank);
        
        int cities[n];
        for (int i = 0; i < n; i++)
            cities[i] = i;

        for (int i = 0; i < n; i++)
        {
            saco[i] = cities[i];
        }

        int currentPos = 1;
        int qnt_restante = n - 1; // quantidade de tarefas restantes

        while (qnt_restante > 0)
        {
            int *message = (int *)malloc(n * sizeof(int));
            message[0] = 0;
            message[1] = currentPos;
            for (int i = 2, j = 1; i < n; i++,j++)
            {
                if (j == currentPos) j++;
                message[i] = j;
            }
            currentPos++;

            MPI_Status status;
            MPI_Recv(0, 0, MPI_INT, MPI_ANY_SOURCE, REQUEST_TAG, MPI_COMM_WORLD, &status);  // recebo por ordem de chegada com any_source

            int sender_rank = status.MPI_SOURCE; // pego o rank do processo que enviou a mensagem

            printf("Master received message from %d and send: ", sender_rank);
                
            for (int i = 0; i < n; i++)
            {
                printf("%d ", message[i]);
            }
            printf("\n");
            MPI_Send(message, n, MPI_INT, sender_rank, WORK_TAG, MPI_COMM_WORLD);   
            qnt_restante--;
            free(message);
        }

        for (int i = 1; i < proc_n; i++)
        {
            MPI_Send(0, 0, MPI_INT, i, KILL_TAG, MPI_COMM_WORLD);
        }
    }
    else
    {
        printf("Process %d started\n", my_rank);
        while (true)
        {
            MPI_Send(0, 0, MPI_INT, 0, REQUEST_TAG, MPI_COMM_WORLD);   
    
            int *message = (int *)malloc(n * sizeof(int));
            
            MPI_Status status;
            MPI_Recv(message, n, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == KILL_TAG)
            {
                free(message);
                printf("Process %d finished\n", my_rank);
                break;
            }
    
            printf("Process %d received message from %d message: ", my_rank, status.MPI_SOURCE);
    
            for (int i = 0; i < n; i++)
            {
                printf("%d ", message[i]);
            }
    
            printf("\n");
    
            int cities[n];
            for (int i = 2; i < n; i++)
                cities[i-2] = message[i];
    
            for (int i = 0; i < n - 2; i++)
            {
                printf("%d ", cities[i]);
            }
    
            printf("\n");
            
            int baseRoute[2] = {message[0], message[1]};
            tryAllRoutes(cities, distanceMatrix, 0, n-2, baseRoute);
        }
        
        // printf("Minimum Cost: %d\n", minCost);
        // printf("Best Route: ");
        // for (int i = 0; i <= n; i++)
        // {
        //     printf("%d ", minPath[i]);
        //     if (i < n)
        //         printf("-> ");
        // }
        // printf("\n");
    
        // printf("Number of paths tried: %d\n", pathCount);
    
        // saveResultToFile("result.txt", coordinates, minPath, n);
    
        // freeDistanceMatrix(distanceMatrix, n);
        // free(minPath);
    }



    MPI_Finalize();

    return 0;
}
