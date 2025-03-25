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
} ProcessInfo;

ProcessInfo processes[MAX_PROCESSES];
int process_count = 0;
int time_max = -1; // -1 significa tiempo ilimitado

void handle_sigint(int sig)
{
	printf("\nRecibido SIGINT. Terminando...\n");
	exit(0);
}

void print_help()
{
	printf("Comandos disponibles:\n");
	printf("start <executable> <arg1> <arg2> ... <argn>: Ejecuta un programa en un nuevo proceso\n");
	printf("info: Muestra información de los procesos en ejecución\n");
	printf("timeout <time>: Termina procesos que excedan el tiempo especificado\n");
	printf("quit: Termina el programa\n");
	printf("help: Muestra esta ayuda\n");
}

void update_process_status()
{
	for (int i = 0; i < process_count; i++)
	{
		if (processes[i].running)
		{
			int status;
			pid_t result = waitpid(processes[i].pid, &status, WNOHANG);

			if (result > 0)
			{
				processes[i].running = 0;
				if (WIFEXITED(status))
				{
					processes[i].exit_code = WEXITSTATUS(status);
					processes[i].signal_value = -1;
				}
				else if (WIFSIGNALED(status))
				{
					processes[i].exit_code = -1;
					processes[i].signal_value = WTERMSIG(status);
				}
			}
		}
	}
}

void print_process_info(ProcessInfo *p)
{
	time_t current_time = time(NULL);
	int execution_time = (int)difftime(current_time, p->start_time);

	printf("%-6d %-20s %-6d %-6d %-6d\n",
				 p->pid,
				 p->name,
				 execution_time,
				 p->running ? -1 : p->exit_code,
				 p->running ? -1 : p->signal_value);
}

void start_process(char **args)
{
	if (args[1] == NULL)
	{
		printf("Error: Debe especificar un ejecutable\n");
		return;
	}

	if (process_count >= MAX_PROCESSES)
	{
		printf("Error: Número máximo de procesos alcanzado (10)\n");
		return;
	}

	pid_t pid = fork();

	if (pid < 0)
	{
		perror("Error al crear proceso");
		return;
	}
	else if (pid == 0)
	{
		// Proceso hijo
		execvp(args[1], &args[1]);
		// Si llegamos aquí, hubo un error
		perror("Error al ejecutar el programa");
		exit(EXIT_FAILURE);
	}
	else
	{
		// Proceso padre
		processes[process_count].pid = pid;
		strncpy(processes[process_count].name, args[1], 255);
		processes[process_count].start_time = time(NULL);
		processes[process_count].exit_code = -1;
		processes[process_count].signal_value = -1;
		processes[process_count].running = 1;

		printf("Proceso iniciado: PID=%d, Programa=%s\n", pid, args[1]);
		process_count++;
	}
}

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

void timeout_processes(int timeout_seconds)
{
	update_process_status();

	int running_count = 0;
	for (int i = 0; i < process_count; i++)
	{
		if (processes[i].running)
		{
			running_count++;
		}
	}

	if (running_count == 0)
	{
		printf("No hay procesos en ejecución. Timeout no se puede ejecutar.\n");
		return;
	}

	printf("Esperando %d segundos antes de terminar procesos...\n", timeout_seconds);
	sleep(timeout_seconds);

	printf("Timeout cumplido!\n");
	printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");

	// Actualizamos estados antes de continuar
	update_process_status();

	for (int i = 0; i < process_count; i++)
	{
		if (processes[i].running)
		{
			time_t current_time = time(NULL);
			int execution_time = (int)difftime(current_time, processes[i].start_time);

			printf("%-6d %-20s %-6d %-6d %-6d\n",
						 processes[i].pid,
						 processes[i].name,
						 execution_time,
						 -1, -1);

			// Enviar SIGTERM
			kill(processes[i].pid, SIGTERM);

			// Esperar 5 segundos
			sleep(5);

			// Verificar si el proceso terminó
			int status;
			if (waitpid(processes[i].pid, &status, WNOHANG) == 0)
			{
				// No terminó, enviar SIGKILL
				kill(processes[i].pid, SIGKILL);
				waitpid(processes[i].pid, &status, 0); // Esperar a que termine
			}

			processes[i].running = 0;
			if (WIFEXITED(status))
			{
				processes[i].exit_code = WEXITSTATUS(status);
				processes[i].signal_value = -1;
			}
			else if (WIFSIGNALED(status))
			{
				processes[i].exit_code = -1;
				processes[i].signal_value = WTERMSIG(status);
			}
		}
	}
}

