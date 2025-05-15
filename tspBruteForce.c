#include <stdio.h>
#include <limits.h>
#include <math.h> // For sqrt and pow
#include <stdlib.h> // For dynamic memory allocation
#include <string.h> // For file operations
#include <stdbool.h> // For boolean type
#include <assert.h> // For debugging with assertions
#include <time.h> // For measuring time

int* minPath; // Dynamically allocate based on the number of cities
int minCost = INT_MAX;
int pathCount = 0; // Global variable to count the number of paths tried

// Dynamically allocate distance matrix
int** allocateDistanceMatrix(int n) {
    int** matrix = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        matrix[i] = (int*)malloc(n * sizeof(int));
    }
    return matrix;
}

// Free dynamically allocated distance matrix
void freeDistanceMatrix(int** matrix, int n) {
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Calculate cost of the full route (includes return to start)
int calculateCost(int* route, int** distanceMatrix, int n) {
    int cost = 0;
    for (int i = 0; i < n - 1; i++) {
        assert(route[i] >= 0 && route[i] < n); // Ensure valid indices
        assert(route[i + 1] >= 0 && route[i + 1] < n);
        cost += distanceMatrix[route[i]][route[i + 1]];
    }
    assert(route[n - 1] >= 0 && route[n - 1] < n);
    assert(route[0] >= 0 && route[0] < n);
    cost += distanceMatrix[route[n - 1]][route[0]]; // return to starting city
    return cost;
}

// Print the current path
void printPath(int* route, int n, int cost) {
    for (int i = 0; i < n; i++) {
        printf("%d -> ", route[i]);
    }
    printf("%d | Cost: %d\n", route[0], cost); // Close the loop
}

// Tries all permutations starting from index `start`
void tryAllRoutes(int* route, int** distanceMatrix, int start, int n) {
    if (start == n - 1) {
        pathCount++; // Increment path count
        int cost = calculateCost(route, distanceMatrix, n);
        printPath(route, n, cost); // Print the current path
        if (cost < minCost) {
            minCost = cost;
            memcpy(minPath, route, sizeof(int) * n);
            minPath[n] = route[0]; // close the loop
        }
        return;
    }

    for (int i = start; i < n; i++) {
        // Swap
        int temp = route[start];
        route[start] = route[i];
        route[i] = temp;

        tryAllRoutes(route, distanceMatrix, start + 1, n);

        // Backtrack
        temp = route[start];
        route[start] = route[i];
        route[i] = temp;
    }
}


// Function to calculate Euclidean distance between two cities
double calculateDistance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Dynamically calculate the distance matrix based on coordinates
void generateDistanceMatrix(int coordinates[][2], int n, int** distanceMatrix) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                distanceMatrix[i][j] = 0;
            } else {
                distanceMatrix[i][j] = (int)calculateDistance(coordinates[i][0], coordinates[i][1], coordinates[j][0], coordinates[j][1]);
            }
        }
    }
}

// Save the result to a file
void saveResultToFile(const char* fileName, int coordinates[][2], int* bestPath, int n) {
    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Error: Could not create result file\n");
        return;
    }

    for (int i = 0; i <= n; i++) {
        int id = bestPath[i];
        fprintf(file, "%d %d\n", coordinates[id][0], coordinates[id][1]);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", file_name);
        return 1;
    }

    int n;
    fscanf(file, "%d", &n);
    int coordinates[n][2];
    for (int i = 0; i < n; i++) {
        fscanf(file, "%d %d", &coordinates[i][0], &coordinates[i][1]);
    }
    fclose(file);

    // Dynamically allocate the distance matrix
    int** distanceMatrix = allocateDistanceMatrix(n);

    // Dynamically allocate the minPath array
    minPath = (int*)malloc((n + 1) * sizeof(int));

    // Generate the distance matrix
    generateDistanceMatrix(coordinates, n, distanceMatrix);

    int cities[n];
    for (int i = 0; i < n; i++) cities[i] = i;

    tryAllRoutes(cities, distanceMatrix, 0, n);
    
    printf("Minimum Cost: %d\n", minCost);
    printf("Best Route: ");
    for (int i = 0; i <= n; i++) {
        printf("%d ", minPath[i]);
        if (i < n) printf("-> ");
    }
    printf("\n");

    // Print the number of paths tried
    printf("Number of paths tried: %d\n", pathCount);

    // Save the result to a file
    saveResultToFile("result.txt", coordinates, minPath, n);

    // Free the dynamically allocated distance matrix and minPath array
    freeDistanceMatrix(distanceMatrix, n);
    free(minPath);

    return 0;
}
