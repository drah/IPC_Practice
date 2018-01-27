#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <time.h>
#include <sys/shm.h>

#define BUFFER_SIZE 40
union semun{
	int val;
	struct semid_ds *buf;
	ushort *array;
} arg;

#define NOWAY 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
int chess(bool board[],int x,int y){
	bool up=true,down=true,left=true,right=true;
	while(up || down || left || right)
	switch(rand()%4+1){
	case UP:if(x>0 && !board[(x-1)*50+y]){
		board[(x-1)*50+y]=true;
		return UP;
		}
		up=false;
		continue;
	case DOWN:if(x<49 && !board[(x+1)*50+y]){
		board[(x+1)*50+y]=true;
		return DOWN;
		}
		down=false;
		continue;
	case LEFT:if(y>0 && !board[x*50+y-1]){
		board[x*50+y-1]=true;
		return LEFT;
		}
		left=false;
		continue;
	case RIGHT:if(y<49 && !board[x*50+y+1]){
		board[x*50+y+1]=true;
		return RIGHT;
		}
		right=false;
		continue;
	}
	return NOWAY;
}

int main (int argc, char* argv[]) {
	//shmget(key,size,mode)
        int shm_key=123;
        int shm_id = shmget(shm_key,32,IPC_CREAT | 0666);
        if(shm_id < 0)
        {
                perror("shmget");
                exit(1);
        }
	//sem get 		
	key_t key;
	int semid;
	int semflg = IPC_CREAT | 0666 ;
	int nsems = 2;
	int nsops = 1;
	struct sembuf *sops = (struct sembuf*)malloc(2*sizeof(struct sembuf));
	//get the semid
	if((semid = semget(key,nsems,semflg)) ==-1){
		perror("semget:semget failed");
		exit(1);
	}
	//sem init:(semid , semIndex, cmd, arg)
	arg.val = 1;
	semctl(semid, 1, SETVAL, arg);
	arg.val = 0;
	semctl(semid, 0, SETVAL, arg);
	//board init
	char *str;
	//char buf[5]="";
	char filename[BUFFER_SIZE]="";
	bool board[2500]={false};
	srand(time(NULL));
	int x,y;
	bool isOver=false;
	//fork
	pid_t cpid;
	cpid = fork();
	
	if ( cpid < 0 ) {
		perror("Fork failed.");
		return -1;
	} else if ( cpid == 0 ) {
		//shm_id = shmget(shm_key,32,0666);
		sprintf(filename,"%d_SHM.txt",getpid());
		FILE *hist=fopen(filename,"w");
		//semop(int semid, struct sembuf *sops, size_t nsops)
                /*sops[0].sem_num = 0;
              	sops[0].sem_op = 0;
                sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;        
		sops[1].sem_num = 0;
                sops[1].sem_op = 1;
                sops[1].sem_flg = SEM_UNDO | IPC_NOWAIT;
                if(semop(semid, sops, 2)!=-1){
			fprintf(hist,"1\n");
			x = rand()%50;
			y = rand()%50;
			fprintf(hist,"%d %d\n",x,y);
	printf("child first push%d %d\n",x,y);
			board[x*50+y] = true;
			
			str=shmat(shm_id,NULL,0);
			sprintf(str,"%d %d",x,y);
			shmdt(str);
	perror("child");
			sops[0].sem_num = 0;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
			semop(semid, sops, 1);	
		}
		else */fprintf(hist,"0\n");
		while(!isOver){			
			sops[0].sem_num = 1;
                        sops[0].sem_op = 0;
                        sops[0].sem_flg = SEM_UNDO ;
                        sops[1].sem_num = 1;
                        sops[1].sem_op = 1;
                        sops[1].sem_flg = SEM_UNDO ;       
			semop(semid, sops, 2);
	//printf("child in critical\n");
			str=shmat(shm_id,0,SHM_RDONLY);
			//strncpy(buf,str,5);
			x = atoi(str);
			y = atoi(str+2);
			shmdt(str);
			if(x==50){
				fprintf(hist,"1");//win
				fclose(hist);
				break;
			}
			printf("childget: %d %d\n",x,y);
			board[x*50+y] = true;

			switch(chess(board,x,y)){
			case UP:x--;/*sprintf(str,"%d %d",x,y);*/break;
			case DOWN:x++;/*sprintf(str,"%d %d",x,y);*/break;
			case LEFT:y--;/*sprintf(str,"%d %d",x,y);*/break;
			case RIGHT:y++;/*sprintf(str,"%d %d",x,y);*/break;
			case NOWAY:
				x=50;y=50;
				fprintf(hist,"0");//loss
				fclose(hist);
				isOver=true;
				break;
			}
	//printf("child push %d %d\n",x,y);
			fprintf(hist,"%d %d\n",x,y);
			str=shmat(shm_id,0,0);
			sprintf(str,"%d %d",x,y);
			shmdt(str);

			sops[0].sem_num = 0;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO;
			semop(semid, sops, 1);
	//printf("child out critical\n");
		}
	} else {
		//shm_id = shmget(shm_key,32,0666);
		sprintf(filename,"%d_SHM.txt",getpid());
		FILE *hist=fopen(filename,"w");
		//semop(int semid, struct sembuf *sops, size_t nsops)
                sops[0].sem_num = 0;
                sops[0].sem_op = 0;
                sops[0].sem_flg = SEM_UNDO;
                sops[1].sem_num = 0;
                sops[1].sem_op = 1;
                sops[1].sem_flg = SEM_UNDO | IPC_NOWAIT;
                if(semop(semid, sops, 2)!=-1){
			fprintf(hist,"1\n");
			x = rand()%50;
			y = rand()%50;
			fprintf(hist,"%d %d\n",x,y);

			board[x*50+y] = true;
	//printf("parent first\n");
		        str=shmat(shm_id,0,0);
			sprintf(str,"%d %d",x,y);  
			shmdt(str);
	//printf("parent push %d %d\n",x,y);
			sops[0].sem_num = 1;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
			semop(semid, sops, 1);
                }
		else fprintf(hist,"0\n");
		while(!isOver){
			sops[0].sem_num = 0;
                        sops[0].sem_op = 0;
                        sops[0].sem_flg = SEM_UNDO ;
                        sops[1].sem_num = 0;
                        sops[1].sem_op = 1;
                        sops[1].sem_flg = SEM_UNDO | IPC_NOWAIT;
                        semop(semid, sops, 2);
	//printf("parent IN critical\n");
			str=shmat(shm_id,0,SHM_RDONLY);
			//strncpy(buf,str,5);
                        x = atoi(str);
                        y = atoi(str+2);
			shmdt(str);
                        if(x==50){
				fprintf(hist,"1");//win
				fclose(hist);
				break;
			}
                       	printf("parentget %d %d\n",x,y);
			board[x*50+y]=true;
			
			switch(chess(board,x,y)){
                        case UP:--x;/*sprintf(str,"%d %d",x,y);*/break;
                        case DOWN:++x;/*sprintf(str,"%d %d",x,y);*/break;
                        case LEFT:--y;/*sprintf(str,"%d %d",x,y);*/break;
                        case RIGHT:++y;/*sprintf(str,"%d %d",x,y);*/break;
                        case NOWAY:
                                x=50;y=50;
                                fprintf(hist,"0");//loss
				fclose(hist);
				isOver=true;
                                break;
                        }
	//printf("parent push %d %d\n",x,y);
			fprintf(hist,"%d %d\n",x,y);
			str=shmat(shm_id,0,0);
			sprintf(str,"%d %d",x,y);
			shmdt(str);

			sops[0].sem_num = 1;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO ;
			semop(semid, sops, 1);
	//printf("parent out critical\n");
		}
		wait(0);
		//remove the SHM
		shmctl(shm_id,IPC_RMID,NULL);
		//remove the sem
		if((semctl(semid, 0, IPC_RMID))==-1){
                	perror("semctl: semctl failed\n");
               		exit(1);
		}
        }

	return 0;
}
