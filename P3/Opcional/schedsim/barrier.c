#include "barrier.h"
#include <errno.h>


#ifdef POSIX_BARRIER

/* Wrapper functions to use pthread barriers */

int sys_barrier_init(sys_barrier_t* barrier, unsigned int nthreads)
{
	return pthread_barrier_init(barrier,NULL,nthreads);
}

int sys_barrier_destroy(sys_barrier_t* barrier)
{
	return pthread_barrier_destroy(barrier);
}

int sys_barrier_wait(sys_barrier_t *barrier)
{
	return pthread_barrier_wait(barrier);
}

#else


int sys_barrier_init(sys_barrier_t *barrier, unsigned int nr_threads)
{

	pthread_mutex_init( &(barrier->mutex) , NULL);
	pthread_cond_init( &(barrier->cond) , NULL);
	barrier->max_threads = nr_threads;
	barrier->nr_threads_arrived = 0;
	
	return 0;
}


int sys_barrier_destroy(sys_barrier_t *barrier)
{
	pthread_mutex_destroy(&(barrier->mutex));
	pthread_cond_destroy(&(barrier->cond));	
	return 0;
}


int sys_barrier_wait(sys_barrier_t *barrier)
{
	/* Implementation outline:
	   - Every thread arriving at the barrier acquires the lock and increments the nr_threads_arrived
	    counter atomically */	

	//Pasa la CPU por la barrera, sumamos uno al contador y echamos el cerrojo	
	pthread_mutex_lock(&(barrier->mutex));
	barrier->nr_threads_arrived++; 
			
		
	/*In the event this is not the last thread to arrive at the barrier, the thread
	       must block in the condition variable
		  */
		 if(barrier->nr_threads_arrived < barrier->max_threads) {
			pthread_cond_wait(&(barrier->cond), &(barrier->mutex));  
		 } //Si no ha llegado el contador al maximo, realizamos wait y pasamos el cerrojo a otro CPU.
	
	 
	/*Otherwise...
	      1. Reset the barrier state in preparation for the next invocation of sys_barrier_wait() and
	      2. Wake up all threads blocked in the barrier
		  */
		  else{ 
			barrier->nr_threads_arrived = 0;
			pthread_cond_broadcast(&(barrier->cond));
		  }  //Si ha llegado el contador tenemos que resetear el contador a 0 y liberar todos los hilos bloqueados por la variable condicional.
	
	
	pthread_mutex_unlock(&(barrier->mutex)); //Quitamos el bloqueo a las demas.
	
	return 0;
}

#endif /* POSIX_BARRIER */
