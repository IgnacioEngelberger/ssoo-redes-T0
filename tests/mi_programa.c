#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("Hola, este es un programa ejecutable en C.\n");

    // Verificar si hay argumentos
    if (argc < 2) {
        printf("Error: No se proporcionaron argumentos.\n");
        return 2;  // Código de error 2 (falta de argumentos)
    }

    printf("Argumentos recibidos:\n");
    for (int i = 1; i < argc; i++) {
        printf("  - %s\n", argv[i]);
    }

    // Simular un proceso que tarda 5 segundos
    printf("Ejecutando tarea...\n");
    sleep(5);

    // Si el primer argumento es "error", simula un fallo
    if (strcmp(argv[1], "error") == 0) {
        printf("Se ha producido un error intencional.\n");
        return 1;  // Código de error 1 (fallo simulado)
    }

    printf("Proceso finalizado exitosamente.\n");
    return 0;  // Código de éxito
}
