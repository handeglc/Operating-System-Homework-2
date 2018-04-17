#include "do_not_submit.h"
#include <pthread.h>
#include <semaphore.h>


typedef struct{
	int id;
	int grid_x;
	int grid_y;
	char state;
	char before; //the state before it sleeps
} ant_st;

typedef struct{
	int num_threads;
	int num_foods;
	int delay_count;
}parent_st;


pthread_mutex_t grid_mutex;
pthread_cond_t *unsleep;
pthread_t * threads;
ant_st *ant_struct;
parent_st parent_struct;


void *parent_func(void *str)
{
	parent_st * const parent_str = str;
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
        if(situation==1){
        	int sleepers=getSleeperN();
        	for (int i = 0; i < sleepers; ++i)
        	{
        		ant_struct[i].before = ant_struct[i].state;
        		ant_struct[i].state = 's';
        	}
        	for (int i = sleepers; i < parent_str->num_threads; ++i)
        	{
        		pthread_cond_signal(unsleep+i);
        	}
        }

        usleep(DRAWDELAY);
        
        // each ant thread have to sleep with code similar to this
        //usleep(getDelay() * 1000 + (rand() % 5000));
        pthread_mutex_unlock(&grid_mutex);
    }
    
    // do not forget freeing the resources you get
    endCurses();

	return NULL;

}


void *ant_func(void *str)
{
	ant_st * const thread_struct = str;
	int i=0;
	int tid=thread_struct->id;
	//printf("\n thread: %d says hi\n",tid );
	while(TRUE){
		pthread_mutex_lock(&grid_mutex);
		if(thread_struct->state == 's'){
			pthread_cond_wait(unsleep+tid,&grid_mutex);
			thread_struct->state = thread_struct->before; //turns back to the state before it sleept
			putCharTo(thread_struct->grid_x,thread_struct->grid_y,thread_struct->state);
		}
		
		//usleep(10000);
		if(!i){
			printf("\n thread: %d says hi\n",tid );
			i++;
		}
		
		pthread_mutex_unlock(&grid_mutex);
	}

	return NULL;

}


int main(int argc, char *argv[]) {
    srand(time(NULL));
    int num_threads=atoi(argv[1]);
    int num_foods=atoi(argv[2]);
    int delay_count=atoi(argv[3]);


    ant_struct=malloc(sizeof(ant_st)*num_threads);
    parent_struct.num_threads = num_threads;
    parent_struct.num_foods = num_foods;
    parent_struct.delay_count = delay_count;
    
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
    /*for (i = 0; i < num_threads; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b)!= '-');
        putCharTo(a, b, 'P');
    }*/
    for (i = 0; i < num_threads; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, '1');
        ant_struct[i].state='1';
        ant_struct[i].grid_x=a;
        ant_struct[i].grid_y=b;
        ant_struct[i].id=i;
    }
    for (i = 0; i < num_foods; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, 'o');
    }
    /*for (i = 0; i < num_threads; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, 'S');
    }
    for (i = 0; i < num_threads; i++) {
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, '$');
    }*/
    //////////////////////////////


  	pthread_t fake_parent;
  	pthread_create(&fake_parent,NULL,parent_func,&parent_struct);
  	//pthread_join(&fake_parent,NULL);


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
