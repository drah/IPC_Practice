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
	//sem get 		
	key_t key = 123;
	int semid;
	int semflg = IPC_CREAT | 0666 ;
	int nsems = 1;
	int nsops = 1;
	struct sembuf *sops = (struct sembuf*)malloc(2*sizeof(struct sembuf));
	//get the semid
	if((semid = semget(key,nsems,semflg)) ==-1){
		perror("semget:semget failed");
		exit(1);
	}
	//sem init:(semid , semIndex, cmd, arg)
	arg.val = 0;
	semctl(semid, 0, SETVAL, arg);
	//fifo set up
	const char path[20] = "named_pipe";
	char str[BUFFER_SIZE] = "";
	int fd;
	pid_t cpid;
	if ( mkfifo(path, 0666) < 0 ) { /* create a named pipe file in system */
		perror("mkfifo failed.");
		return -1;
	}
	//the board
	bool board[2500]={false};
	srand(time(NULL));
	int x,y;
	bool isOver=false;
	//fork
	cpid = fork();
	if ( cpid < 0 ) {
		perror("Fork failed.");
		return -1;
	} else if ( cpid == 0 ) {
		sprintf(str,"%d_FIFO.txt",getpid());
		FILE *hist=fopen(str,"w");
		//semop(int semid, struct sembuf *sops, size_t nsops)
                sops[0].sem_num = 0;
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
			board[x*50+y] = true;
			fd = open(path, O_WRONLY);
			sprintf(str,"%d %d",x,y);
			write(fd,str,sizeof(char)*(strlen(str)+1));
			close(fd);
			sops[0].sem_num = 0;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
			semop(semid, sops, 1);	
			sleep(1);
		}
		else fprintf(hist,"0\n");
		while(!isOver){
			fd = open(path,O_RDONLY);
			read(fd,str,sizeof(char)*BUFFER_SIZE);
			x = atoi(str);
			y = atoi(str+2);
			if(x==50){
				fprintf(hist,"1");//win
				fclose(hist);
				break;
			}
			printf("childget: %d,%d\n",x,y);
			close(fd);
			board[x*50+y] = true;
                        //semop(int semid, struct sembuf *sops, size_t nsops)
                        sops[0].sem_num = 0;
                        sops[0].sem_op = 0;
                        sops[0].sem_flg = SEM_UNDO ;
                        sops[1].sem_num = 0;
                        sops[1].sem_op = 1;
                        sops[1].sem_flg = SEM_UNDO | IPC_NOWAIT ;
                        semop(semid, sops, 2);

			switch(chess(board,x,y)){
			case UP:x--;sprintf(str,"%d %d",x,y);break;
			case DOWN:x++;sprintf(str,"%d %d",x,y);break;
			case LEFT:y--;sprintf(str,"%d %d",x,y);break;
			case RIGHT:y++;sprintf(str,"%d %d",x,y);break;
			case NOWAY:
				sprintf(str,"50 50");
				fprintf(hist,"0");//loss
				fclose(hist);
				isOver=true;
				break;
			}
			fprintf(hist,"%d %d\n",x,y);
			fd = open(path,O_WRONLY);
			write(fd,str,sizeof(char)*(strlen(str)+1));
			close(fd);
			
			sops[0].sem_num = 0;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
			semop(semid, sops, 1);
		}
	} else {
		sprintf(str,"%d_FIFO.txt",getpid());
		FILE *hist=fopen(str,"w");
		//semop(int semid, struct sembuf *sops, size_t nsops)
                sops[0].sem_num = 0;
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
			board[x*50+y] = true;
                        fd = open(path, O_WRONLY);
		        sprintf(str,"%d %d",x,y);  
                        write(fd,str,sizeof(char)*(strlen(str)+1));
                        close(fd);
			sops[0].sem_num = 0;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
			semop(semid, sops, 1);
                        sleep(1);
                }
		else fprintf(hist,"0\n");
		while(!isOver){
			fd = open(path,O_RDONLY);
                        read(fd,str,sizeof(char)*BUFFER_SIZE);
                        x = atoi(str);
                        y = atoi(str+2);
                        if(x==50){
				fprintf(hist,"1");//win
				fclose(hist);
				break;
			}
			printf("parentget: %d %d\n",x,y);
                        close(fd);
                        board[x*50+y] = true;
                        //semop(int semid, struct sembuf *sops, size_t nsops)
                        sops[0].sem_num = 0;
                        sops[0].sem_op = 0;
                        sops[0].sem_flg = SEM_UNDO ;
                        sops[1].sem_num = 0;
                        sops[1].sem_op = 1;
                        sops[1].sem_flg = SEM_UNDO | IPC_NOWAIT;
                        semop(semid, sops, 2);

			switch(chess(board,x,y)){
                        case UP:--x;sprintf(str,"%d %d",x,y);break;
                        case DOWN:++x;sprintf(str,"%d %d",x,y);break;
                        case LEFT:--y;sprintf(str,"%d %d",x,y);break;
                        case RIGHT:++y;sprintf(str,"%d %d",x,y);break;
                        case NOWAY:
                                sprintf(str,"50 50");
                                fprintf(hist,"0");//loss
				fclose(hist);
				isOver=true;
                                break;
                        }
			fprintf(hist,"%d %d\n",x,y);
                        fd = open(path,O_WRONLY);
                        write(fd,str,sizeof(char)*(strlen(str)+1));
                        close(fd);

			sops[0].sem_num = 0;
			sops[0].sem_op = -1;
			sops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;
			semop(semid, sops, 1);
		}
		remove(path);
		//remove the sem
		if((semctl(semid, 0, IPC_RMID))==-1){
                	perror("semctl: semctl failed\n");
               		exit(1);
		}
        }

	return 0;
}