void check_timeout_processes()
{
	if (time_max <= 0)
		return; // No hay tiempo máximo

	update_process_status();

	time_t current_time = time(NULL);
	for (int i = 0; i < process_count; i++)
	{
		if (processes[i].running)
		{
			int execution_time = (int)difftime(current_time, processes[i].start_time);

			if (execution_time >= time_max)
			{
				printf("Proceso %d (%s) ha excedido el tiempo máximo de ejecución (%d segundos).\n",
							 processes[i].pid, processes[i].name, time_max);

				// Enviar SIGTERM
				kill(processes[i].pid, SIGTERM);

				// Esperar 5 segundos
				sleep(5);

				// Verificar si el proceso terminó
				int status;
				if (waitpid(processes[i].pid, &status, WNOHANG) == 0)
				{
					// No terminó, enviar SIGKILL
					printf("Proceso %d no terminó después de SIGTERM. Enviando SIGKILL.\n", processes[i].pid);
					kill(processes[i].pid, SIGKILL);
					waitpid(processes[i].pid, &status, 0); // Esperar a que termine
				}

				processes[i].running = 0;
				if (WIFEXITED(status))
				{
					processes[i].exit_code = WEXITSTATUS(status);
					processes[i].signal_value = -1;
				}
				else if (WIFSIGNALED(status))
				{
					processes[i].exit_code = -1;
					processes[i].signal_value = WTERMSIG(status);
				}
			}
		}
	}
}

void quit_program()
{
	update_process_status();

	int running_count = 0;
	for (int i = 0; i < process_count; i++)
	{
		if (processes[i].running)
		{
			running_count++;
			kill(processes[i].pid, SIGINT);
		}
	}

	if (running_count > 0)
	{
		printf("Esperando 10 segundos a que los procesos terminen...\n");
		sleep(10);

		for (int i = 0; i < process_count; i++)
		{
			if (processes[i].running)
			{
				int status;
				if (waitpid(processes[i].pid, &status, WNOHANG) == 0)
				{
					printf("Enviando SIGKILL a proceso %d\n", processes[i].pid);
					kill(processes[i].pid, SIGKILL);
					waitpid(processes[i].pid, &status, 0);
				}

				processes[i].running = 0;
				if (WIFEXITED(status))
				{
					processes[i].exit_code = WEXITSTATUS(status);
					processes[i].signal_value = -1;
				}
				else if (WIFSIGNALED(status))
				{
					processes[i].exit_code = -1;
					processes[i].signal_value = WTERMSIG(status);
				}
			}
		}
	}

	printf("DCCAdmin finalizado\n");
	printf("%-6s %-20s %-6s %-6s %-6s\n", "PID", "NOMBRE", "TIEMPO", "EXIT", "SIGNAL");
	for (int i = 0; i < process_count; i++)
	{
		print_process_info(&processes[i]);
	}

	exit(0);
}

int main(int argc, char const *argv[])
{
	// Configurar handler para SIGINT (Ctrl+C)
	signal(SIGINT, handle_sigint);

	// Procesar argumentos de línea de comandos
	if (argc > 1)
	{
		time_max = atoi(argv[1]);
		printf("Tiempo máximo de ejecución establecido en %d segundos.\n", time_max);
	}

	printf("Bienvenido a DCCAdmin\n");
	printf("Escriba 'help' para ver los comandos disponibles\n");

	while (1)
	{
		// Comprobar si algún proceso ha excedido el tiempo máximo
		check_timeout_processes();

		printf("dccadmin> ");
		char **input = read_user_input();

		if (input[0] == NULL)
		{
			free_user_input(input);
			continue;
		}

		if (strcmp(input[0], "help") == 0)
		{
			print_help();
		}
		else if (strcmp(input[0], "start") == 0)
		{
			start_process(input);
		}
		else if (strcmp(input[0], "info") == 0)
		{
			show_info();
		}
		else if (strcmp(input[0], "timeout") == 0)
		{
			if (input[1] == NULL)
			{
				printf("Error: Debe especificar un tiempo en segundos\n");
			}
			else
			{
				timeout_processes(atoi(input[1]));
			}
		}
		else if (strcmp(input[0], "quit") == 0)
		{
			free_user_input(input);
			quit_program();
			break; // Por si quit_program() falla
		}
		else
		{
			printf("Comando no reconocido. Use 'help' para ver los comandos disponibles\n");
		}

		free_user_input(input);
	}

	return 0;
}
