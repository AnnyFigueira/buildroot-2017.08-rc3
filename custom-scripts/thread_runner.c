#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>
#include <semaphore.h>
#include <math.h>

int  N;
int buffer;
#define TRUE   1

int volatile running = 1;
sem_t mutex;

char* output;
int pos;

void *run(void *data){
    //printf("run, antes do cast");
    
	char c = data;
	
	int n_chars = buffer/N;
	
	while(n_chars > 0)
	{ 
	    sem_wait(&mutex);
	    output[pos] = c;
	    pos++;
	    sem_post(&mutex);
	    n_chars--;
	}
	
	//printf("run, depois do while");
	
	pthread_exit(NULL);
}

void print_sched(int policy){
	//printf("print_sched, antes do switch\n");
	switch(policy){
		case SCHED_FIFO: 
			printf("SCHED_FIFO\n");
			break;
		case SCHED_RR: 
			printf("SCHED_RR\n");
			break;
		case SCHED_OTHER: 
			printf("SCHED_OTHER\n");
			break;
		default:
			printf("unknown\n");
			break;
	}
	//printf("print_sched, depois do switch\n");
}

void setpriority(pthread_t *thr, int newpolicy, int newpriority){
	int policy = 0;
	int ret;
    struct sched_param param;
    pthread_t t;
    
    if(newpriority > sched_get_priority_max(newpolicy) ||
	   newpriority < sched_get_priority_min(newpolicy)){
		printf("Invalid priority: MIN: %d, MAX: %d", sched_get_priority_min(newpolicy), sched_get_priority_max(newpolicy));
	}

    pthread_getschedparam(*thr, &policy, &param);
    
    print_sched(policy);
    
    //printf("setpriority, depois do print_sched");
    
    param.sched_priority = newpriority;
    
    print_sched(newpolicy);
    
    //printf("setpriority, depois do sched_priority");
    
    ret = pthread_setschedparam(*thr, newpolicy, &param);

    if (ret != 0){
		perror("perror(): ");
	}
}

int main(int argc, char **argv){
    int timesleep;
    
    pthread_t *thr;
    
    N = atoi(argv[1]);
    
    buffer = atoi(argv[2])*1024/sizeof(char);
    
    output = (char*)malloc(sizeof(char)*buffer);
    
    int sched = atoi(argv[3]);
    
    int isched;
    
    switch(sched)
    {
        case SCHED_FIFO: 
			isched = SCHED_FIFO;
			break;
		case SCHED_RR: 
			isched = SCHED_RR;
			break;
		case SCHED_OTHER: 
			isched = SCHED_OTHER;
			break;
		default:
			isched = 0;
			break;
    }
    
    int priority = atoi(argv[4]);
    
    thr = (pthread_t*)malloc(sizeof(pthread_t)*N);
    
    sem_init(&mutex, 0, 1);
    
    if (argc < 4)
    {
		printf("usage: ./%s <execution_time>\n\n", argv[0]);
		exit(0);
	}
	
	/* Pega parâmetros */
	timesleep = 5;
	
	sem_wait(&mutex);
	
	for(int i = 0; i < N; i++) 
	{
	    int data = i+65;
	    pthread_create(&thr[i], NULL, (void *) run, (void *) data);
	    //printf("criou thread"); 
	    setpriority(&thr[i], isched, priority);
	    //printf("setou a prioridade");
	}
	
	sem_post(&mutex);
	
	sleep(timesleep);
    
    running = 0;

    for(int i = 0; i < N; i++) { pthread_join(thr[i], NULL); }
    
    for(int i = 0; i < buffer; i++)
    {
        printf("%c", output[i]);
    }
    
	return 0;
}
