#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h> 

#define MAX_AIRPORTS 10
#define MAX_RUNWAYS 10
#define MIN_LOAD_CAPACITY 1000
#define MAX_LOAD_CAPACITY 12000
#define BACKUP_LOAD_CAPACITY 15000

pthread_t tid[10];//max number of planes is 10
pthread_mutex_t lock;

struct msg_data{
    int airport_num_arrival;
    int airport_num_departure;
    int plane_id;
    int total_weight;
    int plane_type;
    int num_passengers;
    int departure_status;
    int arrival_status;
    int terminate_plane;
    int termination_from_cleanup;
};

struct msgbuf{
    long msg_type;
    struct msg_data data;
};

struct Airport{
    int airport_num;
    int num_of_runways;
    int runway_capacities[MAX_RUNWAYS+1];
    struct msgbuf recv_data;
};

void* thread_func(void *arg){
    struct Airport *msg = (struct Airport *)arg;

    int selected_runway=-1;
    int min_diff=__INT_MAX__;
    for(int i=0;i<msg->num_of_runways;i++){
        int diff=msg->runway_capacities[i]-msg->recv_data.data.total_weight;
        if(diff>0 && diff<min_diff){
            min_diff=diff;
            selected_runway=i;
        }
    }
    if(selected_runway==-1){
        selected_runway=msg->runway_capacities[msg->num_of_runways];
    }

    if(msg->recv_data.data.airport_num_departure==msg->airport_num){
        pthread_mutex_lock(&lock);

        sleep(3);//boarding/loading
        sleep(2);//takeoff

        pthread_mutex_unlock(&lock);

        struct msgbuf msg_send_atc;
        msg_send_atc=msg->recv_data;
        msg_send_atc.data.departure_status=1;

        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",msg->recv_data.data.plane_id,selected_runway,msg->airport_num);

        pthread_exit(NULL);
    }

    else if(msg->recv_data.data.airport_num_arrival==msg->airport_num){
        pthread_mutex_lock(&lock);

        sleep(2);//landing
        sleep(3);//de-boarding/unloading

        pthread_mutex_unlock(&lock);

        struct msgbuf msg_send_atc;
        msg_send_atc=msg->recv_data;
        msg_send_atc.data.arrival_status=1;

        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",msg->recv_data.data.plane_id,selected_runway,msg->airport_num);

        pthread_exit(NULL);
    }
}

int main(){
    int airport_num;//will be 1-10
    printf("Enter Airport Number: ");
    scanf("%d",&airport_num);

    int num_of_runways;//will be an even number between 1-10(inclusive)
    printf("Enter number of Runways: ");
    scanf("%d",&num_of_runways);

    struct Airport airport;
    airport.airport_num=airport_num;
    airport.num_of_runways=num_of_runways;
    
    // int runway_capacities[MAX_RUNWAYS+1];//each value lies in 1000-12000 kgs
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):");
    for(int i=0;i<num_of_runways;i++){
        if(scanf("%d",&airport.runway_capacities[i])==EOF)break;        
    }
    airport.runway_capacities[num_of_runways]=15000;//backup runway
    
    if(pthread_mutex_init(&lock,NULL)!=0){
        printf("\n mutex init has failed\n");
        exit(1);
    }

    //msg queue
    key_t key;
    int msgid;
    // ftok to generate unique key 
    key = ftok("airtrafficcontroller.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT); 

    while(1){
        int i=0;
        int error;
        struct msgbuf msg_recv_atc;
        size_t val=msgrcv(msgid,&msg_recv_atc,sizeof(msg_recv_atc),(10+airport_num),0);
        if(val==-1){
            perror("Error while receiving msg!");
            exit(1);
        }
        else if(val>0){
            airport.recv_data=msg_recv_atc;
            error = pthread_create(&(tid[i++]),NULL,&thread_func,(void*)&airport); 
            if(error!=0){
                perror("Error in thread creation\n");
                exit(1);
            } 
        }
    }

    return 0;
}