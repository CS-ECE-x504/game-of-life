#include "gameoflife.h"
#include "gol_serial.h"
#include "gol_parallel.h"

bool run_cuda=0;

int main(int argc, char** argv) {
    bool cflag = false, pflag = false, vflag = false;
    char* filename = 0;
    int c;

    while((c = getopt(argc, argv, "gcpv:")) != -1) {
        switch(c) {
            case 'c':
                cflag = true;
                break;
            case 'p':
                pflag = true;
                break;
            case 'v':
                vflag = true;
                filename = optarg;
                break;
            case 'g':
                run_cuda=1;
                break;
            case '?':
                print_usage();
                exit(0);
                break;
        }
    }

    if(!vflag && !cflag && !pflag) {
        print_usage();
    }

    if(vflag && (cflag || pflag)) {
        fprintf(stderr, "Error: Cannot run visual mode with correctness or performance tests.\n");
        abort();
    }

    if(cflag) {
        correctness_tests();
    }

    if(pflag) {
        if(cflag) printf("\n");
        performance_tests();
    }

    if(vflag) {
        visual_mode(filename);
    }

    return 0;
}

void print_usage() {
printf("usage: ./gameoflife [-c] [-p] [-v filename] [-g]\n"
       "       -c Runs a suite of correctness tests\n"
       "       -p Runs performance tests\n"
       "       -v Runs a configuration file in visual mode\n"
       "       -g Runs the CUDA version instead of the pthreads version\n");
}

void run_gol_cuda(bool* cells, int n, int m, int iterations, int threads) {
    if(threads == 0){
        int gx = (n / 16);
        int gy = (m / 16);
        if(n%16)
            gx++;
        if(m%16)
            gy++;
        gol_cuda(cells, n, m, iterations, gx, gy, 16, 16);
    }else{
        gol_cuda(cells, n, m, iterations, threads, 1, 16, 16);
    }
}

void visual_mode(const char* filename) {
    bool* cells;
    int n, m;
    input_game(&cells, &n, &m, filename);
    while(1) {
        system("clear");
        print_game(cells, n, m);
        if(run_cuda)
            run_gol_cuda(cells, n, m, 1, 4);
        else
            gol_parallel(cells, n, m, 1, 4);
        //gol_serial(cells, n, m, 1);
        usleep(5000000);
    }
}

void correctness_tests() {
    printf("CORRECTNESS TESTS:\n");
    int steps[] = {1, 2, 5, 10};
    int i, passed = 0;
    for(i = 0; i < 4; ++i) {
        passed += correctness_test("config/glider", 50, 1, steps[i]) ? 1 : 0;
        passed += correctness_test("config/glider", 50, 2, steps[i]) ? 1 : 0;
        passed += correctness_test("config/glider", 50, 3, steps[i]) ? 1 : 0;
        passed += correctness_test("config/pulsar", 50, 2, steps[i]) ? 1 : 0;
        passed += correctness_test("config/pulsar", 50, 4, steps[i]) ? 1 : 0;
        passed += correctness_test("config/pulsar", 50, 5, steps[i]) ? 1 : 0;
        passed += correctness_test("config/tumbler", 50, 3, steps[i]) ? 1 : 0;
        passed += correctness_test("config/tumbler", 50, 5, steps[i]) ? 1 : 0;
        passed += correctness_test("config/tumbler", 50, 7, steps[i]) ? 1 : 0;
        passed += correctness_test("config/large_glider", 50, 4, steps[i]) ? 1 : 0;
        passed += correctness_test("config/large_glider", 50, 8, steps[i]) ? 1 : 0;
        passed += correctness_test("config/large_glider", 50, 9, steps[i]) ? 1 : 0;
        passed += correctness_test("config/xl_glider", 50, 4, steps[i]) ? 1 : 0;
        passed += correctness_test("config/xl_glider", 50, 8, steps[i]) ? 1 : 0;
        if(run_cuda){
            passed += correctness_test("config/xxl_random", 50, 0, steps[i]) ? 1 : 0;
        }
    }
    printf("%d/%d tests passed\n", passed, run_cuda ? 60 : 56);
}

