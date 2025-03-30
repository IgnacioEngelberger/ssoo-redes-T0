#include "help.h"

void print_help()
{
  printf("Comandos disponibles:\n");
  printf("start <executable> <arg1> <arg2> ... <argn>: Ejecuta un programa en un nuevo proceso\n");
  printf("info: Muestra información de los procesos ejecutados\n");
  printf("timeout <time>: Termina procesos que excedan el tiempo especificado\n");
  printf("quit: Termina el programa\n");
  printf("help: Muestra esta ayuda\n");
  printf("\n");
}
