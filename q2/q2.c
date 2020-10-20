II#include <sys/types.h>
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

int students_left,n, m, o, students_gate;

pthread_mutex_t gate;

struct company
{
    int company_id;
    float probability;
    int vaccines_per_batch[5];
    int total_vaccines_made;
    pthread_mutex_t company_mutex;
};

struct zone
{
    int zone_id;
    int slots_remaining;
    int vaccines_remaining;
    int company_id;
    pthread_mutex_t zone_mutex;
    int students_done;
};

struct student
{
    int student_id;
    int vaccines_given;
};

struct company ** company_arr;
struct zone ** zone_arr;
struct student ** student_arr;

void * company_function(void *company_arr)
{
    struct company * comp = (struct company *)company_arr;
    while(students_left>0)
    {
        if(comp->total_vaccines_made==0)
        {
            printf("All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n", comp->company_id);
            int batches = (rand() % (5 - 1 + 1)) + 1;
            printf("Pharmaceutical Company %d is preparing %d batches of vaccines which have success probability %f\n", comp->company_id, batches, comp->probability);
            sleep((rand() % (5 - 2 + 1)) + 2);
            printf("Pharmaceutical Company %d has prepared %d batches of vaccines which have success probability %f\n", comp->company_id, batches, comp->probability);
            pthread_mutex_lock(&comp->company_mutex);
            for(int i=0;i<batches;i++)
            {
                comp->vaccines_per_batch[i] = (rand() % (20 - 10 + 1)) + 10;
                comp->total_vaccines_made += comp->vaccines_per_batch[i];
            }
            pthread_mutex_unlock(&comp->company_mutex);
            // for(int i=0;i<5;i++)
            // {
            //     printf("%d\n", comp->vaccines_per_batch[i]);
            // }
            while(comp->total_vaccines_made!=0 && students_left!=0);
        }
    }
}

void * zone_function(void *zone_arr)
{
    struct zone * zon = (struct zone *)zone_arr;
    int slots_used=0;
    // for(int i=0;i<5;i++)
    // {
    //     printf("%d\n", company_arr[0]->vaccines_per_batch[i]);
    // }
    while(students_left>0)
    {
        if(zon->vaccines_remaining==0)
        {
            printf("Vaccination Zone %d has run out of vaccines\n", zon->zone_id);
            int flag=0;
            while(zon->vaccines_remaining==0 && students_left!=0)
            {
                for(int i=0;i<n;i++)
                {
                    pthread_mutex_lock(&company_arr[i]->company_mutex);
                    for(int j=0;j<5;j++)
                    {
                        // printf("%d\n", company_arr[i]->vaccines_per_batch[j]);
                        if(company_arr[i]->vaccines_per_batch[j]>0)
                        {
                            printf("Pharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has success probability %f\n", i, zon->zone_id, company_arr[i]->probability);
                            zon->vaccines_remaining+=company_arr[i]->vaccines_per_batch[j];
                            zon->company_id=i;
                            company_arr[i]->vaccines_per_batch[j]=0;
                            printf("Pharmaceutical Company %d has delivered vaccines to Vaccination Zone %d, resuming vaccinations now\n", i, zon->zone_id);
                            flag=1;
                            break;
                        }
                    }
                    pthread_mutex_unlock(&company_arr[i]->company_mutex);
                    if(flag==1)
                    {
                        break;
                    }
                }
            }
            if(students_left==0)
            {
                return NULL;
            }
        }
        if(zon->slots_remaining==0)
        {
            slots_used=0;
            zon->students_done=0;
            int max_possible_slots=8;
            if(max_possible_slots>zon->vaccines_remaining)
            {
                max_possible_slots=zon->vaccines_remaining;
            }
            else if(max_possible_slots>students_left)
            {
                max_possible_slots=students_left;
            }
            if(max_possible_slots==0)
            {
                zon->slots_remaining=1;
            }
            else
            {
                zon->slots_remaining=(rand() % (max_possible_slots - 1 + 1)) + 1;
            }
            slots_used=zon->slots_remaining;
            printf("Vaccination Zone %d is ready to vaccinate with %d slots\n", zon->zone_id, zon->slots_remaining);
            // printf("%d %d %d\n", zon->slots_remaining, zon->vaccines_remaining, students_left);
             while(zon->slots_remaining!=0 && students_left!=0 && !(slots_used>zon->slots_remaining && students_gate==0))
            {
                // printf("hi\n");
                // sleep(1);
            }
            if(students_left==0)
            {
                return NULL;
            }
            printf("Vaccination Zone %d entering Vaccination Phase\n", zon->zone_id);
            slots_used-=zon->slots_remaining;
            zon->slots_remaining=-1;
            while (zon->students_done <slots_used)
            {
                /* code */
            }
            pthread_mutex_lock(&company_arr[zon->company_id]->company_mutex);
            company_arr[zon->company_id]->total_vaccines_made-=slots_used;
            pthread_mutex_unlock(&company_arr[zon->company_id]->company_mutex);
            zon->slots_remaining=0;
        }
    }
}

