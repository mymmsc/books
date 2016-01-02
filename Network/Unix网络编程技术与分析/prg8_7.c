// File: prg8_7.c
#include <ctype.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SERVER_FIFO_NAME "./serv_fifo"
#define CLIENT_FIFO_NAME "./cli_%d_fifo"

#define BUFFER_SIZE 20

struct data_to_pass_st
{
	pid_t  client_pid;
	char   some_data[BUFFER_SIZE - 1];
};

int main()
{
	int server_fifo_fd, client_fifo_fd;
	struct data_to_pass_st my_data;
	int times_to_send;
	char client_fifo[256];

	server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
	if (server_fifo_fd == -1)
	{
		fprintf(stderr, "Sorry, no server\n");
		exit(EXIT_FAILURE);
	}

	my_data.client_pid = getpid();
	sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
	if (mkfifo(client_fifo, 0777) == -1)
	{
		fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
		exit(EXIT_FAILURE);
	}

	for (times_to_send = 0; times_to_send < 5; times_to_send++)
	{
		sprintf(my_data.some_data, "Hello from %d", my_data.client_pid); 
		printf("%d sent %s, \n", my_data.client_pid, my_data.some_data);
		write(server_fifo_fd, &my_data, sizeof(my_data));
		client_fifo_fd = open(client_fifo, O_RDONLY);
		if (client_fifo_fd != -1)
		{
			if (read(client_fifo_fd, &my_data, sizeof(my_data)) > 0)
			{
				printf("received: %s\n", my_data.some_data);
			}
			else
			{
				printf("read %s failed!\n", client_fifo);
			}
			close(client_fifo_fd);
		}
		else
		{
			printf("open %s failed!\n", client_fifo);
		}
	}
	close(server_fifo_fd);
	unlink(client_fifo);
	exit(EXIT_SUCCESS);
}
