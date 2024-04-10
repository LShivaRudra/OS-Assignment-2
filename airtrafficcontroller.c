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
    int means_of_comm;
    int airport_num_arrival;
    int airport_num_departure;
    int plane_id;
    int total_weight;
    int plane_type;
    int num_passengers;
    // int departure_status;
    int terminate_plane;
    int termination_from_cleanup;
};
/*in the above means_of_comm differentiates the direction of msg flow:
1 -> plane to ATC 
2 -> ATC to plane
3 -> airport to ATC
4 -> ATC to airport
5 -> cleanup to ATC
6 -> ATC to cleanup
*/

struct msgbuf{
    long msg_type;
    struct msg_data data;
};

int main(){
    int num_airports;//will be from 2-10
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d",&num_airports);

    //msg queue
    struct msgbuf message_recv;
    key_t key;
    int msgid;
    // ftok to generate unique key 
    key = ftok("airtrafficcontroller.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT); 
    
    while(1){
        msgrcv(msgid, &message_recv, sizeof(message_recv), 0, 0); 
        if(message_recv.data.means_of_comm==1){
            struct msgbuf message_send;
            message_send=message_recv;
            message_send.data.means_of_comm=4;
            message_send.msg_type=message_recv.data.airport_num_departure+10;//to differentiate between the plane and airport process
            msgsnd(msgid,&message_send,sizeof(message_send),0);
        }
    }
    
    return 0;
}