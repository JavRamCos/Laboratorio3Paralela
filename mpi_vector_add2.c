/* File:     mpi_vector_add.c
 *
 * Purpose:  Implement parallel vector addition using a block
 *           distribution of the vectors.  This version also
 *           illustrates the use of MPI_Scatter and MPI_Gather.
 *
 * Compile:  mpicc -g -Wall -o mpi_vector_add mpi_vector_add.c
 * Run:      mpiexec -n <comm_sz> ./vector_add
 *
 * Input:    The order of the vectors, n, and the vectors x and y
 * Output:   The sum vector z = x+y
 *
 * Notes:
 * 1.  The order of the vectors, n, should be evenly divisible
 *     by comm_sz
 * 2.  DEBUG compile flag.
 * 3.  This program does fairly extensive error checking.  When
 *     an error is detected, a message is printed and the processes
 *     quit.  Errors detected are incorrect values of the vector
 *     order (negative or not evenly divisible by comm_sz), and
 *     malloc failures.
 *
 * IPP:  Section 3.4.6 (pp. 109 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void Check_for_error(int local_ok, char fname[], char message[],
      MPI_Comm comm);
void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz,
      MPI_Comm comm);
void Read_RandMax(int* randmax, int my_rank, int comm_sz, 
      MPI_Comm comm);
void Allocate_vectors(double** local_x_pp, double** local_y_pp,
      double** local_z_pp, int local_n, MPI_Comm comm);
void Generate_vector(double local_a[], int local_n, int n, char vec_name[],
      int my_rank, MPI_Comm comm,int randmax);
void PrintTopDown_vector(double local_b[], int local_n, int n, char title[],
      int my_rank, MPI_Comm comm);
void Read_Scalar(int* scalar, int my_rank, int comm_sz, 
      MPI_Comm comm);
void Parallel_vector_scalar(int scalar, double local_arr[], int local_n, 
      int my_rank);
void Parallel_vector_dot(double local_x[], double local_y[],
      int local_n, int my_rank, double* result, MPI_Comm comm);
void Display_dot_result(int my_rank, double result);


/*-------------------------------------------------------------------*/
int main(void) {
   srand(time(NULL));
   int n, local_n, randmax;
   int comm_sz, my_rank;
   double *local_x, *local_y, *local_z;
   MPI_Comm comm;
   double tstart, tend;

   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   Read_n(&n, &local_n, my_rank, comm_sz, comm);
   Read_RandMax(&randmax, my_rank, comm_sz, comm);
   // n = 10000000;
   tstart = MPI_Wtime();
   Allocate_vectors(&local_x, &local_y, &local_z, local_n, comm);

   Generate_vector(local_x, local_n, n, "x", my_rank, comm, randmax);
   PrintTopDown_vector(local_x, local_n, n, "Vector x", my_rank, comm);
   Generate_vector(local_y, local_n, n, "y", my_rank, comm, randmax);
   PrintTopDown_vector(local_y, local_n, n, "Vector y", my_rank, comm);

   // Scalar Multiplication
   int scalar;
   Read_Scalar(&scalar, my_rank, comm_sz, comm);
   Parallel_vector_scalar(scalar, local_x, local_n, my_rank);
   PrintTopDown_vector(local_x, local_n, n, "Vector x by scalar", my_rank, comm);
   Parallel_vector_scalar(scalar, local_y, local_n, my_rank);
   PrintTopDown_vector(local_y, local_n, n, "Vector y by scalar", my_rank, comm);

   // dot product
   double result; // Cambiar int result a double result
   Parallel_vector_dot(local_x,local_y,local_n,my_rank,&result,comm);
   Display_dot_result(my_rank,result);

   tend = MPI_Wtime();
   if(my_rank==0)
    printf("\nTook %.3lf s to run\n",(tend-tstart));

   free(local_x);
   free(local_y);
   free(local_z);

   MPI_Finalize();

   return 0;
}  /* main */

/*-------------------------------------------------------------------
 * Function:  Check_for_error
 * Purpose:   Check whether any process has found an error.  If so,
 *            print message and terminate all processes.  Otherwise,
 *            continue execution.
 * In args:   local_ok:  1 if calling process has found an error, 0
 *               otherwise
 *            fname:     name of function calling Check_for_error
 *            message:   message to print if there's an error
 *            comm:      communicator containing processes calling
 *                       Check_for_error:  should be MPI_COMM_WORLD.
 *
 * Note:
 *    The communicator containing the processes calling Check_for_error
 *    should be MPI_COMM_WORLD.
 */
void Check_for_error(
      int       local_ok   /* in */,
      char      fname[]    /* in */,
      char      message[]  /* in */,
      MPI_Comm  comm       /* in */) {
   int ok;

   MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
   if (ok == 0) {
      int my_rank;
      MPI_Comm_rank(comm, &my_rank);
      if (my_rank == 0) {
         fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname,
               message);
         fflush(stderr);
      }
      MPI_Finalize();
      exit(-1);
   }
}  /* Check_for_error */


