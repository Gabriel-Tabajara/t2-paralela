#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include "mpi.h"

int RESULT_TAG = 0;
int REQUEST_TAG = 1;
int KILL_TAG = 2;
int WORK_TAG = 3;

int __coordinates[][2] = {
    {10, 5},
    {29, 24},
    {21, 33},
    {33, 95},
    {49, 18},
    {50, 2},
    {62, 22},
    {73, 86},
    {29, 68},
    {99, 33},
    {9,20},
    {12,98},
    {15, 68},
    {18, 45},
    {20, 10},
    {25, 30},
    {30, 50},
    {35, 70},
    {40, 90},
    {45, 15},
    {50, 25},
    {55, 35},
    {60, 45},
    {65, 55},
    {70, 65},
    {75, 75},
    {80, 85},
    {85, 95},
    {90, 5},
    {95, 15}
};

int *minPath;
int minCost = INT_MAX;
int pathCount = 0;

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

int *mergeCompleteRoute(int *route, int n, int *baseRoute)
{
    int *completeRoute = (int *)malloc((n + 2) * sizeof(int));
    completeRoute[0] = baseRoute[0];
    completeRoute[1] = baseRoute[1];
    for (int i = 2; i < n; i++)
    {
        completeRoute[i] = route[i - 2];
    }
    return completeRoute;
}

int calculateCost(int *route, int **distanceMatrix, int n, int *baseRoute)
{
    int *completeRoute = mergeCompleteRoute(route, n, baseRoute);
    int cost = 0;

    for (int i = 0; i < n - 1; i++)
    {
        assert(completeRoute[i] >= 0 && completeRoute[i] < n);
        assert(completeRoute[i + 1] >= 0 && completeRoute[i + 1] < n);
        cost += distanceMatrix[completeRoute[i]][completeRoute[i + 1]];
    }
    assert(completeRoute[n - 1] >= 0 && completeRoute[n - 1] < n);
    assert(completeRoute[0] >= 0 && completeRoute[0] < n+2);
    cost += distanceMatrix[completeRoute[n - 1]][completeRoute[0]];

    free(completeRoute);
    return cost;
}

void printPath(int *route, int n, int cost, int *baseRoute)
{
    int *completeRoute = mergeCompleteRoute(route, n, baseRoute);

    // for (int i = 0; i < n; i++)
    // {
        //printf("%d -> ", completeRoute[i]);
    // }
    //printf("%d | Cost: %d\n", completeRoute[0], cost);

    free(completeRoute);
}

void tryAllRoutes(int *route, int **distanceMatrix, int start, int n, int *baseRoute)
{
    if (start == n - 1)
    {
        pathCount++;
        int cost = calculateCost(route, distanceMatrix, n + 2, baseRoute);
        printPath(route, n + 2, cost, baseRoute);
        if (cost < minCost)
        {
            int *completeRoute = mergeCompleteRoute(route, n+2, baseRoute);

            memcpy(minPath, completeRoute, sizeof(int) * (n+2));
            minPath[n+2] = completeRoute[0];

            minCost = cost;
            free(completeRoute);
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

int main(int argc, char *argv[])
{
    int CITIES = argc > 1 ? atoi(argv[1]) : 10;

    int coordinates[CITIES][2];

    for (int i = 0; i < CITIES; i++)
    {
        coordinates[i][0] = __coordinates[i][0];
        coordinates[i][1] = __coordinates[i][1];
    }


    int n = sizeof(coordinates) / sizeof(coordinates[0]);

    int my_rank;
    int proc_n;
    int message;
    int saco[n];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    int **distanceMatrix = allocateDistanceMatrix(n);

    minPath = (int *)malloc((n + 1) * sizeof(int));

    generateDistanceMatrix(coordinates, n, distanceMatrix);

    //printf("Number of processes: %d\n", proc_n);
    if (my_rank == 0)
    {

        printf("Master has started\n");
        
        int cities[n];
        for (int i = 0; i < n; i++)
            cities[i] = i;

        for (int i = 0; i < n; i++)
        {
            saco[i] = cities[i];
        }

        int currentPos = 1;
        int qnt_restante = n - 1;

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
            MPI_Recv(0, 0, MPI_INT, MPI_ANY_SOURCE, REQUEST_TAG, MPI_COMM_WORLD, &status);  

            int sender_rank = status.MPI_SOURCE; 
            
            //printf("Master received message from %d and send: ", sender_rank);
                
            // for (int i = 0; i < n; i++)
            // {
                //printf("%d ", message[i]);
            // }
            //printf("\n");
            MPI_Send(message, n, MPI_INT, sender_rank, WORK_TAG, MPI_COMM_WORLD);   
            qnt_restante--;
            free(message);
        }

        for (int i = 1; i < proc_n; i++)
        {
            MPI_Send(0, 0, MPI_INT, i, KILL_TAG, MPI_COMM_WORLD);
        }

        for (int i = 1; i < proc_n; i++)
        {
            int *resultMessage = (int *)malloc((n + 1) * sizeof(int));

            MPI_Status status;
            MPI_Recv(resultMessage, n+1, MPI_INT, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
            //printf("Master received result from %d \n", status.MPI_SOURCE);

            int cost = resultMessage[n];
            if (cost < minCost)
            {
                minCost = cost;
                memcpy(minPath, resultMessage, sizeof(int) * n);
            }
            free(resultMessage);
        }

        printf("Master Best Route with min cost %d: ", minCost);
        for (int i = 0; i <= n; i++)
        {
            printf("%d ", minPath[i]);
            if (i < n)
                printf("-> ");
        }
        printf("\n");

        saveResultToFile("result.txt", coordinates, minPath, n);

        printf("Master finished\n");
    }
    else
    {
        printf("Process %d started\n", my_rank);
        while (true)
        {
            printf("Process %d asking for work\n", my_rank);
            MPI_Send(0, 0, MPI_INT, 0, REQUEST_TAG, MPI_COMM_WORLD);   
    
            int *message = (int *)malloc(n * sizeof(int));
            
            MPI_Status status;
            MPI_Recv(message, n, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == KILL_TAG)
            {
                int *finalMessage = (int *)malloc((n + 1)* sizeof(int));

                memcpy(finalMessage, minPath, sizeof(int) * (n + 1));
                finalMessage[n] = minCost;

                MPI_Send(finalMessage, n+1, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);

                free(message);
                free(finalMessage);
                printf("Process %d finished\n", my_rank);
                break;
            }
    
            //printf("Process %d received message from %d message: ", my_rank, status.MPI_SOURCE);
    
            // for (int i = 0; i < n; i++)
            // {
                //printf("%d ", message[i]);
            // }
    
            //printf("\n");
    
            int cities[n];
            for (int i = 2; i < n; i++)
                cities[i-2] = message[i];
    
            // for (int i = 0; i < n - 2; i++)
            // {
                //printf("%d ", cities[i]);
            // }
    
            //printf("\n");
            
            int baseRoute[2] = {message[0], message[1]};
            tryAllRoutes(cities, distanceMatrix, 0, n-2, baseRoute);
        }
        
        //printf("Process %d Best Route with min cost %d: ", my_rank, minCost);
        // for (int i = 0; i <= n; i++)
        // {
            //printf("%d ", minPath[i]);
            // if (i < n)
                //printf("-> ");
        // }
        //printf("\n");
    }

    MPI_Finalize();

    return 0;
}
