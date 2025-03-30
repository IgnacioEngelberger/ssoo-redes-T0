#ifndef TIMEMAX_MONITOR_H
#define TIMEMAX_MONITOR_H

#include <stdbool.h>
#include "process_monitor.h"

// Inicializa y arranca el proceso de monitoreo de time_max
pid_t start_timemax_monitor(int time_max, struct shared_memory *shared);

// Detiene el proceso de monitore de time_max
void stop_timemax_monitor(pid_t monitor_pid);

#endif // TIMEMAX_MONITOR_H
