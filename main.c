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




pthread_mutex_t grid_mutex;
pthread_mutex_t put_look;
pthread_mutex_t delay_mutex;
pthread_cond_t *unsleep;
pthread_t * threads;
ant_st *ant_struct;

struct timespec start, finish;
double elapsed;


void *ant_func(void *str)
{
	
	//printf("\n thread: %d says hi\n",tid );
	while(TRUE){
		pthread_mutex_lock(&grid_mutex);
		int delay=1;
		ant_st * const thread_struct = str;

		if(thread_struct->state=='E')
			//return NULL;
			pthread_exit(NULL);

		int k=0;
		int tid=thread_struct->id;
		if(thread_struct->state == 'S'){
			//printf("BURAYA BAKARLAR\n");
			pthread_cond_wait(unsleep+tid,&grid_mutex);
			pthread_mutex_lock(&put_look);
			thread_struct->state = thread_struct->before; //turns back to the state before it sleept
			putCharTo(thread_struct->grid_x,thread_struct->grid_y,thread_struct->state);
			pthread_mutex_unlock(&put_look);
		}
		else{
			//pthread_cond_wait(sleep+tid,&grid_mutex);
			pthread_mutex_lock(&put_look);
			thread_struct->before = thread_struct->state;
			int x= thread_struct->grid_x;
			int y= thread_struct->grid_y;
			int av_x,av_y,fo_x,fo_y,a,b;
			fo_x=-1; fo_y=-1;
			//look for a food
			
			for (int i = x-1; i <= x+1; ++i)
			{
				for (int j = y-1; j <= y+1; ++j)
				{
					if(i>0 && i<30 && y>0 && y<30){

						if(lookCharAt(i,j)=='o'){
							fo_x=i;
							fo_y=j;
						}
						if(lookCharAt(i,j)=='-'){
							av_x=i;
							av_y=j;
						}
					}
				}
			}

			do {
	            a = (rand() % 3)-1 +x;
	            b = (rand() % 3)-1 +y;
	            if(a>=0 && a<30 && b>=0 && b<30 && lookCharAt(a,b) == '-')
	            	break;
	        } while (true);

			//sem_wait(states);
			if(thread_struct->state=='1'){
				if(fo_x>-1 && fo_y>-1){
					//usleep(10000);
					putCharTo(x,y,'-');
					putCharTo(fo_x,fo_y,'P');
					thread_struct->state='P';
					thread_struct->grid_x=fo_x;
			        thread_struct->grid_y=fo_y;
			        usleep(1);
				}
				else{
			        //usleep(10000);
			        
			        putCharTo(a, b, '1');
			        putCharTo(x, y, '-');
			        thread_struct->grid_x=a;
			        thread_struct->grid_y=b;
				}
			}
			else if(thread_struct->state=='P'){
		    
				if(fo_x>-1 && fo_y>-1){ //if there is food
					
					putCharTo(x,y,'o'); //leave food
					thread_struct->state='T'; //be tired
					putCharTo(a, b, '1');

			        thread_struct->grid_x=a;
			        thread_struct->grid_y=b;
			        usleep(1);
				}
				else{ //if there is no food
					if(av_x>-1 && av_y>-1){ //if there is an available place
						
						putCharTo(a,b,'P');
						putCharTo(x,y,'-');
				        thread_struct->grid_x=a;
				        thread_struct->grid_y=b;
					}
					
				}
				    //usleep(10000);
			        
			}
			else if(thread_struct->state=='T'){
				if(av_x>-1 && av_y>-1){ //if there is an available place
						putCharTo(x,y,'-');	
						putCharTo(a,b,'1');
						thread_struct->state = '1';
				        thread_struct->grid_x=a;
				        thread_struct->grid_y=b;
					}
			}
			//sem_post(states);
			
			pthread_mutex_unlock(&put_look);

		}
		pthread_mutex_lock(&delay_mutex);
		delay = getDelay();
		pthread_mutex_unlock(&delay_mutex);
		
		//usleep(100);
		pthread_mutex_unlock(&grid_mutex);
		//pthread_mutex_lock(&delay_mutex);
		usleep(delay* 1000 + (rand() % 5000));
		//pthread_mutex_unlock(&delay_mutex);
	}

	

}


