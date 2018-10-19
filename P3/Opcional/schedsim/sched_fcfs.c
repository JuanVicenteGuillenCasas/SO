#include "sched.h"

// Sin expropiación, el primero que entra es el primero que sale, además no hay replanificación ya que ya entran en la cola con su orden correspondiente.

static task_t* pick_next_task_fcfs(runqueue_t* rq)
{
	//Cogemos la tarea que se encuentre en la cabeza de la cola de tareas que es la que más tiempo lleva.
	task_t* t=head_slist(&rq->tasks);
	
	if (t) 
		remove_slist(&rq->tasks,t); // La quitamos de la cola.

	return t;
}

static void enqueue_task_fcfs(task_t* t,runqueue_t* rq, int preempted)
{
		if (t->on_rq || is_idle_task(t)) // Si esta en la cola o es una tarea nula nada que hacer, nos piramos.
		return;

	insert_slist(&rq->tasks,t); //La insertamos detras de todas, ya que es la última en llegar.
}


static task_t* steal_task_fcfs(runqueue_t* rq)
{
	//Cogemos la tarea que se encuentre en la ultima posición de la cola de tareas que es la que menos tiempo lleva.
	task_t* t =tail_slist(&rq->tasks);

	if (t) 
		remove_slist(&rq->tasks,t); //La quitamos de la cola ya que pasara a la cola de otra CPU

	return t;
}

sched_class_t fcfs_sched= {
	.pick_next_task=pick_next_task_fcfs,
	.enqueue_task=enqueue_task_fcfs,
	.steal_task=steal_task_fcfs
};