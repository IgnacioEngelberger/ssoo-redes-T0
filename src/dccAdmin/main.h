#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "../input_manager/manager.h"

#define MAX_PROCESSES 10

typedef struct
{
  pid_t pid;
  char name[256];
  time_t start_time;
  int exit_code;
  int signal_value;
  int running;
  pid_t timeout_id;    // PID del proceso timeout que debe afectar a este proceso (0 si no hay)
  time_t sigterm_time; // Tiempo en que se envió SIGTERM (0 si no se ha enviado)
} ProcessInfo;

// Variables globales declaradas en main.c, referenciadas en otros archivos
extern ProcessInfo processes[MAX_PROCESSES];
extern int process_count;
extern int time_max; // -1 significa tiempo ilimitado

// Declaraciones de funciones generales
void handle_sigint(int sig);
void update_process_status();
void print_process_info(ProcessInfo *p);
void check_timemax_processes(); // Función para verificar los procesos que exceden time_max

#endif // MAIN_H
