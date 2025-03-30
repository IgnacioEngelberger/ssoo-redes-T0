#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "main.h"

void monitor_processes();

#endif // PROCESS_MONITOR_H
