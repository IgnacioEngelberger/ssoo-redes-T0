#include "timeout.h"

void timeout_processes(int timeout_seconds)
{
  update_process_status();

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
    // PROCESO HIJO
    pid_t pid = getpid();

    // Marcar los procesos actuales como afectados por este timeout
    for (int i = 0; i < process_count; i++)
      if (processes[i].running)
        processes[i].timeout_id = pid;

    // Esperar el tiempo especificado
    sleep(timeout_seconds);

    // Después del timeout, terminar los procesos marcados,
    // solo afectar a los procesos que estaban en ejecución cuando se lanzó este timeout
    for (int i = 0; i < process_count; i++)
      if (processes[i].running && processes[i].timeout_id == pid)
      {
        printf("Timeout: Enviando SIGTERM al proceso %d (%s)\n", processes[i].pid, processes[i].name);
        kill(processes[i].pid, SIGTERM);
      }

    exit(0);
  }
  else
  {
    // PROCESO PADRE
    printf("Timeout iniciado en proceso %d por %d segundos\n", timeout_pid, timeout_seconds);
  }
}
