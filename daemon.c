#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

sem_t semaphore;

bool flag = false;
bool flag2 = false;
bool flag3 = false;

void signal_hundler(int signum)
{
	flag = true;
}
void signal_hundler2(int signum)
{
	flag2 = true;
}
void SigChildHandler(int signum)
{
	flag3 = true;
}


int Command(char **argv)
{
	int output = open("output.txt", O_CREAT | O_RDWR | O_APPEND, S_IRWXU);

	close(STDOUT_FILENO);
	dup2(output, STDOUT_FILENO);

	execv(argv[0], argv);
	close(output);
}

int Demon(char **argv)
{
	sem_init(&semaphore, 0, 1);
	pid_t pid;	
	char buf[] = "Signal received\n";
	char buf1[] = "Daemon finished\n";
	char buf2[] = "semaphore wait\n";

	signal(SIGUSR1, signal_hundler);
	signal(SIGCHLD, SigChildHandler);
	signal(SIGTERM, signal_hundler2);

	//----------------------------------------------------
	char string[64256];
	int file = open(argv[1], O_CREAT | O_RDWR, 00755);
	read(file,string,sizeof(string));

	char *line;
	int count=0;
	char *mass_line[20];

	line = strtok(string,"\n");
	while(line!=NULL)
	{
		mass_line[count++] = line;
		line = strtok(NULL,"\n");
	}
	//----------------------------------------------------

	while (!flag2)
	{
		if (flag)
		{
			for (int n=0; n<count;n++){
				pid = fork();

				if (pid == -1)
				{
					printf("Problem with fork!\n");
				}
				else if (pid == 0)
				{
					//-------------------------------------
					char *argv1[20];
					char *arg;	
					int i = 0;

					arg = strtok(mass_line[n], " ");
					argv1[i++] = arg;

					while(arg!=NULL)
					{	
						arg = strtok(NULL, " ");
						argv1[i++]=arg;			
					}

					argv1[i] = NULL;

					int wait = sem_wait(&semaphore);

					if (wait < 0){
						printf("%s",buf2);
					}
					else{
						printf("command complited: %s", argv1[0]);
						Command(argv1);
					}
					//----------------------------------------------
				}
				else if (pid > 0)
					{
						while (1)
						{
							if (flag3)
							{
								waitpid(-1, NULL, 0);
								sem_post(&semaphore);
								flag3 = false;
								break;
							}
							pause();
						}
					}
				flag = false;
			}
		}
		pause();
		//sleep(50);
	}
	sem_destroy(&semaphore);
	printf("%s", buf1);
	return 0;
}

int main(int argc, char *argv[])
{
	int status;
	pid_t pid, newpid;

	pid = fork();

	if (pid == -1)
	{
		printf("Problem!\n");
		exit(1);
	}
	else if (!pid)
	{
		umask(0);
		newpid = getpid();
		printf("pid = %i \n", newpid);
		printf("send signal\n"); //kill -SIGUSR1 pid
		setsid();

		status = Demon(argv);

		return status;
	}
	else
	{
		return 0;
	}
}
