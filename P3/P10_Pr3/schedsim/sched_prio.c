#include "sched.h"

static task_t* pick_next_task_prio(runqueue_t* rq)
{
	task_t* t=head_slist(&rq->tasks); //Cogemos la cabeza de la cola que será la que tenga más prioridad, al estar ordenada.

	if (t) 
		remove_slist(&rq->tasks,t); //Lo mismo que en la anterior, la borramos de la cola.

	return t;
}

static int compare_priority_tasks_cpu_burst(void *t1,void *t2)
{
	task_t* tsk1=(task_t*)t1;
	task_t* tsk2=(task_t*)t2;
	return (tsk1->prio - tsk2->prio);
}


static void enqueue_task_prio(task_t* t,runqueue_t* rq, int preempted)
{
	if (t->on_rq || is_idle_task(t)) // Si esta en la cola o es una tarea nula nada que hacer, nos piramos.
		return;

	if (t->flags & TF_INSERT_FRONT) {	
		t->flags&=~TF_INSERT_FRONT; //Quitamos el flag
		sorted_insert_slist_front(&rq->tasks, t, 1, compare_priority_tasks_cpu_burst);  //Insertamos tarea en la cola.
	} else
		sorted_insert_slist(&rq->tasks, t, 1,compare_priority_tasks_cpu_burst);  //Insertamos tarea en la cola.

    //Si la tarea que estamos metiendo en la cola no es una expropiada, o sea que no estuviera en ejecución.
	if (!preempted) {
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
	task_t* t=tail_slist(&rq->tasks); // Lo mismo que el anterior, al estar ordenado tenemos que la última de la cola es la que peor prioridad lleva.

	if (t) 
		remove_slist(&rq->tasks,t);  //Lo mismo que en la anterior, la borramos de la cola.
	
	return t;
}


sched_class_t prio_sched= {
	.pick_next_task=pick_next_task_prio,
	.enqueue_task=enqueue_task_prio,
	.steal_task=steal_task_prio
};