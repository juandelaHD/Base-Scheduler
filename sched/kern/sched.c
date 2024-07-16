#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <lib/dic.h>
#include <inc/string.h>

#define MAX_SCHED_RUNS                                                         \
	40  // Define el número máximo de corridas del scheduler antes de hacer un boost

static struct DictionaryEntry runs_per_process[MAX_ENTRIES];
int runs_per_process_count = 0;

static int sched_total_calls;
static int sched_total_boost;

void sched_halt(void);

static int sched_runs;  // Cantidad de veces que se corrio el Scheduler desde el ultimo boost

void
_round_robin()
{
	int i = 0;
	if (curenv) {
		i = ENVX(curenv->env_id);
		// Si (i+1) es igual a NENV, entonces i = 0. Si es menor, i = i+1
		i = (i + 1) % NENV;
	}
	int start = i;
	do {
		if (envs[i].env_status == ENV_RUNNABLE) {
			env_run(&envs[i]);
		}
		i = (i + 1) % NENV;
	} while (i != start);
	// si ya dimos la vuelta completa, evaluamos correr el proceso acrtual
	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	}
}

void
_boost()
{
	int i = 0;
	for (i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			envs[i].env_pri = MAX_ENV_PRIORITY;
		}
	}
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin
	sched_total_calls++;
	_round_robin();

#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities

	sched_runs++;
	sched_total_calls++;

	// Si el scheduler corrió MAX_SCHED_RUNS veces, hacer un boost
	if (sched_runs == MAX_SCHED_RUNS) {
		_boost();
		sched_total_boost++;
		sched_runs = 0;
	}
	// Si el entorno actual no es NULL y tiene prioridad mayor a 0, decrementar su prioridad
	else if (curenv && curenv->env_pri > 0) {
		curenv->env_pri--;
	}

	int highest_priority = 0;
	struct Env *next = NULL;

	// Encontrar el siguiente entorno con la prioridad más alta
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE &&
		    envs[i].env_pri >= highest_priority) {
			highest_priority = envs[i].env_pri;
			next = &envs[i];
		}
	}

	// Si encontramos un entorno con prioridad más alta o igual, lo ejecutamos
	if (next != NULL) {
		addEntry(runs_per_process, &runs_per_process_count, next->env_id);
		// Si todos los procesos estan con prioridad 0, ejecuto Round Robin
		// ya que no se pueden decrementar prioridades (es como si no hubiera
		// prioridades ya que todos tienen constantemente la misma)
		if (highest_priority == 0) {
			_round_robin();
		} else {
			// Linea para comprobar cambio de prioridades:
			// cprintf("Running process with id: [%08x] with priority: %d\n", next->env_id, next->env_pri);
			env_run(next);
		}
	} else if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	}


#endif

	// Without scheduler, keep runing the last environment while it exists
	// if (curenv) {
	// env_run(curenv);
	// }

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");

		// Printeo De Estadisticas del Scheduler con Prioridades
		cprintf("STATISTICS:\n");
		cprintf("\tTotal Shed runs: %d\n", sched_total_calls);
		cprintf("\tTotal Shed boosts: %d\n", sched_total_boost);
		printDictionary(runs_per_process, runs_per_process_count);

		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
