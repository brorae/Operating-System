/*
 * Copyright 2021. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로 제작되었습니다.
 */

 /*
NAMED SEMAPHORE
sem_open        Opens/creates a named semaphore for use by a process
sem_wait        lock a semaphore
sem_post        unlock a semaphore
sem_close       Deallocates the specified named semaphore
sem_unlink      Removes a specified named semaphore
 */
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "threadpool.h"

/*
 * 스레드 풀의 FIFO 대기열 길이와 일꾼 스레드의 갯수를 지정한다.
 */
#define QUEUE_SIZE 10
#define NUMBER_OF_BEES 3

/*
 * 스레드를 통해 실행할 작업 함수와 함수의 인자정보 구조체 타입
 */
typedef struct {
    void (*function)(void *p);
    void *data;
} task_t;


/*
 * 스레드 풀의 FIFO 대기열인 worktodo 배열로 원형 버퍼의 역할을 한다.
 */
static task_t worktodo[QUEUE_SIZE];
static pthread_mutex_t mutex;

int head = 0;
int tail = 0;
int temp = 0;
int q_cnt = 0;


static int enqueue(task_t t)
{
    if (q_cnt < QUEUE_SIZE){
        pthread_mutex_lock(&mutex);
        worktodo[tail] = t;
        if (tail == QUEUE_SIZE-1){
            tail = 0;
        }
        else{
            tail++;
        }
        q_cnt++;
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    else{
        return 1;
    }
}

static int dequeue(task_t *t)
{
    if (q_cnt != 0){
        pthread_mutex_lock(&mutex);
        temp = head;
        if(head == QUEUE_SIZE-1){
            head = 0;
        }
        else{
            head++;
        }
        q_cnt--;
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    else{
        return 1;
    }
}


static pthread_t bee[NUMBER_OF_BEES];
static sem_t *sem;


static void *worker(void *param)
{
    while(1){
        sem_wait(sem);
        task_t work;
        if(dequeue(&work)==0){
            work = worktodo[temp];
            (*work.function)(work.data);
        }
    }
    pthread_exit(0);
}


int pool_submit(void (*f)(void *p), void *p)
{
    task_t submit;
    submit.function = f;
    submit.data = p;
    if(enqueue(submit)==1){
        return 1;
    }
    else{
        sem_post(sem);
        return 0;
    }
}


 void pool_init(void)
 {
   pthread_mutex_init(&mutex, NULL);
   sem = sem_open("sem", O_CREAT, 0600, 0);
   for(int i = 0; i < NUMBER_OF_BEES; i++){
       pthread_create(&bee[i], NULL, worker, NULL);
   }
 }


 void pool_shutdown(void)
 {
   for(int i = 0; i < NUMBER_OF_BEES; i++) {
       pthread_cancel(bee[i]);
       pthread_join(bee[i], NULL);
   }
   sem_close(sem); //세마포 종료
   sem_unlink("sem"); //세마포 제거
   pthread_mutex_destroy(&mutex); //mutex 제거
 }