/*-------------------------------------------------------------------
 * Function:  Read_n
 * Purpose:   Get the order of the vectors from stdin on proc 0 and
 *            broadcast to other processes.
 * In args:   my_rank:    process rank in communicator
 *            comm_sz:    number of processes in communicator
 *            comm:       communicator containing all the processes
 *                        calling Read_n
 * Out args:  n_p:        global value of n
 *            local_n_p:  local value of n = n/comm_sz
 *
 * Errors:    n should be positive and evenly divisible by comm_sz
 */
void Read_n(
      int*      n_p        /* out */,
      int*      local_n_p  /* out */,
      int       my_rank    /* in  */,
      int       comm_sz    /* in  */,
      MPI_Comm  comm       /* in  */) {
   int local_ok = 1;
   char *fname = "Read_n";

   if (my_rank == 0) {
      printf("What's the order of the vectors?\n");
      scanf("%d", n_p);
   }
   MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
   if (*n_p < 0 || *n_p % comm_sz != 0) local_ok = 0;
   Check_for_error(local_ok, fname,
         "n should be >= 100,000 and evenly divisible by comm_sz", comm);
   *local_n_p = *n_p/comm_sz;
}  /* Read_n */

/*-------------------------------------------------------------------
 * Function:  Read_RandMax
 * Purpose:   Read limit for random numbers.
 * In args:   my_rank:    process rank in communicator
 *            comm_sz:    number of processes in communicator
 *            comm:       communicator containing all the processes
 *                        calling Read_n
 * Out args:  randmax:        global value of randmax
 *
 * Errors:    randmax is < 0
 */
void Read_RandMax(
      int*      randmax        /* out */,
      int       my_rank    /* in  */,
      int       comm_sz    /* in  */,
      MPI_Comm  comm       /* in  */) {
   int local_ok = 1;
   char *fname = "Read_RandMax";

   if (my_rank == 0) {
      printf("What's the max number for random?\n");
      scanf("%d", randmax);
   }
   MPI_Bcast(randmax, 1, MPI_INT, 0, comm);
   if (*randmax < 0) local_ok = 0;
   Check_for_error(local_ok, fname,
         "randmax should be >= 100,000", comm);
}  /* Read_RandMax */


/*-------------------------------------------------------------------
 * Function:  Allocate_vectors
 * Purpose:   Allocate storage for x, y, and z
 * In args:   local_n:  the size of the local vectors
 *            comm:     the communicator containing the calling processes
 * Out args:  local_x_pp, local_y_pp, local_z_pp:  pointers to memory
 *               blocks to be allocated for local vectors
 *
 * Errors:    One or more of the calls to malloc fails
 */
void Allocate_vectors(
      double**   local_x_pp  /* out */,
      double**   local_y_pp  /* out */,
      double**   local_z_pp  /* out */,
      int        local_n     /* in  */,
      MPI_Comm   comm        /* in  */) {
   int local_ok = 1;
   char* fname = "Allocate_vectors";

   *local_x_pp = malloc(local_n*sizeof(double));
   *local_y_pp = malloc(local_n*sizeof(double));
   *local_z_pp = malloc(local_n*sizeof(double));

   if (*local_x_pp == NULL || *local_y_pp == NULL ||
       *local_z_pp == NULL) local_ok = 0;
   Check_for_error(local_ok, fname, "Can't allocate local vector(s)",
         comm);
}  /* Allocate_vectors */


/*-------------------------------------------------------------------
 * Function:   Generate_vector
 * Purpose:    Read a vector from stdin on process 0 and distribute
 *             among the processes using a block distribution.
 * In args:    local_n:  size of local vectors
 *             n:        size of global vector
 *             vec_name: name of vector being read (e.g., "x")
 *             my_rank:  calling process' rank in comm
 *             comm:     communicator containing calling processes
 *             randmax: global variable for random limit
 * Out arg:    local_a:  local vector read
 *
 * Errors:     if the malloc on process 0 for temporary storage
 *             fails the program terminates
 *
 * Note:
 *    This function assumes a block distribution and the order
 *   of the vector evenly divisible by comm_sz.
 */
void Generate_vector(
      double    local_a[]   /* out */,
      int       local_n     /* in  */,
      int       n           /* in  */,
      char      vec_name[]  /* in  */,
      int       my_rank     /* in  */,
      MPI_Comm  comm        /* in  */,
      int       randmax         /* in  */) {

   double* a = NULL;
   int i;
   int local_ok = 1;
   char* fname = "Generate_vector";

   if (my_rank == 0) {
      a = malloc(n*sizeof(double));
      if (a == NULL) local_ok = 0;
      Check_for_error(local_ok, fname, "Can't allocate temporary vector",
            comm);
      //printf("Enter the vector %s\n", vec_name);
      //fill vec with indez
      for (i = 0; i < n; i++)
         a[i] = rand() % randmax;
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_a, local_n, MPI_DOUBLE, 0,
         comm);
      free(a);
   } else {
      Check_for_error(local_ok, fname, "Can't allocate temporary vector",
            comm);
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_a, local_n, MPI_DOUBLE, 0,
         comm);
   }
}  /* Generate_vector */


