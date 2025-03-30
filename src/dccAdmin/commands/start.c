#include "start.h"

void start_process(char **args, struct shared_memory *shared)
{
  if (args[1] == NULL)
  {
    printf("Error: Debe especificar al menos un ejecutable y sus argumentos\n");
    printf("Uso: start <executable> <arg1> <arg2> ... <argn>\n");
    return;
  }

  if (shared->process_count >= MAX_PROCESSES)
  {
    printf("Error: Número máximo de procesos alcanzado (%d)\n", MAX_PROCESSES);
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
    shared->processes[shared->process_count].pid = pid;
    strncpy(shared->processes[shared->process_count].name, args[1], 255);
    shared->processes[shared->process_count].start_time = time(NULL);
    shared->processes[shared->process_count].exit_code = -1;
    shared->processes[shared->process_count].signal_value = -1;
    shared->processes[shared->process_count].running = 1;
    shared->processes[shared->process_count].timeout_id = 0;   // Inicialmente no está afectado por timeout
    shared->processes[shared->process_count].sigterm_time = 0; // Inicialmente no ha recibido SIGTERM

    shared->process_count++;

    printf("Proceso iniciado: PID=%d, Programa=%s", pid, args[1]);
    // Imprimir argumentos adicionales si existen
    for (int i = 2; args[i] != NULL; i++)
      printf(" %s", args[i]);
    printf("\n\n");
  }
}
