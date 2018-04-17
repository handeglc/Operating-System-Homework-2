/* This program computes the sum of the sequence of successive integer
   numbers (starting with 1) in an array of provided length using
   provided number of threads. We illustrate the use of the mutex,
   condition variable and barrier, although we can get by without the
   latter two in this particular case. In fact, pthread_join can be
   used in place of a barrier. */

#define _GNU_SOURCE  /* This is to unlock barriers in pthread.h, since
		        they are not a part of POSIX */

#include <stdlib.h>  /* This is for malloc */
#include <unistd.h>  /* This is for free */
#include <pthread.h> /* This is for pthreads */
#include <stdio.h>   /* This is for printf, fputs and sscanf */

typedef struct {

/* This structure is passed to the threads and contains all necessary
   information for computations */

  pthread_mutex_t * mutex;      /* Mutex */
  pthread_cond_t * cond;        /* Condition variable */
  pthread_barrier_t * barrier;  /* Barrier */

  unsigned * array;             /* Array with values to sum */

  unsigned number;              /* Number of the current thread */

  unsigned start;               /* Starting array element for
				   summation */

  unsigned count;               /* The number of elements to sum up */

  unsigned sum;                 /* The total sum */

  /* With 4 pointers and 4 integers the structure should be
     dword-aligned on 64-bit machines, in case you want to work with
     an array of such structures */

} my_thread_struct;

void * thread_func(void *);     /* Declaration of the thread function */

int main(int argc, char ** argv){

  /* Counter, array size and number of threads */
  unsigned i, array_size, num_threads;


  /* We expect two command line arguments: the array size and number
     of threads. So, first we need to check the number of command line
     arguments. */
  switch(argc){
  case 1:
    fputs("Provide array size\n",stderr);
    ++argc;
  case 2:
    fputs("Provide number of threads\n",stderr);
    exit(EXIT_FAILURE);
  }

  /* Read array size and number of threads from command line */

  if(!sscanf(argv[1],"%u",&array_size) ||
     !sscanf(argv[2],"%u",&num_threads)){
    fputs("Could not read array size or number of threads\n",stderr);
    exit(EXIT_FAILURE);
  }

  /* Check the validity of received values */

  if(!array_size || !num_threads){
    fputs("Array size or num_threads is zero\n",stderr);
    exit(EXIT_FAILURE);
  }

  /* Check if the number of threads is less than array size */

  if(array_size<num_threads){
    puts("Reducing num_threads to array_size");
    num_threads = array_size;
  }

  /* Allocate the array with thread identifiers. They are not used in
     this particular program, but you should normally do this to
     maintain control over the threads. */

  pthread_t * thread = malloc(sizeof(pthread_t)*num_threads);

  /* Allocating space for the thread structure */

  my_thread_struct * thread_struct = malloc(sizeof(my_thread_struct));

  /* Allocating and initializing mutex, condition variable and barrier */

  thread_struct->mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(thread_struct->mutex,NULL);

  thread_struct->cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(thread_struct->cond,NULL);

  thread_struct->barrier = malloc(sizeof(pthread_barrier_t));
  pthread_barrier_init(thread_struct->barrier,NULL,num_threads+1);


  /* Allocating array and initializing sum */

  thread_struct->array = malloc(sizeof(unsigned)*array_size);
  thread_struct->sum = 0;

  /* Filling arrays with numbers to sum */
    
  for(i=0;i<array_size;++i)
    thread_struct->array[i]=i+1;

  /* Preparing to spawn threads. Mutex + condition variable are used
     to ensure that the thread is spawned and initialized before
     altering the thread structure variables for the next thread */

  /* Lock the mutex to use pthread_cond_wait */
  pthread_mutex_lock(thread_struct->mutex);

  /* Compute the number of elements to sum for each thread and
     initialize the number to 0 */
  thread_struct->count = array_size/num_threads;
  thread_struct->number = 0;

  /* Spawn threads */
  for(i=0;i<num_threads;++i){

    /* Here we first spawn a thread, and then adjust the contents of
       the thread structure thanks to Dimitris' idea of reading the
       structure while mutex is locked (see thread code below). Here
       the command order should be viewed as a silly attempt to hide
       the latency of thread creation, i.e. pthread_create returns
       before the thread is created, and while it is being created we
       adjust thread structure values. Here such thing makes little
       sense, but there could be a situation when hiding latency is
       important. */

    pthread_create(thread+i,NULL,thread_func,thread_struct);

    /* Setting the thread number, start and count */
    ++thread_struct->number;
    thread_struct->start = i*thread_struct->count;

    /* Here we deal with the situation when the array size is not a
       multiple of number of threads */
    if(i==(num_threads-1) && array_size%num_threads)
      thread_struct->count = array_size-thread_struct->count*i;

    /* Here we wait for the signal from the thread that it is spawned
       and initialized */
    pthread_cond_wait(thread_struct->cond,thread_struct->mutex);
  }
  
  /* Unlock the mutex, it will be used by the threads to add their
     partial sums to the sum */
  pthread_mutex_unlock(thread_struct->mutex);

  /* Wait until the threads are done. One can use pthread_join here,
     but I want to illustrate the use of barrier. */
  pthread_barrier_wait(thread_struct->barrier);

  /* Print the result */
  printf("The sum of numbers from 1 to %u is %u\n",
	 array_size,thread_struct->sum);

  /* Free the memory */
  free(thread_struct->array);

  pthread_barrier_destroy(thread_struct->barrier);
  free(thread_struct->barrier);

  pthread_cond_destroy(thread_struct->cond);
  free(thread_struct->cond);

  pthread_mutex_destroy(thread_struct->mutex);
  free(thread_struct->mutex);

  free(thread_struct);

  free(thread);

  /* Done */
  return 0;

}

