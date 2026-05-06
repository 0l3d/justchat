#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#define PORT 9031

int
main(int argc, char const * argv[])
{
	int 		server_fd, new_socket, client_socket[100], max_clients = 100, valread,
			i            , sd, max_sd, opt = 1;
	struct sockaddr_in address;
	socklen_t 	addrlen = sizeof(address);
	char 		buffer   [900] = {0};

	fd_set 		readfd;

	for (i = 0; i < max_clients; i++) {
		client_socket[i] = 0;
	}


	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt SO_REUSEADDR");
		exit(EXIT_FAILURE);
	}
#ifdef SO_REUSEPORT
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		perror("setsockopt SO_REUSEPORT");
	}
#endif

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *) & address,
		 sizeof(address))
	    < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("SERVERLOG: Server initialized: PORT %d\n", PORT);

	int 		act = 0;

	while (1) {
		FD_ZERO(&readfd);
		FD_SET(server_fd, &readfd);
		max_sd = server_fd;
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];
			if (sd > 0) {
				FD_SET(sd, &readfd);
			}
			if (sd > max_sd) {
				max_sd = sd;
			}
		}
		act = select(max_sd + 1, &readfd, NULL, NULL, NULL);
		if ((act < 0) && (errno != EINTR)) {
			perror("select error");
			exit(EXIT_FAILURE);
		}
		if (FD_ISSET(server_fd, &readfd)) {
			if ((new_socket = accept(server_fd, (struct sockaddr *) & address, (socklen_t *) & addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}
			printf("SERVERLOG: New connection! Socket fs is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			char           *welcome_message = "Connection with server is successfully connected. JUSTCHAT NOW!";
			if (send(new_socket, welcome_message, strlen(welcome_message), 0) != strlen(welcome_message)) {
				perror("send");
			}
			for (i = 0; i < max_clients; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					printf("SERVERLOG: Client connected! ID: %d\n", i);
					printf("SERVERLOG: Server welcome message sent by server.\n");
					break;
				}
			}
		}
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];
			if (sd <= 0)
				continue;
			if (FD_ISSET(sd, &readfd)) {
				if ((valread = read(sd, buffer, sizeof(buffer) - 1)) == 0) {
					printf("CLIENTLOG: Client disconnected! ID: %d\n", i);
					close(sd);
					client_socket[i] = 0;
				} else {
					char           *buffer_for_server = strdup(buffer);
					buffer_for_server[valread] = '\0';
					char           *line = strtok(buffer_for_server, "\n");
					while (line) {
						char 		name     [32];
						char 		message  [900];

						if (sscanf(line, "%31[^,], %899[^\n]", name, message) == 2) {
							printf("MESSAGELOG: Message sent! [%s]: %s\n", name, message);
						} else {
							printf("MESSAGELOG: Raw message sent! %s\n", line);
						}

						line = strtok(NULL, "\n");
					}
					buffer[valread] = '\0';
					for (int j = 0; j < max_clients; j++) {
						int 		out_sd = client_socket[j];
						if (out_sd > 0 && out_sd != sd) {
							send(out_sd, buffer, strlen(buffer), 0);
						}
					}
				}
			}
		}

	}
	close(new_socket);

	close(server_fd);
	return 0;
}
