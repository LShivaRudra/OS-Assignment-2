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
#define MIN_DIFF 16000

pthread_t tids[MAX_PLANES];//max number of planes is 10
int thread_index=0;
pthread_mutex_t runway_mutex[MAX_RUNWAYS];

//msg queue
key_t key;
int msgid;

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
    printf("plane id:%d,total_weight:%d,num_pass:%d\n", msg->recv_data.data.plane_id, msg->recv_data.data.total_weight, msg->recv_data.data.num_passengers);

    printf("airport_num:%d,num_of_runways:%d\n", msg->airport_num, msg->num_of_runways);

    int selected_runway = -1;
    int min_diff=MIN_DIFF;

    int num_of_runways = msg->num_of_runways;
    for(int i=0;i<num_of_runways;i++){
        int diff=msg->runway_capacities[i]-msg->recv_data.data.total_weight;
        printf("Difference of (%d-%d)=%d\n",msg->runway_capacities[i],msg->recv_data.data.total_weight,diff);
        
        if(diff>0 && diff<min_diff){
            min_diff=diff;
            selected_runway=i;
        }
    }
    if(selected_runway==-1){
        selected_runway=msg->num_of_runways;
    }

    printf("Selected Runway is: %d\n",selected_runway);
    
    pthread_mutex_lock(&runway_mutex[selected_runway]);

    if(msg->recv_data.data.airport_num_departure==msg->airport_num){
        //Simulate boarding/loading
        sleep(3);

        //Simulate takeoff
        sleep(2);
        
        //Inform the ATC
        struct msgbuf msg_send_atc;
        msg_send_atc.data=msg->recv_data.data;
        msg_send_atc.data.departure_status=1;
        msg_send_atc.data.arrival_status=0;
        msg_send_atc.msg_type=msg->airport_num+10;
        msgsnd(msgid,&msg_send_atc,sizeof(msg_send_atc),0);

        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",msg->recv_data.data.plane_id,selected_runway,msg->airport_num);
    }
    else if(msg->recv_data.data.airport_num_arrival==msg->airport_num){
        //Simulate plane travel
        sleep(30);

        //Simulate landing
        sleep(2);

        //Simulate deboarding/unloading
        sleep(3);

        //Inform ATC
        struct msgbuf msg_send_atc;
        msg_send_atc.data=msg->recv_data.data;
        msg_send_atc.data.departure_status=1;
        msg_send_atc.data.arrival_status=1;
        msg_send_atc.msg_type=msg->airport_num+10;
        msgsnd(msgid,&msg_send_atc,sizeof(msg_send_atc),0);

        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",msg->recv_data.data.plane_id,selected_runway,msg->airport_num);
    }

    pthread_mutex_unlock(&runway_mutex[selected_runway]);
    fflush(stdout);
    pthread_exit(NULL);
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

    for (int i = 0; i<=num_of_runways; i++) {
        pthread_mutex_init(&runway_mutex[i], NULL);
    }

    // ftok to generate unique key 
    key = ftok("airtrafficcontroller.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT); 

    while(1){
        // int i=0;
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
            airport.airport_num=airport_num;
            airport.num_of_runways=num_of_runways;
            // printf("airport_num:%d,num_of_runways:%d\n",airport.airport_num,airport.num_of_runways);
            error = pthread_create(&(tids[thread_index++]),NULL,&thread_func,(void*)&airport); 
            if(error!=0){
                perror("Error in thread creation\n");
                exit(1);
            } 
            fflush(stdout);
        }
    }

    return 0;
}