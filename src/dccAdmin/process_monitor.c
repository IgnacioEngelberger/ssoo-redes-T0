#include "process_monitor.h"
#include "main.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función que se ejecuta en el thread de monitoreo
void monitor_processes()
{

  for (int i = 0; i < process_count; i++)
  {
    if (processes[i].running)
    {
      int status;
      pid_t result = waitpid(processes[i].pid, &status, WNOHANG);

      if (result > 0)
      {
        processes[i].running = 0;
        processes[i].timeout_id = 0;   // Limpiar el timeout al terminar
        processes[i].sigterm_time = 0; // Limpiar el tiempo de SIGTERM al terminar
        if (WIFEXITED(status))
          processes[i].exit_code = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
          processes[i].signal_value = WTERMSIG(status);
      }
    }
  }
}
