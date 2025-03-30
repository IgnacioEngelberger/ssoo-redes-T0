#include "quit.h"

void quit_program(struct shared_memory *shared)
{
  int running_count = 0;
  for (int i = 0; i < shared->process_count; i++)
    if (shared->processes[i].running)
    {
      running_count++;
      kill(shared->processes[i].pid, SIGINT);
    }

  if (running_count > 0)
  {
    printf("Esperando 10 segundos a que los procesos terminen...\n");
    sleep(10);

    for (int i = 0; i < shared->process_count; i++)
    {
      if (shared->processes[i].running)
      {
        int status;
        if (waitpid(shared->processes[i].pid, &status, WNOHANG) == 0)
        {
          printf("Enviando SIGKILL a proceso %d\n", shared->processes[i].pid);
          kill(shared->processes[i].pid, SIGKILL);
          waitpid(shared->processes[i].pid, &status, 0);
        }

        shared->processes[i].running = 0;
        shared->processes[i].timeout_id = 0;   // Limpiar el timeout_id
        shared->processes[i].sigterm_time = 0; // Limpiar el tiempo de SIGTERM
        if (WIFEXITED(status))
        {
          shared->processes[i].exit_code = WEXITSTATUS(status);
          shared->processes[i].signal_value = -1;
        }
        else if (WIFSIGNALED(status))
        {
          shared->processes[i].exit_code = -1;
          shared->processes[i].signal_value = WTERMSIG(status);
        }
      }
    }
  }

  printf("DCCAdmin finalizado\n");
  printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");
  for (int i = 0; i < shared->process_count; i++)
    print_process_info(&shared->processes[i]);

  exit(0);
}
