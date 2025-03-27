#include "main.h"
#include "commands/start.h"
#include "commands/info.h"
#include "commands/timeout.h"
#include "commands/quit.h"
#include "commands/help.h"

// Definiciones de variables globales
ProcessInfo processes[MAX_PROCESSES];
int process_count = 0;
int time_max = -1;             // -1 significa tiempo ilimitado
pid_t timemax_monitor_pid = 0; // PID del proceso que monitorea el time_max

// Implementación de funciones generales
void handle_sigint(int sig)
{
  printf("\nRecibido SIGINT (%d). Terminando...\n", sig);
  if (timemax_monitor_pid > 0)
  {
    kill(timemax_monitor_pid, SIGTERM);
  }
  exit(0);
}

void update_process_status()
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
        {
          processes[i].exit_code = WEXITSTATUS(status);
          processes[i].signal_value = -1;
        }
        else if (WIFSIGNALED(status))
        {
          processes[i].exit_code = -1;
          processes[i].signal_value = WTERMSIG(status);
        }
      }
    }
  }
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

// Función para verificar si los procesos han excedido el tiempo máximo global
void check_timemax_processes()
{
  if (time_max <= 0)
    return; // No hay tiempo máximo

  while (1)
  {
    update_process_status();

    time_t current_time = time(NULL);
    for (int i = 0; i < process_count; i++)
    {
      if (processes[i].running)
      {
        int execution_time = (int)difftime(current_time, processes[i].start_time);

        // Si el proceso ha excedido el tiempo máximo y aún no ha recibido SIGTERM
        if (execution_time >= time_max && processes[i].sigterm_time == 0)
        {
          printf("TimeMax: Proceso %d (%s) ha excedido el tiempo máximo de ejecución (%d segundos).\n",
                 processes[i].pid, processes[i].name, time_max);

          // Enviar SIGTERM
          kill(processes[i].pid, SIGTERM);

          // Registrar el tiempo en que se envió SIGTERM
          processes[i].sigterm_time = current_time;
        }
        // Si ya se envió SIGTERM, verificar si han pasado 5 segundos para enviar SIGKILL
        else if (processes[i].sigterm_time > 0)
        {
          int time_since_sigterm = (int)difftime(current_time, processes[i].sigterm_time);

          if (time_since_sigterm >= 5)
          {
            // Verificar si el proceso aún está en ejecución
            int status;
            if (waitpid(processes[i].pid, &status, WNOHANG) == 0)
            {
              // No terminó después de SIGTERM, enviamos SIGKILL
              printf("TimeMax: Proceso %d (%s) no terminó después de SIGTERM. Enviando SIGKILL.\n",
                     processes[i].pid, processes[i].name);
              kill(processes[i].pid, SIGKILL);

              // Esperamos a que termine, pero sin bloquear todo el proceso
              waitpid(processes[i].pid, &status, WNOHANG);
            }
          }
        }
      }
    }
  }
}

int main(int argc, char const *argv[])
{
  // Configurar handler para SIGINT (Ctrl+C)
  signal(SIGINT, handle_sigint);

  // Procesar argumentos de línea de comandos
  if (argc > 1)
  {
    time_max = atoi(argv[1]);
    printf("Tiempo máximo de ejecución establecido en %d segundos.\n", time_max);
  }

  // Iniciar el proceso de monitoreo de time_max en un proceso separado
  if (time_max > 0)
  {
    timemax_monitor_pid = fork();
    if (timemax_monitor_pid < 0)
    {
      perror("Error al crear proceso de monitoreo de tiempo máximo");
      exit(1);
    }
    else if (timemax_monitor_pid == 0)
    {
      // Proceso hijo
      check_timemax_processes();
      exit(0); // Nunca debería llegar aquí
    }
    else
    {
      printf("Monitoreo de tiempo máximo iniciado en proceso %d\n", timemax_monitor_pid);
    }
  }

  printf("Bienvenido a DCCAdmin\n");
  printf("Escriba 'help' para ver los comandos disponibles\n");

  while (1)
  {
    printf("dccadmin> ");
    char **input = read_user_input();

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
      // Terminar el proceso de monitoreo de time_max
      if (timemax_monitor_pid > 0)
      {
        kill(timemax_monitor_pid, SIGTERM);
        waitpid(timemax_monitor_pid, NULL, 0);
      }
      quit_program();
      break; // Por si quit_program() falla
    }
    else
    {
      printf("Comando no reconocido. Use 'help' para ver los comandos disponibles\n");
    }

    free_user_input(input);
  }

  return 0;
}
