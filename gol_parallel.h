#include "gameoflife.h"
#include <pthread.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
void gol_parallel(bool*, int, int, int, int);
void gol_cuda(char* cells, int n, int m, int iterations, int xBlockDim, int yBlockDim, int xThreadDim, int yThreadDim);

#ifdef __cplusplus
}
#endif
