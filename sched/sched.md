# sched


## Parte 1:
En la siguiente imagen observamos el Stack instruccion a instruccion y el contenido de los registros antes y despues de la ejecuccion de iret
![img](/docs/Stack_instruccion_a_instruccion.png)


En esta otra, vemos la estructura Trapframe en el stack al entrar a context_switch() (luego de haber descartado la direccion de retorno)
![img](/docs/Trapframe_en_stack.png)

## Parte 3:
Decidimos implementar nuestro scheduler con la lógica derivada de una Multi-Level Feedback Queue (MLFQ), intentando continuar con su logica principal evitando el uso de queues.

### Reglas seguidas:

- Cada tarea tiene una prioridad y al momento de crearse lo hace con la prioridad más alta.
- Para cada tarea, no hay una prioridad fija.
- Una vez la tarea usa su asignación de tiempo, se reducirá su prioridad.
- Después de cierta cantidad de llamadas al scheduler (MAX_SCHED_RUNS), se mueven las tareas (boost) a la prioridad más alta (8).
- Si todas las tareas tienen la menor prioridad (0), se ejecuta Round Robin.


### Decisión MAX_SCHED_RUNS:
Creamos un proceso llamado testpriority con el objetivo de buscar un valor optimo para MAX_SCHED_RUNS, logrando que se realize el boost antes de que las prioridades de todos los procesos queden constantemente en cero por largos periodos de tiempo. 


### Funcionamiento paso a paso:

1. Prevención de inanición (Starvation):


    La variable `sched_runs` lleva la cuenta de cuántas veces ha corrido el scheduler. Si `sched_runs` llega a `MAX_SCHED_RUNS` (máximo número de ejecuciones consecutivas), se ejecuta la función `_boost()`, que da una oportunidad a entornos con menor prioridad para evitar que un entorno de alta prioridad monopolice la ejecución (starvation). Luego, `sched_runs` se reinicia a 0.

2. Reducción de prioridad del entorno actual:

    Se verifica si el entorno actualmente en ejecución (`curenv`) no es nulo y su prioridad es mayor de 0. Si se cumplen ambas condiciones, se decrementa la prioridad de `curenv` (se vuelve un poco menos prioritario). Esto evita que un entorno de alta prioridad se quede ejecutando indefinidamente.

3. Búsqueda del entorno con mayor prioridad:

    Se inicializan `highest_priority` a 0 (la prioridad más baja), `next` a `NULL` (ningún entorno seleccionado) y se recorre el array `envs` que contiene todos los entornos. De forma lineal, para cada entorno que esté en estado ejecutable (`ENV_RUNNABLE`) y su prioridad (`env_pri`) sea mayor o igual a `highest_priority`, se actualiza `highest_priority` con la nueva prioridad más alta. Además, se guarda el entorno con la prioridad más alta en `next`.

4. Ejecución del entorno con mayor prioridad:

    Si se encontró un entorno con mayor prioridad en `next`, se llama a la función `env_run` para ejecutarlo.
    En caso de que todos los procesos tengan prioridad minima (0), se ejecura Round Robin. 
    En caso de que no existan otros entornos disponibles para correr, chequeamos si el actual puede seguir ejecutandose.


### Ejemplo de cambio de prioridades:

En la siguiente imagen observamos el cambio de prioridad de los procesos y la ejecucion de un boost
![img](/docs/Shed_priority_change.png)




