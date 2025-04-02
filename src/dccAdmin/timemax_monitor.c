#include <unistd.h>
#include "timemax_monitor.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

void check_timemax_processes(int time_max)
{
  if (time_max <= 0)
    return;

  time_t current_time = time(NULL);
  for (int i = 0; i < process_count; i++)
  {
    if (processes[i].running)
    {
      int execution_time = (int)difftime(current_time, processes[i].start_time);

      if (processes[i].sigterm_time == 0 && execution_time >= time_max)
      {
        printf("TimeMax: Proceso %d (%s) ha excedido el tiempo máximo de ejecución (%d segundos).\n",
               processes[i].pid, processes[i].name, time_max);
        kill(processes[i].pid, SIGTERM);
        processes[i].sigterm_time = current_time;
      }
      else if (processes[i].sigterm_time > 0)
      {
        int time_since_sigterm = (int)difftime(current_time, processes[i].sigterm_time);

        if (time_since_sigterm >= 5)
        {
          int status;
          if (waitpid(processes[i].pid, &status, WNOHANG) == 0)
          {
            printf("TimeMax: Proceso %d (%s) no terminó después de SIGTERM. Enviando SIGKILL.\n",
                   processes[i].pid, processes[i].name);
            kill(processes[i].pid, SIGKILL);
            waitpid(processes[i].pid, &status, WNOHANG);
          }
        }
      }
    }
  }
}
