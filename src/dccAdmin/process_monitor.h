#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "main.h"

// Shared memory structure
struct shared_memory
{
  int process_count;
  ProcessInfo processes[MAX_PROCESSES];
};

// Inicializa y arranca el thread de monitoreo
pid_t start_process_monitor(struct shared_memory *shared);

// Detiene el thread de monitoreo
void stop_process_monitor(pid_t monitor_pid);

#endif // PROCESS_MONITOR_H
