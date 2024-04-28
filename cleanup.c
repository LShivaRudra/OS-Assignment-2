#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>

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
    key_t key = ftok("airtrafficcontroller.c",'A');
    if(key==-1){
        perror("error in ftok");
        exit(1);
    }

    int msgid;
    msgid = msgget(key, 0666 | IPC_CREAT); 
    struct msgbuf message_terminate;
    
    if(msgid<0){
        perror("error in shmget");
        exit(1);
    }

    while(1){
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No) ");
        char answer;
        scanf(" %c",&answer);

        if(answer == 'N' || answer == 'n'){
            continue;
        }
        else if(answer == 'Y' || answer == 'y'){
            break;
        }
        else{
            printf("Invalid input. Please enter Y or N.\n");
        }
    }

    message_terminate.msg_type=22;
    message_terminate.data.termination_from_cleanup=1;
    msgsnd(msgid,&message_terminate,sizeof(message_terminate),0);
    printf("Termination message sent to ATC and Cleanup process terminated!\n");

    return 0;
}
