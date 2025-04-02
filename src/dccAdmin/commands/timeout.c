#include "timeout.h"

void timeout_processes(int timeout_seconds)
{
  int running_count = 0;
  for (int i = 0; i < process_count; i++)
    if (processes[i].running)
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
    pid_t pid = getpid();

    for (int i = 0; i < process_count; i++)
      if (processes[i].running)
        processes[i].timeout_id = pid;

    sleep(timeout_seconds);

    printf("Timeout cumplido!\n");
    for (int i = 0; i < process_count; i++)
      if (processes[i].running && processes[i].timeout_id == pid)
      {
        time_t current_time = time(NULL);
        int execution_time = (int)difftime(current_time, processes[i].start_time);

        printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");
        printf("%-6d %-20s %-6d %-6d %-6d\n",
               processes[i].pid,
               processes[i].name,
               execution_time,
               processes[i].exit_code,
               SIGTERM);

        kill(processes[i].pid, SIGTERM);
      }
    exit(0);
  }
  else
  {
    printf("Timeout iniciado en proceso %d por %d segundos\n", timeout_pid, timeout_seconds);
  }
}