/* This is the definition of the thread function. It takes the pointer
   to the thread structure as an argument */

void * thread_func(void * ts){

  /* Counter and partial sum */
  unsigned i, partial_sum = 0;

  /* Here we create a pointer of proper type for future reference and
     assign the supplied address */
  my_thread_struct * const thread_struct = ts;

  /* Here we lock the mutex. This function will return when the
     pthread_cond_wait in the main thread is called, which means that
     the structure variables have been set. */
  pthread_mutex_lock(thread_struct->mutex);

  /* Read the structure variables, they will be altered for the next
     call of pthread_create in the main thread. We can safely do that
     because the main thread is currently blocking at
     pthread_cond_wait, waiting for the signal. */
  unsigned * const array = thread_struct->array;
  unsigned const number = thread_struct->number;
  unsigned const start = thread_struct->start;
  unsigned const count = thread_struct->count;

  /* Signal the main thread that this thread is alive and initialized,
     this will trigger the mutex lock by pthread_cond_wait */
  pthread_cond_signal(thread_struct->cond);

  /* Unlock the mutex so that pthread_cond_wait in the main thread
     returns */
  pthread_mutex_unlock(thread_struct->mutex);

  /* Set the end variable for the partial summation */
  unsigned const end = count + start;

  /* Perform the partial summation */
  for(i=start;i<end;++i)
    partial_sum += array[i];

  /* Prinf the debugging info */
  printf("Thread %u: summing elements %u to %u, partial sum is %u\n",
	 number,start+1,end,partial_sum);

  /* Lock the mutex before updating the global sum */
  pthread_mutex_lock(thread_struct->mutex);

  /* Update the global sum. If we are here, it means that we possess
     the mutex, which means not other thread can simultaneously be
     here. */
  thread_struct->sum += partial_sum;

  /* Release the lock */
  pthread_mutex_unlock(thread_struct->mutex);

  /* Indicate arrival at the barrier, so that the main thread could
     cross it and report the sum. */
  pthread_barrier_wait(thread_struct->barrier);

  /* Done */
  return NULL;

}