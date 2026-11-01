#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define MAX_QUEUE 5
#define THREAD_COUNT 2

typedef struct {
    int buf[MAX_QUEUE];
    int front;
    int rear;
} CQUEUE;
void CQ_INIT(CQUEUE *cq){
    cq->front=-1;
    cq->rear=-1;
}
bool CQ_isFULL(CQUEUE* cq){
    return (cq->rear+1)%MAX_QUEUE == cq->front;
}
bool CQ_isEmpty(CQUEUE* cq){
    return (cq->rear) == (cq->front);
}
int CQ_Pop(CQUEUE* cq){
    if(!CQ_isEmpty(cq)){
        cq->front = (cq->front+1)%MAX_QUEUE;
        return cq->buf[cq->front];
    }else{
        printf("![ERROR] QUEUE is Empty!\n");
        return -1;
    }
}
void CQ_Push(CQUEUE* cq, int x){
    if(!CQ_isFULL(cq)){
        cq->rear = (cq->rear+1)%MAX_QUEUE;
        cq->buf[cq->rear] = x;
    }else{
        printf("![ERROR] QUEUE is already FULL!\n");
    }
}

CQUEUE cq;
sem_t semWrite;
sem_t semRead;
sem_t mutex;

void* thread_saengsan(void* arg){
    int id = *((int*) arg);
    while(1){
        sleep(rand()%2 + 1);
        
        int x = rand()%101;

        sem_wait(&semWrite);
        
        sem_wait(&mutex);

        CQ_Push(&cq, x);
        printf("[producer %d] produce: %d (buffer i:%d)\n", id, x, cq.rear);
        
        sem_post(&mutex);

        sem_post(&semRead);
    }
    return NULL;
}

void* thread_sobi(void* arg){
    int id = *((int*)arg);
    while(1){
        sleep(rand()%2+2);
        sem_wait(&semRead);

        sem_wait(&mutex);

        int x = CQ_Pop(&cq);
        printf("[consumer %d] consume: %d (buffer i: %d)\n", id, x, cq.rear);

        sem_post(&mutex);

        sem_post(&semWrite);
    }
    return NULL;
}

int main(){
    pthread_t prod_threads[THREAD_COUNT];
    pthread_t cons_threads[THREAD_COUNT];
    int prod_id[THREAD_COUNT];
    int cons_id[THREAD_COUNT];

    srand(time(NULL));
    CQ_INIT(&cq);

    sem_init(&semWrite, 0, MAX_QUEUE);
    sem_init(&semRead, 0, 0);
    sem_init(&mutex, 0, 1);

    for(int i = 0; i < THREAD_COUNT; i++){
        prod_id[i] = i+1;
        pthread_create(&prod_threads[i], NULL, thread_saengsan, (void*)&prod_id[i]);
    }
    for(int i = 0; i < THREAD_COUNT; i++){
        cons_id[i] = i+1;
        pthread_create(&cons_threads[i], NULL, thread_sobi, (void*)&cons_id[i]);
    }

    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(prod_threads[i], NULL);
        pthread_join(cons_threads[i], NULL);
    }

    sem_destroy(&semWrite);
    sem_destroy(&semRead);
    sem_destroy(&mutex);

    return 0;
}