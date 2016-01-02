// File: prg8_6.c
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
	int read_res;
	char client_fifo[256];
	char *tmp_char_ptr;

	mkfifo(SERVER_FIFO_NAME, 0777);
	server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
	if (server_fifo_fd == -1)
	{
		fprintf(stderr, "Server fifo failure\n");
		exit(EXIT_FAILURE);
	}

	//sleep(10); /* lets clients queue for demo purposes */

	do
	{
		read_res = read(server_fifo_fd, &my_data, sizeof(my_data));
		if (read_res > 0)
		{
			tmp_char_ptr = my_data.some_data;
			while (*tmp_char_ptr)
			{
				*tmp_char_ptr = toupper(*tmp_char_ptr);
				tmp_char_ptr++;
			}
			sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
			client_fifo_fd = open(client_fifo, O_WRONLY);
			if (client_fifo_fd != -1)
			{
				write(client_fifo_fd, &my_data, sizeof(my_data));
				close(client_fifo_fd);
			}
		}
	}
	while (read_res > 0);
	close(server_fifo_fd);
	unlink(SERVER_FIFO_NAME);
	exit(EXIT_SUCCESS);
}
