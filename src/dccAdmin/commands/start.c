#include "start.h"

void start_process(char **args)
{
  if (args[1] == NULL)
  {
    printf("Error: Debe especificar un ejecutable\n");
    return;
  }

  if (process_count >= MAX_PROCESSES)
  {
    printf("Error: Número máximo de procesos alcanzado (10)\n");
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
    // Proceso hijo
    execvp(args[1], &args[1]);
    // Si llegamos aquí, hubo un error
    perror("Error al ejecutar el programa");
    exit(EXIT_FAILURE);
  }
  else
  {
    // Proceso padre
    processes[process_count].pid = pid;
    strncpy(processes[process_count].name, args[1], 255);
    processes[process_count].start_time = time(NULL);
    processes[process_count].exit_code = -1;
    processes[process_count].signal_value = -1;
    processes[process_count].running = 1;
    processes[process_count].timeout_id = 0;   // Inicialmente no está afectado por timeout
    processes[process_count].sigterm_time = 0; // Inicialmente no ha recibido SIGTERM

    printf("Proceso iniciado: PID=%d, Programa=%s\n", pid, args[1]);
    process_count++;
  }
}
