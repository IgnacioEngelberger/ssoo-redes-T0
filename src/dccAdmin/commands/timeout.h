#ifndef CMD_TIMEOUT_H
#define CMD_TIMEOUT_H

#include "../main.h"
#include "../process_monitor.h"

// Función para iniciar un timeout para los procesos actuales
void timeout_processes(int timeout_seconds, struct shared_memory *shared);

#endif // CMD_TIMEOUT_H
