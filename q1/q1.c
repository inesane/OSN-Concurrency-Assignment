#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

#define YELLOW "\033[01;33m"
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m" 

int shm_id;

void swap(int *a, int *b)
{
     int t = *a;
     *a = *b;
     *b = t;
}

int *shareMem(size_t size)
{
     key_t mem_key = IPC_PRIVATE;
     shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
     return (int *)shmat(shm_id, NULL, 0);
}

void selectionSort(int arr[], int l, int r)
{
     int i, j, min_idx;
     for (i = l; i < r; i++)
     {
          min_idx = i;
          for (j = i + 1; j < r + 1; j++)
               if (arr[j] < arr[min_idx])
                    min_idx = j;

          swap(&arr[min_idx], &arr[i]);
     }
}

void merge(int arr[], int l, int m, int r)
{
     int i, j, k;
     int n1 = m - l + 1;
     int n2 = r - m;
     int L[n1], R[n2];
     for (i = 0; i < n1; i++)
          L[i] = arr[l + i];
     for (j = 0; j < n2; j++)
          R[j] = arr[m + 1 + j];
     i = 0;
     j = 0;
     k = l;
     while (i < n1 && j < n2)
     {
          if (L[i] <= R[j])
          {
               arr[k] = L[i];
               i++;
          }
          else
          {
               arr[k] = R[j];
               j++;
          }
          k++;
     }
     while (i < n1)
     {
          arr[k] = L[i];
          i++;
          k++;
     }
     while (j < n2)
     {
          arr[k] = R[j];
          j++;
          k++;
     }
}

void normal_mergeSort(int *arr, int low, int high)
{
     if (high - low + 1 < 5)
     {
          selectionSort(arr, low, high);
     }
     else if (low < high)
     {
          int pi = (low + high) / 2;
          normal_mergeSort(arr, low, pi);
          normal_mergeSort(arr, pi + 1, high);
          merge(arr, low, pi, high);
     }
}

void mergeSort(int *arr, int low, int high)
{
     if (high - low + 1 < 5)
     {
          selectionSort(arr, low, high);
     }
     else if (low < high)
     {
          int pi = (low + high) / 2;
          int pid1 = fork();
          int pid2;
          if (pid1 == 0)
          {
               mergeSort(arr, low, pi);
               _exit(1);
          }
          else
          {
               pid2 = fork();
               if (pid2 == 0)
               {
                    mergeSort(arr, pi + 1, high);
                    _exit(1);
               }
               else
               {
                    int status;
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
               }
          }
          merge(arr, low, pi, high);
          return;
     }
}

struct arg
{
     int l;
     int r;
     int *arr;
};

void *threaded_mergeSort(void *a)
{
     struct arg *args = (struct arg *)a;

     int l = args->l;
     int r = args->r;
     int *arr = args->arr;
     if (l > r)
          return NULL;

     else if (r - l + 1 < 5)
     {
          selectionSort(arr, l, r);
     }
     else if (l < r)
     {
          int ind = (l + r) / 2;
          struct arg a1;
          a1.l = l;
          a1.r = ind;
          a1.arr = arr;
          pthread_t tid1;
          pthread_create(&tid1, NULL, threaded_mergeSort, &a1);

          struct arg a2;
          a2.l = ind + 1;
          a2.r = r;
          a2.arr = arr;
          pthread_t tid2;
          pthread_create(&tid2, NULL, threaded_mergeSort, &a2);

          pthread_join(tid1, NULL);
          pthread_join(tid2, NULL);

          merge(arr, l, ind, r);
     }
}

void runSorts(long long int n)
{

     struct timespec ts;

     //getting shared memory
     int *arr = shareMem(sizeof(int) * (n + 1));
     for (int i = 0; i < n; i++)
          scanf("%d", arr + i);

     int brr[n + 1], crr[n + 1];
     for (int i = 0; i < n; i++)
     {
          brr[i] = arr[i];
          crr[i] = arr[i];
     }
     printf(GREEN);
     printf("Running concurrent_mergesort for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     long double st = ts.tv_nsec / (1e9) + ts.tv_sec;

     //multiprocess mergesort
     printf(YELLOW);
     mergeSort(arr, 0, n - 1);
     for (int i = 0; i < n; i++)
     {
          printf("%d ", arr[i]);
     }
     printf("\n");
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
     printf(CYAN);
     printf("time = %Lf\n", en - st);
     long double t1 = en - st;

     pthread_t tid;
     struct arg a;
     a.l = 0;
     a.r = n - 1;
     a.arr = brr;
     printf(GREEN);
     printf("Running threaded_concurrent_mergesort for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     st = ts.tv_nsec / (1e9) + ts.tv_sec;

     //multithreaded mergesort
     pthread_create(&tid, NULL, threaded_mergeSort, &a);
     pthread_join(tid, NULL);
     printf(YELLOW);
     for (int i = 0; i < n; i++)
     {
          printf("%d ", a.arr[i]);
     }
     printf("\n");
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     en = ts.tv_nsec / (1e9) + ts.tv_sec;
     printf(CYAN);
     printf("time = %Lf\n", en - st);
     long double t2 = en - st;

     printf(GREEN);
     printf("Running normal_mergesort for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     st = ts.tv_nsec / (1e9) + ts.tv_sec;

     // normal mergesort
     normal_mergeSort(crr, 0, n - 1);
     printf(YELLOW);
     for (int i = 0; i < n; i++)
     {
          printf("%d ", crr[i]);
     }
     printf("\n");
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
     en = ts.tv_nsec / (1e9) + ts.tv_sec;
     printf(CYAN);
     printf("time = %Lf\n", en - st);
     long double t3 = en - st;

     printf(RED);
     printf("normal_mergesort ran:\n\t[ %Lf ] times faster than concurrent_mergesort\n\t[ %Lf ] times faster than threaded_concurrent_mergesort\n\n\n", t1 / t3, t2 / t3);
     shmdt(arr);
     shmctl(shm_id, IPC_RMID, NULL);
     return;
}

int main()
{

     long long int n;
     scanf("%lld", &n);
     runSorts(n);
     return 0;
}