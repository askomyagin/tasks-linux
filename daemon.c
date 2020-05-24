#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

bool flag = false;
bool flag2 = false;

void signal_hundler(int signum)
{
	flag = true;
}
void signal_hundler2(int signum)
{
	flag2 = true;
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
	pid_t pid;
	char *argv1[20];
	int file;
	char *arg;	
	char buf[] = "Signal received\n";
	char buf1[] = "Daemon finished\n";

	signal(SIGUSR1, signal_hundler);
	signal(SIGTERM, signal_hundler2);

	char string[256];
	int i = 0;
	file = open(argv[1], O_CREAT | O_RDWR, 00755);
	read(file,string,sizeof(string));
	
	arg = strtok(string, " ");
	argv1[i++] = arg;
	
	while(arg!=NULL)
	{
		arg = strtok(NULL, " ");
		argv1[i++]=arg;			
	}

	argv1[i++] = NULL;

	while (!flag2)
	{
		if (flag)
		{
			pid = fork();

			if (pid == -1)
			{
				printf("Problem!\n");
				exit(1);
			}
			else if (pid == 0)
			{
				Command(argv1);
				printf("%s", buf);
			}
			flag = false;
		}
	
		pause();
		//sleep(50);
	}
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