bool correctness_test(const char* filename, int iterations, int threads, int step) {
    int i, n, m;
    bool* board1;
    bool* board2;
    input_game(&board1, &n, &m, filename);
    input_game(&board2, &n, &m, filename);
    for(i = 0; i < iterations; i += step) {
        printf("\rTesting %-25s (%d threads, %d step(s)) %d/%d iterations complete", filename, threads, step, i, iterations);
        fflush(stdout);
        gol_serial(board1, n, m, step);
        if(run_cuda){
            run_gol_cuda(board2, n, m, step, threads);
        }else{
            gol_parallel(board2, n, m, step, threads);
        }
        if(!identical(board1, board2, n, m)) {
            printf("\nError at %s on iteration %d\n", filename, i+1);
            return false;
        }
    }
    printf("\rTesting %-25s (%d threads, %d step(s)) %d/%d iterations complete\n", filename, threads, step, iterations, iterations);
    free(board1);
    free(board2);
    return true;
}

void performance_tests() {
    printf("PERFORMANCE TESTS:\n");
    performance_test("config/xl_glider", 1000, 2);
    performance_test("config/xl_glider", 1000, 4);
    if(run_cuda){
        performance_test("config/xl_glider", 1000, 0);
        performance_test("config/xxl_random", 1000, 0);
    }else{
        performance_test("config/xl_glider", 1000, 8);
        performance_test("config/xxl_random", 1000, 8);
    }
}

void performance_test(const char* filename, int iterations, int threads) {
    struct timeval start, end;
    int n, m, serial_time, parallel_time;
    bool* board1;
    bool* board2;
    input_game(&board1, &n, &m, filename);
    input_game(&board2, &n, &m, filename);
    printf("Running %s (%d iterations, %d threads)\n", filename, iterations, threads);

    printf("Serial time: ");
    fflush(stdout);
    gettimeofday(&start, NULL);
    gol_serial(board1, n, m, iterations);
    gettimeofday(&end, NULL);
    serial_time = ms_difference(start, end);
    printf("%d ms\n", serial_time);

    printf("Parallel time: ");
    fflush(stdout);
    gettimeofday(&start, NULL);

    if(run_cuda){
        run_gol_cuda(board1, n, m, iterations, threads);
    }else{
        gol_parallel(board1, n, m, iterations, threads);
    }

    gettimeofday(&end, NULL);
    parallel_time = ms_difference(start, end);
    printf("%d ms\n", parallel_time);
    
    printf("Speedup: %.4f\n", (float)serial_time/parallel_time);
    free(board1);
    free(board2);
}

int ms_difference(struct timeval start, struct timeval end) {
    int ms = (end.tv_sec - start.tv_sec) * 1000;
    ms += (end.tv_usec - start.tv_usec) / 1000;
    return ms;
}

bool identical(bool* game1, bool* game2, int n, int m) {
    int i;
    for(i = 0; i < n * m; ++i) {
        if(game1[i] != game2[i]) {
            return false;
        }
    }
    return true;
}

void input_game(bool** cells, int* n, int* m, const char* filename) {
    int cell, i, j;
    FILE* in;
    if((in = fopen(filename, "r")) == 0) {
        fprintf(stderr, "Error, could not open file '%s'.\n", filename);
        abort();
    }

    fscanf(in, "%d", n);
    fscanf(in, "%d", m);
    *cells = malloc(*n * *m * sizeof(bool));
    while((cell = fgetc(in)) == '\n');
    for(i = 0; i < *n; ++i) {
        for(j = 0; j < *m; ++j) {
            (*cells)[i * *m + j] = cell != ' ';
            cell = fgetc(in);
        }
        cell = fgetc(in); // eat the new line
    }
    fclose(in);
}

void print_game(bool* cells, int n, int m) {
    static int iteration = 1;
    int i, j;

    // print top border
    printf("+");
    for(i = 0; i < m; ++i) printf("-");
    printf("+\n");

    for(i = 0; i < n; ++i) {
        printf("|");
        for(j = 0; j < m; ++j) {
            printf("%c", cells[i * m + j] ? '#' : ' ');
        }
        printf("|");
        printf("\n");
    }

    // print bottom border
    printf("+");
    for(i = 0; i < m; ++i) printf("-");
    printf("+\n");
    printf("Iteration: %d\n", iteration++);
}
