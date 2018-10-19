#include "sched.h"

#define MAX_PRIO 5

/* Per-CPU private structure to store an array of run queues */
struct prio_private_rq {
    slist_t tasks_prio[MAX_PRIO];
};

/* Initialize class-specific global data structures */
static int sched_init_prio(void)
{
    int cpu,prio;
    struct prio_private_rq* priv_rq;
    runqueue_t* cpu_rq;

    /* 
     * Reserve an array of run queues for each CPU 
     * and store the pointer of the private structure
     * in the "rq_cs_data" field of per-CPU run queues.
     * This makes it possible to retrieve the pointer to 
     * the structure later from the sched_class operations.
     */
    for (cpu=0; cpu<nr_cpus; cpu++) {
        cpu_rq=get_runqueue_cpu(cpu);
        priv_rq=malloc(sizeof(struct prio_private_rq));

        if (priv_rq==NULL) {
            fprintf(stderr,"Cannot allocate memory for private run queues\n");
            return 1;
        }

        for (prio=0; prio<MAX_PRIO; prio++)
            init_slist(&priv_rq->tasks_prio[prio],offsetof(task_t,rq_links));

        /* Assign pointer */
        cpu_rq->rq_cs_data=priv_rq;
    }
    return 0;
}

/* Free up class-specific global data structures */
static void sched_destroy_prio(void)
{
    int cpu;
    runqueue_t* cpu_rq;

    for (cpu=0; cpu<nr_cpus; cpu++) {
        cpu_rq=get_runqueue_cpu(cpu);
        /* Free up data structure */
        free(cpu_rq->rq_cs_data);
        cpu_rq->rq_cs_data=NULL;
    }
}


static task_t* pick_next_task_prio(runqueue_t* rq)
{
	struct prio_private_rq* prq=rq->rq_cs_data;
	int p = 0;
    while(is_empty_slist(&prq->tasks_prio[p]) && p < MAX_PRIO)p++; // vemos la que antes tenga alguna.
    if (p == MAX_PRIO) return NULL; //todas vacias;
    task_t* t=head_slist(&prq->tasks_prio[p]);
    if (t) 
		remove_slist(&prq->tasks_prio[p],t); // Podemos cogerla y la quitamos de la lista.
	return t;
}


static void enqueue_task_prio(task_t* t,runqueue_t* rq, int preempted)
{
    struct prio_private_rq* prq=rq->rq_cs_data;
	int p = t->prio;
	if (is_idle_task(t)) return;
	
	insert_slist(&prq->tasks_prio[p],t); //Insertamos tarea en la cola.
	
    if (!preempted) { //Vemos si podemos expropiar la tarea actual.
		task_t* current=rq->cur_task;

		//Si tenemos permiso de expropiar y la prioridad es mejor en la que metemos que la de la que se está ejecutando damos permiso para la expropiación de ésta.
		if (preemptive_scheduler && t->prio < current->prio) {
			rq->need_resched=TRUE; //Autorizamos a la expropiación.
			current->flags |= TF_INSERT_FRONT; 
		} 
	}
}

static task_t* steal_task_prio(runqueue_t* rq)
{
    struct prio_private_rq* prq=rq->rq_cs_data;
	int p = MAX_PRIO - 1;
    while(is_empty_slist(&prq->tasks_prio[p]) && p >= 0)p--; // Buscamos desde la minima prioridad la primera vacia.
    if(p < 0) return NULL;
    task_t* t=tail_slist(&prq->tasks_prio[p]);
    if (t) 
		remove_slist(&prq->tasks_prio[p],t); // Podemos cogerla y la quitamos de la lista.
	return t;
}

sched_class_t prio_multi_sched= {
    .sched_init=sched_init_prio,
    .sched_destroy=sched_destroy_prio,
    .pick_next_task=pick_next_task_prio,
    .enqueue_task=enqueue_task_prio,
    .steal_task=steal_task_prio
};
