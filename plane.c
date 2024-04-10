#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>

#define MIN_PASSENGERS 1
#define MAX_PASSENGERS 10

#define READ_END 0
#define WRITE_END 1

struct weight_per_customer{
    int luggage_wt;
    int body_wt;
};

struct plane_data{
    int airport_num_arrival;
    int airport_num_departure;
    int plane_id;
    int total_weight;
    int plane_type;
    int num_passengers;
    // int departure_status;
    int terminate_plane;
};

struct msgbuf{
    long msg_type;
    struct plane_data data;
};

int main(){
    struct plane_data data;
    int plane_id;//will be from 1-10
    printf("Enter Plane ID: ");
    scanf("%d",&plane_id);

    int plane_type;//will be 0/1
    printf("Enter Type of Plane: ");//0-cargo and 1-passenger
    scanf("%d",&plane_type);

    int total_weight=0;
    if(plane_type==1){
        total_weight+=7*75;//weight of crew members
        int num_passengers;//will be from 1-10
        printf("Enter Number of Occupied Seats: ");
        scanf("%d",&num_passengers);
        data.num_passengers=num_passengers;

        int pipe_handler[num_passengers][2];
        int plane_pid=getpid();
        printf("Plane %d pid is %d\n\n",plane_id,plane_pid);

        for(int i=0;i<num_passengers;i++){
            if(pipe(pipe_handler[i])<0){
                perror("Error creating a pipe\n");
                exit(1);
            }
            
            pid_t pid=fork();
            if(pid==0){
                close(pipe_handler[i][READ_END]);
                struct weight_per_customer wt;

                printf("passenger %d pid is: %d\n",i+1,getpid());
                
                int luggage_wt;//will be from 0-25(only one value entered)
                printf("Enter Weight of Your Luggage: ");
                scanf("%d",&luggage_wt);

                int body_wt;//will be from 10-100(only one value entered)
                printf("Enter Your Body Weight: ");
                scanf("%d",&body_wt);

                wt.luggage_wt=luggage_wt;
                wt.body_wt=body_wt;
                write(pipe_handler[i][WRITE_END],&wt,sizeof(wt));
                exit(1);
                // if(wt.body_wt==-1 || wt.luggage_wt==-1)
            }
            if(pid>0){
                wait(NULL);
                close(pipe_handler[i][WRITE_END]);
                struct weight_per_customer wt;
                read(pipe_handler[i][READ_END],&wt,sizeof(wt));
                total_weight+=wt.luggage_wt+wt.body_wt;
                printf("%d %d\n\n",wt.luggage_wt,wt.body_wt);
            }
        }

    }

    else if(plane_type==0){
        int num_cargo_items;//will be from 1-100
        printf("Enter Number of Cargo Items: ");
        scanf("%d",&num_cargo_items);

        int avg_wt_cargo;//will be from 1-100
        printf("Enter Average Weight of Cargo Items: ");
        scanf("%d",&avg_wt_cargo);

        total_weight=num_cargo_items*avg_wt_cargo+75*2;
    }

    printf("\nTotal Weight: %d",total_weight);
    printf("\n");

    int airport_num_departure;//will be from 1-10
    printf("Enter Airport Number for Departure: ");
    scanf("%d",&airport_num_departure);

    int airport_num_arrival;//will be from 1-10(will not be same as the departure num)
    printf("Enter Airport Number for Arrival: ");
    scanf("%d",&airport_num_arrival);

    data.airport_num_arrival=airport_num_arrival;
    data.airport_num_departure=airport_num_departure;
    data.plane_id=plane_id;
    data.total_weight=total_weight;
    data.plane_type=plane_type;
    // data.departure_status=0;

    /*message queue*/
    //message to send to ATC
    struct msgbuf message_send;
    key_t key_send;
    int msgid_send;
    // ftok to generate unique key 
    key_send = ftok("plane.c", plane_id);
    msgid_send = msgget(key_send, 0666 | IPC_CREAT); 
    message_send.msg_type=plane_id;
    message_send.data=data;
    msgsnd(msgid_send,&message_send,sizeof(message_send),0);

    //message to recv from ATC
    struct msgbuf message_recv;
    key_t key_recv;
    int msgid_recv;
    // ftok to generate unique key 
    key_recv = ftok("airtrafficcontroller.c", plane_id);
    msgid_recv = msgget(key_recv, 0666 | IPC_CREAT); 
    msgrcv(msgid_recv, &message_recv, sizeof(message_recv), plane_id, 0); 

    while(message_recv.data.terminate_plane==0){}//wait for the ATC to give termination command(after boarding, travel, deboarding and confirmation from the airport)

    msgctl(msgid_send, IPC_RMID, NULL); 
    msgctl(msgid_recv, IPC_RMID, NULL); 

    return 0;
}