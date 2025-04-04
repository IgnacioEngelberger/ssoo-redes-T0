#include "start.h"
#include <unistd.h>

void start_process(char **args)
{
  if (args[1] == NULL)
  {
    printf("Error: Debe especificar al menos un ejecutable y sus argumentos\n");
    printf("Uso: start <executable> <arg1> <arg2> ... <argn>\n");
    return;
  }

  if (active_process_count >= MAX_PROCESSES)
  {
    printf("Error: Número máximo de procesos activos alcanzado (%d)\n", MAX_PROCESSES);
    return;
  }

  pid_t pid = fork();

  if (pid < 0)
  {
    perror("Error al crear proceso");
    return;
  }
  else if (pid == 0)
  {
    execvp(args[1], &args[1]);
    // Si llegamos aquí, hubo un error
    perror("Error al ejecutar el programa");
    exit(EXIT_FAILURE);
  }
  else
  {
    processes[process_count].pid = pid;
    strncpy(processes[process_count].name, args[1], 255);
    processes[process_count].start_time = time(NULL);
    processes[process_count].exit_code = -1;
    processes[process_count].signal_value = -1;
    processes[process_count].running = 1;
    processes[process_count].timeout_id = 0;   
    processes[process_count].sigterm_time = 0; 

    process_count++;
    active_process_count++;

    printf("Proceso iniciado: PID=%d, Programa=%s", pid, args[1]);
    for (int i = 2; args[i] != NULL; i++)
      printf(" %s", args[i]);
    printf("\n\n");
    
  }
  return;
}
