#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int total_sum = 0;
sem_t semaforo1;
sem_t semaforo2;

void * partial_sum1(void * arg) {
  int j = 0;
  int ni=((int*)arg)[0];
  int nf=((int*)arg)[1];

   sem_wait(&semaforo1);
  for (j = ni; j <= nf; j++)
    total_sum = total_sum + j;
   sem_post(&semaforo2);
  pthread_exit(0);
}
void * partial_sum2(void * arg) {
  int j = 0;
  int ni=((int*)arg)[0];
  int nf=((int*)arg)[1];
  sem_wait(&semaforo2);
  for (j = ni; j <= nf; j++)
    total_sum = total_sum + j;
   sem_post(&semaforo1);
  pthread_exit(0);
}

int main(void) {
  pthread_t th1, th2;
  sem_init(&semaforo1 , 0 ,0);
  sem_init(&semaforo2 , 0 ,50005000);
  int num1[2]={  1,   4999};
  int num2[2]={5000, 10000};

  /* Two threads are created */
  pthread_create(&th1, NULL, partial_sum1, (void*)num1);
  pthread_create(&th2, NULL, partial_sum2, (void*)num2);
  /* the main thread waits until both threads complete */
  pthread_join(th1, NULL);
  pthread_join(th2, NULL);

  printf("total_sum=%d and it should be 50005000\n", total_sum);

   sem_destroy(&semaforo1);   sem_destroy(&semaforo2); 

  return 0;
}
