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

#define TAREFAS 7

int *minPath;
int minCost = INT_MAX;
int pathCount = 0;
int qnt_restante = TAREFAS;

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

int calculateCost(int *route, int **distanceMatrix, int n)
{
    int cost = 0;
    for (int i = 0; i < n - 1; i++)
    {
        assert(route[i] >= 0 && route[i] < n);
        assert(route[i + 1] >= 0 && route[i + 1] < n);
        cost += distanceMatrix[route[i]][route[i + 1]];
    }
    assert(route[n - 1] >= 0 && route[n - 1] < n);
    assert(route[0] >= 0 && route[0] < n);
    cost += distanceMatrix[route[n - 1]][route[0]];
    return cost;
}

void printPath(int *route, int n, int cost)
{
    for (int i = 0; i < n; i++)
    {
        printf("%d -> ", route[i]);
    }
    printf("%d | Cost: %d\n", route[0], cost);
}

void tryAllRoutes(int *route, int **distanceMatrix, int start, int n)
{
    if (start == n - 1)
    {
        pathCount++;
        int cost = calculateCost(route, distanceMatrix, n);
        printPath(route, n, cost);
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

        tryAllRoutes(route, distanceMatrix, start + 1, n);

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

int calc(int[][] coorditanes[][2], int n, int[] mipath)
{
}

// mpirun -np 2 ./tspBruteForceParallel
int main(int argc, char *argv[])
{
    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuário na linha de comando (np)
    int message;       // Buffer para as mensagens
    int saco[TAREFAS]; // saco de trabalho

    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    if (my_rank == 0)
    {
        printf("Number of processes: %d\n", proc_n);

        if (argc < 2)
        {
            printf("Usage: %s <file_name>\n", argv[0]);
            return 1;
        }

        const char *file_name = argv[1];
        FILE *file = fopen(file_name, "r");
        if (file == NULL)
        {
            printf("Error: Could not open file %s\n", file_name);
            return 1;
        }

        int n;
        fscanf(file, "%d", &n);
        int coordinates[n][2];
        for (int i = 0; i < n; i++)
        {
            fscanf(file, "%d %d", &coordinates[i][0], &coordinates[i][1]);
        }
        fclose(file);

        int **distanceMatrix = allocateDistanceMatrix(n);

        minPath = (int *)malloc((n + 1) * sizeof(int));

        generateDistanceMatrix(coordinates, n, distanceMatrix);

        int cities[n];
        for (int i = 0; i < n; i++)
            cities[i] = i;

        for (int i = 0; i < n; i++)
        {
            saco[i] = cities[i];
        }

        while (qnt_restante > 0)
        {
            int[n] message = ;
            for 

            MPI_Recv(&message, MPI_ANY_SOURCE, MPI_ANY_TAG, status);  // recebo por ordem de chegada com any_source
            // tryAllRoutes(saco, distanceMatrix, 2, n);
            qnt_restante--;
        }

        MPI_Send(&message, 1, MPI_INT, my_rank+1, 1, MPI_COMM_WORLD);   
        return 0;
    }
    else
    {

        MPI_Recv(&message, 1, MPI_INT, my_rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        tryAllRoutes(cities, distanceMatrix, 2, n);
        printf("Process %d started\n", my_rank);
        return 0;
    }


    printf("Minimum Cost: %d\n", minCost);
    printf("Best Route: ");
    for (int i = 0; i <= n; i++)
    {
        printf("%d ", minPath[i]);
        if (i < n)
            printf("-> ");
    }
    printf("\n");

    printf("Number of paths tried: %d\n", pathCount);

    saveResultToFile("result.txt", coordinates, minPath, n);

    freeDistanceMatrix(distanceMatrix, n);
    free(minPath);

    return 0;
}
