#ifndef CMD_START_H
#define CMD_START_H

#include "../main.h"
#include "../process_monitor.h"

// Función para iniciar un nuevo proceso
void start_process(char **args, struct shared_memory *shared);

#endif // CMD_START_H
