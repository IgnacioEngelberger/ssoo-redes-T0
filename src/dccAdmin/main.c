#include "main.h"
#include "commands/start.h"
#include "commands/info.h"
#include "commands/timeout.h"
#include "commands/quit.h"
#include "commands/help.h"
#include "process_monitor.h"
#include "timemax_monitor.h"

// Definiciones de variables globales
struct shared_memory *shared = NULL;
int time_max = -1;             // -1 significa tiempo ilimitado
pid_t timemax_monitor_pid = 0; // PID del proceso que monitorea el time_max
pid_t process_monitor_pid = 0; // PID del proceso que monitorea los procesos
char **input = NULL;

// Implementación de funciones generales
void handle_sigint(int sig)
{
  if (shared != NULL)
    munmap(shared, sizeof(struct shared_memory));
  printf("\nRecibido SIGINT (%d). Terminando...\n", sig);
  if (input != NULL)
    free_user_input(input);
  stop_timemax_monitor(timemax_monitor_pid);
  stop_process_monitor(process_monitor_pid);
  quit_program(shared);
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
  // Initialize shared memory
  shared = mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared == MAP_FAILED)
  {
    perror("Error creating shared memory");
    exit(1);
  }
  shared->process_count = 0;

  // Procesar argumentos de línea de comandos
  if (argc > 1)
  {
    time_max = atoi(argv[1]);
    printf("Tiempo máximo de ejecución establecido en %d segundos.\n", time_max);
    timemax_monitor_pid = start_timemax_monitor(time_max, shared);
  }

  // Iniciar el thread de monitoreo de procesos
  process_monitor_pid = start_process_monitor(shared);

  // Configurar handler para SIGINT (Ctrl+C)
  signal(SIGINT, handle_sigint);

  printf("Bienvenido a DCCAdmin\n");
  printf("Escriba 'help' para ver los comandos disponibles\n");

  while (1)
  {
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
      start_process(input, shared);
    }
    else if (strcmp(input[0], "info") == 0)
    {
      show_info(shared);
    }
    else if (strcmp(input[0], "timeout") == 0)
    {
      if (input[1] == NULL)
      {
        printf("Error: Debe especificar un tiempo en segundos\n");
      }
      else
      {
        timeout_processes(atoi(input[1]), shared);
      }
    }
    else if (strcmp(input[0], "quit") == 0)
    {
      free_user_input(input);
      if (shared != NULL)
        munmap(shared, sizeof(struct shared_memory));
      stop_timemax_monitor(timemax_monitor_pid);
      stop_process_monitor(process_monitor_pid);
      quit_program(shared);
      break; // Por si quit_program() falla
    }
    else
    {
      printf("Comando no reconocido. Use 'help' para ver los comandos disponibles\n");
    }

    if (input != NULL)
      free_user_input(input);
  }

  return 0;
}
