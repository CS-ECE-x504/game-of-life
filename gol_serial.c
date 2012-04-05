#include "gol_serial.h"

// Computes the given number of iterations for an n x m array of cells.
void gol_serial(bool* cells, int n, int m, int iterations) {
    int t, i, j;
    int neighbors;
    bool* cells_new = malloc(n * m * sizeof(bool));
    bool* temp;

    for(t = 0; t < iterations; ++t) {
        for(i = 0; i < n; ++i) {
            for(j = 0; j < m; ++j) {
                neighbors = 0;
                if(i>0 && j>0) neighbors += cells[(i-1) * m + j-1] ? 1 : 0;
                if(i>0) neighbors += cells[(i-1) * m + j] ? 1 : 0;
                if(i>0 && j<m-1) neighbors += cells[(i-1) * m + j+1] ? 1 : 0;
                if(j>0) neighbors += cells[i * m + j-1] ? 1 : 0;
                if(j<m-1) neighbors += cells[i * m + j+1] ? 1 : 0;
                if(i<n-1 && j>0) neighbors += cells[(i+1) * m + j-1] ? 1 : 0;
                if(i<n-1) neighbors += cells[(i+1) * m + j] ? 1 : 0;
                if(i<n-1 && j<m-1) neighbors += cells[(i+1) * m + j+1] ? 1 : 0;

                cells_new[i * m + j] = neighbors == 3 || (cells[i*m+j] && neighbors == 2);
            }
        }

        temp = cells;
        cells = cells_new;
        cells_new = temp;
    }

    if(iterations % 2) {
        for(i = 0; i < n; ++i) {
            for(j = 0; j < m; ++j) {
                cells_new[i * m + j] = cells[i * m + j];
            }
        }
    }

    free(iterations % 2 ? cells : cells_new);
}
