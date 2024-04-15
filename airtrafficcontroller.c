#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>

#define MIN_PASSENGERS 1
#define MAX_PASSENGERS 10

#define MIN_AIRPORTS 2
#define MAX_AIRPORTS 10

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

int main(){
    int num_airports;//will be from 2-10
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d",&num_airports);

    FILE *fptr;

    fptr = fopen("AirTrafficController.txt", "w");
    
    if (fptr == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    
    // Close the file
    fclose(fptr);

    fptr = fopen("AirTrafficController.txt","a");
    if(fptr== NULL){
        perror("unable to open file");
        exit(1);
    }

    //msg queue
    struct msgbuf message_recv;
    key_t key;
    int msgid;
    // ftok to generate unique key 
    key = ftok("airtrafficcontroller.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT); 
    
    while(1){
        if(msgrcv(msgid, &message_recv, sizeof(message_recv), 0, 0)==-1){
            perror("Error while receiving msg!");
            exit(1);
        }
        else if(message_recv.msg_type==21 && message_recv.data.termination_from_cleanup==1){
            printf("Received termination request from the cleanup!\n");
            for(int i=1;i<=num_airports;i++){
                struct msgbuf airport_termination_msg;
                airport_termination_msg.msg_type=(i+10);
                airport_termination_msg.data.termination_from_cleanup=1;
                msgsnd(msgid,&airport_termination_msg,sizeof(airport_termination_msg),0);
            }
            msgctl(msgid,IPC_RMID,NULL);
            return 0;
        }
        else if(message_recv.msg_type>=1 && message_recv.msg_type<=10){
            struct msgbuf message_send_departure;
            message_send_departure=message_recv;
            message_send_departure.msg_type = message_recv.data.airport_num_departure+10;
            if(msgsnd(msgid,&message_send_departure,sizeof(message_send_departure),0)==-1){
                perror("Error in msgsnd!\n");
                exit(1);
            }
            else{
                printf("This is the msg data being sent:\n");
                printf("plane id:%d,total_weight:%d,num_pass:%d\n",message_send_departure.data.plane_id,message_send_departure.data.total_weight,message_send_departure.data.num_passengers);
                printf("Message sent to departure airport!\n");
                printf("The msg type is %ld\n",message_send_departure.msg_type);
            }
        }
        else if(message_recv.msg_type>=10 && message_recv.msg_type<=20){
            if(message_recv.data.departure_status==1 && message_recv.data.arrival_status==0){
                struct msgbuf message_send_arrival;
                message_send_arrival=message_recv;
                message_send_arrival.msg_type = message_recv.data.airport_num_arrival+10;
                if(msgsnd(msgid,&message_send_arrival,sizeof(message_send_arrival),0)==-1){
                    perror("Error in msgsnd!\n");
                    exit(1);
                }
                else{
                    printf("Message sent to arrival airport!\n");
                    printf("The msg type is %ld\n",message_send_arrival.msg_type);
                    fprintf(fptr, "Plane %d has departed from Airport %d and will land at Airport %d.\n", message_recv.data.plane_id,message_recv.data.airport_num_departure,message_recv.data.airport_num_arrival);
                    printf("Plane %d has departed from Airport %d and will land at Airport %d.\n", message_recv.data.plane_id,message_recv.data.airport_num_departure,message_recv.data.airport_num_arrival);
                    fclose(fptr);
                    fptr = fopen("AirTrafficController.txt", "a");
                }   
            }
            else if(message_recv.data.departure_status==1 && message_recv.data.arrival_status==1){
                struct msgbuf message_send_plane;
                message_send_plane=message_recv;
                message_send_plane.msg_type = message_recv.data.plane_id;
                if(msgsnd(msgid,&message_send_plane,sizeof(message_send_plane),0)==-1){
                    perror("Error in msgsnd!\n");
                    exit(1);
                }
                else{
                    printf("Message sent to plane!\n");
                    printf("The msg type is %ld\n",message_send_plane.msg_type);
                }
            }
        }
    }
    fclose(fptr);
    
    return 0;
}