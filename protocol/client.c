#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9031
#define BUF_SIZE 1024

int
main(void)
{
	int 		status   , client_fd;
	struct sockaddr_in serv_addr;
	char 		buffer   [BUF_SIZE];
	fd_set 		read_fds;
	int 		maxfd;
	char 		username [32];
	char 		ip[64];
	
	printf("Enter a Server [IP/Domain] ->");
	scanf("%s",ip);

	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		fprintf(stderr, "Invalid address\n");
		return -1;
	}
	if ((status = connect(client_fd, (struct sockaddr *) & serv_addr, sizeof(serv_addr))) < 0) {
		perror("connect");
		return -1;
	}
	printf("Connected. Enter a username and type messages, press Enter. Ctrl-C to quit.\n");
	printf("Enter a username ->");
	scanf("%s", username);

	maxfd = (client_fd > STDIN_FILENO) ? client_fd : STDIN_FILENO;

	while (1) {
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		FD_SET(client_fd, &read_fds);

		if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) < 0) {
			perror("select");
			break;
		}
		if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			ssize_t 	n = read(STDIN_FILENO, buffer, BUF_SIZE - 1);
			if (n <= 0)
				break;

			buffer[n] = '\0';
			char 		newbuf   [1024];
			snprintf(newbuf, sizeof(newbuf), "%s, %s\n", username, buffer);

			if (send(client_fd, newbuf, strlen(newbuf), 0) <= 0) {
				perror("send");
				break;
			}
		}
		if (FD_ISSET(client_fd, &read_fds)) {
			ssize_t 	n = read(client_fd, buffer, BUF_SIZE - 1);
			if (n <= 0) {
				printf("Server closed connection\n");
				break;
			}
			buffer[n] = '\0';
			char           *line = strtok(buffer, "\n");
			while (line) {
				char 		name     [32];
				char 		message  [900];

				if (sscanf(line, "%31[^,], %899[^\n]", name, message) == 2) {
					printf("[%s]: %s\n", name, message);
				} else {
					printf("%s\n", line);
				}

				line = strtok(NULL, "\n");
			}
		}
	}

	close(client_fd);
	return 0;
}
