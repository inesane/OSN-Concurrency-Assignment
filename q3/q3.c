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
#include <semaphore.h>
#include <string.h>

#define YELLOW "\033[01;33m"
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"

int k,a,e,c,t1,t2,t;
int errno;

struct player
{
    int player_id;
    char name[20];
    char instrument;
    int arrival_time;
    int var;
    pthread_mutex_t player_mutex;
};

struct stage
{
    char musician_name[20];
    char singer_name[20];    
    int musician_occupied;
    int singer_occupied;
    int stage_id;
    char stage_type;
    pthread_mutex_t stage_mutex;
};

struct player ** player_arr;
struct stage ** stage_arr;

sem_t acoustic_semaphore;
sem_t electric_semaphore;
sem_t coordinator_semaphore;
sem_t musicians_semaphore;

void * tshirt(void *arg)
{
    struct player * play = (struct player *)arg;
    sem_wait(&coordinator_semaphore);
    printf(CYAN);
    printf("%s collecting tshirt\n", play->name);
    sleep(2);
    sem_post(&coordinator_semaphore);
}

void *singer_on_acoustic(void *arg)
{
    struct player *play = (struct player *)arg;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += t;
    struct stage *stag;

    int s = sem_timedwait(&acoustic_semaphore, &ts);

    pthread_mutex_lock(&play->player_mutex);
    if (s == -1)
    {

        if (play->var == -2 || play->var == -1 || play->var == 0)
        {
            if (play->var != -2)
            {
                play->var--;
            }
            else if (play->var == -2)
            {
                printf(RED);
                printf("%s %c left because of impatience\n", play->name, play->instrument);
            }
            // sem_post(&acoustic_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        else if (play->var == 1)
        {
            // sem_post(&acoustic_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        pthread_mutex_unlock(&play->player_mutex);
        return NULL;
    }
    else
    {
        // pthread_mutex_lock(&play->player_mutex);
        if (play->var == 1)
        {
            sem_post(&acoustic_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        play->var = 1;
        pthread_mutex_unlock(&play->player_mutex);
    }
    // printf("2222222222\n");
    for (int i = 0; i < a + e; i++)
    {
        pthread_mutex_lock(&stage_arr[i]->stage_mutex);
        // printf("6 stage checking %s %d\n", play->name, i);
        if (stage_arr[i]->stage_type == 'a' && stage_arr[i]->singer_occupied == 0 && stage_arr[i]->musician_occupied == 0)
        {
            stag = stage_arr[i];
            strcpy(stage_arr[i]->singer_name, play->name);
            stage_arr[i]->singer_occupied = 1;
            pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            break;
        }
        pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
    }
    int tim = (rand() % (t2 - t1 + 1) + t1);
    printf(YELLOW);
    printf("%s performing %c at acoustic stage %d for %d seconds\n", play->name, play->instrument, stag->stage_id, tim);
    sleep(tim);
    printf(BLUE);
    printf("%s performance at acoustic stage number %d ended\n", play->name, stag->stage_id);
    pthread_mutex_lock(&stag->stage_mutex);
    stag->singer_occupied = 0;
    pthread_mutex_unlock(&stag->stage_mutex);
    sem_post(&acoustic_semaphore);
    pthread_t tid;
    pthread_create(&tid, NULL, tshirt, arg);
    pthread_join(tid, NULL);
    return NULL;
}

void * bass_or_vio(void *arg)
{
    struct player * play = (struct player *)arg;
    sleep(play->arrival_time);
    printf(GREEN);
    printf("%s has arrived\n", play->name);
    struct stage * st;
    if(play->instrument=='v')
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += t;
        int s;
        s = sem_timedwait(&acoustic_semaphore, &ts);
        if(s==-1)
        {
            printf(RED);
            printf("%s %c left because of impatience.\n", play->name, play->instrument);
            return NULL;
        }
        else
        {
            for(int i=0;i<a+e;i++)
            {
                pthread_mutex_lock(&stage_arr[i]->stage_mutex);
                // printf("1 stage checking %s %d\n", play->name, i);
                if(stage_arr[i]->musician_occupied==0 && stage_arr[i]->singer_occupied==0 && stage_arr[i]->stage_type=='a')
                {
                    stage_arr[i]->musician_occupied=1;
                    strcpy(stage_arr[i]->musician_name, play->name);
                    st=stage_arr[i];
                    pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
                    break;
                }
                pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            }
                sem_post(&musicians_semaphore);

                int tim = (rand() % (t2-t1+1)+t1);
                printf(YELLOW);
                printf("%s performing %c at acoustic stage %d for %d seconds\n", play->name, play->instrument, st->stage_id, tim);
                sleep(tim);
                if(st->singer_occupied==1)
                {
                    sleep(2);
                    printf(BLUE);
                    printf("%s performance at acoustic stage number %d ended\n", play->name, st->stage_id);
                    printf("%s performance at acoustic stage number %d ended\n", st->singer_name, st->stage_id);

                    char singer_name[20];
                    pthread_mutex_lock(&st->stage_mutex);
                    strcpy(singer_name, st->singer_name);
                    st->singer_occupied=0;
                    st->musician_occupied=0;
                    pthread_mutex_unlock(&st->stage_mutex);
                    sem_post(&acoustic_semaphore);

                    pthread_t tid1, tid2;
                    struct player * person=(struct player *)malloc(sizeof(struct player));
                    strcpy(person->name, singer_name);

                    pthread_create(&tid1, NULL, tshirt, play);
                    pthread_create(&tid2, NULL, tshirt, person);

                    pthread_join(tid1,NULL);
                    pthread_join(tid2,NULL);
                }
                else
                {
                    printf(BLUE);
                    printf("%s performance at acoustic stage number %d ended\n", play->name, st->stage_id);

                    pthread_mutex_lock(&st->stage_mutex);
                    st->singer_occupied=0;
                    st->musician_occupied=0;
                    pthread_mutex_unlock(&st->stage_mutex);
                    sem_post(&acoustic_semaphore);
                    sem_wait(&musicians_semaphore);

                    pthread_t tid;

                    pthread_create(&tid, NULL, tshirt, play);

                    pthread_join(tid, NULL);
                }
            
        }
    }
    else if(play->instrument=='b')
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += t;
        int s;
        s= sem_timedwait(&electric_semaphore, &ts);
        if(s==-1)
        {
            printf(RED);
            printf("%s %c left because of impatience.\n", play->name, play->instrument);
            return NULL;
        }
        else
        {
            for(int i=0;i<a+e;i++)
            {
                pthread_mutex_lock(&stage_arr[i]->stage_mutex);
                // printf("2 stage checking %s %d\n", play->name, i);
                if(stage_arr[i]->musician_occupied==0 && stage_arr[i]->singer_occupied==0 && stage_arr[i]->stage_type=='e')
                {
                    stage_arr[i]->musician_occupied=1;
                    strcpy(stage_arr[i]->musician_name, play->name);
                    st=stage_arr[i];
                    pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
                    break;
                }
                pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            }

                sem_post(&musicians_semaphore);
                int tim = (rand() % (t2-t1+1)+t1);
                printf(YELLOW);
                printf("%s performing %c at electric stage %d for %d seconds\n", play->name, play->instrument, st->stage_id, tim);
                sleep(tim);
                
                // printf("hello\n");
                if(st->singer_occupied==1)
                {
                    sleep(2);
                    printf(BLUE);
                    printf("%s performance at electric stage number %d ended\n", play->name, st->stage_id);
                    printf("%s performance at electric stage number %d ended\n", st->singer_name, st->stage_id);

                    char singer_name[20];
                    pthread_mutex_lock(&st->stage_mutex);
                    strcpy(singer_name, st->singer_name);
                    st->singer_occupied=0;
                    st->musician_occupied=0;
                    pthread_mutex_unlock(&st->stage_mutex);
                    sem_post(&electric_semaphore); 
                    // printf("a1\n");

                    pthread_t tid1, tid2;
                    struct player * person=(struct player *)malloc(sizeof(struct player));
                    strcpy(person->name, singer_name);

                    pthread_create(&tid1, NULL, tshirt, play);
                    pthread_create(&tid2, NULL, tshirt, person);

                    pthread_join(tid1, NULL);
                    pthread_join(tid2, NULL);
                }
                else
                {
                    printf(BLUE);
                    printf("%s performance at electric stage number %d ended\n", play->name, st->stage_id);

                    pthread_mutex_lock(&st->stage_mutex);
                    st->singer_occupied=0;
                    st->musician_occupied=0;
                    pthread_mutex_unlock(&st->stage_mutex);
                    sem_post(&electric_semaphore); 
                    // printf("a2\n");
                    sem_wait(&musicians_semaphore);

                    pthread_t tid;

                    pthread_create(&tid, NULL, tshirt, play);

                    pthread_join(tid, NULL);
                }
            }
        
    }
}

void * choose_electric(void *arg)
{
    struct player *play = (struct player *)arg;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec+=t;
    struct stage *stag;
    int s = sem_timedwait(&electric_semaphore, &ts);

    pthread_mutex_lock(&play->player_mutex);
    if(s==-1)
    {
        if(play->var==0 || play->var==-1)
        {
            if(play->var!=-1)
            {
                play->var=-1;
            }
            else if(play->var==-1)
            {
                printf(RED);
                printf("%s %c left because of impatience\n", play->name, play->instrument);
            }
            // sem_post(&electric_semaphore); printf("a1\n");
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        else if(play->var==1)
        {
            // sem_post(&electric_semaphore); printf("a1\n");
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }

        pthread_mutex_unlock(&play->player_mutex);
        return NULL;
    }
    else
    {
        if(play->var==1)
        {
            sem_post(&electric_semaphore); 
            // printf("a3\n");
            
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        play->var=1;
        pthread_mutex_unlock(&play->player_mutex);
    }

    for(int i=0;i<a+e;i++)
    {
        pthread_mutex_lock(&stage_arr[i]->stage_mutex);
        // printf("3 stage checking %s %d\n", play->name, i);
        if(stage_arr[i]->stage_type=='e' && stage_arr[i]->singer_occupied==0 && stage_arr[i]->musician_occupied==0)
        {
            stag=stage_arr[i];
            strcpy(stage_arr[i]->musician_name, play->name);
            stage_arr[i]->musician_occupied=1;
            pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            break;
        }
        pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
    }
    sem_post(&musicians_semaphore);
    int tim = (rand() % (t2-t1+1)+t1);
    printf(YELLOW);
    printf("%s performing %c at electric stage %d for %d seconds\n", play->name, play->instrument, stag->stage_id, tim);
    sleep(tim);

    if(stag->singer_occupied==1)
    {
        sleep(2);
        // printf("%s\n", stag->singer_name);
        printf(BLUE);
        printf("%s performance at electric stage number %d ended\n", play->name, stag->stage_id);
        printf("%s performance at electric stage number %d ended\n", stag->singer_name, stag->stage_id);

        char singer_name[20];
        pthread_mutex_lock(&stag->stage_mutex);
        sem_post(&electric_semaphore); 
        // printf("a4\n");
        stag->singer_occupied=0;
        stag->musician_occupied=0;
        pthread_mutex_unlock(&stag->stage_mutex);
        pthread_t ti1, ti2;
        struct player *sing=(struct player *)malloc(sizeof(struct player));
        strcpy(sing->name, singer_name);
        pthread_create(&ti1, NULL, tshirt, arg);
        pthread_create(&ti2, NULL, tshirt, sing);
        pthread_join(ti1, NULL);
        pthread_join(ti2, NULL);
        free(sing);
        return NULL;
    }
    else
    {
        printf(BLUE);
        printf("%s performance at electric stage number %d ended\n", play->name, stag->stage_id);
        pthread_mutex_lock(&stag->stage_mutex);
        stag->musician_occupied=0;
        pthread_mutex_unlock(&stag->stage_mutex);
        sem_post(&electric_semaphore); 
        // printf("a5\n");
        sem_wait(&musicians_semaphore);
        pthread_t tid;
        pthread_create(&tid, NULL, tshirt, arg);
        pthread_join(tid, NULL);
    }
    return NULL; 
}

void *singer_on_electric(void *arg)
{
    struct player *play = (struct player *)arg;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += t;
    struct stage *stag;

    int s = sem_timedwait(&electric_semaphore, &ts);

    pthread_mutex_lock(&play->player_mutex);
    if (s == -1)
    {

        if (play->var == -2 || play->var == -1 || play->var == 0)
        {
            if (play->var != -2)
            {
                play->var--;
            }
            else if (play->var == -2)
            {
                printf(RED);
                printf("%s %c left because of impatience\n", play->name, play->instrument);
            }
            // sem_post(&electric_semaphore); printf("a1\n");
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        else if (play->var == 1)
        {
            // sem_post(&electric_semaphore); printf("a1\n");
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        pthread_mutex_unlock(&play->player_mutex);
        return NULL;
    }
    else
    {
        // pthread_mutex_lock(&play->player_mutex);
        if (play->var == 1)
        {
            sem_post(&electric_semaphore);
            // printf("a7\n");
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        play->var = 1;
        pthread_mutex_unlock(&play->player_mutex);
    }
    // printf("1111111111111111\n");
    for (int i = 0; i < a + e; i++)
    {
        pthread_mutex_lock(&stage_arr[i]->stage_mutex);
        // printf("5 stage checking %s %d\n", play->name, i);
        if (stage_arr[i]->stage_type == 'e' && stage_arr[i]->singer_occupied == 0 && stage_arr[i]->musician_occupied == 0)
        {
            stag = stage_arr[i];
            strcpy(stage_arr[i]->singer_name, play->name);
            stage_arr[i]->singer_occupied = 1;
            pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            break;
        }
        pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
    }
    int tim = (rand() % (t2 - t1 + 1) + t1);
    printf(YELLOW);
    printf("%s performing %c at electric stage %d for %d seconds\n", play->name, play->instrument, stag->stage_id, tim);
    sleep(tim);
    // printf("hi\n");
    printf(BLUE);
    printf("%s performance at electric stage number %d ended\n", play->name, stag->stage_id);
    pthread_mutex_lock(&stag->stage_mutex);
    stag->singer_occupied = 0;
    pthread_mutex_unlock(&stag->stage_mutex);
    sem_post(&electric_semaphore);
    // printf("a8\n");
    pthread_t tid;
    pthread_create(&tid, NULL, tshirt, arg);
    pthread_join(tid, NULL);
    return NULL;
}

void *sing_together(void *arg)
{
    struct player *play = (struct player *)arg;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += t;
    struct stage *stag;
    int s = sem_timedwait(&musicians_semaphore, &ts);
    pthread_mutex_lock(&play->player_mutex);
    if (s == -1)
    {
        if (play->var == 0 || play->var == -1 || play->var == -2)
        {
            if (play->var != -2)
            {
                play->var--;
            }
            else if (play->var == -2)
            {
                printf(RED);
                printf("%s %c left because of impatience\n", play->name, play->instrument);
            }
            // sem_post(&musicians_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        else if (play->var == 1)
        {
            // sem_post(&musicians_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        pthread_mutex_unlock(&play->player_mutex);
        return NULL;
    }
    else
    {
        // pthread_mutex_lock(&play->player_mutex);
        if (play->var == 1)
        {
            sem_post(&musicians_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        play->var = 1;
        pthread_mutex_unlock(&play->player_mutex);
    }
    // printf("33333333333333\n");
    for (int i = 0; i < a + e; i++)
    {
        pthread_mutex_lock(&stage_arr[i]->stage_mutex);
        // printf("7 stage checking %s %d\n", play->name, i);
        if (stage_arr[i]->singer_occupied == 0 && stage_arr[i]->musician_occupied == 1)
        {
            stag = stage_arr[i];
            strcpy(stage_arr[i]->singer_name, play->name);
            stage_arr[i]->singer_occupied = 1;
            pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            break;
        }
        pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
    }
    printf(YELLOW);
    printf("%s joined %s's performance, performance extended by 2 seconds\n", stag->singer_name, stag->musician_name);
}

void *choose_acoustic(void *arg)
{
    struct player *play = (struct player *)arg;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += t;
    struct stage *stag;
    int s = sem_timedwait(&acoustic_semaphore, &ts);

    pthread_mutex_lock(&play->player_mutex);
    if (s == -1)
    {
        if (play->var == 0 || play->var == -1)
        {
            if (play->var != -1)
            {
                play->var = -1;
            }
            else if (play->var == -1)
            {
                printf(RED);
                printf("%s %c left because of impatience\n", play->name, play->instrument);
            }
            // sem_post(&acoustic_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        else if (play->var == 1)
        {
            // sem_post(&acoustic_semaphore);
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }

        pthread_mutex_unlock(&play->player_mutex);
        return NULL;
    }
    else
    {
        if (play->var == 1)
        {
            sem_post(&acoustic_semaphore);
            // printf("a6\n");
            pthread_mutex_unlock(&play->player_mutex);
            return NULL;
        }
        play->var = 1;
        pthread_mutex_unlock(&play->player_mutex);
    }

    for (int i = 0; i < a + e; i++)
    {
        pthread_mutex_lock(&stage_arr[i]->stage_mutex);
        // printf("4 stage checking %s %d\n", play->name, i);
        if (stage_arr[i]->stage_type == 'a' && stage_arr[i]->singer_occupied == 0 && stage_arr[i]->musician_occupied == 0)
        {
            stag = stage_arr[i];
            strcpy(stage_arr[i]->musician_name, play->name);
            stage_arr[i]->musician_occupied = 1;
            pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
            break;
        }
        pthread_mutex_unlock(&stage_arr[i]->stage_mutex);
    }
    sem_post(&musicians_semaphore);
    int tim = (rand() % (t2 - t1 + 1) + t1);
    printf(YELLOW);
    printf("%s performing %c at acoustic stage %d for %d seconds\n", play->name, play->instrument, stag->stage_id, tim);
    sleep(tim);

    if (stag->singer_occupied == 1)
    {
        sleep(2);
        // printf("%s\n", stag->singer_name);
        printf(BLUE);
        printf("%s performance at acoustic stage number %d ended\n", play->name, stag->stage_id);
        printf("%s performance at acoustic stage number %d ended\n", stag->singer_name, stag->stage_id);

        char singer_name[20];
        pthread_mutex_lock(&stag->stage_mutex);
        sem_post(&acoustic_semaphore);
        stag->singer_occupied = 0;
        stag->musician_occupied = 0;
        pthread_mutex_unlock(&stag->stage_mutex);
        pthread_t ti1, ti2;
        struct player *sing = (struct player *)malloc(sizeof(struct player));
        strcpy(sing->name, stag->singer_name);
        // printf("sing_name: %s, singer_name")
        pthread_create(&ti1, NULL, tshirt, arg);
        pthread_create(&ti2, NULL, tshirt, sing);
        pthread_join(ti1, NULL);
        pthread_join(ti2, NULL);
        free(sing);
        return NULL;
    }
    else
    {
        // printf("hi\n");
        printf(BLUE);
        printf("%s performance at acoustic stage number %d ended\n", play->name, stag->stage_id);
        pthread_mutex_lock(&stag->stage_mutex);
        stag->musician_occupied = 0; // ??
        pthread_mutex_unlock(&stag->stage_mutex);
        sem_post(&acoustic_semaphore);
        sem_wait(&musicians_semaphore);
        pthread_t tid;
        pthread_create(&tid, NULL, tshirt, arg);
        pthread_join(tid, NULL);
    }
    return NULL;
}

void *singer_split(void *arg)
{
    struct player *play = (struct player *)arg;

    sleep(play->arrival_time);

    printf(GREEN);
    printf("%s %c arrived\n", play->name, play->instrument);

    play->var = 0;

    pthread_t ti1, ti2, ti3;

    pthread_create(&ti1, NULL, singer_on_acoustic, arg);
    pthread_create(&ti2, NULL, singer_on_electric, arg);
    pthread_create(&ti3, NULL, sing_together, arg);

    pthread_join(ti1, NULL);
    pthread_join(ti2, NULL);
    pthread_join(ti3, NULL);

    return NULL;
}

void *two_on_stage(void *arg)
{
    struct player *play = (struct player *)arg;
    sleep(play->arrival_time);
    printf(GREEN);
    printf("%s %c arrived\n", play->name, play->instrument);
    play->var = 0;
    pthread_t ti1, ti2;
    pthread_create(&ti1, NULL, choose_electric, arg);
    pthread_create(&ti2, NULL, choose_acoustic, arg);
    pthread_join(ti1, NULL);
    pthread_join(ti2, NULL);
    return NULL;
}

int main()
{
    srand(time(NULL));
    scanf("%d %d %d %d %d %d %d", &k, &a, &e, &c, &t1, &t2, &t);
    int i, stages;
    stages=a+e;
    char nam[k][20];
    char instru[k];
    int arr[k];
    for(i=0;i<k;i++)
    {
        scanf("%s %c %d", nam[i], &instru[i], &arr[i]);
    }

    pthread_t * performer[k];

    player_arr = (struct player **)malloc(k*sizeof(struct player *));
    stage_arr = (struct stage **)malloc(stages*sizeof(struct stage *));

    for(i=0;i<k;i++)
    {
        player_arr[i] = (struct player *)malloc(sizeof(struct player));
        player_arr[i]->player_id=i;
        strcpy(player_arr[i]->name, nam[i]);
        player_arr[i]->instrument=instru[i];
        player_arr[i]->arrival_time=arr[i];
        player_arr[i]->var=0;
        pthread_mutex_init(&(player_arr[i]->player_mutex), NULL);
    }

    for(i=0;i<stages;i++)
    {
        stage_arr[i] = (struct stage *)malloc(sizeof(struct stage));
        stage_arr[i]->musician_name;
        stage_arr[i]->singer_name;
        stage_arr[i]->musician_occupied=0;
        stage_arr[i]->singer_occupied=0;
        stage_arr[i]->stage_id=i;
        if(i<a)
        {
            stage_arr[i]->stage_type='a';
        }
        else
        {
            stage_arr[i]->stage_type='e';
        }
        pthread_mutex_init(&(stage_arr[i]->stage_mutex), NULL);
    }

    pthread_t musicians[k];
    pthread_t stag[stages];

    sem_init(&acoustic_semaphore, 0, a);
    sem_init(&electric_semaphore, 0, e);
    sem_init(&coordinator_semaphore, 0, c);
    sem_init(&musicians_semaphore, 0, 0);

    for(i=0;i<k;i++)
    {
        if(player_arr[i]->instrument=='v'||player_arr[i]->instrument=='b')
        pthread_create(&musicians[i], NULL, bass_or_vio, player_arr[i]);
        else if(player_arr[i]->instrument=='p'||player_arr[i]->instrument=='g')
        {
            pthread_create(&musicians[i], NULL, two_on_stage, player_arr[i]);
        }
        else if(player_arr[i]->instrument=='s')
        {
            pthread_create(&musicians[i], NULL, singer_split, player_arr[i]);
        }
    }



    for(i=0;i<k;i++)
    {
        pthread_join(musicians[i], NULL);
    }
    printf(RED);
    printf("Finished\n");
    // for(i=0;i<k;i++)
    // {

    // }
}