#include "process_monitor.h"
#include "main.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función que se ejecuta en el thread de monitoreo
static void monitor_processes(struct shared_memory *shared)
{
  while (1)
  {
    for (int i = 0; i < shared->process_count; i++)
    {
      if (shared->processes[i].running)
      {
        printf("Proceso %d está corriendo\n", shared->processes[i].pid);
        int status;
        pid_t result = waitpid(shared->processes[i].pid, &status, WNOHANG);
        printf("waitpid result: %d, status: %d\n", result, status);

        if (result > 0)
        {
          shared->processes[i].running = 0;
          shared->processes[i].timeout_id = 0;   // Limpiar el timeout al terminar
          shared->processes[i].sigterm_time = 0; // Limpiar el tiempo de SIGTERM al terminar
          if (WIFEXITED(status))
            shared->processes[i].exit_code = WEXITSTATUS(status);
          else if (WIFSIGNALED(status))
            shared->processes[i].signal_value = WTERMSIG(status);
        }
      }
    }

    usleep(100000);
  }
}

pid_t start_process_monitor(struct shared_memory *shared)
{
  pid_t monitor_pid = fork();
  if (monitor_pid < 0)
  {
    perror("Error al crear el thread de monitoreo");
    exit(1);
  }
  else if (monitor_pid == 0)
  {
    monitor_processes(shared);
    exit(0); // Nunca debería llegar aquí
  }
  else
  {
    return monitor_pid;
  }
}

void stop_process_monitor(pid_t monitor_pid)
{
  if (monitor_pid > 0)
  {
    kill(monitor_pid, SIGTERM);
    waitpid(monitor_pid, NULL, 0);
  }
}
