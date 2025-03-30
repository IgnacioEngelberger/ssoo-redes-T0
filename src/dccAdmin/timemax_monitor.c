#include <unistd.h>
#include "timemax_monitor.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

// Función que se ejecuta en el proceso de monitoreo de time_max
static void check_timemax_processes(int time_max, struct shared_memory *shared)
{
  if (time_max <= 0)
    return; // No hay tiempo máximo

  while (1)
  {
    time_t current_time = time(NULL);
    for (int i = 0; i < shared->process_count; i++)
    {
      if (shared->processes[i].running)
      {
        int execution_time = (int)difftime(current_time, shared->processes[i].start_time);

        // Si el proceso ha excedido el tiempo máximo y aún no ha recibido SIGTERM
        if (shared->processes[i].sigterm_time == 0 && execution_time >= time_max)
        {
          printf("TimeMax: Proceso %d (%s) ha excedido el tiempo máximo de ejecución (%d segundos).\n",
                 shared->processes[i].pid, shared->processes[i].name, time_max);

          // Enviar SIGTERM
          kill(shared->processes[i].pid, SIGTERM);

          // Registrar el tiempo en que se envió SIGTERM
          shared->processes[i].sigterm_time = current_time;
        }
        // Si ya se envió SIGTERM, verificar si han pasado 5 segundos para enviar SIGKILL
        else if (shared->processes[i].sigterm_time > 0)
        {
          int time_since_sigterm = (int)difftime(current_time, shared->processes[i].sigterm_time);

          if (time_since_sigterm >= 5)
          {
            // Verificar si el proceso aún está en ejecución
            int status;
            if (waitpid(shared->processes[i].pid, &status, WNOHANG) == 0)
            {
              // No terminó después de SIGTERM, enviamos SIGKILL
              printf("TimeMax: Proceso %d (%s) no terminó después de SIGTERM. Enviando SIGKILL.\n",
                     shared->processes[i].pid, shared->processes[i].name);
              kill(shared->processes[i].pid, SIGKILL);

              // Esperamos a que termine, pero sin bloquear todo el proceso
              waitpid(shared->processes[i].pid, &status, WNOHANG);
            }
          }
        }
      }
    }
    usleep(100000);
  }
}

pid_t start_timemax_monitor(int time_max, struct shared_memory *shared)
{
  if (time_max <= 0)
    return 0; // No hay tiempo máximo, no necesitamos monitoreo

  pid_t monitor_pid = fork();
  if (monitor_pid < 0)
  {
    perror("Error al crear proceso de monitoreo de tiempo máximo");
    exit(1);
  }
  else if (monitor_pid == 0)
  {
    // Proceso hijo
    check_timemax_processes(time_max, shared);
    exit(0); // Nunca debería llegar aquí
  }
  else
  {
    return monitor_pid;
  }
}

void stop_timemax_monitor(pid_t monitor_pid)
{
  if (monitor_pid > 0)
  {
    kill(monitor_pid, SIGTERM);
    waitpid(monitor_pid, NULL, 0);
  }
}
