#include "main.h"
#include "commands/start.h"
#include "commands/info.h"
#include "commands/timeout.h"
#include "commands/quit.h"
#include "commands/help.h"
#include "process_monitor.h"
#include "timemax_monitor.h"

// Definiciones de variables globales
ProcessInfo processes[MAX_PROCESSES*10];
int active_process_count = 0;
int process_count = 0;
int time_max = -1;
char **input = NULL;

// Implementación de funciones generales
void handle_sigint(int sig)
{
  printf("\nRecibido SIGINT (%d). Terminando...\n", sig);
  quit_program();
}

void print_process_info(ProcessInfo *p)
{
  time_t current_time = time(NULL);
  int execution_time = (int)difftime(current_time, p->start_time);

  printf("%-6d %-20s %-6d %-6d %-6d\n",
         p->pid,
         p->name,
         execution_time,
         p->running ? -1 : p->exit_code,
         p->running ? -1 : p->signal_value);
}

int main(int argc, char const *argv[])
{
  if (argc > 1)
  {
    time_max = atoi(argv[1]);
    printf("Tiempo máximo de ejecución establecido en %d segundos.\n", time_max);
  }

  // Configurar handler para SIGINT (Ctrl+C)
  signal(SIGINT, handle_sigint);

  printf("Bienvenido a DCCAdmin\n");
  printf("Escriba 'help' para ver los comandos disponibles\n");

  while (1)
  {
    if (time_max != -1)
      check_timemax_processes(time_max);

    monitor_processes();

    printf("dccadmin> ");
    input = read_user_input();

    if (input[0] == NULL)
    {
      free_user_input(input);
      continue;
    }

    if (strcmp(input[0], "help") == 0)
    {
      print_help();
    }
    else if (strcmp(input[0], "start") == 0)
    {
      start_process(input);
    }
    else if (strcmp(input[0], "info") == 0)
    {
      show_info();
    }
    else if (strcmp(input[0], "timeout") == 0)
    {
      if (input[1] == NULL)
      {
        printf("Error: Debe especificar un tiempo en segundos\n");
      }
      else
      {
        timeout_processes(atoi(input[1]));
      }
    }
    else if (strcmp(input[0], "quit") == 0)
    {
      free_user_input(input);
      quit_program();
      exit(0);
    }
    else
    {
      printf("Comando no reconocido. Use 'help' para ver los comandos disponibles\n");
    }

    free_user_input(input);
  }

  return 0;
}
