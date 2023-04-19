/* File:     vector_add.c
 *
 * Purpose:  Implement vector addition
 *
 * Compile:  gcc -g -Wall -o vector_add vector_add.c
 * Run:      ./vector_add
 *
 * Input:    The order of the vectors, n, and the vectors x and y
 * Output:   The sum vector z = x+y
 *
 * Note:
 *    If the program detects an error (order of vector <= 0 or malloc
 * failure), it prints a message and terminates
 *
 * IPP:      Section 3.4.6 (p. 109)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Read_n(int* n_p);
void Read_RandMax(int* randmax);
void Allocate_vectors(double** x_pp, double** y_pp, double** z_pp, int n);
void Generate_vector(double a[], int n, char vec_name[],int randmax);
void Print_vector(double b[], int n, char title[]);
void PrintTopDown_vector(double b[], int n, char title[]);
void Vector_sum(double x[], double y[], double z[], int n);

/*---------------------------------------------------------------------*/
int main(void) {
   clock_t start = clock();
   int n, randmax;
   double *x, *y, *z;

   Read_n(&n);
   Read_RandMax(&randmax);
   Allocate_vectors(&x, &y, &z, n);
   
   Generate_vector(x, n, "x",randmax);
   Generate_vector(y, n, "y",randmax);
   PrintTopDown_vector(x, n, "Vector x");
   PrintTopDown_vector(y, n, "Vector y");

   Vector_sum(x, y, z, n);

   // Print_vector(z, n, "The sum is");

   free(x);
   free(y);
   free(z);

   clock_t difference = clock() - start;
   int msec = difference * 1000 / CLOCKS_PER_SEC;

   printf("\nTook %d.%d s to run\n",msec/1000,msec%1000);

   return 0;
}  /* main */

/*---------------------------------------------------------------------
 * Function:  Read_n
 * Purpose:   Get the order of the vectors from stdin
 * Out arg:   n_p:  the order of the vectors
 *
 * Errors:    If n <= 0, the program terminates
 */
void Read_n(int* n_p /* out */) {
   printf("What's the order of the vectors?\n");
   scanf("%d", n_p);
   if (*n_p <= 0) {
      fprintf(stderr, "Order should be positive\n");
      exit(-1);
   }
}  /* Read_n */

/*---------------------------------------------------------------------
 * Function:  Read_RandMax
 * Purpose:   Get the max number for random
 * Out arg:   randmax:  random max number
 *
 * Errors:    If randmax <= 0, the program terminates
 */
void Read_RandMax(int* randmax /* out */) {
   printf("What's the max number for random?\n");
   scanf("%d", randmax);
   if (randmax <= 0) {
      fprintf(stderr, "Max number should be positive\n");
      exit(-1);
   }
}  /* Read_RandMax */

/*---------------------------------------------------------------------
 * Function:  Allocate_vectors
 * Purpose:   Allocate storage for the vectors
 * In arg:    n:  the order of the vectors
 * Out args:  x_pp, y_pp, z_pp:  pointers to storage for the vectors
 *
 * Errors:    If one of the mallocs fails, the program terminates
 */
void Allocate_vectors(
      double**  x_pp  /* out */, 
      double**  y_pp  /* out */, 
      double**  z_pp  /* out */, 
      int       n     /* in  */) {
   *x_pp = malloc(n*sizeof(double));
   *y_pp = malloc(n*sizeof(double));
   *z_pp = malloc(n*sizeof(double));
   if (*x_pp == NULL || *y_pp == NULL || *z_pp == NULL) {
      fprintf(stderr, "Can't allocate vectors\n");
      exit(-1);
   }
}  /* Allocate_vectors */

/*---------------------------------------------------------------------
 * Function:  Generate_vector
 * Purpose:   Read a vector from stdin
 * In args:   n:  order of the vector
 *            vec_name:  name of vector (e.g., x)
 * Out arg:   a:  the vector to be read in
 */
void Generate_vector(
      double  a[]         /* out */, 
      int     n           /* in  */, 
      char    vec_name[]  /* in  */,
      int     randmax     /* in  */) {
   int i;
   for (i = 0; i < n; i++)
      a[i] = rand() % randmax;
   printf("Vector %s generated ...\n", vec_name);
}  /* Read_vector */  

/*---------------------------------------------------------------------
 * Function:  Print_vector
 * Purpose:   Print the contents of a vector
 * In args:   b:  the vector to be printed
 *            n:  the order of the vector
 *            title:  title for print out
 */
void Print_vector(
      double  b[]     /* in */, 
      int     n       /* in */, 
      char    title[] /* in */) {
   int i;
   printf("%s\n", title);
   for (i = 0; i < n; i++)
      printf("%f ", b[i]);
   printf("\n");
}  /* Print_vector */

/*---------------------------------------------------------------------
 * Function:  PrintTopDown_vector
 * Purpose:   Print the contents of a vector
 * In args:   b:  the vector to be printed
 *            n:  the order of the vector
 *            title:  title for print out
 */
void PrintTopDown_vector(
      double  b[]     /* in */, 
      int     n       /* in */, 
      char    title[] /* in */) {
   int i;
   printf("%s\n", title);
   printf("0 - 10: [");
   for(int i = 0;i < 9;i++) 
      printf("%lf,",b[i]);
   printf("%lf]\n",b[9]);
   printf("%d - %d: [",n-10,n);
   for(int i = n-10;i < n-1;i++) 
      printf("%lf,",b[i]);
   printf("%lf]\n",b[n-1]);
}  /* PrintTopDown_vector */

/*---------------------------------------------------------------------
 * Function:  Vector_sum
 * Purpose:   Add two vectors
 * In args:   x:  the first vector to be added
 *            y:  the second vector to be added
 *            n:  the order of the vectors
 * Out arg:   z:  the sum vector
 */
void Vector_sum(
      double  x[]  /* in  */, 
      double  y[]  /* in  */, 
      double  z[]  /* out */, 
      int     n    /* in  */) {
   int i;

   for (i = 0; i < n; i++)
      z[i] = x[i] + y[i];
}  /* Vector_sum */
