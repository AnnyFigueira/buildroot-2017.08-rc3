#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

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
	    case SCHED_IDLE:
	        printf("SCHED_IDLE\n");
	        break;
	    case SCHED_LOW_IDLE:
	        printf("SCHED_LOW_IDLE\n");
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
    
    char* s = argv[3];
    
    int s_size = strlen(s);
    
    int sched = atoi(argv[3]);
    
    if (s_size > 6)
    {
        if (s[6] == 'F') sched = SCHED_FIFO;
        else if (s[6] == 'R') sched = SCHED_RR;
        else if (s[6] == 'O') sched = SCHED_OTHER;
        else if (s[6] == 'I') sched = SCHED_IDLE;
        else if (s[6] == 'L') sched = SCHED_LOW_IDLE;
    }
    
    printf("Sched: %d \n", sched);
    
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
	    setpriority(&thr[i], sched, priority);
	    //printf("setou a prioridade");
	}
	
	//time_t start;
	
	//start = time(NULL);
	
	unsigned long x = 0xFFFFFFFF;
	struct timespec start, finish;
	long delta_usecs;
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	sem_post(&mutex);
	
	//sleep(timesleep);
    
    running = 0;

    for(int i = 0; i < N; i++) { pthread_join(thr[i], NULL); }
    
    //time_t end;
    
    //end = time(NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &finish);
    
    int end_pos = 0;
    
    char* order = (char*)malloc(sizeof(char)*buffer);
    
    char previous = -1;
    
    char current;
    
    for (int i = 0; i < buffer; i++)
    {
        current = output[i];
        if (current != previous)
        {
            order[end_pos] = current;    
            previous = current;
            end_pos++;
        }
    }
    
    for(int i = 0; i < end_pos; i++)
    {
        printf("%c", order[i]);
    }
    
    printf("\n");
    
    int* amount = (int*)malloc(sizeof(int)*N);
    
    for(int i = 0; i < N; i++)
    {
        amount[i] = 0;
    }    
    
    int cur_pos = 0;
    
    for(int i = 0; i < end_pos; i++)
    {
        cur_pos = order[i]-65;
        amount[cur_pos] = amount[cur_pos] + 1;
    }
    
    for(int i = 0; i < N; i++)
    {
        char cur = 0;
        cur = i+65;
        printf ("%c = %d \n", cur, amount[i]);
    }
    
    //double seconds =  difftime(end, start);
    
    delta_usecs = (finish.tv_sec - start.tv_sec) * 1000000 + (finish.tv_nsec - start.tv_nsec) / 1000;
    
    printf("Tempo de execução do sched %d: %dus\n", sched, delta_usecs);
       
	return 0;
}
