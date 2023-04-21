# Laboratorio3Paralela

Repositorio para Laboratorio 3 de Computacion Paralela y Distribuida

## Integrantes

- Javier Alejandro Ramirez Cospin, Carne No. 18099
- Jose Javier Hurtarte Hernandez, Carne No. 19707

## Instrucciones

Para compilar vector_add2.c:

```
gcc vector_add2.c -o vector_add2
```

Para ejecutar vector_add2:

```
./vector_add2
```

Para compilar mpi_run_add2.c:

```
mpicc mpi_vector_add2.c -o mpi_vector_add2
```

Para ejecutar mpi_run_add2:

```
mpirun -np <num_proc> mpi_vector_add2
```

donde:
 
 - num_proc: Numero de procesos a instanciar con el programa.
