#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>       // for clock_t, clock(), CLOCKS_PER_SEC
#include <unistd.h>     // for sleep()
#include <pthread.h>

long double PI_threads[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

// data struct input parallel
typedef struct  {
    long double start;
    long double end;
    int it;
    int pi_p;
}INTERVAL;

// time struct
struct timeval current_time = {0,0};

// Output file
FILE *fp;

float calculate_pi(int slaps[2])
{
    /*
    Calculate pi in sequential form.
    Params: int array[2] -> [0]: steps, [1]: end
    Return: float PI
    */
    int i;
    long double PI = 0;
    long double x=slaps[0], y, step, area;
    step = (2.0/(slaps[1]));
    
    while(x<1)
    { 
        y = sqrt(1-pow(x,2));
        area = y*step;
        PI = PI + area;
        x = x+step;
        i = i+1;
    }
    PI = PI*4; //mirror
    return PI;
}

void *calculate_pi_threads(void *input)
{
    /*
    Calculate pi in parallel form.
    Params: void pointer struct
    */
    int i, pi_p, laps;
    uint begin, finish; // pi_p = position Pi thread
    long double x, end, y, step, area;
    
    gettimeofday(&current_time, NULL);
    begin = current_time.tv_sec*1000000 + current_time.tv_usec;
    // initialize variable with data input
    end = (((INTERVAL*)input)->end);
    pi_p = (((INTERVAL*)input)->pi_p);
    laps = (((INTERVAL*)input)->it);
    if (pi_p == 0)
    {
        x = (((INTERVAL*)input)->start);
    }
    else
    {
        x = (((INTERVAL*)input)->start);
        x = x/(long double)laps;
    }
    step = (2.0/(long double)laps);
    if (pi_p == 0)
        end = (long double)(pi_p+1.0)*end/laps;
    else
        end = (long double)end/laps;
    gettimeofday(&current_time, NULL);
    begin = current_time.tv_sec*1000000 + current_time.tv_usec;
    while(x<end)
    {
        y = sqrt(1-pow(x,2));
        area = y*step;
        PI_threads[pi_p] = PI_threads[pi_p] + area;
        x = x+step;
    }
    PI_threads[pi_p] = PI_threads[pi_p]*4; //mirror to both axes
    gettimeofday(&current_time, NULL);
    finish = current_time.tv_sec*1000000 + current_time.tv_usec;
    printf("Elapsed thread %d time: %f seconds\n", pi_p,(float)(finish - begin) / 1000000);
    fprintf(fp, "%f; ", (float)(finish - begin) / 1000000);
}

void create_threads(int N, int laps)
{
    pthread_t thread[N];
    int i, it;
    int64_t begin, finish;
    long double start[N], end[N];

    gettimeofday(&current_time, NULL);
    begin = current_time.tv_sec*1000000 + current_time.tv_usec;
    //struct interval *interv = (struct interval *)malloc(sizeof(struct interval));
    INTERVAL* interv = malloc(N * sizeof *interv);

    it = laps/N;
    for(i=0; i<N; i++)
    {
        if(i == 0)
            start[i] = (i*it);
        else
            start[i] = (i*it) + 0.00001;
        end[i] = (i+1)*it;
        // compose array to call function calculate
        interv[i].start = start[i];
        interv[i].end = end[i];
        interv[i].it = laps;
        interv[i].pi_p = i;
    };
    //clock_t tic_p = clock();
    for(i=0; i<N; i++)
        pthread_create(&thread[i], NULL, calculate_pi_threads, (void *)&interv[i]);
    for(i=0; i<N; i++)
        pthread_join(thread[i], NULL);
    
    //clock_t toc_p = clock();
    gettimeofday(&current_time, NULL);
    finish = current_time.tv_sec*1000000 + current_time.tv_usec;
    printf("\nElapsed real time: %.10f seconds\n", (float)(finish - begin)/1000000);
}

int main()
{
    long double PI_calculated, PI_parallel=0;
    int i, j, k, vector[2];
    uint begin, finish;
    // Initialize output file
    fp = fopen("/l/disk0/mvmelo/Documentos/arq_avancada/log_times4.txt", "w"); 
    // Serial approach to evaluate execution time
    printf("Serial Pi\n");
    fprintf(fp, "\nSerial TImes\nIterations; Time; Value\n");
    for(j=100; j<=1000000000; j=j*10)
    {
        vector[0] = 0;
        vector[1] = j;
        // start time
        clock_t tic_p = clock();
        // invoke calculate function
        PI_calculated = calculate_pi(vector);
        // finish time
        clock_t toc_p = clock();

        printf("Iterations: %d\n", j);
        fprintf(fp, "\n%d; ", j);
        printf("Elapsed: %f seconds\n", (double)(toc_p - tic_p) / CLOCKS_PER_SEC);
        fprintf(fp, "%f;", (double)(toc_p - tic_p) / CLOCKS_PER_SEC);
        printf("Pi calculated value: %.10Lf\n", PI_calculated);
        fprintf(fp, "%Lf", PI_calculated);
    }
    sleep(5);
    // Prallel approach
    
    printf("\nParallel Pi\n");
    fprintf(fp, "\nParallel Times\nTime; Threads; Iterations; Value\n");
    for(i=2; i<=8; i++)
    {
        for(k=100; k<=1000000000; k=k*10)
        {
            create_threads(i, k);
            for(j=0; j<i; j++)
            {
                PI_parallel = PI_parallel + PI_threads[j];
            }    
            //sleep(1);
            printf("Thread's number: %d\n", i);
            fprintf(fp, "%d; ", i);
            printf("Iterations: %d\n", k);
            fprintf(fp, "%d; ", k);
            printf("PI parallel calculated value: %.10Lf\n\n", PI_parallel);
            fprintf(fp, "%Lf\n ", PI_parallel);
            // clean variables to next iteration
            for(j=0; j<8; j++)
                PI_threads[j] = 0.0;
            PI_parallel = 0;
        }
    }
    fclose(fp);
}