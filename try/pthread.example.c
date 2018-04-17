/* pthread.example.c SJ */
/* gcc -Wall pthread.example.c -lpthread */
/* an example of pthreads and mutexes */
/* same structure as shm.sem.example.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>


/* number of processes */
#define P 3
/* how many semaphores */
#define MUTNUM 2

#define DELAY 10000000

void *manipulate(void *p);

/* parameter structure */
struct param {
	int *array;
	int N;	/* number of elements */
	int i;
	pthread_mutex_t *locks;
};


int main(int argc, char **argv)
{
  pthread_t threads[P];

  pthread_mutex_t locks[MUTNUM];

  int i, val, N;
  void *ret = &val;

  int *array; /* this we'll manipulate */

  struct param params[P];
  
  N = (argc > 1) ? atoi(argv[1]) : 10;

  /* allocate memory with ordinary malloc() */
  array = (int*)malloc(N*sizeof(int));
  if (!array) {
	  perror("malloc: ");
	  exit(1);
  }

  /* init array */
  for (i = 0; i < N; i++)
	  array[i] = 0;

  /* initialize mutex set (non-recursive lock) */
  for (i = 0; i < MUTNUM; i++)
	  pthread_mutex_init(&locks[i], NULL);

  /* create child threads */
 
  for (i = 0; i < P; i++) {
	  /* contruct parameters */
	  params[i].array = array;
	  params[i].N = N;
	  params[i].i = i;
	  params[i].locks = locks;
	  
	  if (pthread_create(&threads[i], NULL, manipulate, &params[i])) {
			  perror("pthread_create: ");
			  exit(1);
	  }
			  
  }

  for (i = 0; i < P; i++)
	  pthread_join(threads[i], &ret);
 

  /* print array contents */

  for (i = 0; i < N; i++) {
	  printf("%d ", array[i]);
  }
  printf("\n");

  /* destroy mutex set (non-recursive lock) */

  for (i = 0; i < MUTNUM; i++)
	  pthread_mutex_destroy(&locks[i]);


  free(array);
  return 0;
}


void *manipulate(void *p)
{
	struct param *params = (struct param *)p;
	
	int i, j, max, sum, tmp, loc, delay;
	int *array = params->array; /* input array */
	int N = params->N; 			/* array size */
	int thr = params->i;		/* some index */
	pthread_mutex_t *locks = params->locks;	/* mutexes */
	int mret;
	

	/* srand() and rand() are shared but should be thread-safe */
	srand(thr + getpid());

	for (i = 0; i < 10; i++) {

		delay = rand()%DELAY;
		for (j = 0; j<delay; j++)
			;

		/* read rest of array */
		max = sum = array[1];
		for (j = 2; j < N; j++) {
			/* read just once to avoid corruption */
			tmp = array[j];
			sum += tmp;
			if (tmp > max)
				max = tmp;
		}

		/* some random position */
		loc = rand()%(N-1)+1;

		/* get exclusive write access to the rest of array (mut1) */
		/* wait on the mutex 1*/
		mret = pthread_mutex_lock(&locks[1]);
		if (mret) {
			perror("pthread_mutex_lock");
			pthread_exit(NULL);
		}


			/* now modify array */
			/* others may read this concurrently */

			array[loc] = max + 1;

		/* release mutex */
		mret = pthread_mutex_unlock(&locks[1]);
		if (mret) {
			perror("pthread_mutex_unlock");
			pthread_exit(NULL);
		}

		/* now we go for index 0 */
		/* we need exclusive access to both read and write */

		/* wait on the mutex 0 */
		mret = pthread_mutex_lock(&locks[0]);
		if (mret) {
			perror("pthread_mutex_lock");
			pthread_exit(NULL);
		}

			/* now modify index */
			/* no else reads or writes this concurrently */

			if (array[0] < sum)
				array[0] = sum;

		/* release mutex */
		mret = pthread_mutex_unlock(&locks[0]);
		if (mret) {
			perror("pthread_mutex_unlock");
			pthread_exit(NULL);
		}
	}

	pthread_exit(NULL);

} /* manipulate() */