/*-------------------------------------------------------------------
 * Function:  PrintTopDown_vector
 * Purpose:   Print a vector that has a block distribution to stdout
 * In args:   local_b:  local storage for vector to be printed
 *            local_n:  order of local vectors
 *            n:        order of global vector (local_n*comm_sz)
 *            title:    title to precede print out
 *            comm:     communicator containing processes calling
 *                      PrintTopDown_vector
 *
 * Error:     if process 0 can't allocate temporary storage for
 *            the full vector, the program terminates.
 *
 * Note:
 *    Assumes order of vector is evenly divisible by the number of
 *    processes
 */
void PrintTopDown_vector(
      double    local_b[]  /* in */,
      int       local_n    /* in */,
      int       n          /* in */,
      char      title[]    /* in */,
      int       my_rank    /* in */,
      MPI_Comm  comm       /* in */) {

   double* b = NULL;
   int i;
   int local_ok = 1;
   char* fname = "PrintTopDown_vector";

   if (my_rank == 0) {
      b = malloc(n*sizeof(double));
      if (b == NULL) local_ok = 0;
      Check_for_error(local_ok, fname, "Can't allocate temporary vector",
            comm);
      MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE,
            0, comm);
      printf("%s\n", title);
      printf("0 - 10: [");
      for (i = 0; i < 9; i++)
         printf("%lf,", b[i]);
      printf("%lf]\n", b[9]);
      printf("%d - %d: [",n-10,n);
      for (i = n-10; i < n-1; i++)
         printf("%lf,", b[i]);
      printf("%lf]\n", b[n-1]);
      free(b);
   } else {
      Check_for_error(local_ok, fname, "Can't allocate temporary vector",
            comm);
      MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE, 0,
         comm);
   }
}  /* PrintTopDown_vector */

/*-------------------------------------------------------------------
 * Function:  Read_Scalar
 * Purpose:   Read scalar to multiply vectors with.
 * In args:   my_rank:    process rank in communicator
 *            comm_sz:    number of processes in communicator
 *            comm:       communicator containing all the processes
 *                        calling Read_n
 * Out args:  scalar:        global value of scalar
 *
 * Errors:    None
 */
void Read_Scalar(
      int*      scalar        /* out */,
      int       my_rank    /* in  */,
      int       comm_sz    /* in  */,
      MPI_Comm  comm       /* in  */) {
   int local_ok = 1;
   char *fname = "Read_Scalar";

   if (my_rank == 0) {
      printf("\nWhat's the number for the scalar?\n");
      scanf("%d", scalar);
   }
   MPI_Bcast(scalar, 1, MPI_INT, 0, comm);
}  /* Read_Scalar */

/*-------------------------------------------------------------------
 * Function:  Parallel_vector_scalar
 * Purpose:   Multiply a vector by a scalar that's been distributed among the processes
 * In args:   my_rank:  calling process' rank in comm
 *            local_n:  the number of components in local_x, local_y,
 *                      and local_z
 *            scalar: Scalar number to multiply vectors with
 * Out arg:   local_arr:  local storage of the vector multiplied by the scalar
 */
void Parallel_vector_scalar(
      int     scalar,
      double  local_arr[]  /* out */,
      int     local_n    /* in  */,
      int     my_rank) {
   int local_i;

   for (local_i = 0; local_i < local_n; local_i++)
      local_arr[local_i] = local_arr[local_i] * scalar;
      //local_arr[i] = local_arr[i]*scalar;
}  /* Parallel_vector_scalar */

/*-------------------------------------------------------------------
 * Function:  Parallel_vector_dot
 * Purpose:   Add a vector that's been distributed among the processes
 * In args:   local_x:  local storage of one of the vectors being added
 *            local_y:  local storage for the second vector being added
 *            local_n:  the number of components in local_x and local_y
 *            my_rank: calling process' rank in comm
 * Out arg:   result:  local storage for the dot product of the two vectors
 */
void Parallel_vector_dot(
      double    local_x[]   /* in  */,
      double    local_y[]   /* in  */,
      int       local_n     /* in  */,
      int       my_rank     /* in  */,
      double*   result      /* out */,
      MPI_Comm  comm        /* in  */) {

   double local_dot = 0.0;
   int i;

   for (i = 0; i < local_n; i++)
      local_dot += local_x[i]*local_y[i];

   
   MPI_Reduce(&local_dot, result, 1, MPI_DOUBLE, MPI_SUM, 0, comm);

}  /* Parallel_vector_dot */

/*-------------------------------------------------------------------
 * Function:  Display_dot_result
 * Purpose:   Add a vector that's been distributed among the processes
 * In args:   local_x:  local storage of one of the vectors being added
 *            local_y:  local storage for the second vector being added
 *            local_n:  the number of components in local_x and local_y
 *            my_rank: calling process' rank in comm
 * Out arg:   result:  local storage for the dot product of the two vectors
 */
void Display_dot_result(
      int     my_rank    /* in  */,
      double result      /* in  */) {
   if(my_rank == 0) {
      printf("\nResult of dot product: %lf\n",result);
   }
}  /* Display_dot_result */
