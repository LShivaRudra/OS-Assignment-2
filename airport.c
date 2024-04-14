#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <pthread.h> 

#define MAX_PLANES 10
#define MAX_AIRPORTS 10
#define MAX_RUNWAYS 10
#define MIN_LOAD_CAPACITY 1000
#define MAX_LOAD_CAPACITY 12000
#define BACKUP_LOAD_CAPACITY 15000

pthread_t tids[MAX_PLANES];//max number of planes is 10
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

typedef struct{
    int airport_num;
    int num_of_runways;
    int runway_capacities[MAX_RUNWAYS+1];
    struct msgbuf recv_data;
}Airport;

void* thread_func(void *arg){
    Airport *msg = (Airport *)arg;
    printf("Hello to the thread func!\n");
    printf("plane id:%d,total_weight:%d,num_pass:%d",msg->recv_data.data.plane_id,msg->recv_data.data.total_weight,msg->recv_data.data.num_passengers);
    fflush(stdout);
}

int main(){
    int airport_num;//will be 1-10
    printf("Enter Airport Number: ");
    scanf("%d",&airport_num);

    int num_of_runways;//will be an even number between 1-10(inclusive)
    printf("Enter number of Runways: ");
    scanf("%d",&num_of_runways);

    Airport airport;
    airport.airport_num=airport_num;
    airport.num_of_runways=num_of_runways;
    
    // int runway_capacities[MAX_RUNWAYS+1];//each value lies in 1000-12000 kgs
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):");
    for(int i=0;i<num_of_runways;i++){
        if(scanf("%d",&airport.runway_capacities[i])==EOF)break;        
    }
    airport.runway_capacities[num_of_runways]=15000;//backup runway

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
        long msg_type=(airport_num+10);
        msg_recv_atc.msg_type=msg_type;
        if(msgrcv(msgid, &msg_recv_atc, sizeof(msg_recv_atc), msg_type, 0)<=0){
            perror("Error while receiving msg!");
            exit(1);
        }

        else{
            printf("Msg recvd from the atc!\n");
            // printf("plane id:%d,total_weight:%d,num_pass:%d",msg_recv_atc.data.plane_id,msg_recv_atc.data.total_weight,msg_recv_atc.data.num_passengers); 
            // fflush(stdout);  
            airport.recv_data=msg_recv_atc;
            error = pthread_create(&(tids[i++]),NULL,&thread_func,(void*)&airport); 
            if(error!=0){
                perror("Error in thread creation\n");
                exit(1);
            } 
            fflush(stdout);
        }
    }

    return 0;
}