#!/bin/bash

echo "Hola, este es un script de prueba."

# Verifica si se proporcionó un argumento
if [ "$#" -eq 0 ]; then
    echo "Error: No se proporcionaron argumentos."
    exit 2  # Código de error 2 (error de argumentos)
fi

# Muestra los argumentos recibidos
echo "Argumentos recibidos: $@"

# Simula un proceso que tarda 3 segundos
echo "Procesando..."
sleep 15

# Si el primer argumento es "error", simulamos un fallo
if [ "$1" == "error" ]; then
    echo "Se ha producido un error intencional."
    exit 1  # Código de error 1 (fallo genérico)
fi

# Si todo sale bien
echo "Proceso completado exitosamente."
exit 0  # Código de éxito
return 0