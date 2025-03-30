#include "info.h"

void show_info()
{
  printf("Procesos en ejecución:\n");
  printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");

  for (int i = 0; i < process_count; i++)
    print_process_info(&processes[i]);

  if (process_count == 0)
    printf("No se han ejecutado procesos.\n");

  printf("\n");
}
