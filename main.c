#include "do_not_submit.h"
#include <pthread.h>
#include <semaphore.h>


typedef struct{
	char state;
	int id;
	int grid_x;
	int grid_y;
} ant_st;


pthread_mutex_t grid_mutex;
pthread_cond_t *unsleep;
pthread_t * threads;
ant_st *ant_struct;


void *parent_func(void *str)
{

    // you have to have following command to initialize ncurses.
    startCurses();
    
    // You can use following loop in your program. But pay attention to 
    // the function calls, they do not have any access control, you 
    // have to ensure that.
    char c;
    int situation = 0;
    while (TRUE) {
    	pthread_mutex_lock(&grid_mutex);
        drawWindow();
        
        c = 0;
        c = getch();

        if (c == 'q' || c == ESC) break;
        if (c == '+') {
            setDelay(getDelay()+10);
            situation=2;
        }
        if (c == '-') {
            setDelay(getDelay()-10);
            situation=2;
        }
        if (c == '*') {
            setSleeperN(getSleeperN()+1);
            situation=1;
        }
        if (c == '/') {
            setSleeperN(getSleeperN()-1);
            situation=1;
        }
        /*if(situation==1){
        	int sleepers=getSleeperN();
        	for (int i = 0; i < sleepers; ++i)
        	{
        		
        	}
        }*/

        usleep(DRAWDELAY);
        
        // each ant thread have to sleep with code similar to this
        usleep(getDelay() * 1000 + (rand() % 5000));
        pthread_mutex_unlock(&grid_mutex);
    }
    
    // do not forget freeing the resources you get
    endCurses();

	return NULL;

}


void *ant_func(void *str)
{
	ant_st * const thread_struct = str;

	int tid=thread_struct->id;
	printf("\n thread: %d says hi\n",tid );
	while(TRUE){
		pthread_mutex_lock(&grid_mutex);
		if(thread_struct->state == 's')
			pthread_cond_wait(unsleep+tid,&grid_mutex);
		//usleep(10000);
		//printf("\n thread: %d says hi\n",tid );
		pthread_mutex_unlock(&grid_mutex);
	}

	return NULL;

}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    int num_threads=5;
    //////////////////////////////
    // Fills the grid randomly to have somthing to draw on screen.
    // Empty spaces have to be -.
    // You should get the number of ants and foods from the arguments 
    // and make sure that a food and an ant does not placed at the same cell.
    // You must use putCharTo() and lookCharAt() to access/change the grid.
    // You should be delegating ants to separate threads
    int i,j;
    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            putCharTo(i, j, '-');
        }
    }
    int a,b;
    for (i = 0; i < 5; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b)!= '-');
        putCharTo(a, b, 'P');
    }
    for (i = 0; i < 5; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, '1');
    }
    for (i = 0; i < 5; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, 'o');
    }
    for (i = 0; i < 5; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, 'S');
    }
    for (i = 0; i < 5; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, '$');
    }
    //////////////////////////////


  	pthread_t fake_parent;
  	pthread_create(&fake_parent,NULL,parent_func,NULL);
  	//pthread_join(&fake_parent,NULL);

    ant_struct=malloc(sizeof(ant_st)*num_threads);
    for (i = 0; i < num_threads; ++i)
    {
    	ant_struct[i].id=i;
    }

    pthread_t * threads = malloc(sizeof(pthread_t)*num_threads);

  	pthread_mutex_init(&grid_mutex,NULL);

  	unsleep = malloc(sizeof(pthread_cond_t)*num_threads);
  	for (i = 0; i <num_threads ; ++i)
  	{
  		pthread_cond_init(unsleep+i,NULL);
  		pthread_create(threads+i,NULL,ant_func,ant_struct+i);

  	}
  	for (i = 0; i <num_threads ; ++i)
  	{
  		pthread_join(threads[i],NULL);

  	}


    
    return 0;
}
