#include "quit.h"

void quit_program()
{
  int running_count = 0;
  for (int i = 0; i < process_count; i++)
    if (processes[i].running)
    {
      running_count++;
      kill(processes[i].pid, SIGINT);
    }

  if (running_count > 0)
  {
    printf("Esperando 10 segundos a que los procesos terminen...\n");
    sleep(10);

    for (int i = 0; i < process_count; i++)
    {
      if (processes[i].running)
      {
        int status;
        if (waitpid(processes[i].pid, &status, WNOHANG) == 0)
        {
          printf("Enviando SIGKILL a proceso %d\n", processes[i].pid);
          kill(processes[i].pid, SIGKILL);
          waitpid(processes[i].pid, &status, 0);
        }

        processes[i].running = 0;
        processes[i].timeout_id = 0;   // Limpiar el timeout_id
        processes[i].sigterm_time = 0; // Limpiar el tiempo de SIGTERM
        if (WIFEXITED(status))
          processes[i].exit_code = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
          processes[i].signal_value = WTERMSIG(status);
      }
    }
  }

  printf("DCCAdmin finalizado\n");
  printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");
  for (int i = 0; i < process_count; i++)
    print_process_info(&processes[i]);

  exit(0);
}
