#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
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

int main(){
    int plane_id;//will be from 1-10
    printf("Enter Plane ID: ");
    scanf("%d",&plane_id);
    printf("\n");

    int plane_type;//will be 0/1
    printf("Enter Type of Plane: ");//0-cargo and 1-passenger
    scanf("%d",&plane_type);
    printf("\n");

    if(plane_type==1){
        int num_passengers;//will be from 1-10
        printf("Enter Number of Occupied Seats: ");
        scanf("%d",&num_passengers);

        int pipe_handler[num_passengers][2];

        printf("Plane %d pid is %d\n",plane_id,getpid());

        for(int i=0;i<num_passengers;i++){
            if(pipe(pipe_handler[i]<0)){
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
                printf("\n");

                int body_wt;//will be from 10-100(only one value entered)
                printf("Enter Your Body Weight: ");
                scanf("%d",&body_wt);
                printf("\n");

                wt.luggage_wt=luggage_wt;
                wt.body_wt=body_wt;
                write(pipe_handler[i][WRITE_END],&wt,sizeof(wt));
                // if(wt.body_wt==-1 || wt.luggage_wt==-1)
            }
            if(pid>0){
                close(pipe_handler[i][WRITE_END]);
                struct weight_per_customer wt;
                read(pipe_handler[i][READ_END],&wt,sizeof(wt));
            }

        }

    }

    return 0;
}