void * student_function(void *student_arr)
{
    struct student * stud = (struct student *)student_arr;
    sleep(rand () % (5 - 0 + 1) + 0);
    pthread_mutex_lock(&gate);
    students_gate++;
    pthread_mutex_unlock(&gate);
    while (1)
    {
        if(stud->vaccines_given<3)
        {
            stud->vaccines_given++;
            printf("Student %d has arrived for his %d round of Vaccination\n", stud->student_id, stud->vaccines_given);
            printf("Student %d is waiting to be allocated a slot on a Vaccination Zone\n", stud->student_id);
            int check=0;
            while(1)
            {
                for(int i=0;i<m;i++)
                {
                    pthread_mutex_lock(&zone_arr[i]->zone_mutex);
                    if(zone_arr[i]->slots_remaining>0)
                    {
                        printf("Student %d assigned a slot on the Vaccination Zone %d and waiting to be vaccinated\n", stud->student_id, i);
                        zone_arr[i]->slots_remaining--;
                        zone_arr[i]->vaccines_remaining--;
                        pthread_mutex_unlock(&zone_arr[i]->zone_mutex);
                        // pthread_mutex_lock(&company_arr[i]->company_mutex);
                        // company_arr[i]->total_vaccines_made--;
                        // pthread_mutex_unlock(&company_arr[i]->company_mutex);
                        pthread_mutex_lock(&gate);
                        students_gate--;
                        // printf("someh\n");
                        pthread_mutex_unlock(&gate);
                        check=1;
                        // while(students_left!=0 && zone_arr[i]->slots_remaining!=0);
                        while (zone_arr[i]->slots_remaining!=-1)
                        {
                            // printf("hi\n");
                        }
                        pthread_mutex_lock(&zone_arr[i]->zone_mutex);
                        zone_arr[i]->students_done++;
                        pthread_mutex_unlock(&zone_arr[i]->zone_mutex);
                        printf("Student %d on Vaccination Zone %d has been vaccinated which has success probability %f\n", stud->student_id, i, company_arr[i]->probability);
                        int antibody = 100*(company_arr[i]->probability);
                        if((rand() % (100 - 1 + 1) + 1) < antibody)
                        {
                            pthread_mutex_lock(&gate);
                            students_left--;
                            // printf("%d\n", students_left);
                            //sleep(4);
                            pthread_mutex_unlock(&gate);
                            printf("Student %d has tested positive for antibodies\n", stud->student_id);
                            return NULL;
                        }
                        else
                        {
                            printf("Student %d has tested negative for antibodies\n", stud->student_id);
                            pthread_mutex_lock(&gate);
                            students_gate++;
                            pthread_mutex_unlock(&gate);
                        }
                        break;
                    }
                    pthread_mutex_unlock(&zone_arr[i]->zone_mutex);
                }
                if(check==1)
                {
                    break;
                }
            }
        }
        else
        {
            pthread_mutex_lock(&gate);
            // printf("%d\n", students_left);
            students_left--;
            students_gate--;
            pthread_mutex_unlock(&gate);
            break;
        }
    }
}

int main()
{
    int i, j;
    scanf("%d %d %d", &n, &m, &o);
    if(n<=0 || m<=0)
    {
        printf("Simulation Over.\n");
        return 0;
    }
    students_left=o;
    students_gate=0;
    float probabilities[n];
    for (i = 0; i < n; i++)
    {
        scanf("%f", &probabilities[i]);
    }

    pthread_t * company_thread;
    pthread_t * zone_thread;
    pthread_t * student_thread;

    company_arr = (struct company **)malloc(n*sizeof(struct company *));
    zone_arr = (struct zone **)malloc(m*sizeof(struct zone *));
    student_arr = (struct student **)malloc(o*sizeof(struct student *));
    company_thread = (pthread_t *)malloc(n*sizeof(pthread_t));
    zone_thread = (pthread_t *)malloc(m*sizeof(pthread_t));
    student_thread = (pthread_t *)malloc(o*sizeof(pthread_t));

    for (i = 0; i < n; i++)
    {
        company_arr[i] = (struct company *)malloc(sizeof(struct company));
        company_arr[i]->company_id = i;
        company_arr[i]->probability = probabilities[i];
        for (j = 0; j < 5; j++)
        {
            company_arr[i]->vaccines_per_batch[j] = 0;
        }
        company_arr[i]->total_vaccines_made = 0;
        pthread_mutex_init(&(company_arr[i]->company_mutex), NULL);
        pthread_t temp;
        company_thread[i]=temp;
    }

    for (i = 0; i < m; i++)
    {
        zone_arr[i] = (struct zone *)malloc(sizeof(struct zone));
        zone_arr[i]->zone_id = i;
        zone_arr[i]->slots_remaining = 0;
        zone_arr[i]->vaccines_remaining = 0;
        zone_arr[i]->company_id = -1;
        pthread_mutex_init(&(zone_arr[i]->zone_mutex), NULL);
        pthread_t temp;
        zone_thread[i]=temp;
    }

    for (i = 0; i < o; i++)
    {
        student_arr[i] = (struct student *)malloc(sizeof(struct student));
        student_arr[i]->student_id = i;
        student_arr[i]->vaccines_given = 0;
        pthread_t temp;
        student_thread[i]=temp;
    }

    for (i = 0; i < n; i++)
    {
        pthread_create(&company_thread[i], NULL, company_function, company_arr[i]);
    }

    for (i = 0; i < m; i++)
    {
        pthread_create(&zone_thread[i], NULL, zone_function, zone_arr[i]);
    }

    for (i = 0; i < o; i++)
    {
        pthread_create(&student_thread[i], NULL, student_function, student_arr[i]);
    }

    for (i = 0; i < n; i++)
    {
        pthread_join(company_thread[i], NULL);
    }

    for (i = 0; i < m; i++)
    {
        pthread_join(zone_thread[i], NULL);
    }

    for (i = 0; i < o; i++)
    {
        pthread_join(student_thread[i], NULL);
    }
    printf("Simulation Over.\n");
    return 0;
}