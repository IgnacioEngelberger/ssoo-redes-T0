#include "timeout.h"

void timeout_processes(int timeout_seconds, struct shared_memory *shared)
{
  int running_count = 0;
  for (int i = 0; i < shared->process_count; i++)
    if (shared->processes[i].running)
      running_count++;

  if (running_count == 0)
  {
    printf("No hay procesos en ejecución. Timeout no se puede ejecutar.\n");
    return;
  }

  // Crear un proceso hijo para manejar el timeout
  pid_t timeout_pid = fork();
  if (timeout_pid < 0)
  {
    perror("Error al crear proceso de timeout");
    return;
  }
  else if (timeout_pid == 0)
  {
    // PROCESO HIJO
    pid_t pid = getpid();

    // Marcar los procesos actuales como afectados por este timeout
    for (int i = 0; i < shared->process_count; i++)
      if (shared->processes[i].running)
        shared->processes[i].timeout_id = pid;

    // Esperar el tiempo especificado
    sleep(timeout_seconds);

    // Después del timeout, terminar los procesos marcados,
    // solo afectar a los procesos que estaban en ejecución cuando se lanzó este timeout
    printf("Timeout cumplido!\n");
    for (int i = 0; i < shared->process_count; i++)
      if (shared->processes[i].running && shared->processes[i].timeout_id == pid)
      {
        time_t current_time = time(NULL);
        int execution_time = (int)difftime(current_time, shared->processes[i].start_time);

        printf("%-6d %-20s %-6d %-6d %-6d\n",
               shared->processes[i].pid,
               shared->processes[i].name,
               execution_time,
               shared->processes[i].exit_code,
               shared->processes[i].signal_value);

        kill(shared->processes[i].pid, SIGTERM);
      }

    exit(0);
  }
  else
  {
    // PROCESO PADRE
    printf("Timeout iniciado en proceso %d por %d segundos\n", timeout_pid, timeout_seconds);
  }
}
