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
sem_t *states;
pthread_cond_t *unsleep;
pthread_t * threads;
ant_st *ant_struct;
int running_threads;




void *ant_func(void *str)
{
	
	//printf("\n thread: %d says hi\n",tid );
	while(TRUE){
		pthread_mutex_lock(&grid_mutex);
		//int delay=10;
		ant_st * const thread_struct = str;
		int k=0;
		int tid=thread_struct->id;
		if(thread_struct->state == 'S'){
			//printf("BURAYA BAKARLAR\n");
			pthread_cond_wait(unsleep+tid,&grid_mutex);
			//printf("are you here????????????\n");
			thread_struct->state = thread_struct->before; //turns back to the state before it sleept
			putCharTo(thread_struct->grid_x,thread_struct->grid_y,thread_struct->state);
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
			//delay = getDelay();
			pthread_mutex_unlock(&put_look);

		}
		
		
		//usleep(100);
		
		
		pthread_mutex_unlock(&grid_mutex);
		pthread_mutex_lock(&delay_mutex);
		usleep(getDelay()* 1000 + (rand() % 5000));
		pthread_mutex_unlock(&delay_mutex);
	}

	return NULL;

}


int main(int argc, char *argv[]) {
	clock_t begin = clock();
    srand(time(NULL));
    int num_threads=atoi(argv[1]);
    int num_foods=atoi(argv[2]);
    int delay_count=atoi(argv[3]);

    pthread_mutex_init(&grid_mutex,NULL);
  	pthread_mutex_init(&put_look,NULL);
  	pthread_mutex_init(&delay_mutex,NULL);

    ant_struct=malloc(sizeof(ant_st)*num_threads);
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
    pthread_t * threads = malloc(sizeof(pthread_t)*num_threads);

  	unsleep = malloc(sizeof(pthread_cond_t)*num_threads);
  	states = malloc(sizeof(pthread_cond_t)*num_threads);
  	for (i = 0; i <num_threads ; ++i)
  	{
  		pthread_cond_init(unsleep+i,NULL);
  		sem_init(states+i,0,0);
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
            //situation=2;
        }
        if (c == '-') {
            setDelay(getDelay()-10);
            //situation=2;
        }
        pthread_mutex_unlock(&delay_mutex);
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
        	
        	for (int i = sleepers; i < num_threads; ++i)
        	{
        		
				pthread_cond_signal(unsleep+i);
				
				usleep(1);
        	}
        	
        }
        
        
		clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        //printf("time:::::::::: %f\n",time_spent );
        if(time_spent>delay_count)
        	break;
        usleep(DRAWDELAY);
        
        
        // each ant thread have to sleep with code similar to this
        //usleep(getDelay() * 1000 + (rand() % 5000));
        pthread_mutex_unlock(&grid_mutex);
        
    }
    
    // do not forget freeing the resources you get
    endCurses();



    
    return 0;
}
