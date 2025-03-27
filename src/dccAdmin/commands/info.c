#include "info.h"

void show_info()
{
  update_process_status();

  printf("Procesos en ejecución:\n");
  printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");

  int running_count = 0;
  for (int i = 0; i < process_count; i++)
  {
    print_process_info(&processes[i]);
    if (processes[i].running)
      running_count++;
  }

  if (running_count == 0)
  {
    printf("No hay procesos en ejecución.\n");
  }
}