int main(int argc, char *argv[]) {
	//clock_t begin = clock();
	clock_gettime(CLOCK_MONOTONIC, &start);
    srand(time(NULL));
    int num_threads=atoi(argv[1]);
    int num_foods=atoi(argv[2]);
    int delay_count=atoi(argv[3]);

    pthread_mutex_init(&grid_mutex,NULL);
  	pthread_mutex_init(&put_look,NULL);
  	pthread_mutex_init(&delay_mutex,NULL);

    ant_struct=malloc(sizeof(ant_st)*num_threads);
    //////////////////////////////
    
    int i,j;
    for (i = 0; i < GRIDSIZE; i++) {
        for (j = 0; j < GRIDSIZE; j++) {
            putCharTo(i, j, '-');
        }
    }
    int a,b;

    for (i = 0; i < num_threads; i++) {
    	
        do {
            a = rand() % GRIDSIZE;
            b = rand() % GRIDSIZE;
        }while (lookCharAt(a,b) != '-');
        putCharTo(a, b, '1');
        ant_struct[i].state='1';
        ant_struct[i].before='1';
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

    //////////////////////////////
    threads = malloc(sizeof(pthread_t)*num_threads);

  	unsleep = malloc(sizeof(pthread_cond_t)*num_threads);

  	for (i = 0; i <num_threads ; ++i)
  	{
  		pthread_cond_init(unsleep+i,NULL);
  		pthread_create(threads+i,NULL,ant_func,ant_struct+i);

  	}


  	startCurses();
  	char c;
    int situation = 0;
    while (TRUE) {
    	
        drawWindow();
        pthread_mutex_lock(&grid_mutex);
        c = 0;
        c = getch();

        if (c == 'q' || c == ESC) break;
        pthread_mutex_lock(&delay_mutex);
        if (c == '+') {
            setDelay(getDelay()+10);
            
        }
        if (c == '-') {
            setDelay(getDelay()-10);
            
        }
        pthread_mutex_unlock(&delay_mutex);
        situation = 0;
        if (c == '*') {
            setSleeperN(getSleeperN()+1);
            situation=1;
        }
        if (c == '/') {
            setSleeperN(getSleeperN()-1);
            situation=1;
        }
        
        if(situation==1){
        	pthread_mutex_lock(&put_look);
        	int sleepers=getSleeperN();
        	char e;
        	for (int i = 0; i < sleepers && i<num_threads; ++i)
        	{
        		if(ant_struct[i].state=='P')
        			putCharTo(ant_struct[i].grid_x,ant_struct[i].grid_y,'$');
        		else if(ant_struct[i].state=='1')
        			putCharTo(ant_struct[i].grid_x,ant_struct[i].grid_y,'S');
        		usleep(1);
        		ant_struct[i].state = 'S';
        		
        	}
       		pthread_mutex_unlock(&put_look); 	
        	for (int i = sleepers; i < num_threads; ++i)
        	{
        		
				pthread_cond_signal(unsleep+i);
				
				usleep(1);
        	}
        	
        }
        
        
        
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
		if(elapsed>delay_count){
			for (int i = 0; i < num_threads; ++i)
			{
				ant_struct[i].state='E';
				usleep(20);
			}
        	
        	break;
        }
        usleep(DRAWDELAY);
        
        
        // each ant thread have to sleep with code similar to this
        //usleep(getDelay() * 1000 + (rand() % 5000));
        pthread_mutex_unlock(&grid_mutex);
        
    }
    
    // do not forget freeing the resources you get
    pthread_mutex_unlock(&grid_mutex);
    endCurses();
    for (i = 0; i <num_threads ; ++i)
  	{
  		//free(ant_struct+i);
  		//pthread_join(threads[i],0);
  		
  		pthread_kill(threads[i],0);
  		pthread_cond_destroy(unsleep+i);

  	}
  	
    free(ant_struct);
    free(threads);
    free(unsleep);
	pthread_mutex_destroy(&grid_mutex);
	pthread_mutex_destroy(&put_look);
	pthread_mutex_destroy(&delay_mutex);
    
    return 0;
}
